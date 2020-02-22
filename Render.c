#include "Render.h"

#include <ft2build.h>
#include <stdlib.h>
#include FT_FREETYPE_H

struct RSys_State_t RSys_State;

RSys_Size RSys_GetSize() {
	RSys_Size sz = {-1, -1};
	SDL_GetWindowSize(RSys_State.Window, &sz.Width, &sz.Height);
	sz.AspectRatio = (r32) sz.Width / sz.Height;
	return sz;
}

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
	    SDL_CreateWindow("GL Spiral", 0, 0, Width, Height,
	                     SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

	// If it wasn't created, error and exit.
	if(!RSys_State.Window) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
		                         "SDL_CreateWindow() failed", SDL_GetError(),
		                         NULL);
		exit(EXIT_FAILURE);
	}

	// Create an OpenGL context for the window,
	// according to the earlier parameters.
	RSys_State.GLContext = SDL_GL_CreateContext(RSys_State.Window);

	// If our parameters couldn't be met, error and exit.
	if(!RSys_State.GLContext) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
		                         "SDL_GL_CreateContext() failed",
		                         SDL_GetError(), NULL);
		exit(EXIT_FAILURE);
	}

	// Load OpenGL functions.
	if(!gladLoadGL()) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "gladLoadGL() failed",
		                         "OpenGL couldn't be loaded.", NULL);
		exit(EXIT_FAILURE);
	}

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

	// Enable multisampling.
	// glEnable(GL_MULTISAMPLE);

	// Set a fake last render time.
	RSys_State.LastFrameTime = SDL_GetTicks();
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

// ---=== 2D rendering ===---

static struct R2D_State {
	Array *Fonts;
	i32 ActiveFont;
	FT_Library Library;

	GLuint TextVAO;
	GLuint TextTexture;
} R2D_State;

static const char *Font_GetFTErrorString(FT_Error e);

void R2D_Init() {
	R2D_State.Fonts = Array_Init(sizeof(FT_Face));
	R2D_State.ActiveFont = -1;

	R2D_State.Library = NULL;
	FT_Error error = FT_Init_FreeType(&R2D_State.Library);
	if(error)
		Log_Error("Freetype 2 failed to load (reason: %s)",
		          Font_GetFTErrorString(error));

	glGenVertexArrays(1, &R2D_State.TextVAO);
	glBindVertexArray(R2D_State.TextVAO);

	GLuint TextVBOs[2];
	glGenBuffers(2, TextVBOs);
	glBindBuffer(GL_ARRAY_BUFFER, TextVBOs[0]);
}
void R2D_UseFont(i32 fontIndex) { R2D_State.ActiveFont = fontIndex; }

i32 R2D_LoadFont_TTF(const char *fontFile) {
	if(!R2D_State.Library) {
		Log_Warning("Can't load TTF font %s because Freetype failed to load.",
		            fontFile);
		return -1;
	}

	FT_Error error;
	FT_Face face = NULL;

	error = FT_New_Face(R2D_State.Library, fontFile, 0, &face);
	if(error) {
		Log_Error("Font \"%s\" failed to load (reason: %s)", fontFile,
		          Font_GetFTErrorString(error));
		return -1;
	}

	error = FT_Set_Char_Size(face,    /* handle to face object           */
	                         0,       /* char_width in 1/64th of points  */
	                         16 * 64, /* char_height in 1/64th of points */
	                         300,     /* horizontal device resolution    */
	                         300);    /* vertical device resolution      */
	if(error) {
		Log_Error("Failed to set font size for font \"%s\" (reason: %s)",
		          fontFile, Font_GetFTErrorString(error));
		return -1;
	}
	FT_Select_Charmap(face, FT_ENCODING_UNICODE);

	Array_Push(R2D_State.Fonts, (u8 *) &face);
	R2D_UseFont(R2D_State.Fonts->ArraySize - 1);
	return R2D_State.Fonts->ArraySize - 1;
}

void R2D_DrawText(Vec2 pos, Vec4 color, const char *fmt, ...) {
	char txt[512] = {0};

	va_list args;
	va_start(args, fmt);
	vsprintf(txt, fmt, args);
	va_end(args);

	Vec2 pen = pos;

	FT_Face face = (FT_Face) Array_Get(R2D_State.Fonts, R2D_State.ActiveFont);
	FT_Bitmap *bmp = &face->glyph->bitmap;

	for(u32 i = 0; i < strlen(txt); i++) {
		FT_Load_Char(face, txt[i], FT_LOAD_RENDER);
	}
}

static const char *Font_GetFTErrorString(FT_Error e) {
#undef FTERRORS_H_
#define FT_ERROR_START_LIST switch(e) {
#define FT_ERRORDEF(e, v, s) \
	case v: {                \
		return s;            \
	}
#define FT_ERROR_END_LIST }

#include FT_ERRORS_H
	return "";
}

