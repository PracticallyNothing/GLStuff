#ifndef RENDER_H
#define RENDER_H

#include <SDL.h>

#include "Camera.h"
#include "Common.h"
#include "Math3D.h"
#include "SDL_events.h"
#include "Shader.h"
#include "Phys.h"

// 
// Rendering system
//

typedef struct RSys_Size_t RSys_Size;

struct RSys_Size_t {
	i32 Width, Height;
	r32 AspectRatio;
};

void RSys_LogVideoDriverInfo(void);

void RSys_Init(u32 Width, u32 Height);
void RSys_Quit();

void RSys_FinishFrame();
void RSys_HandleWindowEvent(const SDL_WindowEvent *);

void RSys_SetFPSCap(u32 capFps);
void RSys_SetFrametimeCap(r32 capMs);

bool8 RSys_NeedRedraw();
RSys_Size RSys_GetSize();

GLuint RSys_GetTempVAO();
void RSys_FreeTempVAO(GLuint);

typedef struct RSys_Texture RSys_Texture;

struct RSys_Texture {
	GLuint Id;
	i32 Width, Height;
	i32 NumComponents;
};
RSys_Texture RSys_TextureFromMemory(const u8* buf, u32 bufSize, u32 numComponents);
RSys_Texture RSys_TextureFromFile(const char *filename);

enum RSys_RT_Type {
	RenderTarget_DefaultRT = -1,

	RenderTarget_Texture,
	RenderTarget_Renderbuffer,
};

typedef struct RSys_RT RSys_RT;

struct RSys_RT {
	enum RSys_RT_Type Type;

	GLuint FramebufferId;
	GLuint ColorAttachmentId, DepthAttachmentId;
};

RSys_RT RSys_RT_Init(enum RSys_RT_Type type);
void RSys_RT_Free(RSys_RT);
void RSys_RT_ReadFrom(RSys_RT);
void RSys_RT_DrawTo(RSys_RT);


// 
// 2D
//

typedef struct R2D_Rect        R2D_Rect;
typedef struct R2D_Triangle    R2D_Triangle;
typedef struct R2D_Spritesheet R2D_Spritesheet;

struct R2D_Rect {
	Vec2 Position;
	Vec2 Size;
	union {
		RGBA Color;
		Vec2 UVs[4];
	};
};

struct R2D_Triangle {
	Vec2 Points[3];
	union {
		Vec2 UVs[3];
		RGBA Color;
	};
};

struct R2D_Spritesheet {
	GLuint TextureId;
	u32 Width, Height;
	u32 SpriteWidth, SpriteHeight;
};


extern R2D_Spritesheet 
	R2D_DefaultFont_Small,
	R2D_DefaultFont_Medium,
	R2D_DefaultFont_Large;

void R2D_DrawTriangle (const R2D_Triangle *triangle);
void R2D_DrawTriangles(const R2D_Triangle *triangles, u32 NumTriangles);

void R2D_DrawRects(const R2D_Rect *Rects, u32 NumRects, bool8 Fill);

void R2D_DrawRectImage(Vec2 Position, Vec2 Size, GLuint TextureID, const Vec2 *TextureUVs);

void R2D_DrawText(Vec2 pos, RGBA fg, RGBA bg, const R2D_Spritesheet *font, const char *fmt, ...);

Vec2 R2D_GetTextExtents(const R2D_Spritesheet *font, const char *fmt, ...);

void R2D_DrawConsole();


//
// 3D
//

enum R3D_ShaderType {
	R3D_Shader_None = -1,

	R3D_Shader_Unlit = 0,
	R3D_Shader_Lit,
	R3D_Shader_Depth,

	R3D_Shader_NumShaders,
};

struct R3D_State_t {
	enum R3D_ShaderType CurrentShader;
	Shader *Shaders[R3D_Shader_NumShaders];
	i32 DebugMode;
	void *Scene;
};

extern Shader *R3D_Shader_UnlitColor,
	          *R3D_Shader_UnlitTextured;

extern struct R3D_State_t R3D_State;

typedef struct Actor Actor;
typedef struct Light Light;

struct Actor {
	enum {
		RenderMode_Wireframe,
		RenderMode_UnlitColor,
		RenderMode_UnlitDiffuse,
		RenderMode_Lit,
	} RenderMode;

	bool8 CastShadow;
	GLuint VAO, ElementBuffer;
};

enum R3D_Light_Type {
	LightType_Point,
	LightType_Directional,
	LightType_Spotlight,
};

struct Light {
	enum R3D_Light_Type Type;

	RGB Ambient;
	RGB Diffuse;
	RGB Specular;

	union {
		struct PointLight {
			Vec3 Position;
			r32 ConstantAttenuation;
			r32 LinearAttenuation;
			r32 QuadraticAttenuation;
		} Point;

		struct DirectionalLight {
			Vec3 Direction;
		} Dir;

		struct Spotlight {
			Vec3 Position;
			Vec3 Direction;
			r32 CutoffAngle_Inner;
			r32 CutoffAngle_Outer;
		} Spot;
	};
};

enum R3D_Node_Type {
	Node_None = -1,

	Node_Camera,
	Node_Actor,
	Node_Particles,
	Node_Light,

	Node_NumTypes
};

typedef struct R3D_Node R3D_Node;
typedef struct R3D_Scene R3D_Scene;

DEF_ARRAY(Node, R3D_Node);

struct R3D_Node {
	const R3D_Node *Parent;
	Array_Node Children;

	Transform3D LocalTransform;

	enum R3D_Node_Type Type;

	union {
		Camera Camera;
		Actor Actor;
		Light Light;
	};
};

struct R3D_Scene {
	R3D_Node Root;
	Camera *ActiveCamera;
};

void R3D_Init();
void R3D_RenderScene(R3D_Scene *Scene);

R3D_Node* R3D_Node_Create(enum R3D_Node_Type);
R3D_Node* R3D_Node_AttachNew(enum R3D_Node_Type);

void R3D_DrawLine(Camera cam, Vec3 start, Vec3 end, RGBA color);
void R3D_DrawLines(Camera cam, Vec3 *linePoints, u32 numLines, RGBA color);
void R3D_DrawTriangle(Camera cam, Vec3 a, Vec3 b, Vec3 c, RGBA color);
void R3D_DrawWireSphere(Camera cam, Vec3 center, r32 radius, RGBA color);

#endif
