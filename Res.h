#ifndef RES_H
#define RES_H

#include "Common.h"
#include "Math3D.h"
#include "Phys.h"

struct Res_Mesh {
	u32 Id;
	char *Name;

	u32 NumVertices;
	Vec3 *Points;
	Vec2 *UVs;
	Vec3 *Normals;

	u32 NumIndices;
	u32 *Indices;
};

struct Res_Image {
	u32 Id;
	char *Name;

	u32 Width, Height;
	u32 BitsPerPixel;
	u32 NumComponents;

	u8 *Data;
};

struct Res_Material {
	u32 Id;
	char *Name;

	RGB AmbientColor;
	RGB DiffuseColor;
	RGB SpecularColor;
	r32 SpecularExponent;
	r32 Alpha;

	struct Res_Image *AmbientMap;
	struct Res_Image *DiffuseMap;
	struct Res_Image *SpecularMap;
	struct Res_Image *SpecularExponentMap;
	struct Res_Image *NormalMap;
};

struct Res_Sound {
	u32 Id;

	u8 *Data;
	u32 Size;
};

void Res_Init();
void Res_SyncLoad(const char* file);
void Res_AddLoadJob(const char* file);

#endif
