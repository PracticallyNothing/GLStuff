#ifndef GLTF_H
#define GLTF_H

#include "Common.h"

// TODO: Finish reading the spec
//       (https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#concepts)

typedef struct GLTF_Node GLTF_Node;
typedef struct GLTF_Scene GLTF_Scene;
typedef struct GLTF_Asset GLTF_Asset;

//struct GLTF_

struct GLTF_Node {
	u32* childrenIdx;
	u32 numChildren;

	const char* name;
};

struct GLTF_Scene {
	GLTF_Node* root;
};

// The glTF asset contains zero or more scenes.
// It may also contain an optional target scene, which is the scene to be shown
// first, after the asset has been loaded.
struct GLTF_Asset {
	u32 numScenes;
	u32 numNodes;

	GLTF_Scene* scenes;
	GLTF_Node* nodes;

	// Index of first scene to show after loading has finished.
	u32 firstSceneIdx;
};

GLTF_Asset* GLTF_LoadFile(const char* filename);

#endif
