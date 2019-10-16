#ifndef WAVEFRONT_OBJ_H
#define WAVEFRONT_OBJ_H

#include "Common.h"
#include "Math3D.h"

enum WObj_IllumMode {
	WObj_IllumMode_ColorOnAmbientOff = 0,
	WObj_IllumMode_ColorOnAmbientOn = 1,
	WObj_IllumMode_HighlightOn = 2,
	WObj_IllumMode_ReflectOnRaytraceOn = 3,
	WObj_IllumMode_Transparency = 4,
	WObj_IllumMode_ReflectOnRaytraceOff = 8,
	WObj_IllumMode_ShadowInvisSurface = 10,
};

typedef struct WObj_Material {
	char Name[256];  // newmtl matName

	RGB AmbientColor;      // Ka <r> [<g> <b>]
	RGB DiffuseColor;      // Kd <r> [<g> <b>]
	RGB SpecularColor;     // Ks <r> [<g> <b>]
	r32 SpecularExponent;  // Ns 1.00

	i32 OpacityHalo;         // d -halo <value>
	r32 Opacity;             // d <value> | Tr <value> (Tr = 1 - d)
	RGB TransmissionFilter;  // Tf <r> [<g> <b>]

	r32 OpticalDensity;  // Ni <value>
	r32 Sharpness;       // sharpness <value>

	char AmbientMapFile[256];           // map_Ka <file>
	char DiffuseMapFile[256];           // map_Kd <file>
	char SpecularMapFile[256];          // map_Ks <file>
	char SpecularExponentMapFile[256];  // map_Ns <file>
	char OpacityMapFile[256];           // map_d <file>
	char NormalMapFile[256];            // bump <file>

	enum WObj_IllumMode IllumMode;  // illum <mode>
} WObj_Material;

typedef struct WObj_Vertex {
	Vec3 Position;
	Vec2 UV;
	Vec3 Normal;
} WObj_Vertex;

typedef struct WObj_Object {
	WObj_Material *Material;

	u32 NumVertices;
	WObj_Vertex *Vertices;

	u32 NumIndices;
	u32 *Indices;
} WObj_Object;

typedef struct WObj_Library {
	u32 NumMaterials;
	WObj_Material *Materials;

	u32 NumObjects;
	WObj_Object *Objects;
} WObj_Library;

extern WObj_Library *WObj_FromFile(const char *filename);

#endif
