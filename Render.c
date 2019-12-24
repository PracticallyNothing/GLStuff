#include "Render.h"

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

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
	                    SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

	RSys_State.Window =
	    SDL_CreateWindow("GL Spiral", 0, 0, Width, Height,
	                     SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

	if(!RSys_State.Window) {
		printf("SDL_CreateWindow() failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	RSys_State.GLContext = SDL_GL_CreateContext(RSys_State.Window);
	if(!RSys_State.GLContext) {
		printf("SDL_GL_CreateContext() failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	gladLoadGL();

	glClearColor(0, 0, 0, 1);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glEnable(GL_MULTISAMPLE);

	RSys_State.LastFrameTime = SDL_GetTicks();
}

void RSys_FinishFrame() {
	SDL_GL_SwapWindow(RSys_State.Window);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	RSys_State.LastFrameDT = SDL_GetTicks() - RSys_State.LastFrameTime;
	RSys_State.LastFrameTime = SDL_GetTicks();
}

void RSys_Quit() {
	SDL_GL_DeleteContext(RSys_State.GLContext);
	SDL_DestroyWindow(RSys_State.Window);
	SDL_Quit();
}
