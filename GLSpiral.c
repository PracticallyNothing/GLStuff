#include <SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "Camera.h"
#include "Common.h"
#include "Math3D.h"
#include "Render.h"
#include "Shader.h"
#include "JSON.h"
#include "Audio.h"

void DrawGrid(OrbitCamera c, i32 size)
{
	Vec3 *points = Allocate(sizeof(Vec3) * 4 * size);
	for(u32 i = 0; i < size; i++)
	{
		points[(i*4)+0] = V3(i-size/2.0, 0, -size);
		points[(i*4)+1] = V3(i-size/2.0, 0,  size);

		points[(i*4)+2] = V3(-size, 0, i-size/2.0);
		points[(i*4)+3] = V3( size, 0, i-size/2.0);
	}
	R3D_DrawLines(OrbitCamera_ToCamera(c), points, size*2, V4(0.8, 0.8, 0.8, 1));
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

bool8 OrbitCamera_Control(CamControl* cc, SDL_Event e)
{
	switch(e.type) {
		default:
			return 0;
		case SDL_MOUSEMOTION: {
			if(cc->MouseDragging) {
				r32 dx = (e.motion.x - cc->InitialDragMousePos.x) / 512.0 * cc->Sensitivity;
				r32 dy = (e.motion.y - cc->InitialDragMousePos.y) / 256.0 * cc->Sensitivity;

				cc->Camera->Yaw = (-dx) * Pi_Half + cc->InitialYawPitch.x;
				cc->Camera->Pitch = Clamp_R32(
					(-dy) * Pi_Half + cc->InitialYawPitch.y,
					DegToRad(0.1),
					DegToRad(179.9)
				);
			}
		} break;
		case SDL_MOUSEBUTTONDOWN: {
			if(e.button.button == SDL_BUTTON_RIGHT) {
				// Begin camera drag if it hasn't been started yet.
				cc->MouseDragging = 1;

				cc->InitialDragMousePos = V2(e.button.x, e.button.y);
				cc->InitialYawPitch     = V2(cc->Camera->Yaw, cc->Camera->Pitch);
			}
		} break;
		case SDL_MOUSEWHEEL: {
			//Cam.VerticalFoV = MAX(Pi_Quarter + 0.05, MIN(Pi, Cam.VerticalFoV - e.wheel.y * 0.1));
			cc->Camera->Radius = MAX(2, MIN(50, cc->Camera->Radius - e.wheel.y));
		} break;
		case SDL_MOUSEBUTTONUP: {
			if(e.button.button == SDL_BUTTON_RIGHT)
				cc->MouseDragging = 0;
		} break;
	}

	return 1;
}

typedef struct Configuration Configuration;
struct Configuration {
	u32 ScreenWidth;
	u32 ScreenHeight;
	u32 FPSCap;
};

static const 
Configuration DefaultConf = {
	.ScreenWidth = 1280,
	.ScreenHeight = 720,
	.FPSCap = 60,
};

Configuration ReadJSONConf()
{
	Configuration Conf = DefaultConf;

	Log(INFO, "[Main] Loading conf.json.", "");
	JSON_Value v = JSON_FromFile("conf.json");
	if(v.Type == JSON_Error)
	{
		Log(ERROR, "[Main] Couldn't load config, using default values.", "");
		return Conf;
	}

	enum {
		CONF_ScreenWidth,
		CONF_ScreenHeight,
		CONF_FPS
	};

	const char* ConfStrings[] = {
		"ScreenWidth",
		"ScreenHeight",
		"FPS",
	};

	u128 ConfHashes[sizeof(ConfStrings) / sizeof(char*)];
	for(u32 i = 0; i < sizeof(ConfStrings) / sizeof(char*); ++i)
		ConfHashes[i] = Hash_String_MD5(ConfStrings[i]);

	for(u32 i = 0; i < v.Object.Map.Size; ++i)
	{
		u128 *k = v.Object.Map.Keys + i;
		JSON_Value *val = v.Object.Map.Values + i;

		if(Hash_Equal(k, ConfHashes + CONF_ScreenWidth)) {
			if(val->Type == JSON_Number) 
				Conf.ScreenWidth = val->Number;
		} else if(Hash_Equal(k, ConfHashes + CONF_ScreenHeight)) {
			if(val->Type == JSON_Number) 
				Conf.ScreenHeight = val->Number;
		} else if(Hash_Equal(k, ConfHashes + CONF_FPS)) {
			if(val->Type == JSON_Number) 
				Conf.FPSCap = val->Number;
		}
	}
	JSON_Free(&v);

	return Conf;
}

u32 CurrentSize = 3;
static const u32 Sizes[] = {
	256, 144,
	640, 360, 
	1280, 720,
	1920, 1080,
	3840, 2160
};

int main(int argc, char *argv[]) {
	u32 StartupTime = SDL_GetTicks();
	srand(time(NULL));

	Configuration Conf = ReadJSONConf();
	RSys_Init(Conf.ScreenWidth, Conf.ScreenHeight);
	RSys_SetFPSCap(Conf.FPSCap);

	//RGB ClearColor = HexToRGB("52a9e0");
	glClearColor(0, 0, 0, 0);

	//Audio_Init(NULL);
	Audio_Buffer buffers[3] = {
		Audio_Buffer_FromFile("res/audio/left.wav"),
		Audio_Buffer_FromFile("res/audio/right.wav"),
		Audio_Buffer_FromFile("res/audio/test.wav"),
	};
	Audio_Buffer *b = buffers;

	RT def = RT_Init(640, 480);
	RT_Clear(def);
	//RT_Clear(def2);

	Audio_Source src = Audio_Source_Init();
	Audio_Source_SetBuffer(src, *b);

	Audio_SourceProps srcProps = Audio_Source_ReadProps(src);
	srcProps.Loop = 1;
	srcProps.PosRelative = AL_FALSE;
	srcProps.Pitch = 1;
	Audio_Source_SetProps(src, &srcProps);
	Audio_Source_Play(src);
	Audio_Source_SeekTo(src, 5);

	Shader *s = Shader_FromFile("res/shaders/3d/unlit-tex.glsl");

	Log(INFO, "[Main] Startup time: %u ms", SDL_GetTicks() - StartupTime);
	SDL_GL_SetSwapInterval(-1);

	float Time = 0;

	OrbitCamera Cam = {
		.Center      = V3C(0,0,0),
		.ZNear       = 1e-2,
		.ZFar        = 1e10,
		.VerticalFoV = Pi_Half,
		.Yaw         = DegToRad(0),
		.Pitch       = DegToRad(1),
		.Radius      = 20,
		.AspectRatio = RT_GetCurrentAspectRatio()
	};
	CamControl cControl = {&Cam, 1, 0, V2(0,0), V2(0,0)};

	Camera cc = OrbitCamera_ToCamera(Cam);
	Audio_Listener_SyncToCamera(cc);

	TextStyle style = {
		.Align = Align_Center,
		.Anchor = Anchor_Center,
		.Color = V4(1,1,1,1),
		.BackgroundEnabled = 1,
		.Background = V4(0,0,0,1),
		.Font = &Font_Large
	};

	while(1) {
		SDL_Event e;
		while(SDL_PollEvent(&e)) {
			if(OrbitCamera_Control(&cControl, e))
				continue;

			switch(e.type) {
				case SDL_QUIT: goto end;
				case SDL_KEYUP: {
					switch(e.key.keysym.sym) {
						case SDLK_ESCAPE:
							goto end;
						case SDLK_SPACE:
							if(Audio_Source_ReadState(src) == SourceState_Playing)
								Audio_Source_Pause(src);
							else
								Audio_Source_Play(src);
							break;
						case SDLK_TAB:
							//Log(INFO, "Switched buffers (%d to %d).", b - buffers, (b - buffers + 1) % 3);
							//b = buffers + ((b - buffers + 1) % 3);
							//Audio_Src_SetBuffer(src, *b);
							//Audio_Src_Play(src);
							CurrentSize++;
							CurrentSize %= sizeof(Sizes) / sizeof(Sizes[0]) / 2;
							RT_SetSize(&def, Sizes[CurrentSize*2], Sizes[CurrentSize*2+1]);
							Log(INFO, "[Main] Current size: %ux%u", Sizes[CurrentSize*2], Sizes[CurrentSize*2+1]);
							break;
						case SDLK_r: {
							Vec2 sz = RT_GetScreenSize();
							RT_SetSize(&def, sz.w/2, sz.h/2);
						} break;
						default:
							break;
					}
				} break;
				case SDL_WINDOWEVENT: {
					switch(e.window.event) {
						case SDL_WINDOWEVENT_CLOSE: goto end;
						case SDL_WINDOWEVENT_SIZE_CHANGED:
						case SDL_WINDOWEVENT_RESIZED: {
							i32 w = e.window.data1, h = e.window.data2;
							RT_SetSize(&def, w/2, h/2);
							glViewport(0, 0, w, h);
							Cam.AspectRatio = (r32) w/h;
							break;
						}
					}
				} break;
				case SDL_MOUSEMOTION:     {} break;
				case SDL_MOUSEBUTTONDOWN: {} break;
				case SDL_MOUSEWHEEL:      {} break;
				case SDL_MOUSEBUTTONUP:   {} break;
				default:                  {} break;
			}
		}

		cc = OrbitCamera_ToCamera(Cam);
		Audio_Listener_SyncToCamera(cc);

		// Render frame to back buffer.
		if(RSys_NeedRedraw()) {
			RT_Clear(def);
			RT_Use(def);

			DrawGrid(Cam, 200);

			srcProps.Position = V3(5*sinf(Time), 0, 5*cosf(Time));
			srcProps.Direction = Vec3_Norm(Vec3_Neg(srcProps.Position));
			Audio_Source_SetProps(src, &srcProps);

			r32 playpos = Audio_Source_ReadPlayheadPos(src);
			Audio_BufferProps bProps = Audio_Buffer_ReadProps(*b);
			r32 len = bProps.LenSeconds;

			R3D_DrawWireSphere(cc, srcProps.Position, 0.5, V4(1,1,1,1));

			R3D_DrawLine(cc, V3(0,0,0), srcProps.Position, V4(1,1,1,1));
			R3D_DrawLine(cc, srcProps.Position, Vec3_Add(srcProps.Position, srcProps.Direction), V4(1,1,1,1));
			R3D_DrawLine(cc, srcProps.Position, Vec3_Add(srcProps.Position, Vec3_Norm(Vec3_Sub(srcProps.Position, cc.Position))), V4(1,1,1,1));

			glClear(GL_DEPTH_BUFFER_BIT);
			Vec3 x = Vec3_Neg(V3(cc.Position.x, 0, 0)),
				 y = V3(0, cc.Position.y, 0),
				 z = Vec3_Neg(V3(0, 0, cc.Position.z));
			R3D_DrawLine(cc, V3(0,0,0), x, V4(1, 0.3, 0.3, 1));
			R3D_DrawLine(cc, x, Vec3_Add(x, z), V4(0.3, 0.3, 1, 1));
			R3D_DrawLine(cc, Vec3_Add(x, z), Vec3_Add(y, Vec3_Add(x,z)), V4(0.3, 1, 0.3, 1));

			RT_UseDefault();
			Vec2 scrSz = RT_GetScreenSize();
			Vec2 halfScrSz = Vec2_DivScal(scrSz, 2);

			Rect2D screen[4] = {
				(Rect2D){ .Position = V2C(0, 0), .Size = halfScrSz },
				(Rect2D){ .Position = V2C(halfScrSz.x, 0), .Size = halfScrSz },
				(Rect2D){ .Position = V2C(0, halfScrSz.y), .Size = halfScrSz },
				(Rect2D){ .Position = halfScrSz, .Size = halfScrSz },
			};
			for(u32 i = 0; i < 4; i++)
				Rect2D_DrawImage(screen[i], def.Color.Id, 0);

			Rect2D horizDiv = {
				.Position = V2C(0, halfScrSz.y - 2),
				.Size = V2C(scrSz.x, 4),
				.Color = V4C(0,0,0,1)
			};
			Rect2D vertDiv = {
				.Position = V2C(halfScrSz.x - 2, 0),
				.Size = V2C(4, scrSz.y),
				.Color = V4C(0,0,0,1)
			};
			Rect2D_Draw(horizDiv, 1);
			Rect2D_Draw(vertDiv, 1);

			Text2D_Draw(V2(10,10), &style,
				"%s Playing sound: %02.0f:%05.2f/%02.0f:%05.2f (%d channels)",
				(Audio_Source_ReadState(src) == SourceState_Playing ? ">>" : "||"),
				floor(playpos / 60.0f),
				fmodf(playpos, 60),
				floor(len / 60.0f),
				fmodf(len, 60),
				bProps.NumChannels
			);

			// Display the work onto the screen.
			RSys_FinishFrame();
			Time += 1e-2;
		}
	}

end:
	Shader_Free(s);
	Audio_Quit();

	RSys_Quit();
}
