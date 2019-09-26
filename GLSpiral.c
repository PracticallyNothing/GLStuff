#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Common.h"
#include "Math3D.h"
#include "Shader.h"
#include "stb_image.h"

struct Transform2D_t {
    Vec2 Position;
    r32 Rotation;
    Vec2 Scale;
    struct Transform2D_t *Parent;
};

struct Transform3D_t {
    Vec3 Position;
    Quat Rotation;
    Vec3 Scale;
    struct Transform3D_t *Parent;
};

typedef struct Transform2D_t Transform2D;
typedef struct Transform3D_t Transform3D;

const u32 g_Width = 1280, g_Height = 720;
GLuint g_ShaderProgram;

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

void PrintVideoDriverInfo(void);
void GL_InitAttribs(void);


void Shader_LoadDefault(void);

GLuint Texture_FromFile(const char *filename);
void Transform2D_Mat3(Transform2D t, Mat3 out);
void Transform3D_Mat4(Transform3D t, Mat4 out);

i32 main(void) {
    u32 StartupTime = SDL_GetTicks();

    SDL_Window *Window;
    SDL_GLContext GLContext;

    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
	printf("SDL_Init() failed: %s\n", SDL_GetError());
	exit(EXIT_FAILURE);
    }
    PrintVideoDriverInfo();

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

    Shader_LoadDefault();

    Mat4 PerspMat, ModelMat;

    Mat4_OrthoProj(PerspMat, 0, g_Width, 0, g_Height, 0.0001, 10000);

#if 0
    Quat q;
    Mat4 TransMat, RotMat;

    q = Quat_RotAxis((Vec3){0, 0, 1}, Pi_Quarter);
    Mat4_RotateQuat(RotMat, q);
    Mat4_Identity(TransMat);
    Mat4_Translate(TransMat, (Vec3){g_Width / 2, g_Height / 2, 0});

    Mat4_Copy(ModelMat, TransMat);
    Mat4_MultMat(ModelMat, RotMat);
#else
    Transform3D tr = {.Position = {g_Width / 2, g_Height / 2, 0},
		      .Rotation = Quat_RotAxis((Vec3){0, 0, 1}, Pi_Quarter),
		      .Scale = {1, 1, 1},
		      .Parent = NULL};
    Transform3D_Mat4(tr, ModelMat);
#endif

    i32 PerspLoc = glGetUniformLocation(g_ShaderProgram, "persp");
    i32 ModelLoc = glGetUniformLocation(g_ShaderProgram, "model");
    glUniformMatrix4fv(PerspLoc, 1, GL_TRUE, PerspMat);
    glUniformMatrix4fv(ModelLoc, 1, GL_TRUE, ModelMat);

    SDL_GL_LoadLibrary(NULL);

    //--------------------------------//

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

    GLuint VAO, VBO_Pos;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glGenBuffers(1, &VBO_Pos);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_Pos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_Positions), g_Positions,
		 GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);

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

    GLuint TestBMP = Texture_FromFile("test1.bmp");

    u32 Ticks;
    u64 RenderTime;
    u32 LastPrintTime, LastFrameRenderTime, LastFrameDisplayTime;
    SDL_Event e;
    r32 time = 0;

    printf("Startup time: %u ms\n", SDL_GetTicks() - StartupTime);

    while(1) {
	Ticks = SDL_GetTicks();

	while(SDL_PollEvent(&e)) {
	    switch(e.type) {
	    case SDL_QUIT: goto end;
	    case SDL_KEYDOWN:
		switch(e.key.keysym.sym) {}
	    case SDL_KEYUP:
		switch(e.key.keysym.sym) {
		case SDLK_ESCAPE:
		case SDLK_q: goto end;
		}
		break;
	    case SDL_WINDOWEVENT:
		switch(e.window.event) {
		case SDL_WINDOWEVENT_CLOSE: goto end;
		case SDL_WINDOWEVENT_SIZE_CHANGED:
		case SDL_WINDOWEVENT_RESIZED: {
		    i32 w = e.window.data1, h = e.window.data2;
		    glViewport(0, 0, w, h);
		    Mat4_OrthoProj(PerspMat, 0, w, 0, h, 0.01, 1000.0);
		    glUniformMatrix4fv(PerspLoc, 1, GL_TRUE, PerspMat);
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

	    // Draw everything:

	    // glBindFramebuffer(GL_DRAW_FRAMEBUFFER, Framebuf);
	    // glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	    // Render to screen quad:
	    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	    glActiveTexture(GL_TEXTURE0);
	    glBindTexture(GL_TEXTURE_2D, TestBMP);
	    glUniform1i(glGetUniformLocation(g_ShaderProgram, "fbo_color"), 0);
	    glBindVertexArray(VAO_ScreenQuad);
	    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	    SDL_GL_SwapWindow(Window);

	    RenderTime = SDL_GetPerformanceCounter() - RenderTime;
	    LastFrameRenderTime = Ticks;
	    time += 0.01;
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
    glDeleteBuffers(1, &VBO_Pos);
    glDeleteVertexArrays(1, &VAO);
    SDL_GL_DeleteContext(GLContext);
    SDL_DestroyWindow(Window);
    SDL_Quit();
}

void PrintVideoDriverInfo(void) {
    i32 NumVideoDrivers = SDL_GetNumVideoDrivers();
    if(NumVideoDrivers < 0) {
	printf("SDL_GetNumVideoDrivers() failed: %s\n", SDL_GetError());
	exit(EXIT_FAILURE);
    }

    printf("Num video drivers: %d\n", NumVideoDrivers);

    for(i32 i = 0; i < NumVideoDrivers; ++i) {
	printf("#%d: %s\n", i, SDL_GetVideoDriver(i));
    }
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

// const char *const VertexShaderText =
// 	"#version 330\n"
// 	"uniform mat4 perspective;\n"
// 	"in vec3 pos;\n"
// 	"void main() { gl_Position = perspective * vec4(pos, 1); }";
// const char *const FragmentShaderText =
// 	"#version 330\n"
// 	"out vec4 color;\n"
// 	"void main() { color = vec4(0, 1, 0, 1); }";

GLuint Shader_FromFile(GLenum type, const char *filename) {
    FILE *f;
    char *src;
    u64 size;

    f = fopen(filename, "r");
    if(!f) {
	printf("%s shader file \"%s\" couldn't be opened for reading.",
	       (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment"), filename);
	return 0;
    }

    fseek(f, 0, SEEK_END);
    size = ftell(f);
    src = malloc(sizeof(char) * size + 1);

    fseek(f, 0, SEEK_SET);
    fread(src, sizeof(char), size, f);
    src[size] = '\0';

    GLuint res = Shader_FromSrc(type, src);

    fclose(f);
    free(src);
    return res;
}
GLuint Shader_FromSrc_Ex(GLenum type, const char *shaderSrc) {
    u32 Shader;
    i32 ShaderOK;

    Shader = glCreateShader(type);
    glShaderSource(Shader, 1, &shaderSrc, NULL);
    glCompileShader(Shader);

    glGetShaderiv(Shader, GL_COMPILE_STATUS, &ShaderOK);
    if(ShaderOK != GL_TRUE) {
	i32 LogLength;
	char *Log;

	glGetShaderiv(Shader, GL_INFO_LOG_LENGTH, &LogLength);
	Log = malloc(sizeof(char) * LogLength);
	glGetShaderInfoLog(Shader, LogLength, NULL, Log);

	printf("%s shader compilation failed.\n%s\n",
	       (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment"), Log);
	free(Log);
    }

    return Shader;
}

GLuint Shader_Link(GLuint VertexShader, GLuint FragmentShader) {
    GLuint ShaderProgram;
    i32 ShaderProgramOK;

    ShaderProgram = glCreateProgram();
    glAttachShader(ShaderProgram, VertexShader);
    glAttachShader(ShaderProgram, FragmentShader);
    glLinkProgram(ShaderProgram);

    glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &ShaderProgramOK);
    if(ShaderProgramOK != GL_TRUE) {
	i32 LogLength;
	char *Log;

	glGetProgramiv(ShaderProgram, GL_INFO_LOG_LENGTH, &LogLength);
	Log = malloc(sizeof(char) * LogLength);
	glGetProgramInfoLog(ShaderProgram, LogLength, NULL, Log);

	printf("Shader program linking failed.\n%s\n", Log);
	free(Log);
    }
    return ShaderProgram;
}

void Shader_LoadDefault(void) {
    GLuint VertexShader, FragmentShader, ShaderProgram;

    VertexShader = Shader_FromFile(GL_VERTEX_SHADER, "vert.glsl");
    FragmentShader = Shader_FromFile(GL_FRAGMENT_SHADER, "frag.glsl");
    ShaderProgram = Shader_Link(VertexShader, FragmentShader);
    g_ShaderProgram = ShaderProgram;

    glDeleteShader(VertexShader);
    glDeleteShader(FragmentShader);
    glUseProgram(ShaderProgram);
    printf("Shaders compiled and linked successfully.\n");
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
    Mat4_Copy(out, scale);
}
