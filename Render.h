#ifndef RENDER_H
#define RENDER_H

#include <SDL.h>

#include "Camera.h"
#include "Common.h"
#include "Math3D.h"
#include "Shader.h"
#include "Transform.h"

enum LightType {
	LightType_Point,
	LightType_Directional,
	LightType_Spotlight,
};

typedef struct Light {
	enum LightType Type;

	Vec3 Ambient;
	Vec3 Diffuse;
	Vec3 Specular;

	union LightData {
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
	} L;
} Light;

// ---=== Rendering system ===---

typedef struct RSys_Size_t RSys_Size;

struct RSys_Size_t {
	i32 Width, Height;
	r32 AspectRatio;
};

extern void RSys_Init(u32 Width, u32 Height);
extern void RSys_Quit();
extern void RSys_FinishFrame();

extern u64 RSys_GetLastFrameTime();
extern RSys_Size RSys_GetSize();

extern GLuint RSys_GetTempVAO();
extern void RSys_FreeTempVAO(GLuint);

struct RSys_Texture {
	GLuint Id;
	i32 Width, Height;
	i32 NumComponents;
};
extern struct RSys_Texture RSys_TextureFromFile(const char *filename);

typedef struct RenderTarget_t RenderTarget;

enum RenderTarget_Type {
	RenderTarget_DefaultRT = -1,

	RenderTarget_Texture,
	RenderTarget_Renderbuffer,
};

struct RenderTarget_t {
	enum RenderTarget_Type Type;

	GLuint FramebufferId;
	GLuint ColorAttachmentId, DepthAttachmentId;
};

RenderTarget RenderTarget_Init(enum RenderTarget_Type type);
void RenderTarget_Free(RenderTarget);
void RenderTarget_ReadFrom(RenderTarget);
void RenderTarget_DrawTo(RenderTarget);

// ---=== 2D rendering ===---
struct R2D_Rect {
	Vec2 Position;
	Vec2 Size;
	union {
		RGBA Color;
		Vec2 UVs[4];
	};
};

struct R2D_Spritesheet {
	GLuint TextureId;
	u32 Width, Height;
	u32 SpriteWidth, SpriteHeight;
};

extern struct R2D_Spritesheet R2D_DefaultFont;

extern void R2D_Init();
extern void R2D_DrawRects(const struct R2D_Rect *Rects, u32 NumRects,
                          bool8 Fill);
extern void R2D_DrawRectImage(Vec2 Position, Vec2 Size, GLuint TextureID,
                              const Vec2 *TextureUVs);
extern void R2D_DrawText(Vec2 pos, RGBA fg, RGBA bg,
                         const struct R2D_Spritesheet *font, const char *fmt,
                         ...);
extern Vec2 R2D_GetTextExtents(const struct R2D_Spritesheet *font,
                               const char *fmt, ...);
// ---===##############===---

// ---=== 3D rendering ===---

enum R3D_ShaderType {
	R3D_Shader_None = -1,

	R3D_Shader_Unlit = 0,
	R3D_Shader_Lit,
	R3D_Shader_Depth,

	R3D_Shader_NumShaders,
};

struct R3D_State_t {
	enum R3D_ShaderType CurrentShader;
	struct Shader *Shaders[R3D_Shader_NumShaders];
	i32 DebugMode;
	void *Scene;
};

extern struct R3D_State_t R3D_State;

typedef struct R3D_SceneNode_t R3D_SceneNode;
enum R3D_SceneNode_Type {
	R3D_SceneNode_None = -1,

	R3D_SceneNode_Camera,
	R3D_SceneNode_StaticMesh,
	R3D_SceneNode_SkinnedMesh,
	R3D_SceneNode_Particles,
	R3D_SceneNode_Light,

	R3D_SceneNode_NumTypes
};

struct R3D_SceneNode_t {
	R3D_SceneNode *Parent;
	Array *Children;

	Transform3D Transform;

	enum R3D_SceneNode_Type Type;

	union {
		Camera Camera;
		void *StaticMesh;
		void *SkinnedMesh;
		void *Particles;
		Light *Light;
	};
};

typedef struct R3D_Scene_t {
	R3D_SceneNode Root;
} R3D_Scene;

void R3D_Init();
void R3D_UseShader(enum R3D_ShaderType type);
void R3D_Render();
void R3D_SetScene(R3D_Scene *Scene);
void R3D_DebugMode(i32 mode);

#endif
