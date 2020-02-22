#include <SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Common.h"
#include "Editor.h"
#include "Math3D.h"
#include "Render.h"
#include "Shader.h"
#include "WavefrontOBJ.h"
#include "stb_image.h"

typedef struct GPUModel {
	GLuint VAO;
	GLuint VBOs[3];
	GLuint ElementBuffer;
	u32 NumVertices;
	u32 NumIndices;
} GPUModel;

r32 g_ScreenQuad[][4] = {
    {-1, +1, 0, 1},
    {-1, -1, 0, 0},
    {+1, +1, 1, 1},
    {+1, -1, 1, 0},
};

Vec3 g_CubePos[] = {
    /* 4 */ {-1, -1, -1},
    /* 5 */ {1, 1, -1},
    /* 6 */ {1, -1, -1},
    /* 7 */ {-1, 1, -1},

    /* 0 */ {-1, -1, 1},
    /* 1 */ {1, 1, 1},
    /* 2 */ {1, -1, 1},
    /* 3 */ {-1, 1, 1},
};
i32 g_CubeInds[] = {0, 2, 1, 3, 0, 1, 7, 4, 3, 4, 0, 3, 1, 2, 5, 2, 6, 5,
                    7, 5, 6, 7, 6, 4, 3, 1, 7, 1, 5, 7, 0, 6, 2, 0, 4, 6};

GLuint Texture_FromFile(const char *filename);
void RenderLines(Vec3 *linePoints, i32 numLines);
void RenderLineCircle(r32 radius);

void WObj_ToGPUModel(GPUModel *out, const WObj_Object *obj);

GLuint WireframeCube_VAO = 0;
GLuint WireframeCube_Pos;
GLuint WireframeCube_Inds;

int main(int argc, char *argv[]) {
	u32 StartupTime = SDL_GetTicks();

	RSys_Init(1280, 720);
	R2D_Init();
	i32 idx = R2D_LoadFont_TTF("/usr/share/fonts/TTF/UbuntuMono-R.ttf");

	SDL_Event e;
	u32 Ticks = 0;

	printf("Startup time: %u ms\n", SDL_GetTicks() - StartupTime);
	SDL_GL_SetSwapInterval(-1);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	Editor_Init();
	const Vec4 Colors[] = {
	    V4C(1, 0, 0, 0.25), V4C(0, 1, 0, 0.25), V4C(0, 0, 1, 0.25),
	    V4C(1, 1, 0, 0.25), V4C(0, 1, 1, 0.25), V4C(1, 1, 1, 0.25),
	};

	while(1) {
		Ticks = SDL_GetTicks();

		while(SDL_PollEvent(&e)) {
			switch(e.type) {
				case SDL_QUIT: goto end;
				case SDL_KEYUP:
					switch(e.key.keysym.sym) {
						case SDLK_ESCAPE:
						case SDLK_q: goto end;
						default: Editor_HandleInput(&e); break;
					}
					break;
				case SDL_WINDOWEVENT:
					switch(e.window.event) {
						case SDL_WINDOWEVENT_CLOSE: goto end;
						case SDL_WINDOWEVENT_SIZE_CHANGED:
						case SDL_WINDOWEVENT_RESIZED: {
							i32 w = e.window.data1, h = e.window.data2;
							glViewport(0, 0, w, h);
							break;
						}
					}
					break;
				default: Editor_HandleInput(&e); break;
			}
		}

		i32 N = 128;

		// Render frame to back buffer.
		if(Ticks - RSys_State.LastFrameTime > 16) {
			// Editor rendering
			Editor_Render();

			RSys_Size sz = RSys_GetSize();
			for(i32 y = 0; y < N; y++) {
				for(i32 x = 0; x < N; x++) {
					R2D_DrawRect(V2(sz.Width / N * x, sz.Height / N * y),
					             V2(sz.Width / N, sz.Height / N),
					             Colors[(x + y * N) %
					                    (sizeof(Colors) / sizeof(Colors[0]))],
					             1);
				}
			}

			// Display the work onto the screen.
			RSys_FinishFrame();
		}

		Ticks = SDL_GetTicks();
	}

end:
	RSys_Quit();
}


GLuint Texture_FromFile(const char *filename) {
	GLuint Texture;
	u8 *Data;
	i32 ImgWidth, ImgHeight, CompPerPixel;

	stbi_set_flip_vertically_on_load(1);
	Data = stbi_load(filename, &ImgWidth, &ImgHeight, &CompPerPixel, 0);
	if(!Data) return 0;

	glGenTextures(1, &Texture);
	glBindTexture(GL_TEXTURE_2D, Texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	printf("CompPerPixel: %d\n", CompPerPixel);

	GLenum Format;
	switch(CompPerPixel) {
		case 1: Format = GL_RED; break;
		case 2: Format = GL_RG; break;
		case 3: Format = GL_RGB; break;
		case 4: Format = GL_RGBA; break;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ImgWidth, ImgHeight, 0, Format,
	             GL_UNSIGNED_BYTE, Data);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(Data);
	return Texture;
}

GLuint Lines_VAO = 0, Lines_Pos = 0;
void RenderLines(Vec3 *linePoints, i32 numLines) {
	if(Lines_VAO == 0) {
		glGenVertexArrays(1, &Lines_VAO);
		glBindVertexArray(Lines_VAO);

		glGenBuffers(1, &Lines_Pos);
		glBindBuffer(GL_ARRAY_BUFFER, Lines_Pos);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	}
	glBindBuffer(GL_ARRAY_BUFFER, Lines_Pos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vec3) * 2 * numLines, linePoints,
	             GL_DYNAMIC_DRAW);
	glBindVertexArray(Lines_VAO);
	glDrawArrays(GL_LINES, 0, numLines * 2);
}

void RenderLineCircle(r32 radius) {
	Vec3 *lines = malloc(sizeof(Vec3) * 2 * 32);

	for(int i = 0; i < 32; i++) {
		r32 angle = Tau / 32 * i;
		r32 nextAngle = Tau / 32 * (i + 1);
		lines[i * 2] = V3(radius * sinf(angle), -5, radius * cosf(angle));
		lines[i * 2 + 1] =
		    V3(radius * sinf(nextAngle), -5, radius * cosf(nextAngle));
	}

	RenderLines(lines, 32);
	free(lines);
}

#define VBO_POS 0
#define VBO_UV 1
#define VBO_NORM 2

void WObj_ToGPUModel(GPUModel *out, const WObj_Object *obj) {
	glGenVertexArrays(1, &out->VAO);
	glBindVertexArray(out->VAO);
	glGenBuffers(3, out->VBOs);
	glGenBuffers(1, &out->ElementBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, out->ElementBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * obj->NumIndices,
	             obj->Indices, GL_STATIC_DRAW);

	out->NumVertices = obj->NumVertices;
	out->NumIndices = obj->NumIndices;

	Array *Positions = Array_Init(sizeof(Vec3));
	Array *UVs = Array_Init(sizeof(Vec2));
	Array *Normals = Array_Init(sizeof(Vec3));

	for(i32 i = 0; i < obj->NumVertices; i++) {
		WObj_Vertex *vtx = &obj->Vertices[i];
		Array_Push(Positions, (u8 *) &vtx->Position);
		Array_Push(UVs, (u8 *) &vtx->UV);
		Array_Push(Normals, (u8 *) &vtx->Normal);
	}

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, out->VBOs[VBO_POS]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vec3) * obj->NumVertices,
	             Positions->Data, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, out->VBOs[VBO_UV]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vec2) * obj->NumVertices, UVs->Data,
	             GL_STATIC_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, out->VBOs[VBO_NORM]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vec3) * obj->NumVertices,
	             Normals->Data, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	Array_Free(Positions);
	Array_Free(UVs);
	Array_Free(Normals);
}
