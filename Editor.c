#include "Editor.h"

#include "Common.h"
#include "Render.h"
#include "SDL_keycode.h"

Vec2 InitialDragMousePos = V2C(0, 0);
Vec2 InitialYawPitch = V2C(0, 0);
SDL_Cursor *Cursor_Normal, *Cursor_Rotate;

OrbitCamera Cam;
struct Shader *UnlitColor;

void Editor_Init() {
	// Setup camera
	Cam = (OrbitCamera){.ZNear = 1e-2,
	                    .ZFar = 1e10,
	                    .VerticalFoV = Pi_Half + Pi_Quarter,
	                    .Yaw = DegToRad(290),
	                    .Pitch = DegToRad(45),
	                    .Radius = 2};

	UnlitColor = Shader_FromFile("res/shaders/editor/terrain.glsl");

	Cursor_Normal = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
	Cursor_Rotate = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
}

const r32 speed = 1;
bool8 shift = 0;
enum { Drag_None = 0, Drag_Rotate, Drag_Move } DragMode = Drag_None;
Vec3 DragMoveOffset = V3C(0, 0, 0);

void Editor_HandleInput(SDL_Event *e) {
	Vec3 camDir = Vec3_Norm(Vec3_Neg(OrbitCamera_GetOffset(Cam)));
	Vec3 camRight = Vec3_Norm(Vec3_Cross(V3(0, 1, 0), camDir));
	Vec3 camUp = Vec3_Norm(Vec3_Cross(camRight, camDir));

	Vec3 camDirXZ = Vec3_Norm(V3(camDir.x, 0, camDir.z));

	switch(e->type) {
		case SDL_KEYDOWN:
			switch(e->key.keysym.sym) {
				case SDLK_LSHIFT:
				case SDLK_RSHIFT: shift = 1; break;

				default: break;
			}
			break;
		case SDL_KEYUP:
			switch(e->key.keysym.sym) {
				case SDLK_LSHIFT:
				case SDLK_RSHIFT: shift = 0; break;
				case SDLK_c:
					if(e->key.keysym.mod & KMOD_CTRL) {
						Cam.Center = V3(0, 0, 0);
						if(DragMode == Drag_Move) DragMode = Drag_None;
						DragMoveOffset = V3(0, 0, 0);
					}
					break;

				default: break;
			}
			break;

		case SDL_MOUSEWHEEL:
			Cam.Radius = Clamp_R32(Cam.Radius - e->wheel.y * 0.25f, 1, 10);
			break;
		case SDL_MOUSEBUTTONDOWN:
			if(e->button.button == SDL_BUTTON_RIGHT) {
				// Begin camera drag if it hasn't been started yet.
				if(!DragMode) {
					if(shift)
						DragMode = Drag_Move;
					else
						DragMode = Drag_Rotate;

					InitialDragMousePos = V2(e->button.x, e->button.y);
					InitialYawPitch = V2(Cam.Yaw, Cam.Pitch);

					SDL_SetCursor(Cursor_Rotate);
				}
			}
			break;
		case SDL_MOUSEMOTION: {
			if(DragMode) {
				r32 dx = (e->motion.x - InitialDragMousePos.x) / 512.0;
				r32 dy = (e->motion.y - InitialDragMousePos.y) / 256.0;

				if(DragMode == Drag_Rotate) {
					Cam.Yaw = (-dx) * Pi_Half + InitialYawPitch.x;
					Cam.Pitch = Clamp_R32((-dy) * Pi_Half + InitialYawPitch.y,
					                      DegToRad(1), DegToRad(89));
				} else if(DragMode == Drag_Move) {
					DragMoveOffset = Vec3_Add(Vec3_MultScal(camDirXZ, dy * 2),
					                          Vec3_MultScal(camRight, -dx * 4));
				}
			}
		} break;
		case SDL_MOUSEBUTTONUP:
			if(e->button.button == SDL_BUTTON_RIGHT) {
				DragMode = Drag_None;
				Cam.Center = Vec3_Add(Cam.Center, DragMoveOffset);
				DragMoveOffset = V3(0, 0, 0);
				SDL_SetCursor(Cursor_Normal);
			}
			break;
		default: {
			// printf("Unknown event type %u\n", e->type);
			break;
		}
	}
}

void Editor_Render() {
	// Set up Model-View-Projection matrix
	Mat4 vp;
	{
		Cam.Center = Vec3_Add(Cam.Center, DragMoveOffset);
		Cam.AspectRatio = RSys_GetSize().AspectRatio;

		Mat4 view;
		OrbitCamera_Mat4(Cam, view, vp);
		Mat4_MultMat(vp, view);

		Cam.Center = Vec3_Sub(Cam.Center, DragMoveOffset);
	}


	u32 vao = RSys_GetTempVAO();
	glBindVertexArray(vao);
	u32 vbos[2] = {0, 0};
	glGenBuffers(2, vbos);

	Vec3 pos[4] = {V3C(-1, 0, 1), V3C(-1, 0, -1), V3C(1, 0, 1), V3C(1, 0, -1)};
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pos), pos, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	Vec2 uv[4] = {V2C(-1, 1), V2C(-1, -1), V2C(1, 1), V2C(1, -1)};
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, vbos[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(uv), uv, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	Shader_Use(UnlitColor);
	Shader_UniformMat4(UnlitColor, "MVP", vp);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glBindVertexArray(0);
	RSys_FreeTempVAO(vao);

	const char *fmt =
	    "        [Camera] X: %.2f, Y: %.2f, Z: %.2f\n"
	    "[DragMoveOffset] X: %.2f, Y: %.2f, Z: %.2f";

	RSys_Size sz = RSys_GetSize();
	Vec2 textSz = R2D_GetTextExtents(
	    &R2D_DefaultFont, fmt, Cam.Center.x, Cam.Center.y, Cam.Center.z,
	    DragMoveOffset.x, DragMoveOffset.y, DragMoveOffset.z);

	R2D_DrawText(V2(sz.Width - textSz.x - 20, sz.Height - textSz.y - 20),
	             V4(1, 1, 1, 1), V4(0, 0, 0, 0), &R2D_DefaultFont, fmt,
	             Cam.Center.x, Cam.Center.y, Cam.Center.z, DragMoveOffset.x,
	             DragMoveOffset.y, DragMoveOffset.z);
}
