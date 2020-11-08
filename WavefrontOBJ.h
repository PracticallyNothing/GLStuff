#ifndef WAVEFRONT_OBJ_H
#define WAVEFRONT_OBJ_H

#include "Common.h"
#include "Math3D.h"

// WARNING:
//   This importer assumes the following:
//   	- All vertices, uv coordinates and normals are given before any face declarations
//   	- 

enum WObj_IllumMode {
	WObj_IllumMode_ColorOnAmbientOff = 0,
	WObj_IllumMode_ColorOnAmbientOn = 1,
	WObj_IllumMode_HighlightOn = 2,
	WObj_IllumMode_ReflectOnRaytraceOn = 3,
	WObj_IllumMode_Transparency = 4,
	WObj_IllumMode_ReflectOnRaytraceOff = 8,
	WObj_IllumMode_ShadowInvisSurface = 10,
};

typedef struct WObj_Material WObj_Material;
typedef struct WObj_Vertex   WObj_Vertex;
typedef struct WObj_Object   WObj_Object;
typedef struct WObj_Library  WObj_Library;

struct WObj_Material {
	char *Name;  // newmtl matName

	RGB AmbientColor;      // Ka <r> [<g> <b>]
	RGB DiffuseColor;      // Kd <r> [<g> <b>]
	RGB SpecularColor;     // Ks <r> [<g> <b>]
	r32 SpecularExponent;  // Ns 1.00

	i32 OpacityHalo;         // d -halo <value>
	r32 Opacity;             // d <value> | Tr <value> (Tr = 1 - d)
	RGB TransmissionFilter;  // Tf <r> [<g> <b>]

	r32 OpticalDensity;  // Ni <value>
	r32 Sharpness;       // sharpness <value>

	char *AmbientMapFile;           // map_Ka <file>
	char *DiffuseMapFile;           // map_Kd <file>
	char *SpecularMapFile;          // map_Ks <file>
	char *SpecularExponentMapFile;  // map_Ns <file>
	char *OpacityMapFile;           // map_d <file>
	char *NormalMapFile;            // bump <file>

	enum WObj_IllumMode IllumMode;  // illum <mode>
};


struct WObj_Vertex {
	Vec3 Position;
	Vec2 UV;
	Vec3 Normal;
};

struct WObj_Object {
	char *Name;

	WObj_Material *Material;

	u32       NumVertices;
	WObj_Vertex *Vertices;

	u32  NumIndices;
	u32*    Indices;
};

struct WObj_Library {
	u32 NumMaterials;
	WObj_Material *Materials;

	u32 NumObjects;
	WObj_Object *Objects;
};

extern WObj_Library *WObj_FromFile(const char *filename);
extern void WObj_Library_Free(WObj_Library *);

#endif
