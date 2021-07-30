#ifndef RENDER_H
#define RENDER_H

#include <SDL.h>

#include "Camera.h"
#include "Common.h"
#include "Math3D.h"
#include "Phys.h"
#include "SDL_events.h"
#include "Shader.h"

//
// Rendering system
//

void RSys_LogVideoDriverInfo(void);

void RSys_Init(u32 Width, u32 Height);
void RSys_Quit();

void RSys_FinishFrame();
void RSys_HandleWindowEvent(const SDL_WindowEvent*);

void RSys_SetFPSCap(u32 capFps);
void RSys_SetFrametimeCap(r32 capMs);
bool8 RSys_NeedRedraw();

GLuint VAO_GetTemp();
void VAO_FreeTemp(GLuint);

enum Texture_Format {
	Format_Red  = GL_RED,  // Only one color per pixel
	Format_RG   = GL_RG,   // Two colors per pixel
	Format_RGB  = GL_RGB,  // Three colors per pixel, Red Green Blue
	Format_RGBA = GL_RGBA, // Four colors per pixel, Red Green Blue Alpha

	Format_BGR  = GL_BGR,  // Three colors per pixel, Blue Green Red
	Format_BGRA = GL_BGRA, // Four colors per pixel, Blue Green Red Alpha

	Format_Depth        = GL_DEPTH_COMPONENT, // Pixels record depth
	Format_DepthStencil = GL_DEPTH_STENCIL    // Pixels record depth + stencil
};

enum Texture_Filter {
	Filter_Nearest = GL_NEAREST, // Looks pixelated
	Filter_Linear  = GL_LINEAR,  // Looks blurry
};
enum Texture_Wrap {
	Wrap_Repeat = GL_REPEAT, // If sampling outside texture, return pixel from inside texture
	Wrap_ClampToBorder = GL_CLAMP_TO_BORDER // If sampling outside texture, return black
};

// OpenGL texture
typedef struct Texture Texture;

// OpenGL texture.
struct Texture {
	GLuint Id;                  // OpenGL ID
	u32 Width, Height;          // Width and Height in pixels
	enum Texture_Format Format; // The format of the pixels
	enum Texture_Filter Filter; // Magnification and minification filter
	enum Texture_Wrap Wrap;     // How to wrap the texture if we read outside its bounds
	bool8 HasMipmaps;           // Whether the texture has smaller versions of itself generated
};

Texture Texture_Init(u32 Width,                  // Width in pixels
                     u32 Height,                 // Height in pixels
                     enum Texture_Format Format, // Format of the pixels
                     enum Texture_Filter Filter, // How to resize the texture
                     enum Texture_Wrap Wrap      // How to sample outside the texture
);
void Texture_SetData(Texture* Texture, // Target texture
                     const u8* Data,
                     u32 Width,
                     u32 Height,
                     bool8 GenMipmaps);
Texture Texture_FromFile(const char* File);
void Texture_Free(Texture);

// Render target type.
typedef struct RT RT;

// Render target.
struct RT {
	GLuint Id;            // OpenGL ID.
	u32 Width, Height;    // Width and height in pixels
	Texture Color, Depth; // Color and Depth attachments
};

/// Create a render target
RT RT_Init(u32 Width, u32 Height);

/// Create a render target using screen size
RT RT_InitScreenSize();

/// Free a render target and its attachments
void RT_Free(RT);

/// Make all render commands draw on the chosen render target
void RT_Use(RT Target);

/// Use the default render target, which is the actual screen.
void RT_UseDefault();

/// Copy the pixels of the source to the destination
/// @param[in] Source      Where to READ pixels from
/// @param[in] Destination Where to WRITE pixels to
void RT_Blit(RT Source, RT Destination);

/// Copy the pixels of the source to the screen
void RT_BlitToScreen(RT Source);

/// Reset the contents of a render target
void RT_Clear(RT);

/// Set a new size for the render target
void RT_SetSize(RT*, u32 Width, u32 Height);

/// Get the size of the current render target
Vec2 RT_GetCurrentSize();

/// Get the aspect ratio of the current render target
r32 RT_GetCurrentAspectRatio();

/// Get the size of the screen
Vec2 RT_GetScreenSize();

/// Get the aspect ratio of the screen
r32 RT_GetScreenAspectRatio();

//
// 2D
//

typedef struct Rect2D Rect2D;
typedef struct Tri2D Tri2D;
typedef struct Spritesheet Spritesheet;
typedef struct TextStyle TextStyle;

struct Rect2D {
	Vec2 Position;
	Vec2 Size;
	union {
		RGBA Color;
		Vec2 UVs[4];
	};
};

struct Tri2D {
	Vec2 Points[3];
	union {
		Vec2 UVs[3];
		RGBA Color;
	};
};

struct Spritesheet {
	const Texture* Texture;
	u32 SpriteWidth, SpriteHeight;
};

struct TextStyle {
	bool8 ClipEnabled;
	Rect2D Clip; // Cutoff rectangle.

	RGBA Color;              // Text color
	RGBA Background;         // Background behind letter
	bool8 BackgroundEnabled; // Whether the letters have a background

	bool8 WrapText;

	enum {
		Anchor_TopLeft,
		Anchor_TopMiddle,
		Anchor_TopRight,

		Anchor_CenterLeft,
		Anchor_Center,
		Anchor_CenterRight,

		Anchor_BottomLeft,
		Anchor_BottomMiddle,
		Anchor_BottomRight,
	} Anchor;

	enum { Align_Left, Align_Center, Align_Right } Align;

	const Spritesheet* Font;
};
extern const TextStyle TextStyle_Default;
extern Spritesheet Font_Small, Font_Medium, Font_Large;

void Tri2D_Draw(Tri2D Triangle);
void Tri2D_DrawMany(const Tri2D* Triangles, u32 NumTris);

void Rect2D_Draw(Rect2D rect, bool8 Fill); // Draw a Rect2D onto the screen.
void Rect2D_DrawMany(const Rect2D* Rects,
                     u32 NumRects,
                     bool8 Fill); // Draw many Rect2Ds onto the screen.
void Rect2D_DrawImage(Rect2D rect,
                      GLuint TextureID,
                      bool8 UseRectUVs); // Draw a Rect2D with an image inside.

Vec2 Text2D_Draw(Vec2 pos,
                 const TextStyle* style,
                 const char* fmt,
                 ...); // Shows text on screen, returns last pen location.
Vec2 Text2D_Size(const TextStyle* style,
                 const char* fmt,
                 ...); // Returns the width and height of some text.

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
	Shader* Shaders[R3D_Shader_NumShaders];
	i32 DebugMode;
	void* Scene;
};

void R3D_Init(); // Init 3D stuff, called by RSys_Init()
void R3D_DrawLine(Camera cam, Vec3 Start, Vec3 End, RGBA Color);
void R3D_DrawLines(Camera cam, Vec3* LinePoints, u32 NumLines, RGBA Color);
void R3D_DrawTriangle(Camera cam, Vec3 A, Vec3 B, Vec3 C, RGBA Color);
void R3D_DrawWireSphere(Camera cam, Vec3 Center, r32 Radius, RGBA Color);

extern Shader *R3D_Shader_UnlitColor, *R3D_Shader_UnlitTextured;

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

	RGB Color;

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
	const R3D_Node* Parent;
	Array_Node Children;

	Transform3D LocalTransform;

	AABB LocalBox; // AABB of just the stuff inside the node.
	AABB SumBox;   // An AABB around all of the node's children.

	enum R3D_Node_Type Type;

	union {
		Camera Camera;
		Actor Actor;
		Light Light;
	};
};

struct R3D_Scene {
	R3D_Node Root;
	// TODO: Give cameras parent nodes
	Camera* ActiveCamera;

	bool8 SunEnabled;
	struct {
		Vec3 Direction;
		RGB Color;
	} Sun;
};

R3D_Scene* Scene_Init();
void Scene_Render(R3D_Scene*); // Draw a scene.

R3D_Node* R3D_Node_Create(enum R3D_Node_Type);
R3D_Node* R3D_Node_AttachNew(enum R3D_Node_Type);
void R3D_CalcTransform(const R3D_Node*, Mat4 out);

#endif
