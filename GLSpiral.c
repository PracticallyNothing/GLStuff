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
		points[(i*4)+0] = V3((r32) i-size/2, 0, -size);
		points[(i*4)+1] = V3((r32) i-size/2, 0,  size);

		points[(i*4)+2] = V3(-size, 0, ((r32) i)-size/2);
		points[(i*4)+3] = V3( size, 0, ((r32) i)-size/2);
	}
	R3D_DrawLines(OrbitCamera_ToCamera(c), points, size*2, V4(0.8, 0.8, 0.8, 1));
	Free(points);
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

	JSON_Value v = JSON_FromFile("conf.json");
	if(v.Type == JSON_Error)
	{
		Log(ERROR, "Couldn't load config, using default values.", "");
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

int main(int argc, char *argv[]) {
	u32 StartupTime = SDL_GetTicks();
	srand(time(NULL));

	Configuration Conf = ReadJSONConf();
	RSys_Init(Conf.ScreenWidth, Conf.ScreenHeight);
	RSys_SetFPSCap(Conf.FPSCap);

	RGB ClearColor = HexToRGB("52a9e0");
	glClearColor(ClearColor.r, ClearColor.g, ClearColor.b, 1);

	Audio_Init(NULL);
	Audio_Buffer buffers[3] = {
		Audio_Buffer_FromFile("res/audio/left.wav"),
		Audio_Buffer_FromFile("res/audio/right.wav"),
		Audio_Buffer_FromFile("res/audio/test.wav"),
	};
	Audio_Buffer *b = buffers;

	Audio_Src src = Audio_Src_Init();
	Audio_Src_SetBuffer(src, *b);

	Audio_SrcProps srcProps = Audio_Src_ReadProps(src);
	srcProps.Loop = 1;
	srcProps.PosRelative = AL_FALSE;
	srcProps.Pitch = 1;
	Audio_Src_SetProps(src, &srcProps);
	Audio_Src_Play(src);
	Audio_Src_SeekTo(src, 5);

	Shader *s = Shader_FromFile("res/shaders/3d/unlit-tex.glsl");

	Log(INFO, "Startup time: %u ms", SDL_GetTicks() - StartupTime);
	SDL_GL_SetSwapInterval(-1);

	float Time = 0;

	Vec2 InitialDragMousePos = V2C(0, 0);
	Vec2 InitialYawPitch = V2C(0, 0);
	bool8 MouseDragging = 0;

	OrbitCamera Cam = {
		.Center = V3C(0,0,0),
		.ZNear = 1e-2,
		.ZFar = 1e10,
		.VerticalFoV = Pi_Half,
		.Yaw   = DegToRad(0),
		.Pitch = DegToRad(45),
		.Radius = 20,
		.AspectRatio = RSys_GetSize().AspectRatio
	};

	Camera cc = OrbitCamera_ToCamera(Cam);
	Audio_Listener_SyncToCamera(cc);

	while(1) {
		SDL_Event e;
		while(SDL_PollEvent(&e)) {
			switch(e.type) {
				case SDL_QUIT: goto end;
				case SDL_KEYUP: {
					switch(e.key.keysym.sym) {
						case SDLK_ESCAPE:
							goto end;
						case SDLK_SPACE:
							if(Audio_Src_ReadState(src) == SourceState_Playing)
								Audio_Src_Pause(src);
							else
								Audio_Src_Play(src);
							break;
						case SDLK_TAB:
							Log(INFO, "Switched buffers (%d to %d).", b - buffers, (b - buffers + 1) % 3);
							b = buffers + ((b - buffers + 1) % 3);
							Audio_Src_Stop(src);
							Audio_Src_SetBuffer(src, *b);
							Audio_Src_Play(src);
							break;
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
							glViewport(0, 0, w, h);
							Cam.AspectRatio = (r32) w/h;
							break;
						}
					}
				} break;
				case SDL_MOUSEMOTION: {
					if(MouseDragging) {
						r32 dx = (e.motion.x - InitialDragMousePos.x) / 512.0;
						r32 dy = (e.motion.y - InitialDragMousePos.y) / 256.0;

						Cam.Yaw = (-dx) * Pi_Half + InitialYawPitch.x;
						Cam.Pitch = Clamp_R32(
							(-dy) * Pi_Half + InitialYawPitch.y,
							DegToRad(0.1),
							DegToRad(179.9)
						);
						//Log(INFO, "New Pitch: %.2f", RadToDeg(Cam.Pitch));
					}
				} break;
				case SDL_MOUSEBUTTONDOWN: {
					if(e.button.button == SDL_BUTTON_RIGHT) {
						// Begin camera drag if it hasn't been started yet.
						MouseDragging = 1;

						InitialDragMousePos = V2(e.button.x, e.button.y);
						InitialYawPitch     = V2(Cam.Yaw, Cam.Pitch);
					}
				} break;
				case SDL_MOUSEWHEEL: {
					//Cam.VerticalFoV = MAX(Pi_Quarter + 0.05, MIN(Pi, Cam.VerticalFoV - e.wheel.y * 0.1));
					Cam.Radius = MAX(2, MIN(50, Cam.Radius - e.wheel.y));
				} break;
				case SDL_MOUSEBUTTONUP: {
					if(e.button.button == SDL_BUTTON_RIGHT)
						MouseDragging = 0;
				} break;
				default: {
				} break;
			}
		}

		cc = OrbitCamera_ToCamera(Cam);
		Audio_Listener_SyncToCamera(cc);

		// Render frame to back buffer.
		if(RSys_NeedRedraw()) {
			DrawGrid(Cam, 200);

			srcProps.Position = V3(5*sinf(Time), 0, 5*cosf(Time));
			srcProps.Direction = Vec3_Norm(Vec3_Neg(srcProps.Position));
			Audio_Src_SetProps(src, &srcProps);

			r32 playpos = Audio_Src_ReadPlayheadPos(src);
			Audio_BufferProps bProps = Audio_Buffer_ReadProps(*b);
			r32 len = bProps.LenSeconds;

			R3D_DrawWireSphere(cc, srcProps.Position, 0.5, V4(1,1,1,1));

			/*
			Vec3 a = Vec3_Add(srcProps.Position, V3( 0.5, 0, -0.5)),
				 b = Vec3_Add(srcProps.Position, V3(-0.5, 0, -0.5)),
				 c = Vec3_Add(srcProps.Position, V3( 0,   0,  0.5));
			R3D_DrawTriangle(cc, a, b, c,  V4(1,1,1,1));
			*/

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

			R2D_DrawText(
				V2(10,10),
				V4(1,1,1,1),
				V4(0,0,0,0),
				&R2D_DefaultFont_Large,
				"%s Playing sound: %02.0f:%05.2f/%02.0f:%05.2f (%d channels)",
				(Audio_Src_ReadState(src) == SourceState_Playing ? ">>" : "||"),
				floor(playpos / 60.0f), fmodf(playpos, 60),
				floor(len / 60.0f),     fmodf(len, 60),
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
