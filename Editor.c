#include "Editor.h"

#include "Render.h"

typedef struct TerrainPiece_t {
	Vec2 Position;
	bool32 Disabled;
} TerrainPiece;

struct Editor_State_t {
	Array *Terrain;
	Vec2 CursorPosition;

	Camera Camera;
	r32 CameraPitch;
	r32 CameraYaw;
	r32 CameraDistance;

	Shader *WireShader, *TerrainShader;
	GLuint CursorVAO, CursorInds;
	GLuint TerrainVAO;
} Editor_State;

const i32 CameraTimeMs = 500;
u32 LastMove = 0;
Vec2 TargetAtMoveStart = V2(0, 0);

bool32 InsideCameraDrag = 0;
Vec2 InitialDragMousePos = V2(0, 0);
Vec2 InitialYawPitch = V2(0, 0);
SDL_Cursor *Cursor_Normal, *Cursor_Rotate;

static const char *Editor_WireSrc[2] = {
    // Vertex shader
    "#version 330\n"
    ""
    "in vec3 pos;\n"
    ""
    "uniform mat4 MVP;\n"
    ""
    "void main() { gl_Position = MVP * vec4(pos, 1); }",

    // Fragment shader
    "#version 330\n"
    ""
    "out vec4 fColor;\n"
    ""
    "uniform vec3 color;\n"
    ""
    "void main() { fColor = vec4(0, 1, 0, 1); }"};

static const char *Editor_TerrainSrc[2] = {
    // Vertex shader
    "#version 330\n"
    ""
    "in vec3 pos;\n"
    "in vec2 uv;\n"
    ""
    "out vec2 fUV;\n"
    ""
    "uniform mat4 MVP;\n"
    ""
    "void main() {"
    "  gl_Position = MVP * vec4(pos, 1);"
    "  fUV = uv;"
    "}",

    // Fragment shader
    "#version 330\n"
    "in vec2 fUV;\n"
    "out vec4 fColor;\n"
    "void main() {\n"
    "    float thickness = 0.05;\n"
    "    \n"
    "   if((fUV.x > -1 + thickness && fUV.x < 1 - thickness) && \n"
    "      (fUV.y > -1 + thickness && fUV.y < 1 - thickness)) {\n"
    "       fColor = vec4(0.3, 0.3, 0.3, 1);\n"
    "   } else {\n"
    "       fColor = vec4(0.5, 0.5, 0.5, 1);\n"
    "   }\n"
    "}"};

static TerrainPiece *Editor_TerrainUnderCursor() {
	for(u32 i = 0; i < Editor_State.Terrain->ArraySize; i++) {
		TerrainPiece *piece = Array_Get(Editor_State.Terrain, i);

		if(piece->Position.x == Editor_State.CursorPosition.x &&
		   piece->Position.y == Editor_State.CursorPosition.y) {
			return piece;
		}
	}
	return NULL;
}

void Editor_Init() {
	// Generate shaders
	Editor_State.WireShader =
	    Shader_FromSrc(Editor_WireSrc[0], Editor_WireSrc[1]);

	Editor_State.TerrainShader =
	    Shader_FromSrc(Editor_TerrainSrc[0], Editor_TerrainSrc[1]);

	// Reset cursor position
	Editor_State.CursorPosition = V2(0, 0);

	// Setup camera
	Editor_State.Camera = (Camera){.Mode = CameraMode_Perspective,
	                               .ZNear = 0.01,
	                               .ZFar = 1e10,
	                               .VerticalFoV = Pi_Half + Pi_Quarter,
	                               .Up = V3(0, 1, 0)};
	Editor_State.CameraYaw = DegToRad(290);
	Editor_State.CameraPitch = DegToRad(45);
	Editor_State.CameraDistance = 2;

	// Generate cursor.
	{
		glGenVertexArrays(1, &Editor_State.CursorVAO);
		glBindVertexArray(Editor_State.CursorVAO);

		Vec3 CursorPos[8] = {
		    {0.54, 0.2, 0.54},    {-0.54, 0.2, 0.54},  {-0.54, -0.2, 0.54},
		    {0.54, -0.2, 0.54},   {0.54, 0.2, -0.54},  {-0.54, 0.2, -0.54},
		    {-0.54, -0.2, -0.54}, {0.54, -0.2, -0.54},
		};

		u8 CursorInds[18] = {0, 1, 2, 3, 0, 4, 5, 6, 7,
		                     4, 5, 1, 2, 6, 7, 3, 0, 4};

		GLuint CursorVBOs[2];
		glGenBuffers(2, CursorVBOs);
		glBindBuffer(GL_ARRAY_BUFFER, CursorVBOs[0]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, CursorVBOs[1]);

		glBufferData(GL_ARRAY_BUFFER, sizeof(CursorPos), CursorPos,
		             GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(CursorInds), CursorInds,
		             GL_STATIC_DRAW);

		Editor_State.CursorInds = CursorVBOs[1];
	}

	{
		glGenVertexArrays(1, &Editor_State.TerrainVAO);
		glBindVertexArray(Editor_State.TerrainVAO);

		Vec3 TerrainPos[4] = {
		    {-0.5, 0, 0.5},
		    {-0.5, 0, -0.5},
		    {0.5, 0, -0.5},
		    {0.5, 0, 0.5},
		};

		Vec2 TerrainUV[4] = {
		    {-1, +1},
		    {-1, -1},
		    {+1, -1},
		    {+1, +1},
		};

		GLuint TerrainVBOs[2];
		glGenBuffers(2, TerrainVBOs);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, TerrainVBOs[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(TerrainPos), TerrainPos,
		             GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

		glBindBuffer(GL_ARRAY_BUFFER, TerrainVBOs[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(TerrainUV), TerrainUV,
		             GL_STATIC_DRAW);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	}

	Editor_State.Terrain = Array_Init(sizeof(TerrainPiece));
	{
		TerrainPiece Default = {.Position = V2(0, 0)};
		Array_Push(Editor_State.Terrain, &Default);
	}

	Cursor_Normal = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
	Cursor_Rotate = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
}


const r32 speed = 1;

void Editor_HandleInput(SDL_Event *e) {
	Vec3 CameraDirection = Vec3_Norm(
	    Vec3_Sub(Editor_State.Camera.Target, Editor_State.Camera.Position));
	Vec3 CameraRight =
	    Vec3_Norm(Vec3_Cross(Editor_State.Camera.Up, CameraDirection));

	switch(e->type) {
		case SDL_KEYDOWN:
			switch(e->key.keysym.sym) {
					// bool32 shift = e->key.keysym.mod & KMOD_SHIFT;

				case SDLK_LEFT:
					Editor_State.CursorPosition.x -= speed;
					goto SetLastMove;
				case SDLK_RIGHT:
					Editor_State.CursorPosition.x += speed;
					goto SetLastMove;
				case SDLK_UP:
					Editor_State.CursorPosition.y += speed;
					goto SetLastMove;
				case SDLK_DOWN:
					Editor_State.CursorPosition.y -= speed;
					goto SetLastMove;

				SetLastMove:
					TargetAtMoveStart = V2(Editor_State.Camera.Target.x,
					                       Editor_State.Camera.Target.z);
					LastMove = SDL_GetTicks();
					break;

				case SDLK_SPACE: {
					TerrainPiece *underCursor = Editor_TerrainUnderCursor();
					if(!underCursor) {
						TerrainPiece newPiece;
						newPiece.Position = Editor_State.CursorPosition;
						newPiece.Disabled = 0;
						Array_Push(Editor_State.Terrain, &newPiece);
						printf("Placed terrain at x: %.2f, y: %.2f\n",
						       newPiece.Position.x, newPiece.Position.y);
					} else {
						underCursor->Disabled = !underCursor->Disabled;
						printf("Toggled terrain at x: %.2f, y: %.2f\n",
						       Editor_State.CursorPosition.x,
						       Editor_State.CursorPosition.y);
					}
				} break;
			}
			break;
		case SDL_MOUSEWHEEL:
			Editor_State.CameraDistance = Clamp_R32(
			    Editor_State.CameraDistance - e->wheel.y * 0.25f, 1, 10);
			break;
		case SDL_MOUSEBUTTONDOWN:
			if(e->button.button == SDL_BUTTON_MIDDLE) {
				// Begin camera drag if it hasn't been started yet.
				if(!InsideCameraDrag) {
					SDL_SetCursor(Cursor_Rotate);

					InsideCameraDrag = 1;
					InitialDragMousePos = V2(e->button.x, e->button.y);
					InitialYawPitch =
					    V2(Editor_State.CameraYaw, Editor_State.CameraPitch);
				}
			}
			break;
		case SDL_MOUSEMOTION: {
			bool32 shift = e->key.keysym.mod & KMOD_SHIFT;

			if(InsideCameraDrag) {
				r32 dx = (e->motion.x - InitialDragMousePos.x) / 512.0;
				r32 dy = (e->motion.y - InitialDragMousePos.y) / 256.0;

				Editor_State.CameraYaw = (-dx) * Pi_Half + InitialYawPitch.x;
				Editor_State.CameraPitch =
				    Clamp_R32((-dy) * Pi_Half + InitialYawPitch.y, DegToRad(1),
				              DegToRad(89));
			}
		} break;
		case SDL_MOUSEBUTTONUP:
			if(e->button.button == SDL_BUTTON_MIDDLE) {
				SDL_SetCursor(Cursor_Normal);
				InsideCameraDrag = 0;
			}
			break;
		default: {
			// printf("Unknown event type %u\n", e->type);
			break;
		}
	}
}

void Editor_Render() {
	// Position camera
	Editor_State.Camera.Target = (Vec3){
	    .x = Lerp_EaseInOut(TargetAtMoveStart.x, Editor_State.CursorPosition.x,
	                        (r32)(SDL_GetTicks() - LastMove) / CameraTimeMs),

	    .y = 0,

	    .z = Lerp_EaseInOut(TargetAtMoveStart.y, Editor_State.CursorPosition.y,
	                        (r32)(SDL_GetTicks() - LastMove) / CameraTimeMs),
	};


	{
		Vec3 CameraOffset = V3(0, 0, 0);
		CameraOffset.x =
		    sinf(Editor_State.CameraPitch) * cosf(Editor_State.CameraYaw);
		CameraOffset.z =
		    sinf(Editor_State.CameraPitch) * sinf(Editor_State.CameraYaw);
		CameraOffset.y = cosf(Editor_State.CameraPitch);
		CameraOffset = Vec3_MultScal(CameraOffset, Editor_State.CameraDistance);

		Editor_State.Camera.Position =
		    Vec3_Add(Editor_State.Camera.Target, CameraOffset);
	}

	Editor_State.Camera.AspectRatio = RSys_GetSize().AspectRatio;

	// Set up Model-View-Projection matrix
	Mat4 VP;
	{
		Mat4 View;
		Camera_Mat4(Editor_State.Camera, View, VP);
		Mat4_MultMat(VP, View);
	}

	// Render the wireframe cursor
	{
		Transform3D CursorTransform = {
		    .Position = {.x = Editor_State.CursorPosition.x,
		                 .y = 0,
		                 .z = Editor_State.CursorPosition.y},
		    .Rotation = Quat_Identity,
		    .Scale = V3(1, 1, 1)};

		Mat4 Model;
		Transform3D_Mat4(CursorTransform, Model);

		Mat4 MVP;
		Mat4_Copy(MVP, VP);
		Mat4_MultMat(MVP, Model);

		Shader_Use(Editor_State.WireShader);
		Shader_UniformMat4(Editor_State.WireShader, "MVP", MVP);
		glBindVertexArray(Editor_State.CursorVAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Editor_State.CursorInds);
		glDrawElements(GL_LINE_LOOP, 18, GL_UNSIGNED_BYTE, NULL);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	// Render all visible terrain pieces
	{
		glBindVertexArray(Editor_State.TerrainVAO);
		Transform3D Transform = {.Position = V3(0, 0, 0),
		                         .Rotation = Quat_Identity,
		                         .Scale = V3(1, 1, 1)};

		Shader_Use(Editor_State.TerrainShader);

		for(u32 i = 0; i < Editor_State.Terrain->ArraySize; i++) {
			TerrainPiece *piece = Array_Get(Editor_State.Terrain, i);
			if(piece->Disabled) continue;

			Transform.Position =
			    (Vec3){.x = piece->Position.x, .y = 0, .z = piece->Position.y};

			Mat4 Model;
			Transform3D_Mat4(Transform, Model);

			Mat4 MVP;
			Mat4_Copy(MVP, VP);
			Mat4_MultMat(MVP, Model);

			Shader_UniformMat4(Editor_State.WireShader, "MVP", MVP);

			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		}
	}
}
