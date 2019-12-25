#include "Editor.h"

#include "Render.h"

typedef struct TerrainPiece_t {
	Vec2 Position;
} TerrainPiece;

struct Editor_State_t {
	Array *Terrain;
	Vec2 CursorPosition;

	Camera Camera;
	Shader *WireShader, *TerrainShader;
	GLuint CursorVAO, CursorInds;
	GLuint TerrainVAO;
} Editor_State;

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
    "    if((fUV.x > 1 + thickness/2 && fUV.y < 1 - thickness/2) ||\n"
    "       (fUV.x < 1 - thickness/2 && fUV.y > 1 + thickness/2)) {\n"
    "        fColor = vec4(0.3, 0.3, 0.3, 1);\n"
    "    } else if((fUV.x > 1 + thickness/2 && fUV.y > 1 + thickness/2) ||\n"
    "             (fUV.x < 1 - thickness/2 && fUV.y < 1 - thickness/2)) {\n"
    "        fColor = vec4(0.5, 0.5, 0.5, 1);\n"
    "    } else {\n"
    "        fColor = vec4(0.4, 0.4, 0.4, 1);\n"
    "    }\n"
    "}"};

void Editor_Init() {
	Editor_State.WireShader =
	    Shader_FromSrc(Editor_WireSrc[0], Editor_WireSrc[1]);

	Editor_State.TerrainShader =
	    Shader_FromSrc(Editor_TerrainSrc[0], Editor_TerrainSrc[1]);

	Editor_State.CursorPosition = V2(0, 0);

	Editor_State.Camera = (Camera){.Mode = CameraMode_Perspective,
	                               .ZNear = 0.01,
	                               .ZFar = 1000,
	                               .VerticalFoV = Pi_Half + Pi_Quarter,
	                               .Up = V3(0, 1, 0)};

	// Generate cursor.
	{
		glGenVertexArrays(1, &Editor_State.CursorVAO);
		glBindVertexArray(Editor_State.CursorVAO);

		Vec3 CursorPos[8] = {
		    {0.5, 0.2, 0.5},    {-0.5, 0.2, 0.5},  {-0.5, -0.2, 0.5},
		    {0.5, -0.2, 0.5},   {0.5, 0.2, -0.5},  {-0.5, 0.2, -0.5},
		    {-0.5, -0.2, -0.5}, {0.5, -0.2, -0.5},
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
		    {0, 2},
		    {0, 0},
		    {2, 0},
		    {2, 2},
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
}

void Editor_HandleInput(SDL_Event *e) {
	switch(e->type) {
		case SDL_KEYDOWN:
			switch(e->key.keysym.sym) {
				case SDLK_LEFT: Editor_State.CursorPosition.x--; break;
				case SDLK_RIGHT: Editor_State.CursorPosition.x++; break;
				case SDLK_UP: Editor_State.CursorPosition.y++; break;
				case SDLK_DOWN: Editor_State.CursorPosition.y--; break;
			}
			break;
		case SDL_KEYUP:
			switch(e->key.keysym.sym) {
				case SDLK_SPACE: {
					bool32 spaceOccupied = 0;

					for(u32 i = 0; i < Editor_State.Terrain->ArraySize; i++) {
						TerrainPiece *piece =
						    Array_Get(Editor_State.Terrain, i);

						if(piece->Position.x == Editor_State.CursorPosition.x &&
						   piece->Position.y == Editor_State.CursorPosition.y) {
							spaceOccupied = 1;
							break;
						}
					}
					if(!spaceOccupied) {
						TerrainPiece newPiece;
						newPiece.Position = Editor_State.CursorPosition;
						Array_Push(Editor_State.Terrain, &newPiece);
						printf("Placed terrain at x: %.2f, y: %.2f\n",
						       newPiece.Position.x, newPiece.Position.y);
					}
				} break;
			}
			break;
		case SDL_MOUSEWHEEL:
			Editor_State.Camera.VerticalFoV =
			    Clamp_R32(Editor_State.Camera.VerticalFoV - e->wheel.y * 0.05f,
			              0.05f * Pi, 0.95f * Pi);
			break;
		default: {
			// printf("Unknown event type %u\n", e->type);
			break;
		}
	}
}

// Thank you,
// http://ogldev.atspace.co.uk/www/tutorial13/tutorial13.html
void Camera_Mat4_Ex(Camera c, Mat4 out_view, Mat4 out_proj) {
	Mat4 Translation, Rotation;

	Mat4_Identity(Translation);
	Mat4_Identity(Rotation);

	Mat4_Translate(Translation, Vec3_Neg(c.Position));

	Vec3 tgt = Vec3_Sub(c.Target, c.Position);

	Vec3 N = Vec3_Norm(tgt);
	Vec3 U = Vec3_Norm(Vec3_Cross(c.Up, tgt));
	Vec3 V = Vec3_Cross(N, U);

	// printf("%.2f %.2f %.2f\n", N.x, N.y, N.z);

	// N points towards the target point
	// U points to the right of the camera
	// V points towards the sky

	Rotation[0] = U.x;
	Rotation[1] = U.y;
	Rotation[2] = U.z;

	Rotation[4] = V.x;
	Rotation[5] = V.y;
	Rotation[6] = V.z;

	Rotation[8] = N.x;
	Rotation[9] = N.y;
	Rotation[10] = N.z;

	Mat4_MultMat(Rotation, Translation);
	Mat4_Copy(out_view, Rotation);

	switch(c.Mode) {
		case CameraMode_Orthographic:
			Mat4_OrthoProj(out_proj, 0, c.ScreenWidth, 0, c.ScreenHeight,
			               c.ZNear, c.ZFar);
			break;
		case CameraMode_Perspective:
			Mat4_RectProj(out_proj, c.VerticalFoV, c.AspectRatio, c.ZNear,
			              c.ZFar);
			break;

		case CameraMode_NumModes: exit(EXIT_FAILURE);
	}
}

void Editor_Render() {
	// Position camera
	Editor_State.Camera.Target = (Vec3){.x = Editor_State.CursorPosition.x,
	                                    .y = 0,
	                                    .z = Editor_State.CursorPosition.y};
	Editor_State.Camera.Position =
	    Vec3_Add(Editor_State.Camera.Target, V3(1.2, 1.1, -1.6));

	Editor_State.Camera.AspectRatio = RSys_GetSize().AspectRatio;

	// Set up Model-View-Projection matrix
	Mat4 VP;
	{
		Mat4 View;
		Camera_Mat4_Ex(Editor_State.Camera, View, VP);
		Mat4_MultMat(VP, View);
	}


	// Render the wireframe cursor
	{
		Transform3D CursorTransform = {.Position = Editor_State.Camera.Target,
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
