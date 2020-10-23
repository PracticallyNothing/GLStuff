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
#include "WavefrontOBJ.h"
#include "stb_image.h"
#include "Phys.h"

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

int main(int argc, char *argv[]) {
	u32 StartupTime = SDL_GetTicks();
	srand(time(NULL));

	RSys_Init(1280, 720);
	RGB ClearColor = HexToRGB("52a9e0");
	glClearColor(ClearColor.r, ClearColor.g, ClearColor.b, 1);

	SDL_Event e;
	u32 Ticks = 0;

	// Wavefront OBJ to OpenGL setup
	WObj_Library *Speedboat = WObj_FromFile("res/models/speedboat_2.obj");
	if(!Speedboat)
	{
		Log(FATAL, "speedboat.obj couldn't load.", "");
		return -1;
	}

	u32 *VAOs         = Allocate(sizeof(u32) * Speedboat->NumObjects);
	u32 *IndexBuffers = Allocate(sizeof(u32) * Speedboat->NumObjects);
	glGenVertexArrays(Speedboat->NumObjects, VAOs);
	glGenBuffers(Speedboat->NumObjects, IndexBuffers);

	//struct RSys_Texture testUV = RSys_TextureFromFile("res/textures/test-uv.jpg");

	//struct RSys_Texture noise      = RSys_TextureFromFile("res/textures/noise.jpg");
	//struct RSys_Texture foam       = RSys_TextureFromFile("res/textures/Foam002_2K_Color.jpg");
	//struct RSys_Texture waveHeight = RSys_TextureFromFile("res/textures/wave_height.png");

	for(u32 i = 0; i < Speedboat->NumObjects; i++)
	{
		glBindVertexArray(VAOs[i]);
		u32 VBO;
		glGenBuffers(1, &VBO);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(
				GL_ARRAY_BUFFER,
				Speedboat->Objects[i].NumVertices * sizeof(WObj_Vertex),
				Speedboat->Objects[i].Vertices,
				GL_STATIC_DRAW
				);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(WObj_Vertex), (void*) offsetof(WObj_Vertex, Position));
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(WObj_Vertex), (void*) offsetof(WObj_Vertex, UV));
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(WObj_Vertex), (void*) offsetof(WObj_Vertex, Normal));

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBuffers[i]);
		glBufferData(
				GL_ELEMENT_ARRAY_BUFFER,
				Speedboat->Objects[i].NumIndices * sizeof(u32),
				Speedboat->Objects[i].Indices,
				GL_STATIC_DRAW
				);
	}

	// Generate water tile.
	const u32 w = 20;
	const u32 l = 20;
	const u32 sz = w*l;
	Vec3 *pos = Allocate(sizeof(Vec3) * sz * 6);
	for(u32 z = 0; z < w; z++)
		for(u32 x = 0; x < l; x++)
		{
			pos[(x + z*w)*6 + 0] = V3(1.0/w *  x   , 0, 1.0/l * (z+1));
			pos[(x + z*w)*6 + 1] = V3(1.0/w *  x   , 0, 1.0/l *  z   );
			pos[(x + z*w)*6 + 2] = V3(1.0/w * (x+1), 0, 1.0/l * (z+1));
			pos[(x + z*w)*6 + 3] = V3(1.0/w *  x   , 0, 1.0/l *  z   );
			pos[(x + z*w)*6 + 4] = V3(1.0/w * (x+1), 0, 1.0/l *  z   );
			pos[(x + z*w)*6 + 5] = V3(1.0/w * (x+1), 0, 1.0/l * (z+1));
		}
	u32 WaterTileVAO = 0;
	u32 WaterTileVBO = 0;
	glGenVertexArrays(1, &WaterTileVAO);
	glBindVertexArray(WaterTileVAO);
	glGenBuffers(1, &WaterTileVBO);
	glBindBuffer(GL_ARRAY_BUFFER, WaterTileVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vec3)*sz*6, pos, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	Free(pos);

	struct Shader
		*s      = Shader_FromFile("res/shaders/3d/unlit-tex.glsl"),
		*sWire  = Shader_FromFile("res/shaders/3d/unlit-col.glsl"),
		*sWater = Shader_FromFile("res/shaders/3d/water.glsl");

	Log(Log_Info, "Startup time: %u ms", SDL_GetTicks() - StartupTime);
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
		.Yaw = DegToRad(290),
		.Pitch = DegToRad(45),
		.Radius = 20,
		.AspectRatio = RSys_GetSize().AspectRatio
	};

	while(1) {
		Ticks = SDL_GetTicks();

		while(SDL_PollEvent(&e)) {
			switch(e.type) {
				case SDL_QUIT: goto end;
				case SDL_KEYUP: {
					switch(e.key.keysym.sym) {
						case SDLK_ESCAPE:
							goto end;
						case SDLK_r:
							// Shader_Reload(sWater);
							// Shader_Reload(sWire);
							// Shader_Reload(s);
							Log(INFO, "Generating new triangles.", "");
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

		// Render frame to back buffer.
		if(Ticks - RSys_GetLastFrameTime() > 16) {
			DrawGrid(Cam, 200);

			// Display the work onto the screen.
			RSys_FinishFrame();
			Time += 1e-2;
		}

		Ticks = SDL_GetTicks();
	}

end:
	RSys_Quit();

	WObj_Library_Free(Speedboat);
	Shader_Free(s);
	Shader_Free(sWire);
	Shader_Free(sWater);
	Alloc_PrintInfo();

	Alloc_FreeAll();
}
