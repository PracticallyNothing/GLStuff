#include "Render.h"

#include <stdlib.h>
#include <string.h>

#include "Math3D.h"
#include "SDL_video.h"
#include "stb_image.h"

// Convert an error to a string.
static const char* 
GL_ErrorToString(GLenum err) 
{
	switch(err) {
		case GL_NO_ERROR:                      return "GL OK";
		case GL_INVALID_ENUM:                  return "GL_INVALID_ENUM";
		case GL_INVALID_VALUE:                 return "GL_INVALID_VALUE";
		case GL_INVALID_OPERATION:             return "GL_INVALID_OPERATION";
		case GL_OUT_OF_MEMORY:                 return "GL_OUT_OF_MEMORY";
		case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";

		case GL_FRAMEBUFFER_COMPLETE:                      return "Framebuffer OK";
		case GL_FRAMEBUFFER_UNDEFINED:                     return "GL_FRAMEBUFFER_UNDEFINED";
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:         return "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: return "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:        return "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:        return "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
		case GL_FRAMEBUFFER_UNSUPPORTED:                   return "GL_FRAMEBUFFER_UNSUPPORTED";
		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:        return "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
		case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:      return "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS";

		default: return "Unknown error";
	}
}

struct RSys_State {
	SDL_Window *Window;
	SDL_GLContext GLContext;

	bool8 DrawEnable;

	r32 FrametimeCap;
	u64 LastFrameTime;
	u64 LastFrameDT;

	GLuint *TempVAOs;
	bool8 *VAOIsTaken;
	u32 NumVAOs, NumTakenVAOs;

	GLuint *TempVBOs;
	bool8 *VBOIsTaken;
	u32 NumVBOs, NumTakenVBOs;
} RSys_State;

void RSys_SetFPSCap(u32 capFps) { RSys_State.FrametimeCap = 1000.0 / capFps; }
void RSys_SetFrametimeCap(r32 capMs) { RSys_State.FrametimeCap = capMs; }

void RSys_HandleWindowEvent(const SDL_WindowEvent *e)
{
	// TODO: This may disable drawing too liberally.
	//       E.g. the player has the window open, but wants to look at another,
	//       yet the game decides to stop drawing without pausing.
	switch(e->type)
	{
		case SDL_WINDOWEVENT_LEAVE:
		case SDL_WINDOWEVENT_FOCUS_LOST:
		case SDL_WINDOWEVENT_HIDDEN:
			RSys_State.DrawEnable = 0;
		case SDL_WINDOWEVENT_ENTER:
		case SDL_WINDOWEVENT_FOCUS_GAINED:
		case SDL_WINDOWEVENT_SHOWN:
			RSys_State.DrawEnable = 1;
	}
}

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
		Log(FATAL, "SDL_CreateWindow() failed: %s", SDL_GetError());

	// Create an OpenGL context for the window,
	// according to the earlier parameters.
	RSys_State.GLContext = SDL_GL_CreateContext(RSys_State.Window);

	// If our parameters couldn't be met, error and exit.
	if(!RSys_State.GLContext)
		Log(FATAL, "SDL_GL_CreateContext() failed: %s", SDL_GetError());

	// Load OpenGL functions.
	if(!gladLoadGL())
		Log(FATAL, "%s", "gladLoadGL() failed, OpenGL couldn't be loaded.");

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

	RSys_State.DrawEnable = 1;

	R2D_Init();
	R3D_Init();

	Log(INFO, "[Render] Started", "");
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

GLuint VAO_GetTemp() {
	if(RSys_State.NumTakenVAOs == RSys_State.NumVAOs) { RSys_AllocMoreVAOs(); }

	for(int i = 0; i < RSys_State.NumVAOs; i++) {
		if(!RSys_State.VAOIsTaken[i]) {
			RSys_State.VAOIsTaken[i] = 1;
			RSys_State.NumTakenVAOs++;
			return RSys_State.TempVAOs[i];
		}
	}

	// Uh oh...
	Log(ERROR, "Could not find temp VAO, even though there should be.", "");
	return 0;
}

GLuint VBO_GetTemp() {
	if(RSys_State.NumTakenVBOs == RSys_State.NumVBOs) { RSys_AllocMoreVBOs(); }

	for(int i = 0; i < RSys_State.NumVBOs; i++) {
		if(!RSys_State.VBOIsTaken[i]) {
			RSys_State.VBOIsTaken[i] = 1;
			RSys_State.NumTakenVBOs++;
			return RSys_State.TempVBOs[i];
		}
	}

	// Uh oh...
	Log(ERROR, "Could not find temp Vertex Buffer, even though there should be.", "");
	return 0;
}

void VBO_GetManyTemp(GLuint *Output, u32 N) {
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

void VBO_FreeTemp(GLuint VBO) {
	for(int i = 0; i < RSys_State.NumVBOs; i++) {
		if(VBO == RSys_State.TempVBOs[i]) {
			RSys_State.VBOIsTaken[i] = 0;
			RSys_State.NumTakenVBOs--;
			return;
		}
	}

	Log(WARN, "Attempted to free temp VBO %d, which doesn't exist.", VBO);
}

void VBO_FreeManyTemp(const GLuint* VBOs, u32 N) {
	while(N--)
		VBO_FreeTemp(VBOs[N]);
}

void VAO_FreeTemp(GLuint VAO) {
	for(int i = 0; i < RSys_State.NumVAOs; i++) {
		if(VAO == RSys_State.TempVAOs[i]) {
			RSys_State.VAOIsTaken[i] = 0;
			RSys_State.NumTakenVAOs--;
			return;
		}
	}

	Log(WARN, "Attempted to free temp VAO %d, which doesn't exist.", VAO);
}

bool8 RSys_NeedRedraw() {
	return (SDL_GetTicks() - RSys_State.LastFrameTime) > RSys_State.FrametimeCap;
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

Texture Texture_Init(
	u32 w, u32 h,
	enum Texture_Format fmt,
	enum Texture_Filter filt,
	enum Texture_Wrap wrap)
{
	glGetError();

	Texture t = {
		.Id = 0,
		.Width = w,
		.Height = h,
		.Format = fmt,
		.Filter = filt,
		.Wrap = wrap,
		.HasMipmaps = 0
	};
	glGenTextures(1, &t.Id);

	GLenum err = glGetError();
	if(!t.Id || err != GL_NO_ERROR)
	{
		Log(ERROR, "[Render] Texture init fail - %s (%d).", GL_ErrorToString(err), err);
		return t;
	}
	glBindTexture(GL_TEXTURE_2D, t.Id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filt);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filt);

	Texture_SetData(&t, NULL, w, h, 0);

	return t;
}

void Texture_SetData(Texture* t, const u8* data, u32 width, u32 height, bool8 mips)
{
	glGetError();

	glBindTexture(GL_TEXTURE_2D, t->Id);

	GLenum intFmt = t->Format == Format_Depth ? Format_Depth : Format_RGBA;
	GLenum type   = t->Format == Format_Depth ? GL_UNSIGNED_INT : GL_UNSIGNED_BYTE;

	glTexImage2D(
		GL_TEXTURE_2D, // target:         Target texture
		0,             // level:          LOD level, 0 because we don't have custom LOD
		intFmt,        // internalFormat: How to store the data
		width,         // width:          Width in pixels
		height,        // height:         Height in pixels
		0,             // border:         No idea, docs.gl says "This value must be 0."
		t->Format,     // format:         Pixel format
		type,          // type:           How each pixel is encoded
		data           // data:           Pointer to pixel data
	);

	GLenum err = glGetError();
	if(err != GL_NO_ERROR) {
		Log(ERROR, "[Render] Texture %d set data fail: %s (%d)", t->Id, GL_ErrorToString(err), err);
		glBindTexture(GL_TEXTURE_2D, 0);
		return;
	}

	t->Width = width;
	t->Height = height;
	t->HasMipmaps = data && width && height && mips;

	if(t->HasMipmaps)
		glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);
}

Texture Texture_FromFile(const char *filename) {
	stbi_set_flip_vertically_on_load(0);

	i32 w, h, nComp;
	u8 *Data = stbi_load(filename, &w, &h, &nComp, 0);

	if(!Data) {
		Log(ERROR, "[Render] Texture %s load fail - file doens't exist.", filename);
		return (Texture){0};
	}

	enum Texture_Format fmt;
	switch(nComp) {
		case 1: fmt = Format_Red;  break;
		case 2: fmt = Format_RG;   break;
		case 3: fmt = Format_RGB;  break;
		case 4: fmt = Format_RGBA; break;
	}

	Texture t = Texture_Init(w, h, fmt, Filter_Linear, Wrap_Repeat);
	Texture_SetData(&t, Data, w, h, 1);

	stbi_image_free(Data);

	return t;
}
void Texture_Free(Texture t) { glDeleteTextures(1, &t.Id); }

//
// RT
//

static RT RT_Current = {.Id = 0}; // Currently bound render target, 0 means screen

RT RT_Init(u32 w, u32 h)
{
	RT rt;
	rt.Width = w;
	rt.Height = h;

	glGenFramebuffers(1, &rt.Id);
	glBindFramebuffer(GL_FRAMEBUFFER, rt.Id);

	rt.Color = Texture_Init(w, h, Format_RGBA,  Filter_Nearest, Wrap_ClampToBorder);
	rt.Depth = Texture_Init(w, h, Format_Depth, Filter_Nearest, Wrap_ClampToBorder);

	Log(INFO, "[Render] RT %u with Color %u and Depth %u.", rt.Id, rt.Color.Id, rt.Depth.Id);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rt.Color.Id, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  GL_TEXTURE_2D, rt.Depth.Id, 0);

	GLenum res = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(res != GL_FRAMEBUFFER_COMPLETE) {
		Log(ERROR, "[Render] RT create fail: %s (%d)", GL_ErrorToString(res), res);
		return (RT){.Id = 0};
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return rt;
}

RT RT_InitScreenSize()
{
	Vec2 sz = RT_GetScreenSize();
	return RT_Init(sz.w, sz.h);
}

void RT_Free(RT rt)
{
	Texture_Free(rt.Color);
	Texture_Free(rt.Depth);
	glDeleteFramebuffers(1, &rt.Id);
}

void RT_UseDefault() { 
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	Vec2 sz = RT_GetScreenSize();
	glViewport(0, 0, sz.w, sz.h);
	RT_Current = (RT){ .Id = 0 };
}
void RT_Use(RT rt)   { 
	if(rt.Id == 0)
		RT_UseDefault();

	glBindFramebuffer(GL_FRAMEBUFFER, rt.Id);
	glViewport(0, 0, rt.Width, rt.Height);
	RT_Current = rt;
}

void RT_Blit(RT src, RT dest)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, src.Id);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dest.Id);
	glBlitFramebuffer(
		0, 0, src.Width, src.Height,               // Source region, x y width height
		0, 0, dest.Width, dest.Height,             // Destination region, x y width height
		GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, // What to copy
		GL_LINEAR                                  // How to account for different sizes.
	);
}

void RT_BlitToScreen(RT src)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, src.Id);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	
	Vec2 sz = RT_GetScreenSize();

	glBlitFramebuffer(
		0, 0, src.Width, src.Height,               // Source region, x y width height
		0, 0, sz.w, sz.h,                 // Destination region, x y width height
		GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, // What to copy
		GL_LINEAR                                  // How to account for different sizes.
	);
}

void RT_Clear(RT rt)
{
	glBindFramebuffer(GL_FRAMEBUFFER, rt.Id);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RT_SetSize(RT *rt, u32 w, u32 h)
{
	if(!rt)
		return;

	Texture_SetData(&rt->Color, NULL, w, h, 0);
	Texture_SetData(&rt->Depth, NULL, w, h, 0);
	rt->Width = w;
	rt->Height = h;
}

Vec2 RT_GetCurrentSize()
{
	if(RT_Current.Id == 0)
		return RT_GetScreenSize();
	return V2(RT_Current.Width, RT_Current.Height);
}

r32 RT_GetCurrentAspectRatio()
{
	if(RT_Current.Id == 0)
		return RT_GetScreenAspectRatio();
	return ((r32)RT_Current.Width)/RT_Current.Height;
}

Vec2 RT_GetScreenSize()
{
	i32 w, h;
	SDL_GetWindowSize(RSys_State.Window, &w, &h);
	return V2(w, h);
}

r32 RT_GetScreenAspectRatio()
{
	i32 w, h;
	SDL_GetWindowSize(RSys_State.Window, &w, &h);
	return ((r32)w)/h;
}
//
// -- 2D --
//

static struct R2D_State {
	GLuint TextVAO;
	GLuint TextTexture;
} R2D_State;

Spritesheet Font_Small, Font_Medium, Font_Large;
Shader *RectShader = NULL;
Shader *TextShader = NULL;

void R2D_Init() {
	glGenVertexArrays(1, &R2D_State.TextVAO);

	RectShader = Shader_FromFile("res/shaders/ui/rect.glsl");
	TextShader = Shader_FromFile("res/shaders/ui/text.glsl");

	Texture *tex = malloc(sizeof(Texture)*3);
	tex[0] = Texture_FromFile("res/textures/font_mono_6x12.png"),
	tex[1] = Texture_FromFile("res/textures/font_mono_7x15.png"),
	tex[2] = Texture_FromFile("res/textures/font_mono_15x29.png");

	Font_Small = (Spritesheet){
		.Texture = tex,
	    .SpriteWidth = 6,
	    .SpriteHeight = 12,
	};

	Font_Medium = (Spritesheet){
		.Texture = tex+1,
	    .SpriteWidth = 7,
	    .SpriteHeight = 15,
	};

	Font_Large = (Spritesheet){
		.Texture = tex+2,
	    .SpriteWidth = 15,
	    .SpriteHeight = 29,
	};
}

void Rect2D_Draw(Rect2D rect, bool8 fill) { Rect2D_DrawMany(&rect, 1, fill); }

void Rect2D_DrawMany(const Rect2D *Rects, u32 NumRects, bool8 Fill) {
	Vec2 *Pos = Allocate(sizeof(Vec2) * NumRects * 4);
	RGBA *Color = Allocate(sizeof(RGBA) * NumRects * 4);
	u32 *Inds = Allocate(sizeof(u32) * NumRects * (Fill ? 6 : 8));

	// The vertices are assumed to be in the order
	// top left,
	// top right,
	// bottom left,
	// bottom right.

	for(u32 i = 0; i < NumRects; i++) {
		Rect2D r = Rects[i];

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

	GLuint VAO = VAO_GetTemp();
	glBindVertexArray(VAO);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	GLuint VBOs[3];
	VBO_GetManyTemp(VBOs, 3);

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
	Vec2 sz = RT_GetCurrentSize();
	Camera cam = {.Mode = CameraMode_Orthographic,
	              .ScreenWidth  = sz.w,
	              .ScreenHeight = sz.h,
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
	VBO_FreeManyTemp(VBOs, 3);
	Free(Pos);
	Free(Color);
	Free(Inds);
	VAO_FreeTemp(VAO);
}

void Tri2D_DrawMany(const Tri2D *tris, u32 numTris)
{
	Vec2 *Pos = Allocate(sizeof(Vec2) * numTris * 3);
	RGBA *Color = Allocate(sizeof(RGBA) * numTris * 3);

	// The vertices are assumed to be in the order
	// top left,
	// top right,
	// bottom left,
	// bottom right.

	for(u32 i = 0; i < numTris; i++) {
		Tri2D t = tris[i];

		Pos[i * 4 + 0] = t.Points[0];
		Pos[i * 4 + 1] = t.Points[1];
		Pos[i * 4 + 2] = t.Points[2];

		Color[i * 4 + 0] = t.Color;
		Color[i * 4 + 1] = t.Color;
		Color[i * 4 + 2] = t.Color;
	}

	GLuint VAO = VAO_GetTemp();
	glBindVertexArray(VAO);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	GLuint VBOs[2];
	VBO_GetManyTemp(VBOs, 2);
	
	// Add positions
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vec2) * numTris * 3, Pos, GL_STREAM_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	// Add colors
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vec4) * numTris * 3, Color, GL_STREAM_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, NULL);

	// Drawing the tris
	Mat4 view, proj;
	Vec2 sz = RT_GetCurrentSize();
	Camera cam = {.Mode = CameraMode_Orthographic,
	              .ScreenWidth  = sz.w,
	              .ScreenHeight = sz.h,
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
	VBO_FreeManyTemp(VBOs, 2);
	Free(Pos);
	Free(Color);
	VAO_FreeTemp(VAO);
}

Vec2 Text2D_Size(const TextStyle* s, const char *fmt, ...) {
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

	return V2(
		s->Font->SpriteWidth  * maxWidth, 
		s->Font->SpriteHeight * height
	);
}

void Rect2D_DrawImage(Rect2D r, GLuint TextureID, bool8 UseRectUVs) {
	GLuint VAO = VAO_GetTemp();
	glBindVertexArray(VAO);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	GLuint VBO = VBO_GetTemp();
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vec2), NULL);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vec2),
	                      (void *) (sizeof(Vec2) * 4));

	const Vec2 UVs[4] = { 
		V2C(0,1), 
		V2C(0,0), 
		V2C(1,1),
		V2C(1,0), 
	};

	Vec2 PosAndUV[8] = {
	    V2(r.Position.x, r.Position.y),                       // Top left
	    V2(r.Position.x, r.Position.y + r.Size.y),            // Bottom left
	    V2(r.Position.x + r.Size.x, r.Position.y),            // Top right
	    V2(r.Position.x + r.Size.x, r.Position.y + r.Size.y), // Bottom right

	    (UseRectUVs ? r.UVs[0] : UVs[0]),
	    (UseRectUVs ? r.UVs[1] : UVs[1]),
	    (UseRectUVs ? r.UVs[2] : UVs[2]),
	    (UseRectUVs ? r.UVs[3] : UVs[3]),
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vec2) * 8, PosAndUV, GL_DYNAMIC_DRAW);

	Mat4 view, proj;
	Vec2 sz = RT_GetCurrentSize();
	Camera cam = {
		.Mode = CameraMode_Orthographic,
	    .ScreenWidth  = sz.w,
	    .ScreenHeight = sz.h,
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

	VBO_FreeTemp(VBO);
	VAO_FreeTemp(VAO);
}

Vec2 Text2D_Draw(Vec2 pos, const TextStyle* style, const char *fmt, ...) {
	char msg[512] = {0};

	va_list args;
	va_start(args, fmt);
	vsprintf(msg, fmt, args);
	va_end(args);

	u32 numChars = 0;
	for(u32 i = 0; i < strlen(msg); i++)
		numChars += (style->BackgroundEnabled ? isprint(msg[i]) : isgraph(msg[i])) ? 1 : 0;

	if(!numChars) return pos;

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
				if(!style->BackgroundEnabled) {
					pen.x += style->Font->SpriteWidth;
					continue;
				}
				break;
			case '\t': pen.x += style->Font->SpriteWidth * 4; continue;
			case '\n':
				pen.x = pos.x;
				pen.y += style->Font->SpriteHeight;
				continue;
		}

		Pos[j * 4 + 0] = pen;
		Pos[j * 4 + 1] = V2(pen.x + style->Font->SpriteWidth, pen.y);
		Pos[j * 4 + 2] = V2(pen.x, pen.y + style->Font->SpriteHeight);
		Pos[j * 4 + 3] = V2(pen.x + style->Font->SpriteWidth, pen.y + style->Font->SpriteHeight);

		if(msg[i] == ' ') {
			UV[j * 4 + 0] = V2(0, 0);
			UV[j * 4 + 1] = V2(0, 0);
			UV[j * 4 + 2] = V2(0, 0);
			UV[j * 4 + 3] = V2(0, 0);
		} else {
			const Vec2 uvSize = V2((r32) style->Font->SpriteWidth  / style->Font->Texture->Width,
			                       (r32) style->Font->SpriteHeight / style->Font->Texture->Height);

			u32 charsPerRow = style->Font->Texture->Width / style->Font->SpriteWidth;
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

		pen.x += style->Font->SpriteWidth;
	}

	GLuint VAO = VAO_GetTemp();
	glBindVertexArray(VAO);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	GLuint VBOs[3];
	VBO_GetManyTemp(VBOs, 3);
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
	Vec2 sz = RT_GetCurrentSize();
	Camera cam = {.Mode = CameraMode_Orthographic,
	              .ScreenWidth = sz.w,
	              .ScreenHeight = sz.h,
	              .Position = V3(0, 0, -1),
	              .Target = V3(0, 0, 0),
	              .Up = V3(0, 1, 0),
	              .ZNear = 0.01,
	              .ZFar = 1000};

	Camera_Mat4(cam, view, proj);
	Shader_Use(TextShader);
	Shader_UniformMat4(TextShader, "proj", proj);
	Shader_UniformMat4(TextShader, "view", view);

	Shader_Uniform4f(TextShader, "fg", style->Color);
	Shader_Uniform4f(TextShader, "bg", style->Background);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, style->Font->Texture->Id);

	glDrawElements(GL_TRIANGLES, numChars * 6, GL_UNSIGNED_INT, 0);

	// Cleanup
	Free(Pos);
	Free(UV);
	Free(Inds);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	VBO_FreeManyTemp(VBOs, 3);
	VAO_FreeTemp(VAO);

	return pen;
}


//
// 3D
//

Shader *R3D_Shader_UnlitColor,
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
	GLuint vao = VAO_GetTemp();
	GLuint vbo = VBO_GetTemp();

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
	VBO_FreeTemp(vbo);
	VAO_FreeTemp(vao);
}

void R3D_DrawTriangle(Camera cam, Vec3 a, Vec3 b, Vec3 c, RGBA color)
{
	// Two-sided triangle vertices.
	Vec3 pos[6] = { a, b, c,
	                b, a, c };

	GLuint vao = VAO_GetTemp();
	GLuint vbo = VBO_GetTemp();

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
	VBO_FreeTemp(vbo);
	VAO_FreeTemp(vao);
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

DEF_ARRAY(Transform3D, Transform3D);
DECL_ARRAY(Transform3D, Transform3D);

DEF_ARRAY(NodePtr, R3D_Node*);
DECL_ARRAY(NodePtr, R3D_Node*);

void R3D_CalcTransform(const R3D_Node* r, Mat4 out)
{
	if(r == NULL)
	{
		Mat4_Identity(out);
		return;
	}

	Mat4 m, t;
	R3D_CalcTransform(r->Parent, m);
	Transform3D_Mat4(r->LocalTransform, t);

	Mat4_MultMat(m, t);
	Mat4_Copy(out, m);
}

// TODO: Finish this
void R3D_RenderScene(R3D_Scene *scene)
{
	if(!scene || !scene->ActiveCamNode)
		return;

	Shader_Use(R3D_Shader_UnlitTextured);

	Mat4 camTr;
	R3D_CalcTransform(scene->ActiveCamNode, camTr);

	Array_Transform3D transforms = (Array_Transform3D){0};
	Array_NodePtr stk = (Array_NodePtr){0};

	Array_Transform3D_Push(&transforms, &stk.Data[0]->LocalTransform);
	Array_NodePtr_PushVal(&stk, &scene->Root);

	while(stk.Size)
	{
		R3D_Node* curr = stk.Data[0];

		// Draw this object
		if(curr->Type == Node_Actor)
		{
		}
		
		// Add this node's children to the stack
		for(u32 i = 0; i < curr->Children.Size; ++i)
			Array_NodePtr_PushVal(&stk, curr->Children.Data+i);
	}
}
