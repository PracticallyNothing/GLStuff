#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Common.h"
#include "Math3D.h"
#include "Shader.h"
#include "stb_image.h"

typedef struct Transform2D_t Transform2D;
typedef struct Transform3D_t Transform3D;
typedef struct Camera_t Camera;

struct Transform2D_t {
	Vec2 Position;
	r32 Rotation;
	Vec2 Scale;
	Transform2D *Parent;
};

struct Transform3D_t {
	Vec3 Position;
	Quat Rotation;
	Vec3 Scale;
	Transform3D *Parent;
};

struct Camera_t {
	Vec3 Position;
	Vec3 Target;
	Vec3 Up;
};

const char *const VertexShaderText =
    "#version 330\n"
    "\n"
    "in vec3 pos;\n"
    "out vec3 fPos;\n"
    "\n"
    "uniform mat4 persp;\n"
    "uniform mat4 view;\n"
    "uniform mat4 model;\n"
    "\n"
    "void main() {\n"
    "   gl_Position = persp * view * model * vec4(pos, 1);\n"
    "   fPos = (model * vec4(pos,1)).xyz;\n"
    "}";
const char *const FragmentShaderText =
    "#version 330\n"
    "in vec3 fPos;"
    "out vec4 color;\n"
    "void main() {\n"
    "    color = vec4(fPos, 1);\n"
    "}";

const u32 g_Width = 1280, g_Height = 720;

r32 g_Positions[][2] = {
    {-200, 200},
    {200, 200},
    {-200, -200},
    {200, -200},
};

r32 g_ScreenQuad[][4] = {
    {-1, 1, 0, 1},
    {-1, -1, 0, 0},
    {1, 1, 1, 1},
    {1, -1, 1, 0},
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

void GL_InitAttribs(void);

GLuint Texture_FromFile(const char *filename);
void Transform2D_Mat3(Transform2D t, Mat3 out);
void Transform3D_Mat4(Transform3D t, Mat4 out);
void Camera_Mat4(Camera c, Mat4 out);
void RenderLines(Vec3 *linePoints, i32 numLines);
void RenderLineCircle(r32 radius);

i32 main(void) {
	u32 StartupTime;

	SDL_Window *Window;
	SDL_GLContext GLContext;

	Shader *s, *s2;

	Mat4 PerspMat, ViewMat, ModelMat;
	Transform3D Transform;
	Camera Camera;

	StartupTime = SDL_GetTicks();

	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
		printf("SDL_Init() failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	PC_PrintVideoDriverInfo();

	GL_InitAttribs();

	Window = SDL_CreateWindow("GL Spiral", 0, 0, g_Width, g_Height,
	                          SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

	if(!Window) {
		printf("SDL_CreateWindow() failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	GLContext = SDL_GL_CreateContext(Window);
	if(!GLContext) {
		printf("SDL_GL_CreateContext() failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	// TODO: Error checking.
	gladLoadGL();

	glClearColor(0, 0, 0, 1);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_MULTISAMPLE);

	s = Shader_FromFiles("vert.glsl", "frag.glsl");
	s2 = Shader_FromSrc(VertexShaderText, FragmentShaderText);

	Transform = (Transform3D){.Position = {0, 0, -50},
	                          .Rotation = Quat_Identity,
	                          .Scale = {1, 1, 1},
	                          .Parent = NULL};

	GLuint Framebuf;
	glGenFramebuffers(1, &Framebuf);

#if 0
    GLuint Renderbuf_Color, Renderbuf_Depth;
    glGenRenderbuffers(1, &Renderbuf_Color);
    glGenRenderbuffers(1, &Renderbuf_Depth);

    // Color attachment
    glBindRenderbuffer(GL_RENDERBUFFER, Renderbuf_Color);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, g_Width, g_Height);

    // Depth attachment
    glBindRenderbuffer(GL_RENDERBUFFER, Renderbuf_Depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, g_Width,
			  g_Height);

    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // Framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, Framebuf);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			      GL_RENDERBUFFER, Renderbuf_Color);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
			      GL_RENDERBUFFER, Renderbuf_Depth);
#else
	GLuint Texture_Color, Texture_Depth;

	glGenTextures(1, &Texture_Color);
	glBindTexture(GL_TEXTURE_2D, Texture_Color);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, g_Width, g_Height, 0, GL_RGBA,
	             GL_UNSIGNED_BYTE, NULL);

	glGenTextures(1, &Texture_Depth);
	glBindTexture(GL_TEXTURE_2D, Texture_Depth);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, g_Width, g_Height, 0,
	             GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);

	glBindFramebuffer(GL_FRAMEBUFFER, Framebuf);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
	                       Texture_Color, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
	                       Texture_Depth, 0);
#endif

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		printf("Framebuffer not complete.\n");
	}

	//--------------------------------//

#if 0
	// TODO: Make this prettier.
	GLuint VAO, VBO_Pos;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glGenBuffers(1, &VBO_Pos);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_Pos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_Positions), g_Positions,
	             GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
#endif

	// Create screen quad
	GLuint VAO_ScreenQuad, VBO_ScreenQuad;
	glGenVertexArrays(1, &VAO_ScreenQuad);
	glBindVertexArray(VAO_ScreenQuad);
	glGenBuffers(1, &VBO_ScreenQuad);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_ScreenQuad);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_ScreenQuad), g_ScreenQuad,
	             GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(r32) * 4, NULL);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(r32) * 4,
	                      (void *) (2 * sizeof(r32)));

	// Create 3D cube
	GLuint Cube_VAO, Cube_Pos, Cube_Inds;
	glGenVertexArrays(1, &Cube_VAO);
	glBindVertexArray(Cube_VAO);
	glGenBuffers(1, &Cube_Pos);
	glGenBuffers(1, &Cube_Inds);
	glBindBuffer(GL_ARRAY_BUFFER, Cube_Pos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_CubePos), (r32 *) g_CubePos,
	             GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Cube_Inds);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(g_CubeInds),
	             (i32 *) g_CubeInds, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	GLuint TestBMP = Texture_FromFile("test1.bmp");

	u32 Ticks;
	u64 RenderTime;
	u32 LastPrintTime, LastFrameRenderTime;
	SDL_Event e;
	r32 time = 0;
	i32 StopTime = 0;

	printf("Startup time: %u ms\n", SDL_GetTicks() - StartupTime);

	while(1) {
		Ticks = SDL_GetTicks();

		while(SDL_PollEvent(&e)) {
			switch(e.type) {
				case SDL_QUIT: goto end;
				case SDL_KEYDOWN:
					switch(e.key.keysym.sym) {}
					break;
				case SDL_KEYUP:
					switch(e.key.keysym.sym) {
						case SDLK_ESCAPE:
						case SDLK_q: goto end;
						case SDLK_SPACE:
							StopTime = !StopTime;
							printf("Time %s!\n",
							       (StopTime ? "stopped" : "unstopped"));
							break;
					}
					break;
				case SDL_WINDOWEVENT:
					switch(e.window.event) {
						case SDL_WINDOWEVENT_CLOSE: goto end;
						case SDL_WINDOWEVENT_SIZE_CHANGED:
						case SDL_WINDOWEVENT_RESIZED: {
							i32 w = e.window.data1, h = e.window.data2;
							glViewport(0, 0, w, h);
							printf("Viewport change to %dx%d\n", w, h);
							// Mat4_OrthoProj(PerspMat, 0, w, 0, h, 0.01,
							// 1000.0);
							//
							// Shader_UniformMat4(s, "persp", PerspMat);
							break;
						}
					}
			}
		}

		// Render frame to back buffer.
		if(Ticks - LastFrameRenderTime > 16) {
			RenderTime = SDL_GetPerformanceCounter();
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, Framebuf);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			i32 realW, realH;
			SDL_GetWindowSize(Window, &realW, &realH);

			// --- Draw everything --- //

			Transform.Position.z = 0;
			// Transform.Rotation = Quat_RotAxis(V3(1, 1, 1), time);
			// Transform.Scale = V3(0.5, 0.5, 0.5);

			Camera.Position.x = 15 * sinf(time);
			Camera.Position.y = 5 * sinf(time);
			Camera.Position.z = 15 * cosf(time);
			// Camera.Position.y = 5;
			// Camera.Position.z = -15;

			// Camera.Target = tr.Position;
			Camera.Target = V3(0, 0, 0);
			// Camera.Target = Vec3_Add(Camera.Position, V3(0, 0, 1));

			Camera.Up = V3(0, 1, 0);

			Camera_Mat4(Camera, ViewMat);
			// Transform3D_Mat4(Transform, ModelMat);
			Mat4_RectProj(PerspMat, DegToRad(90.0), (1.0f * g_Width) / g_Height,
			              0.001f, 1000.0f);

			Shader_Use(s2);
			Shader_UniformMat4(s2, "persp", PerspMat);
			Shader_UniformMat4(s2, "view", ViewMat);
			Mat4_Identity(ModelMat);
			Shader_UniformMat4(s2, "model", ModelMat);

			Vec3 line[] = {V3(0, -5, 0),
			               Vec3_Add(Camera.Position, V3(0, -5, 0))};
			RenderLines(line, 1);
			RenderLineCircle(15);

			glBindVertexArray(Cube_VAO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Cube_Inds);

			const i32 N = 150;

			for(int z = -(N / 2); z <= N / 2; z++) {
				for(int x = -(N / 2); x <= N / 2; x++) {
					Transform.Position.x = x * 10.0;
					Transform.Position.z = z * 10.0;
					Transform.Rotation = Quat_RotAxis(V3(1, 1, 1), x * time);
					Transform3D_Mat4(Transform, ModelMat);
					Shader_UniformMat4(s2, "model", ModelMat);
					glDrawElements(GL_TRIANGLES,
					               sizeof(g_CubeInds) / sizeof(u32),
					               GL_UNSIGNED_INT, NULL);
				}
			}
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

			// --- Render to screen quad --- //

			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			Shader_Use(s);
			Shader_Uniform1i(s, "fbo_color", 0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, Texture_Color);
			glBindVertexArray(VAO_ScreenQuad);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			SDL_GL_SwapWindow(Window);

			RenderTime = SDL_GetPerformanceCounter() - RenderTime;
			LastFrameRenderTime = Ticks;
			if(!StopTime) time += 0.01;
		}

		Ticks = SDL_GetTicks();

		// Print render time.
		if(Ticks - LastPrintTime > 500) {
			printf("[%10d] Render time: %f ms (%10ld ticks)\n", SDL_GetTicks(),
			       (r64)(1000 * RenderTime) / SDL_GetPerformanceFrequency(),
			       RenderTime);
			LastPrintTime = Ticks;
		}
	}

end:
	// glDeleteBuffers(1, &VBO_Pos);
	// glDeleteVertexArrays(1, &VAO);
	SDL_GL_DeleteContext(GLContext);
	SDL_DestroyWindow(Window);
	SDL_Quit();
}

void GL_InitAttribs(void) {
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

void Transform2D_Mat3(Transform2D t, Mat3 out) {
	Mat3 translate, rotate, scale, parent;

	Mat3_Identity(translate);
	Mat3_Identity(rotate);
	Mat3_Identity(scale);
	Mat3_Identity(parent);

	// Translation
	translate[2] = t.Position.x;
	translate[5] = t.Position.y;

	// Rotation
	rotate[0] = cosf(t.Rotation);
	rotate[1] = -sinf(t.Rotation);
	rotate[3] = sinf(t.Rotation);
	rotate[4] = cosf(t.Rotation);

	// Scale
	scale[0] = t.Scale.x;
	scale[4] = t.Scale.y;

	// Parent
	if(t.Parent && t.Parent != &t) Transform2D_Mat3(*t.Parent, parent);

	Mat3_MultMat(scale, rotate);
	Mat3_MultMat(scale, translate);
	Mat3_MultMat(scale, parent);
	Mat3_Copy(out, translate);
}

void Transform3D_Mat4(Transform3D t, Mat4 out) {
	Mat4 translate, rotate, scale, parent;

	Mat4_Identity(translate);
	Mat4_Identity(rotate);
	Mat4_Identity(scale);
	Mat4_Identity(parent);

	// Translation
	Mat4_Translate(translate, t.Position);

	// Rotation
	Mat4_RotateQuat(rotate, t.Rotation);

	// Scale
	Mat4_Scale(scale, t.Scale);

	// Parent matrix
	if(t.Parent && t.Parent != &t) Transform3D_Mat4(*t.Parent, parent);

	Mat4_MultMat(rotate, scale);
	Mat4_MultMat(translate, rotate);
	Mat4_MultMat(parent, translate);
	Mat4_Copy(out, parent);
}

// Thank you,
// http://ogldev.atspace.co.uk/www/tutorial13/tutorial13.html
void Camera_Mat4(Camera c, Mat4 out) {
	Mat4 Translation, Rotation;

	Mat4_Identity(Translation);
	Mat4_Identity(Rotation);

	Mat4_Translate(Translation, Vec3_Neg(c.Position));

	Vec3 tgt = Vec3_Sub(c.Target, c.Position);

	Vec3 N = Vec3_Norm(tgt);
	Vec3 U = Vec3_Norm(Vec3_Cross(c.Up, tgt));
	Vec3 V = Vec3_Cross(N, U);

	// printf("%.2f %.2f %.2f\n", N.x, N.y, N.z);

	// N points towards the target point
	// U points to the right of the camera
	// V points towards the sky

	Rotation[0] = U.x;
	Rotation[1] = U.y;
	Rotation[2] = U.z;

	Rotation[4] = V.x;
	Rotation[5] = V.y;
	Rotation[6] = V.z;

	Rotation[8] = N.x;
	Rotation[9] = N.y;
	Rotation[10] = N.z;


	// Mat4_MultMat(Translation, Rotation);
	// Mat4_Copy(out, Translation);

	Mat4_MultMat(Rotation, Translation);
	Mat4_Copy(out, Rotation);
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
}

#if 0
// Potential GUI Code

GUI *g_GameGUI, *g_DebugGUI;

void InitGUI() {
	g_GameGUI = GUI_Init();

	GUI_AddFPSCounter(g_GameGUI, FPSPos_TopLeft, FPSType_FPS);

	// --- Debug GUI --- //
	
	GUI_Panel *LogPanel;

	g_DebugGUI = GUI_Init();
	GUI_AddFPSCounter(g_DebugGUI, FPSPos_TopLeft, FPSType_Frametime);

	LogPanel = GUI_AddPanel(
		g_DebugGUI,  // target GUI
		0, 0,        // x, y
		500, 300,    // width, height
		"Debug log", // title
		PanelButtons_None
	);

	GUI_Panel_AddTextList(
		LogPanel, // target panel
		0, 0,     // x, y
		500, -1,  // width, height
		500,      // num entries
	);
}

#endif
