#include <SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "Audio.h"
#include "Camera.h"
#include "Common.h"
#include "JSON.h"
#include "Math3D.h"
#include "Render.h"
#include "Shader.h"
#include "WavefrontOBJ.h"

#include "Utils.h"

void DrawGrid(OrbitCamera c, i32 size) {
	Vec3* points = Allocate(sizeof(Vec3) * 4 * size);
	for(u32 i = 0; i < size; i++) {
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
	cc->Camera->Pitch =
	    Clamp_R32((-dy) * Pi_Half + cc->InitialYawPitch.y, DegToRad(0.1), DegToRad(179.9));
}

void OrbitCamera_EndDrag(CamControl* cc) { cc->MouseDragging = 0; }

typedef struct Configuration Configuration;
struct Configuration {
	u32 ScreenWidth;
	u32 ScreenHeight;
	u32 FPSCap;
};

static const Configuration DefaultConf = {
    .ScreenWidth  = 1280,
    .ScreenHeight = 720,
    .FPSCap       = 60,
};

Configuration ReadJSONConf() {
	Configuration Conf = DefaultConf;

	Log(INFO, "[Main] Loading conf.json.", "");
	JSON_Value v = JSON_FromFile("conf.json");
	if(v.Type == JSON_Error) {
		Log(ERROR, "[Main] Couldn't load config, using default values.", "");
		return Conf;
	}

	enum { CONF_ScreenWidth, CONF_ScreenHeight, CONF_FPS };

	const char* ConfStrings[] = {
	    "ScreenWidth",
	    "ScreenHeight",
	    "FPS",
	};

	u128 ConfHashes[sizeof(ConfStrings) / sizeof(char*)];
	for(u32 i = 0; i < sizeof(ConfStrings) / sizeof(char*); ++i)
		ConfHashes[i] = Hash_String_MD5(ConfStrings[i]);

	for(u32 i = 0; i < v.Object.Map.Size; ++i) {
		u128* k         = v.Object.Map.Keys + i;
		JSON_Value* val = v.Object.Map.Values + i;

		if(Hash_Equal(k, ConfHashes + CONF_ScreenWidth)) {
			Conf.ScreenWidth = val->Number;
		} else if(Hash_Equal(k, ConfHashes + CONF_ScreenHeight)) {
			Conf.ScreenHeight = val->Number;
		} else if(Hash_Equal(k, ConfHashes + CONF_FPS)) {
			Conf.FPSCap = val->Number;
		}
	}
	JSON_Free(&v);

	return Conf;
}

int main(int argc, char* argv[]) {
	u32 StartupTime = SDL_GetTicks();
	srand(time(NULL));

	Configuration Conf = ReadJSONConf();
	RSys_Init(Conf.ScreenWidth, Conf.ScreenHeight);
	RSys_SetFPSCap(Conf.FPSCap);

	RT def = {0};
	RT_InitScreenSize(def);

	WObj_Library* lib = WObj_FromFile("res/meshes/sphere.obj");
	GPUModel sphere   = {0};
	WObj_ToGPUModel(&sphere, lib->Objects + 0);

	Shader* s = Shader_FromFile("res/shaders/3d/unlit-tex.glsl");

	Log(INFO, "[Main] Startup time: %u ms", SDL_GetTicks() - StartupTime);
	SDL_GL_SetSwapInterval(-1);

	float Time = 0;

	OrbitCamera Cam = {.Center      = V3C(0, 0, 0),
	                   .ZNear       = 1e-2,
	                   .ZFar        = 1e10,
	                   .VerticalFoV = Pi_Half,
	                   .Yaw         = DegToRad(0),
	                   .Pitch       = DegToRad(0.01),
	                   .Radius      = 20,
	                   .AspectRatio = RT_GetCurrentAspectRatio()};

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

	while(1) {
		SDL_Event e;
		while(SDL_PollEvent(&e)) {
			if(OrbitCamera_Control(&cControl, e)) continue;

			switch(e.type) {
				case SDL_QUIT: goto end;
				case SDL_KEYUP:
					switch(e.key.keysym.sym) {
						case SDLK_ESCAPE: goto end;
						case SDLK_r:
							ChangeDisplaySize = 15;
							NewSize           = RT_GetScreenSize();
							break;
						default: break;
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
					OrbitCamera_MoveByDrag(&cControl,
					                       V2(e.motion.x - cControl.InitialDragMousePos.x,
					                          e.motion.y - cControl.InitialDragMousePos.y));
					break;
				case SDL_MOUSEBUTTONDOWN:
					if(e.button.button == SDL_BUTTON_RIGHT)
						OrbitCamera_StartDrag(&cControl, V2(e.button.x, e.button.y));
					break;
				case SDL_MOUSEWHEEL: {
					const r32 minFov = DegToRad(60);
					const r32 maxFov = DegToRad(140);

					cControl.Camera->VerticalFoV =
					    CLAMP(cControl.Camera->VerticalFoV - e.wheel.y * 0.1, minFov, maxFov);

					// cc->Camera->Radius = MAX(2, MIN(50, cc->Camera->Radius - e.wheel.y));
				} break;
				case SDL_MOUSEBUTTONUP:
					if(e.button.button == SDL_BUTTON_RIGHT) OrbitCamera_EndDrag(&cControl);
					break;
				default: {
				} break;
			}
		}

		cc = OrbitCamera_ToCamera(Cam);
		Audio_Listener_SyncToCamera(cc);

		// Render frame to back buffer.
		if(RSys_NeedRedraw()) {
			if(ChangeDisplaySize) {
				ChangeDisplaySize--;

				if(!ChangeDisplaySize) RT_SetSize(&def, NewSize.x, NewSize.y);
			}
			RT_Clear(def);
			RT_Use(def);

			DrawGrid(Cam, 200);

			GPUModel_Render(&sphere);

			RT_UseDefault();

			Vec2 scrSz    = RT_GetScreenSize();
			Rect2D screen = {V2C(0, 0), scrSz};
			Rect2D_DrawImage(screen, def.Color.Id, 0);

			// Display the work onto the screen.
			RSys_FinishFrame();
			Time += 1e-2;
		}
	}

end:
	Shader_Free(s);
	Audio_Quit();

	RSys_Quit();
	Log(INFO, "[Main] Quit.", "");
}
