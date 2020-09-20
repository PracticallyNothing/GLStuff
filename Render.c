#include "Render.h"

#include <stdlib.h>
#include <string.h>

#include "stb_image.h"

struct RSys_State {
	SDL_Window *Window;
	SDL_GLContext GLContext;
	u64 LastFrameTime;
	u64 LastFrameDT;

	GLuint *TempVAOs;
	bool8 *VAOIsTaken;
	u32 NumVAOs, NumTakenVAOs;

	GLuint *TempVBOs;
	bool8 *VBOIsTaken;
	u32 NumVBOs, NumTakenVBOs;

} RSys_State;

u64 RSys_GetLastFrameTime() { return RSys_State.LastFrameTime; }

void RSys_LogVideoDriverInfo(void) {
	i32 NumVideoDrivers = SDL_GetNumVideoDrivers();
	if(NumVideoDrivers < 0) {
		Log(ERROR, "SDL_GetNumVideoDrivers() failed: %s\n", SDL_GetError());
		return;
	}

	Log(INFO, "Num video drivers: %d\n", NumVideoDrivers);

	for(i32 i = 0; i < NumVideoDrivers; ++i)
		Log(INFO, "#%d: %s\n", i, SDL_GetVideoDriver(i));
}

RSys_Size RSys_GetSize() {
	RSys_Size sz = {-1, -1};
	SDL_GetWindowSize(RSys_State.Window, &sz.Width, &sz.Height);
	sz.AspectRatio = (r32) sz.Width / sz.Height;
	return sz;
}

void R2D_Init();

void RSys_Init(u32 Width, u32 Height) {
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
		printf("SDL_Init() failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	// Set all OpenGL attributes to make sure we get what we want.

	// Make sure that we're using OpenGL 3.3 Core.
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
	                    SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	// Enable the backbuffer and set it to have 32 bits for color
	// and 16 bits for depth.
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 1);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 1);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 1);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 1);
	//SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);

	// Enable 4x multisampling.
	// SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	// SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

	// Create the actual window.
	RSys_State.Window = SDL_CreateWindow(
		"Boyan's Game",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		Width, Height, 
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
	);

	// If it wasn't created, error and exit.
	if(!RSys_State.Window)
		Log(Log_Fatal, "SDL_CreateWindow() failed: %s", SDL_GetError());

	// Create an OpenGL context for the window,
	// according to the earlier parameters.
	RSys_State.GLContext = SDL_GL_CreateContext(RSys_State.Window);

	// If our parameters couldn't be met, error and exit.
	if(!RSys_State.GLContext)
		Log(Log_Fatal, "SDL_GL_CreateContext() failed: %s", SDL_GetError());

	// Load OpenGL functions.
	if(!gladLoadGL())
		Log(Log_Fatal, "%s", "gladLoadGL() failed, OpenGL couldn't be loaded.");

	// Set a few default parameters.

	// Set the clear color to black.
	glClearColor(0, 0, 0, 1);

	// Enable depth testing and set less than or equal mode
	// for more stable rendering.
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	// Enable face culling.
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	// Enable multisampling.
	// glEnable(GL_MULTISAMPLE);

	// Set a fake last render time.
	RSys_State.LastFrameTime = SDL_GetTicks();

	static const u32 NUM_VAOS = 4;
	static const u32 NUM_VBOS = 32;

	RSys_State.TempVAOs = Allocate(sizeof(GLuint) * NUM_VAOS);
	RSys_State.VAOIsTaken = Allocate(sizeof(bool8) * NUM_VAOS);
	RSys_State.NumTakenVAOs = 0;
	RSys_State.NumVAOs = NUM_VAOS;

	RSys_State.TempVBOs = Allocate(sizeof(GLuint) * NUM_VBOS);
	RSys_State.VBOIsTaken = Allocate(sizeof(bool8) * NUM_VBOS);
	RSys_State.NumTakenVBOs = 0;
	RSys_State.NumVBOs = NUM_VBOS;

	memset((void *) RSys_State.VAOIsTaken, 0, sizeof(bool8) * NUM_VAOS);
	memset((void *) RSys_State.VBOIsTaken, 0, sizeof(bool8) * NUM_VBOS);

	glGenVertexArrays(RSys_State.NumVAOs, RSys_State.TempVAOs);
	glGenBuffers(RSys_State.NumVBOs, RSys_State.TempVBOs);

	GL_Initialized = 1;

	R2D_Init();
	R3D_Init();
}

void RSys_AllocMoreVAOs() {
	u32 N = RSys_State.NumVAOs * 2;

	RSys_State.TempVAOs = Reallocate(RSys_State.TempVAOs, sizeof(GLuint) * N);
	glGenVertexArrays(RSys_State.NumVAOs,
	                  &RSys_State.TempVAOs[RSys_State.NumVAOs]);

	RSys_State.VAOIsTaken = Reallocate(RSys_State.VAOIsTaken, sizeof(bool8) * N);
	memset((void *) (RSys_State.VAOIsTaken + RSys_State.NumVAOs), 0, sizeof(bool8) * (N / 2));
}

void RSys_AllocMoreVBOs() {
	u32 N = RSys_State.NumVBOs * 2;

	RSys_State.TempVBOs = Reallocate(RSys_State.TempVBOs, sizeof(GLuint) * N);
	glGenBuffers(RSys_State.NumVBOs,
			     &RSys_State.TempVBOs[RSys_State.NumVBOs]);

	RSys_State.VBOIsTaken = Reallocate(RSys_State.VBOIsTaken, sizeof(bool8) * N);
	memset((void *) (RSys_State.VBOIsTaken + RSys_State.NumVBOs), 0, sizeof(bool8) * (N / 2));
}

GLuint RSys_GetTempVAO() {
	if(RSys_State.NumTakenVAOs == RSys_State.NumVAOs) { RSys_AllocMoreVAOs(); }

	for(int i = 0; i < RSys_State.NumVAOs; i++) {
		if(!RSys_State.VAOIsTaken[i]) {
			RSys_State.VAOIsTaken[i] = 1;
			RSys_State.NumTakenVAOs++;
			return RSys_State.TempVAOs[i];
		}
	}

	// Uh oh...
	Log(Log_Error, "Could not find temp VAO, even though there should be.", "");
	return 0;
}

GLuint RSys_GetTempVBO() {
	if(RSys_State.NumTakenVBOs == RSys_State.NumVBOs) { RSys_AllocMoreVBOs(); }

	for(int i = 0; i < RSys_State.NumVBOs; i++) {
		if(!RSys_State.VBOIsTaken[i]) {
			RSys_State.VBOIsTaken[i] = 1;
			RSys_State.NumTakenVBOs++;
			return RSys_State.TempVBOs[i];
		}
	}

	// Uh oh...
	Log(Log_Error, "Could not find temp Vertex Buffer, even though there should be.", "");
	return 0;
}

void RSys_GetTempVBOs(GLuint *Output, u32 N) {
	if(!Output || !N)
		return;

	while(RSys_State.NumTakenVBOs + N >= RSys_State.NumVBOs) { RSys_AllocMoreVBOs(); }

	for(int i = 0; i < RSys_State.NumVBOs && N; i++) {
		if(!RSys_State.VBOIsTaken[i]) {
			RSys_State.VBOIsTaken[i] = 1;
			RSys_State.NumTakenVBOs++;
			Output[N-1] = RSys_State.TempVBOs[i];
			N--;
		}
	}
}

void RSys_FreeTempVBO(GLuint VBO) {
	for(int i = 0; i < RSys_State.NumVBOs; i++) {
		if(VBO == RSys_State.TempVBOs[i]) {
			RSys_State.VBOIsTaken[i] = 0;
			RSys_State.NumTakenVBOs--;
			return;
		}
	}

	Log(Log_Warning, "Attempted to free temp VBO %d, which doesn't exist.", VBO);
}

void RSys_FreeTempVBOs(const GLuint* VBOs, u32 N) {
	while(N--)
		RSys_FreeTempVBO(VBOs[N]);
}

void RSys_FreeTempVAO(GLuint VAO) {
	for(int i = 0; i < RSys_State.NumVAOs; i++) {
		if(VAO == RSys_State.TempVAOs[i]) {
			RSys_State.VAOIsTaken[i] = 0;
			RSys_State.NumTakenVAOs--;
			return;
		}
	}

	Log(Log_Warning, "Attempted to free temp VAO %d, which doesn't exist.", VAO);
}

void RSys_FinishFrame() {
	// Put what's rendered into the backbuffer onto the screen.
	SDL_GL_SwapWindow(RSys_State.Window);

	// Clear the screen for the next draw.
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// Record some info about the time it took to render.
	RSys_State.LastFrameDT = SDL_GetTicks() - RSys_State.LastFrameTime;
	RSys_State.LastFrameTime = SDL_GetTicks();
}

void RSys_Quit() {
	SDL_GL_DeleteContext(RSys_State.GLContext);
	SDL_DestroyWindow(RSys_State.Window);
	SDL_Quit();
}

struct RSys_Texture RSys_TextureFromFile(const char *filename) {
	struct RSys_Texture Texture = {.Id = 0};

	stbi_set_flip_vertically_on_load(0);
	u8 *Data = stbi_load(filename, &Texture.Width, &Texture.Height,
	                     &Texture.NumComponents, 0);
	if(!Data) return Texture;

	glGenTextures(1, &Texture.Id);
	glBindTexture(GL_TEXTURE_2D, Texture.Id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	GLenum Format;
	switch(Texture.NumComponents) {
		case 1: Format = GL_RED; break;
		case 2: Format = GL_RG; break;
		case 3: Format = GL_RGB; break;
		case 4: Format = GL_RGBA; break;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Texture.Width, Texture.Height, 0,
	             Format, GL_UNSIGNED_BYTE, Data);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(Data);

	return Texture;
}

// ---=== 2D rendering ===---

static struct R2D_State {
	GLuint TextVAO;
	GLuint TextTexture;
} R2D_State;

struct R2D_Spritesheet R2D_DefaultFont_Small,
					   R2D_DefaultFont_Medium,
					   R2D_DefaultFont_Large;
struct Shader *RectShader = NULL;
struct Shader *TextShader = NULL;

void R2D_Init() {
	glGenVertexArrays(1, &R2D_State.TextVAO);

	RectShader = Shader_FromFile("res/shaders/ui/rect.glsl");
	TextShader = Shader_FromFile("res/shaders/ui/text.glsl");

	struct RSys_Texture 
		small  = RSys_TextureFromFile("res/textures/font_mono_6x12.png"),
		medium = RSys_TextureFromFile("res/textures/font_mono_7x15.png"),
		large  = RSys_TextureFromFile("res/textures/font_mono_15x29.png");

	R2D_DefaultFont_Small = (struct R2D_Spritesheet){
	    .TextureId = small.Id,
	    .Width = small.Width,
	    .Height = small.Height,
	    .SpriteWidth = 6,
	    .SpriteHeight = 12,
	};

	R2D_DefaultFont_Medium = (struct R2D_Spritesheet){
	    .TextureId = medium.Id,
	    .Width = medium.Width,
	    .Height = medium.Height,
	    .SpriteWidth = 7,
	    .SpriteHeight = 15,
	};

	R2D_DefaultFont_Large = (struct R2D_Spritesheet){
	    .TextureId = large.Id,
	    .Width = large.Width,
	    .Height = large.Height,
	    .SpriteWidth = 15,
	    .SpriteHeight = 29,
	};
}

void R2D_DrawRects(const struct R2D_Rect *Rects, u32 NumRects, bool8 Fill) {
	Vec2 *Pos = Allocate(sizeof(Vec2) * NumRects * 4);
	RGBA *Color = Allocate(sizeof(RGBA) * NumRects * 4);
	u32 *Inds = Allocate(sizeof(u32) * NumRects * (Fill ? 6 : 8));

	// The vertices are assumed to be in the order
	// top left,
	// top right,
	// bottom left,
	// bottom right.

	for(u32 i = 0; i < NumRects; i++) {
		struct R2D_Rect r = Rects[i];

		Pos[i * 4 + 0] = r.Position;
		Pos[i * 4 + 1] = V2(r.Position.x + r.Size.x, r.Position.y);
		Pos[i * 4 + 2] = V2(r.Position.x, r.Position.y + r.Size.y);
		Pos[i * 4 + 3] = Vec2_Add(r.Position, r.Size);

		Color[i * 4 + 0] = r.Color;
		Color[i * 4 + 1] = r.Color;
		Color[i * 4 + 2] = r.Color;
		Color[i * 4 + 3] = r.Color;

		if(Fill) {
			Inds[i * 6 + 0] = 0 + i * 4;
			Inds[i * 6 + 1] = 2 + i * 4;
			Inds[i * 6 + 2] = 1 + i * 4;
			Inds[i * 6 + 3] = 1 + i * 4;
			Inds[i * 6 + 4] = 2 + i * 4;
			Inds[i * 6 + 5] = 3 + i * 4;
		} else {
			Inds[i * 8 + 0] = 0 + i * 4;
			Inds[i * 8 + 1] = 1 + i * 4;
			Inds[i * 8 + 2] = 1 + i * 4;
			Inds[i * 8 + 3] = 3 + i * 4;
			Inds[i * 8 + 4] = 3 + i * 4;
			Inds[i * 8 + 5] = 2 + i * 4;
			Inds[i * 8 + 6] = 2 + i * 4;
			Inds[i * 8 + 7] = 0 + i * 4;
		}
	}

	GLuint VAO = RSys_GetTempVAO();
	glBindVertexArray(VAO);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	GLuint VBOs[3];
	RSys_GetTempVBOs(VBOs, 3);

	// Add positions
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vec2) * NumRects * 4, Pos,
	             GL_STREAM_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	// Add colors
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vec4) * NumRects * 4, Color,
	             GL_STREAM_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBOs[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
	             sizeof(u32) * NumRects * (Fill ? 6 : 8), Inds, GL_STREAM_DRAW);

	// Drawing the rectangles
	Mat4 view, proj;
	RSys_Size size = RSys_GetSize();
	Camera cam = {.Mode = CameraMode_Orthographic,
	              .ScreenWidth = size.Width,
	              .ScreenHeight = size.Height,
	              .Position = V3(0, 0, -1),
	              .Target = V3(0, 0, 0),
	              .Up = V3(0, 1, 0),
	              .ZNear = 0.01,
	              .ZFar = 1000};

	Camera_Mat4(cam, view, proj);
	Shader_Use(RectShader);
	Shader_Uniform1i(RectShader, "type", 0);
	Shader_UniformMat4(RectShader, "proj", proj);
	Shader_UniformMat4(RectShader, "view", view);

	glDrawElements((Fill ? GL_TRIANGLES : GL_LINES), NumRects * (Fill ? 6 : 8),
	               GL_UNSIGNED_INT, 0);

	// Cleanup
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	RSys_FreeTempVBOs(VBOs, 3);
	Free(Pos);
	Free(Color);
	Free(Inds);
	RSys_FreeTempVAO(VAO);
}

void R2D_DrawTriangles(const struct R2D_Triangle *tris, u32 numTris)
{
	Vec2 *Pos = Allocate(sizeof(Vec2) * numTris * 3);
	RGBA *Color = Allocate(sizeof(RGBA) * numTris * 3);

	// The vertices are assumed to be in the order
	// top left,
	// top right,
	// bottom left,
	// bottom right.

	for(u32 i = 0; i < numTris; i++) {
		struct R2D_Triangle t = tris[i];

		Pos[i * 4 + 0] = t.Points[0];
		Pos[i * 4 + 1] = t.Points[1];
		Pos[i * 4 + 2] = t.Points[2];

		Color[i * 4 + 0] = t.Color;
		Color[i * 4 + 1] = t.Color;
		Color[i * 4 + 2] = t.Color;
	}

	GLuint VAO = RSys_GetTempVAO();
	glBindVertexArray(VAO);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	GLuint VBOs[2];
	RSys_GetTempVBOs(VBOs, 2);
	
	// Add positions
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vec2) * numTris * 3, Pos, GL_STREAM_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	// Add colors
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vec4) * numTris * 3, Color, GL_STREAM_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, NULL);

	// Drawing the rectangles
	Mat4 view, proj;
	RSys_Size size = RSys_GetSize();
	Camera cam = {.Mode = CameraMode_Orthographic,
	              .ScreenWidth = size.Width,
	              .ScreenHeight = size.Height,
	              .Position = V3(0, 0, -1),
	              .Target = V3(0, 0, 0),
	              .Up = V3(0, 1, 0),
	              .ZNear = 0.01,
	              .ZFar = 1000};

	Camera_Mat4(cam, view, proj);
	Shader_Use(RectShader);
	Shader_Uniform1i(RectShader, "type", 0);
	Shader_UniformMat4(RectShader, "proj", proj);
	Shader_UniformMat4(RectShader, "view", view);

	glDrawArrays(GL_TRIANGLES, 0, numTris * 3);

	// Cleanup
	RSys_FreeTempVBOs(VBOs, 2);
	Free(Pos);
	Free(Color);
	RSys_FreeTempVAO(VAO);
}

Vec2 R2D_GetTextExtents(const struct R2D_Spritesheet *font, const char *fmt,
                        ...) {
	char msg[512] = {0};

	va_list args;
	va_start(args, fmt);
	vsprintf(msg, fmt, args);
	va_end(args);

	u32 maxWidth = 0, width = 0, height = 1;

	for(u32 i = 0; i < strlen(msg); ++i) {
		if(msg[i] == '\n') {
			maxWidth = MAX(maxWidth, width);
			height++;
			width = 0;
			continue;
		} else if(msg[i] == '\t') {
			width += 3;
		}

		width++;
	}

	maxWidth = MAX(maxWidth, width);

	return V2(font->SpriteWidth * maxWidth, font->SpriteHeight * height);
}

void R2D_DrawRectImage(Vec2 Position, Vec2 Size, GLuint TextureID,
                       const Vec2 *TextureUVs) {
	GLuint VAO = RSys_GetTempVAO();
	glBindVertexArray(VAO);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	GLuint VBO = RSys_GetTempVBO();
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vec2), NULL);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vec2),
	                      (void *) (sizeof(Vec2) * 4));

	const Vec2 UVs[4] = { 
		V2C(0,0), 
		V2C(0,1), 
		V2C(1,0),
		V2C(1,1), 
	};

	if(!TextureUVs) 
		TextureUVs = UVs;

	Vec2 PosAndUV[8] = {
	    V2(Position.x, Position.y),                    // Top left
	    V2(Position.x, Position.y + Size.y),           // Bottom left
	    V2(Position.x + Size.x, Position.y),           // Top right
	    V2(Position.x + Size.x, Position.y + Size.y),  // Bottom right

	    TextureUVs[0],
	    TextureUVs[1],
	    TextureUVs[2],
	    TextureUVs[3],
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vec2) * 8, PosAndUV, GL_DYNAMIC_DRAW);

	RSys_Size size = RSys_GetSize();

	Mat4 view, proj;
	Camera cam = {
	    .Mode = CameraMode_Orthographic,
	    .ScreenWidth = size.Width,
	    .ScreenHeight = size.Height,
	    .Position = V3(0, 0, -1),
	    .Target = V3(0, 0, 0),
	    .Up = V3(0, 1, 0),
	    .ZNear = 0.01,
	    .ZFar = 1000,
	};

	Camera_Mat4(cam, view, proj);

	Shader_Use(RectShader);
	Shader_UniformMat4(RectShader, "proj", proj);
	Shader_UniformMat4(RectShader, "view", view);
	Shader_Uniform1i(RectShader, "type", 1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TextureID);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	RSys_FreeTempVBO(VBO);
	RSys_FreeTempVAO(VAO);
}

void R2D_DrawText(Vec2 pos, RGBA fg, RGBA bg,
                  const struct R2D_Spritesheet *font, const char *fmt, ...) {
	static const r64 minAlpha = 0.001;

	char msg[512] = {0};

	va_list args;
	va_start(args, fmt);
	vsprintf(msg, fmt, args);
	va_end(args);

	u32 numChars = 0;
	for(u32 i = 0; i < strlen(msg); i++) {
		numChars +=
		    ((bg.a <= minAlpha) ? isgraph(msg[i]) : isprint(msg[i])) ? 1 : 0;
	}

	if(!numChars) return;

	Vec2 *Pos = Allocate(sizeof(Vec2) * numChars * 4);
	Vec2 *UV = Allocate(sizeof(Vec2) * numChars * 4);
	u32 *Inds = Allocate(sizeof(u32) * numChars * 6);

	// The vertices are assumed to be in the order
	// top left,
	// top right,
	// bottom left,
	// bottom right.

	Vec2 pen = pos;

	// i tracks the current character in the string
	// j tracks only the visible characters
	for(u32 i = 0, j = 0; i < strlen(msg) && j < numChars; i++) {
		switch(msg[i]) {
			case ' ':
				if(bg.a <= minAlpha) {
					pen.x += font->SpriteWidth;
					continue;
				}
				break;
			case '\t': pen.x += font->SpriteWidth * 4; continue;
			case '\n':
				pen.x = pos.x;
				pen.y += font->SpriteHeight;
				continue;
		}

		Pos[j * 4 + 0] = pen;
		Pos[j * 4 + 1] = V2(pen.x + font->SpriteWidth, pen.y);
		Pos[j * 4 + 2] = V2(pen.x, pen.y + font->SpriteHeight);
		Pos[j * 4 + 3] =
		    V2(pen.x + font->SpriteWidth, pen.y + font->SpriteHeight);

		if(msg[i] == ' ') {
			UV[j * 4 + 0] = V2(0, 0);
			UV[j * 4 + 1] = V2(0, 0);
			UV[j * 4 + 2] = V2(0, 0);
			UV[j * 4 + 3] = V2(0, 0);
		} else {
			const Vec2 uvSize = V2((r32) font->SpriteWidth / font->Width,
			                       (r32) font->SpriteHeight / font->Height);

			u32 charsPerRow = font->Width / font->SpriteWidth;
			u32 x = (msg[i] - '!') % charsPerRow;
			u32 y = (msg[i] - '!') / charsPerRow;

			const Vec2 uvTopLeft = V2(uvSize.x * x, uvSize.y * y);

			UV[j * 4 + 0] = uvTopLeft;
			UV[j * 4 + 1] = V2(uvTopLeft.x + uvSize.x, uvTopLeft.y);
			UV[j * 4 + 2] = V2(uvTopLeft.x, uvTopLeft.y + uvSize.y);
			UV[j * 4 + 3] = Vec2_Add(uvTopLeft, uvSize);
		}

		Inds[j * 6 + 0] = 0 + j * 4;
		Inds[j * 6 + 1] = 2 + j * 4;
		Inds[j * 6 + 2] = 1 + j * 4;
		Inds[j * 6 + 3] = 1 + j * 4;
		Inds[j * 6 + 4] = 2 + j * 4;
		Inds[j * 6 + 5] = 3 + j * 4;

		j++;

		pen.x += font->SpriteWidth;
	}

	GLuint VAO = RSys_GetTempVAO();
	glBindVertexArray(VAO);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	GLuint VBOs[3];
	RSys_GetTempVBOs(VBOs, 3);
	// Add positions
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vec2) * numChars * 4, Pos,
	             GL_STREAM_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	// Add colors
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vec2) * numChars * 4, UV,
	             GL_STREAM_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBOs[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * numChars * 6, Inds,
	             GL_STREAM_DRAW);

	// Drawing the rectangles
	Mat4 view, proj;
	RSys_Size size = RSys_GetSize();
	Camera cam = {.Mode = CameraMode_Orthographic,
	              .ScreenWidth = size.Width,
	              .ScreenHeight = size.Height,
	              .Position = V3(0, 0, -1),
	              .Target = V3(0, 0, 0),
	              .Up = V3(0, 1, 0),
	              .ZNear = 0.01,
	              .ZFar = 1000};

	Camera_Mat4(cam, view, proj);
	Shader_Use(TextShader);
	Shader_UniformMat4(TextShader, "proj", proj);
	Shader_UniformMat4(TextShader, "view", view);

	Shader_Uniform4f(TextShader, "fg", fg);
	Shader_Uniform4f(TextShader, "bg", bg);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, font->TextureId);

	glDrawElements(GL_TRIANGLES, numChars * 6, GL_UNSIGNED_INT, 0);

	// Cleanup
	Free(Pos);
	Free(UV);
	Free(Inds);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	RSys_FreeTempVBOs(VBOs, 3);
	RSys_FreeTempVAO(VAO);
}

// ---=== 3D rendering ===---
struct Shader *R3D_Shader_UnlitColor,
			  *R3D_Shader_UnlitTextured;

void R3D_Init()
{
	R3D_Shader_UnlitColor    = Shader_FromFile("res/shaders/3d/unlit-col.glsl");
	R3D_Shader_UnlitTextured = Shader_FromFile("res/shaders/3d/unlit-tex.glsl");
}

void R3D_DrawLine(Camera cam, Vec3 start, Vec3 end, RGBA color)
{
	Vec3 arr[2] = {start, end};
	R3D_DrawLines(cam, arr, 1, color);
}
void R3D_DrawLines(Camera cam, Vec3 *linePoints, u32 numLines, RGBA color)
{
	GLuint vao = RSys_GetTempVAO();
	GLuint vbo = RSys_GetTempVBO();

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vec3) * numLines * 2, linePoints, GL_STREAM_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	Mat4 model, view, MVP;
	Transform3D_Mat4(Transform3D_Default, model);
	Camera_Mat4(cam, view, MVP);
	Mat4_MultMat(MVP, view);
	Mat4_MultMat(MVP, model);

	Shader_Use(R3D_Shader_UnlitColor);
	Shader_UniformMat4(R3D_Shader_UnlitColor, "MVP", MVP);
	Shader_Uniform4f(R3D_Shader_UnlitColor, "color", color);

	glDrawArrays(GL_LINES, 0, numLines*2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	RSys_FreeTempVBO(vbo);
	RSys_FreeTempVAO(vao);
}

void R3D_DrawTriangle(Camera cam, Vec3 a, Vec3 b, Vec3 c, RGBA color)
{
	// Two-sided triangle vertices.
	Vec3 pos[6] = { a, b, c,
	                b, a, c };

	GLuint vao = RSys_GetTempVAO();
	GLuint vbo = RSys_GetTempVBO();

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vec3) * 6, pos, GL_STREAM_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	Mat4 model, view, MVP;
	Transform3D_Mat4(Transform3D_Default, model);
	Camera_Mat4(cam, view, MVP);
	Mat4_MultMat(MVP, view);
	Mat4_MultMat(MVP, model);

	Shader_Use(R3D_Shader_UnlitColor);
	Shader_UniformMat4(R3D_Shader_UnlitColor, "MVP", MVP);
	Shader_Uniform4f(R3D_Shader_UnlitColor, "color", color);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	RSys_FreeTempVBO(vbo);
	RSys_FreeTempVAO(vao);
}

void R3D_DrawWireSphere(Camera cam, Vec3 center, r32 radius, RGBA color)
{
	const u32 numPoints = 30;
	Vec3 *points = Allocate(sizeof(Vec3) * (numPoints+1) * 2 * 2);

	// Generate a single ring.
	for(u32 i = 0; i < numPoints+1; i++)
	{
		r32 f1 = Tau * ((r32)  i    / numPoints);
		r32 f2 = Tau * ((r32) (i+1) / numPoints);
		points[(i*2)]   = Vec3_Add(center, Vec3_MultScal(V3(sin(f1), 0, cos(f1)), radius));
		points[(i*2)+1] = Vec3_Add(center, Vec3_MultScal(V3(sin(f2), 0, cos(f2)), radius));

		points[(i*2)+(numPoints+1)*2]   = Vec3_Add(center, Vec3_MultScal(V3(sin(f1), cos(f1), 0), radius));
		points[(i*2)+(numPoints+1)*2+1] = Vec3_Add(center, Vec3_MultScal(V3(sin(f2), cos(f2), 0), radius));
	}

	R3D_DrawLines(cam, points, (numPoints+1)*2, color);

	Free(points);
}
