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

bool8 RectInitialized = 0;
Shader *RectShader = NULL;
GLuint RectVAO = 0;
GLuint RectVBO = 0;

void R2D_DrawRect(Vec2 Position, Vec2 Size, Vec4 Color, bool8 OutlineOnly) {
	if(!RectInitialized) {
		Log_Debug("Initializing Rect...");
		glGenVertexArrays(1, &RectVAO);
		glBindVertexArray(RectVAO);

		glGenBuffers(1, &RectVBO);
		glBindBuffer(GL_ARRAY_BUFFER, RectVBO);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);

		RectShader = Shader_FromFile("res/shaders/ui_rect.glsl");

		RectInitialized = 1;
	}

	glBindVertexArray(RectVAO);
	glBindBuffer(GL_ARRAY_BUFFER, RectVBO);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	Vec2 pos[4];
	pos[0] = V2(Position.x, Position.y);           // Top left
	pos[1] = V2(Position.x, Position.y + Size.y);  // Bottom left
	if(OutlineOnly) {
		pos[2] = V2(Position.x + Size.x, Position.y + Size.y);  // Bottom right
		pos[3] = V2(Position.x + Size.x, Position.y);           // Top right
	} else {
		pos[2] = V2(Position.x + Size.x, Position.y);           // Top right
		pos[3] = V2(Position.x + Size.x, Position.y + Size.y);  // Bottom right
	}
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vec2) * 4, pos, GL_DYNAMIC_DRAW);

	RSys_Size size = RSys_GetSize();

	Mat4 view, proj;
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
	Shader_Uniform4f(RectShader, "color", Color);

	glDrawArrays((OutlineOnly ? GL_LINE_LOOP : GL_TRIANGLE_STRIP), 0, 4);
}

void R2D_DrawRectImage(Vec2 Position, Vec2 Size, GLuint TextureID,
                       Vec2 TextureUVs[4]) {
	if(!RectInitialized) {
		glGenVertexArrays(1, &RectVAO);
		glBindVertexArray(RectVAO);

		glGenBuffers(1, &RectVBO);
		glBindBuffer(GL_ARRAY_BUFFER, RectVBO);

		RectShader = Shader_FromFile("res/shaders/ui_rect.glsl");

		RectInitialized = 1;
	}

	glBindVertexArray(RectVAO);
	glBindBuffer(GL_ARRAY_BUFFER, RectVBO);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vec2), NULL);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vec2),
	                      (void *) (sizeof(Vec2) * 4));

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
}

Array *DrawCmdQueue = NULL;

typedef struct DrawCmd {
	enum {
		DrawCmd_DrawRect,
		DrawCmd_DrawRectOutline,
		DrawCmd_DrawImage,
	} Type;

	union {
		struct {
			Vec2 RectPos;
			Vec2 RectSize;

			union {
				Vec4 RectColor;
				struct {
					GLuint RectTexture;
					Vec2 RectUVOffset;
					Vec2 RectUVSize;
				};
			};
		};
	};
} DrawCmd;

void R2D_PushRect(Vec2 Position, Vec2 Size, RGBA Color, bool8 OutlineOnly) {
	if(!DrawCmdQueue) DrawCmdQueue = Array_Init(sizeof(DrawCmd));

	DrawCmd cmd = {
	    .Type = (OutlineOnly ? DrawCmd_DrawRectOutline : DrawCmd_DrawRect),
	    .RectPos = Position,
	    .RectSize = Size,
	    .RectColor = Color,
	};
	Array_Push(DrawCmdQueue, (u8 *) &cmd);
}

void R2D_FinishRender() {
	if(!RectShader) RectShader = Shader_FromFile("res/shaders/ui_rect.glsl");

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

	for(u32 i = 0; i < DrawCmdQueue->ArraySize; i++) {
		DrawCmd *cmd = (DrawCmd *) Array_Get(DrawCmdQueue, i);
		switch(cmd->Type) {
			case DrawCmd_DrawRect: break;
		}
	}


	Array_Clear(DrawCmdQueue);
}
