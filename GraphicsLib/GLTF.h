#ifndef GLTF_H
#define GLTF_H

#include "Common.h"
#include "Math3D.h"

// TODO: Finish reading the spec
//       (https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#concepts)
//
// The units for all linear distances are meters.
// All angles are in radians.
// Positive rotation is counterclockwise.

typedef struct GLTF_Accessor GLTF_Accessor;
typedef struct GLTF_BufferView GLTF_BufferView;
typedef struct GLTF_Buffer GLTF_Buffer;
typedef struct GLTF_Node GLTF_Node;
typedef struct GLTF_Scene GLTF_Scene;
typedef struct GLTF_Asset GLTF_Asset;

struct GLTF_Accessor {
	const char* name;

	GLTF_BufferView* bufferView;
	u32 byteOffset;

	enum {
		Component_Byte          = 5120,
		Component_UnsignedByte  = 5121,
		Component_Short         = 5122,
		Component_UnsignedShort = 5123,
		Component_UnsignedInt   = 5125,
		Component_Float         = 5126
	} componentType;

	bool8 normalized;

	u32 count;

	enum {
		Type_Scalar,

		Type_Vec2,
		Type_Vec3,
		Type_Vec4,

		Type_Mat2,
		Type_Mat3,
		Type_Mat4,
	} type;

	bool8 hasMinMax;
	union {
		r32 scalar;

		Vec2 vec2;
		Vec3 vec3;
		Vec4 vec4;

		Mat2 mat2;
		Mat3 mat3;
		Mat4 mat4;
	} min, max;
};

struct GLTF_BufferView {
	GLTF_Buffer* buffer;
	u32 offset;
	u32 length;
	u32 target;
};

struct GLTF_Buffer {
	const char* name;
	u32 size;
	u8* data;
};

struct GLTF_Node {
	GLTF_Node** children;
	u32 numChildren;

	const char* name;

	enum { Transform_TRS, Transform_Mat4 } transformType;

	union {
		// Translation, rotation, scale
		Transform3D trs;
		// A 4x4 matrix
		Mat4 matrix;
	};
};

struct GLTF_Scene {
	GLTF_Node** rootNodes;
};

// The glTF asset contains zero or more scenes.
// It may also contain an optional target scene, which is the scene to be shown
// first, after the asset has been loaded.
struct GLTF_Asset {
	u32 numScenes;
	u32 numNodes;

	GLTF_Scene* scenes;
	GLTF_Node* nodes;

	// Scene to show after loading has finished.
	GLTF_Scene* firstScene;
};

GLTF_Asset* GLTF_LoadFile(const char* filename);

#endif
