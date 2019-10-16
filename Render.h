#ifndef RENDER_H
#define RENDER_H

#include "Common.h"
#include "Math3D.h"
#include "Shader.h"

typedef struct Camera {
	Vec3 Position;
	Vec3 Target;
	Vec3 Up;
} Camera;

enum AttachmentType {
	AttachmentType_None = 0,
	AttachmentType_Texture,
	AttachmentType_Renderbuffer
};

enum MultisampleMode {
	MultisampleMode_None = 0,
	MultisampleMode_X2,
	MultisampleMode_X4,
};

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

typedef struct RenderTarget {
	u32 Width;
	u32 Height;

	GLuint Framebuffer;

	enum AttachmentType ColorAttachmentType;
	enum AttachmentType DepthAttachmentType;
	enum AttachmentType StencilAttachmentType;

	GLuint ColorAttachment;
	GLuint DepthAttachment;
	GLuint StencilAttachment;
} RenderTarget;

typedef struct RenderCtx {
	Mat4 PerspectiveMatrix;
	Camera Camera;
	Shader *Shader;
	RenderTarget *RenderTarget;
} RenderCtx;

extern void RenderTarget_Init(RenderTarget *);
extern void RenderTarget_AddColor(RenderTarget *, enum AttachmentType,
                                  enum MultisampleMode);
extern void RenderTarget_AddDepth(RenderTarget *, enum AttachmentType);
extern void RenderTarget_AddStencil(RenderTarget *, enum AttachmentType);
extern void RenderTarget_Use(RenderTarget *);
extern void RenderTarget_UseDefault();

extern void RenderCtx_Init(RenderCtx *);
extern void RenderCtx_Use(RenderCtx *);

// ------------------ //
// Steps to render:
// 1. Bind the render target
// 2. Clear it
// 3. Set up PVM matrix (Projection, View, Model)
// 4. Bind shader and set up uniforms
// 5. Bind all necessary textures
// 6. Bind a vertex array
// 7. glDrawArrays()/glDrawElements()
// ------------------ //
// 1-4 -> RenderCtx
// 4-7 -> Model
//
// So a rendering function would ultimately only need these two parameters,
// e.g.:
/////////////////////////////////////////////////////////////////////////////
//                                                                         //
// void Render(const RenderCtx *ctx, const Models *models, i32 numModels); //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
// However, the models array sucks, especially if every model gets its own
// texture and VAO.
//
// A better approach would be to pass a sort of RenderPkg,
// which contains the required textures and VAOs in a sorted manner, which also
// describes the dependencies between 3D mesh data and textures (or implies them
// somehow).
//
/////////////////////////////////////////////////////////////////////////////

#endif
