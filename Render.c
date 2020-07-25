#include "Render.h"

#include <stdlib.h>
#include <string.h>

#include "Common.h"
#include "stb_image.h"

struct RSys_State {
	SDL_Window *Window;
	SDL_GLContext GLContext;
	u64 LastFrameTime;
	u64 LastFrameDT;

	GLuint *TempVAOs;
	bool8 *VAOIsTaken;
	u32 NumVAOs, NumTakenVAOs;
} RSys_State;

u64 RSys_GetLastFrameTime() { return RSys_State.LastFrameTime; }

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
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);

	// Enable 4x multisampling.
	// SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	// SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

	// Create the actual window.
	RSys_State.Window =
	    SDL_CreateWindow("Boyan's Game", 0, 0, Width, Height,
	                     SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

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
	glClearColor(1, 1, 1, 1);

	// Enable depth testing and set less than or equal mode
	// for more stable rendering.
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	// Enable face culling.
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// Enable multisampling.
	// glEnable(GL_MULTISAMPLE);

	// Set a fake last render time.
	RSys_State.LastFrameTime = SDL_GetTicks();

	static const u32 NUM_VAOS = 4;

	RSys_State.TempVAOs = malloc(sizeof(GLuint) * NUM_VAOS);
	RSys_State.VAOIsTaken = malloc(sizeof(bool8) * NUM_VAOS);
	RSys_State.NumTakenVAOs = 0;
	RSys_State.NumVAOs = NUM_VAOS;

	memset((void *) RSys_State.VAOIsTaken, 0, sizeof(bool8) * NUM_VAOS);

	glGenVertexArrays(RSys_State.NumVAOs, RSys_State.TempVAOs);

	GL_Initialized = 1;

	R2D_Init();
}

void RSys_AllocMoreVAOs() {
	u32 N = RSys_State.NumVAOs * 2;

	RSys_State.TempVAOs = realloc(RSys_State.TempVAOs, sizeof(GLuint) * N);
	glGenVertexArrays(RSys_State.NumVAOs,
	                  &RSys_State.TempVAOs[RSys_State.NumVAOs]);

	RSys_State.VAOIsTaken = realloc(RSys_State.VAOIsTaken, sizeof(bool8) * N);
	memset((void *) (RSys_State.VAOIsTaken + RSys_State.NumVAOs), 0,
	       sizeof(bool8) * (N / 2));
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

void RSys_FreeTempVAO(GLuint VAO) {
	for(int i = 0; i < RSys_State.NumVAOs; i++) {
		if(VAO == RSys_State.TempVAOs[i]) {
			RSys_State.VAOIsTaken[i] = 0;
			RSys_State.NumTakenVAOs--;
			return;
		}
	}

	Log(Log_Warning, "Attempted to free temp VAO %d, which doesn't exist.",
	    VAO);
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
	glBindVertexArray(R2D_State.TextVAO);

	GLuint TextVBOs[2];
	glGenBuffers(2, TextVBOs);
	glBindBuffer(GL_ARRAY_BUFFER, TextVBOs[0]);

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
	Vec2 *Pos = malloc(sizeof(Vec2) * NumRects * 4);
	RGBA *Color = malloc(sizeof(RGBA) * NumRects * 4);
	u32 *Inds = malloc(sizeof(u32) * NumRects * (Fill ? 6 : 8));

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

	GLuint VBOs[3] = {0};
	glGenBuffers(3, VBOs);
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
	glDeleteBuffers(3, VBOs);
	free(Pos);
	free(Color);
	free(Inds);
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

	GLuint VBO = 0;
	glGenBuffers(1, &VBO);
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

	glDeleteBuffers(1, &VBO);
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

	Vec2 *Pos = malloc(sizeof(Vec2) * numChars * 4);
	Vec2 *UV = malloc(sizeof(Vec2) * numChars * 4);
	u32 *Inds = malloc(sizeof(u32) * numChars * 6);

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

	GLuint VBOs[3] = {0};
	glGenBuffers(3, VBOs);
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
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDeleteBuffers(3, VBOs);
	free(Pos);
	free(UV);
	free(Inds);
	RSys_FreeTempVAO(VAO);
}
