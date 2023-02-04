#include <SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "GraphicsLib/Audio.h"
#include "GraphicsLib/Camera.h"
#include "GraphicsLib/Common.h"
#include "GraphicsLib/JSON.h"
#include "GraphicsLib/Math3D.h"
#include "GraphicsLib/Render.h"
#include "GraphicsLib/Shader.h"
#include "GraphicsLib/WavefrontOBJ.h"

#include "GraphicsLib/Utils.h"

void DrawGrid(OrbitCamera c, i32 size) {
	Vec3* points = Allocate(sizeof(Vec3) * 4 * size);
	for(i32 i = 0; i < size; i++) {
		points[(i * 4) + 0] = V3(i - size / 2.0, 0, -size);
		points[(i * 4) + 1] = V3(i - size / 2.0, 0, size);

		points[(i * 4) + 2] = V3(-size, 0, i - size / 2.0);
		points[(i * 4) + 3] = V3(size, 0, i - size / 2.0);
	}
	R3D_DrawLines(OrbitCamera_ToCamera(c), points, size * 2, V4(0.8, 0.8, 0.8, 1));
	Free(points);
}

typedef struct CamControl CamControl;
struct CamControl {
	OrbitCamera* Camera;

	r32 Sensitivity;

	bool8 MouseDragging;
	Vec2 InitialDragMousePos;
	Vec2 InitialYawPitch;
};

void OrbitCamera_StartDrag(CamControl* cc, Vec2 mousePos) {
	cc->MouseDragging       = 1;
	cc->InitialDragMousePos = mousePos;
	cc->InitialYawPitch     = V2(cc->Camera->Yaw, cc->Camera->Pitch);
}

void OrbitCamera_MoveByDrag(CamControl* cc, Vec2 dragAmt) {
	if(!cc->MouseDragging) return;

	r32 dx = dragAmt.x / 512.0 * cc->Sensitivity;
	r32 dy = dragAmt.y / 256.0 * cc->Sensitivity;

	cc->Camera->Yaw = (-dx) * Pi_Half + cc->InitialYawPitch.x;
	cc->Camera->Pitch = Clamp_R32(
		(-dy) * Pi_Half + cc->InitialYawPitch.y,
		DegToRad(0.1),
		DegToRad(179.9)
	);
}

void OrbitCamera_EndDrag(CamControl* cc) { cc->MouseDragging = 0; }

typedef struct Triangle {
	union {
		Vec3 points[3];
		struct { Vec3 a, b, c; };
	};
} Triangle;

DEF_ARRAY(Triangle, Triangle);
DECL_ARRAY(Triangle, Triangle);

Triangle Triangle_Random() {
	Triangle res;

#define RANDOM ((float) rand() / (float) RAND_MAX) * 5 - 2.5
	res.a = (Vec3) V3C(RANDOM, RANDOM, RANDOM);
	res.b = (Vec3) V3C(RANDOM, RANDOM, RANDOM);
	res.c = (Vec3) V3C(RANDOM, RANDOM, RANDOM);
#undef RANDOM

	return res;
}

Array_Triangle LoadCases() {
	Array_Triangle tris = {0};

	FILE* f = fopen("/tmp/broken-tris.bin", "r");

	if(!f) {
		Log(WARN, "Tris file couldn't be opened.", "");
		return tris;
	}

	u32 numCases;
	fread(&numCases, sizeof(u32), 1, f);
	Log(DEBUG, "Loaded %d cases.", numCases/2);

	Array_Triangle_Prealloc(&tris, numCases);
	fread(tris.Data, sizeof(Triangle), numCases, f);
	tris.Size = numCases;

	fclose(f);

	return tris;
}

void SaveCases(const Array_Triangle* tris) {
	if(tris->Size == 0)
		return;

	FILE* f = fopen("/tmp/broken-tris.bin", "w");
	if(!f) {
		Log(ERROR, "Couldn't save cases - file failed to open.", "");
		return;
	}

	fwrite(&tris->Size, sizeof(u32), 1, f);
	fwrite(tris->Data, sizeof(Triangle), tris->Size, f);

	Log(DEBUG, "Wrote %d cases to disk.", tris->Size/2);

	fclose(f);
}

int main() {
	u32 StartupTime = SDL_GetTicks();
	srand(time(NULL));

	Array_Triangle tris = LoadCases();

	RSys_Init(1280, 720);
	RSys_SetFPSCap(60);

	RT def = RT_InitScreenSize();

	Shader* s = Shader_FromFile("res/shaders/3d/unlit-tex.glsl");

	Log(INFO, "[Main] Startup time: %u ms", SDL_GetTicks() - StartupTime);
	SDL_GL_SetSwapInterval(-1);

	float Time = 0;

	OrbitCamera Cam = {
		.Center      = V3C(0, 0, 0),
		.ZNear       = 1e-2,
		.ZFar        = 1e10,
		.VerticalFoV = Pi_Half,
		.Yaw         = DegToRad(0),
		.Pitch       = DegToRad(0.01),
		.Radius      = 20,
		.AspectRatio = RT_GetCurrentAspectRatio()
	};

	CamControl cControl = {
		.Camera              = &Cam,
		.InitialDragMousePos = V2(0, 0),
		.InitialYawPitch     = V2(0, 0),
		.Sensitivity         = 1,
		.MouseDragging       = 0,
	};

	Camera cc = OrbitCamera_ToCamera(Cam);

	bool8 ChangeDisplaySize = 0;
	Vec2 NewSize            = V2(0, 0);

	bool8 broken_mode = 0;
	i32 broken_index = 0;
	Triangle a = Triangle_Random(),
	         b = Triangle_Random();

	bool8 recalcNeeded = 1;
	Intersection intersection;

	while(1) {
		if(recalcNeeded) {
			if(broken_mode && tris.Size > 0)
				intersection = TriTri_Intersect(
					tris.Data[broken_index*2  ].points,
					tris.Data[broken_index*2+1].points
				);
			else
				intersection = TriTri_Intersect(a.points, b.points);

			recalcNeeded = 0;
		}

		SDL_Event e;
		while(SDL_PollEvent(&e)) {
			switch(e.type) {
				case SDL_QUIT:
					goto end;
				case SDL_KEYUP:
					switch(e.key.keysym.sym) {
						case SDLK_ESCAPE:
							goto end;
						case SDLK_d:
							broken_mode = !broken_mode;
							break;
						case SDLK_z:
							if(!broken_mode) {
								Array_Triangle_PushVal(&tris, a);
								Array_Triangle_PushVal(&tris, b);
							}
							break;
						case SDLK_LEFT:
							if(broken_mode && tris.Size > 0) {
								broken_index--;
								if(broken_index < 0)
									broken_index = tris.Size / 2 - 1;
								recalcNeeded = 1;
							}
							break;
						case SDLK_RIGHT:
							if(broken_mode && tris.Size > 0) {
								broken_index++;
								broken_index %= tris.Size / 2;
								recalcNeeded = 1;
							}
							break;
						case SDLK_HOME:
							if(broken_mode) {
								broken_index = 0;
								recalcNeeded = 1;
							}
							break;
						case SDLK_END:
							if(broken_mode) {
								broken_index = tris.Size / 2 - 1;
								recalcNeeded = 1;
							}
							break;
						case SDLK_r:
							if(!broken_mode) {
								a = Triangle_Random();
								b = Triangle_Random();
								recalcNeeded = 1;
							}
							break;
					}
					break;
				case SDL_WINDOWEVENT:
					switch(e.window.event) {
						case SDL_WINDOWEVENT_CLOSE: goto end;
						case SDL_WINDOWEVENT_SIZE_CHANGED:
						case SDL_WINDOWEVENT_RESIZED: {
							i32 w = e.window.data1;
							i32 h = e.window.data2;

							RT_SetSize(&def, w, h);
							glViewport(0, 0, w, h);
							Cam.AspectRatio = (r32) w / h;
							break;
						}
					}
					break;
				case SDL_MOUSEMOTION:
					OrbitCamera_MoveByDrag(
						&cControl,
						V2(e.motion.x - cControl.InitialDragMousePos.x,
						   e.motion.y - cControl.InitialDragMousePos.y)
					);
					break;
				case SDL_MOUSEBUTTONDOWN:
					if(e.button.button == SDL_BUTTON_RIGHT)
						OrbitCamera_StartDrag(&cControl, V2(e.button.x, e.button.y));
					break;
				case SDL_MOUSEWHEEL: {
					const r32 minFov = DegToRad(60);
					const r32 maxFov = DegToRad(140);

					cControl.Camera->VerticalFoV =
						CLAMP(cControl.Camera->VerticalFoV - e.wheel.y * 0.1,
							  minFov, maxFov);

					/*
					cc->Camera->Radius = MAX(
						2, MIN(50, cc->Camera->Radius - e.wheel.y)
					);
					*/
				} break;
				case SDL_MOUSEBUTTONUP:
					if(e.button.button == SDL_BUTTON_RIGHT)
						OrbitCamera_EndDrag(&cControl);
					break;
				default:
					break;
			}
		}

		cc = OrbitCamera_ToCamera(Cam);
		// Audio_Listener_SyncToCamera(cc);

		// Render frame to back buffer.
		if(RSys_NeedRedraw()) {
			if(ChangeDisplaySize) {
				ChangeDisplaySize--;

				if(!ChangeDisplaySize) RT_SetSize(&def, NewSize.x, NewSize.y);
			}
			RT_Clear(def);
			RT_Use(def);

			//DrawGrid(Cam, 200);

			if(broken_mode) {
				Triangle t1 = tris.Data[broken_index*2];
				Triangle t2 = tris.Data[broken_index*2+1];

				R3D_DrawTriangle(cc, t1.a, t1.b, t1.c, V4(1, 0.8, 0.8, 1));
				R3D_DrawTriangle(cc, t2.a, t2.b, t2.c, V4(0.8, 1, 0.8, 1));
			} else {
				R3D_DrawTriangle(cc, a.a, a.b, a.c, V4(1, 0.8, 0.8, 1));
				R3D_DrawTriangle(cc, b.a, b.b, b.c, V4(0.8, 1, 0.8, 1));
			}

			RT_UseDefault();

			Vec2 scrSz = RT_GetScreenSize();
			Rect2D screen = {.Position = V2C(0, 0), .Size = scrSz};
			Rect2D_DrawImage(screen, def.Color.Id, 0);

			if(broken_mode) {
				Text2D_Draw(V2(0, 0), &TextStyle_Default,
				            "[%d/%d]", broken_index+1, tris.Size / 2);
			}

			const char* randInfo = 
				"[d] - enter broken mode\n"
				"[r] - New random triangles\n"
				"[z] - Save case as broken";

			const char* brokenInfo = 
				"[d] - exit broken mode\n"
				"<- / -> - previous/next case\n"
				"[home] - first case\n"
				"[end]  - last case";

			const char* info = broken_mode ? brokenInfo : randInfo;

			Vec2 sz = Text2D_Size(&TextStyle_Default, info);

			Text2D_Draw(
				V2(scrSz.w - sz.w, 0), 
				&TextStyle_Default,
				info
			);

			const char* intersectInfo = intersection.Occurred ? "Intersection!" : "No intersection.";
			sz = Text2D_Size(&TextStyle_Default, intersectInfo);

			Text2D_Draw(V2(scrSz.w / 2 - sz.w / 2, scrSz.h - sz.h), &TextStyle_Default, "%s", intersectInfo);

			// Display the work onto the screen.
			RSys_FinishFrame();
			Time += 1e-2;
		}
	}

end:
	Shader_Free(s);
	// Audio_Quit();

	SaveCases(&tris);
	Array_Triangle_Free(&tris);

	RSys_Quit();
	Log(INFO, "[Main] Quit.", "");
}
