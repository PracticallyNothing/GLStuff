#include "WavefrontOBJ.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Common.h"

// TODO: Replace atof() with custom function.

#define READ_NEXT_WORD(tgt) {                                             \
	while(Char_IsNewline(Buffer[i]) || Char_IsWhitespace(Buffer[i])) ++i; \
	if(i >= Size) break;                                                  \
	i32 j = i;                                                            \
	while(!Char_IsNewline(Buffer[++j]) && !Char_IsWhitespace(Buffer[j])); \
	strncpy(tgt, (char *) Buffer + i, j - i);                             \
	i = j;                                                                \
}

#define READ_ALLOC_NEXT_WORD(tgt) {                                       \
	while(Char_IsNewline(Buffer[i]) || Char_IsWhitespace(Buffer[i])) ++i; \
	if(i >= Size) break;                                                  \
	i32 j = i;                                                            \
	while(!Char_IsNewline(Buffer[++j]) && !Char_IsWhitespace(Buffer[j])); \
	tgt = calloc(sizeof(char), j-i+1);                                    \
	strncpy(tgt, (char *) Buffer + i, j - i);                             \
	i = j;                                                                \
}
#define SKIP_TO_NEXT_LINE() {           \
	while(!Char_IsNewline(Buffer[++i]));\
	++i;                                \
	continue;                           \
}

bool32 Char_IsWhitespace(char c) { return c == '\t' || c == ' '; }
bool32 Char_IsNewline(char c)    { return c == '\r' || c == '\n'; }
bool32 Char_IsLetter(char c)     { return ('A' >= c && c <= 'Z') || ('a' >= c && c <= 'z'); }
bool32 Char_IsDigit(char c)      { return '0' >= c && c <= '9'; }

DEF_ARRAY(WMat,  struct WObj_Material);
DEF_ARRAY(WObj,  struct WObj_Object);
DEF_ARRAY(WVert, struct WObj_Vertex);

DECL_ARRAY(WMat,  struct WObj_Material);
DECL_ARRAY(WObj,  struct WObj_Object);
DECL_ARRAY(WVert, struct WObj_Vertex);

void WObj_ReadMtl(const char *filename, struct Array_WMat *Mats) {
	u8 *Buffer = malloc(Kilobytes(256));
	u32 Size = 0;
	File_ReadToBuffer(filename, Buffer, Kilobytes(256), &Size);
	WObj_Material *CurrMat = NULL;

	u32 i = 0;
	while(i < Size) {
		if(Buffer[i] == '#') { SKIP_TO_NEXT_LINE(); }

		char Command[64] = {0};
		READ_NEXT_WORD(Command);

		if(strncmp(Command, "newmtl", 6) == 0) {
			WObj_Material NewMaterial;
			memset(&NewMaterial, 0, sizeof(WObj_Material));
			READ_ALLOC_NEXT_WORD(NewMaterial.Name);
			Array_WMat_Push(Mats, &NewMaterial);
			CurrMat = (WObj_Material *) Mats->Data + (Mats->Size - 1);
		} else if(strncmp(Command, "Ka", 2) == 0) {
			char r[32] = {0};
			char g[32] = {0};
			char b[32] = {0};
			READ_NEXT_WORD(r);
			READ_NEXT_WORD(g);
			READ_NEXT_WORD(b);
			CurrMat->AmbientColor.r = atof(r);
			CurrMat->AmbientColor.g = atof(g);
			CurrMat->AmbientColor.b = atof(b);
		} else if(strncmp(Command, "Kd", 2) == 0) {
			char r[32] = {0};
			char g[32] = {0};
			char b[32] = {0};
			READ_NEXT_WORD(r);
			READ_NEXT_WORD(g);
			READ_NEXT_WORD(b);
			CurrMat->DiffuseColor.r = atof(r);
			CurrMat->DiffuseColor.g = atof(g);
			CurrMat->DiffuseColor.b = atof(b);
		} else if(strncmp(Command, "Ks", 2) == 0) {
			char r[32] = {0};
			char g[32] = {0};
			char b[32] = {0};
			READ_NEXT_WORD(r);
			READ_NEXT_WORD(g);
			READ_NEXT_WORD(b);
			CurrMat->SpecularColor.r = atof(r);
			CurrMat->SpecularColor.g = atof(g);
			CurrMat->SpecularColor.b = atof(b);
		} else if(strncmp(Command, "Ns", 2) == 0) {
			char value[32] = {0};
			READ_NEXT_WORD(value);
			CurrMat->SpecularExponent = atof(value);
		} else if(strncmp(Command, "Ni", 2) == 0) {
			char value[32] = {0};
			READ_NEXT_WORD(value);
			CurrMat->OpticalDensity = atof(value);
		} else if(strncmp(Command, "sharpness", 9) == 0) {
			char value[32] = {0};
			READ_NEXT_WORD(value);
			CurrMat->Sharpness = atof(value);
		} else if(strncmp(Command, "Ke", 2) == 0) {
			// Unsupported tag.
			// (I have no idea what it does. The MTL spec doesn't mention it.
			//  I assume it has something to do with emission, but even if it
			//  does, real-time emissive surfaces aren't something I'm even
			//  going to try and figure out, at least not for now.)
			SKIP_TO_NEXT_LINE();
		} else if(strncmp(Command, "d", 1) == 0) {
			char value[32] = {0};
			READ_NEXT_WORD(value);
			CurrMat->Opacity = atof(value);
		} else if(strncmp(Command, "Tr", 2) == 0) {
			char value[32] = {0};
			READ_NEXT_WORD(value);
			CurrMat->Opacity = 1.0 - atof(value);
		} else if(strncmp(Command, "Tf", 2) == 0) {
			char r[32] = {0};
			char g[32] = {0};
			char b[32] = {0};
			READ_NEXT_WORD(r);
			READ_NEXT_WORD(g);
			READ_NEXT_WORD(b);
			CurrMat->TransmissionFilter.r = atof(r);
			CurrMat->TransmissionFilter.g = atof(g);
			CurrMat->TransmissionFilter.b = atof(b);
		} 
		else if(strncmp(Command, "map_Ka", 6) == 0) { READ_ALLOC_NEXT_WORD(CurrMat->AmbientMapFile); } 
		else if(strncmp(Command, "map_Kd", 6) == 0) { READ_ALLOC_NEXT_WORD(CurrMat->DiffuseMapFile); }
		else if(strncmp(Command, "map_Ks", 6) == 0) { READ_ALLOC_NEXT_WORD(CurrMat->SpecularMapFile); }
		else if(strncmp(Command, "map_Ns", 6) == 0) { READ_ALLOC_NEXT_WORD(CurrMat->SpecularExponentMapFile); }
		else if(strncmp(Command, "map_d",  5) == 0) { READ_ALLOC_NEXT_WORD(CurrMat->OpacityMapFile); } 
		else if(strncmp(Command, "bump",   6) == 0) { READ_ALLOC_NEXT_WORD(CurrMat->NormalMapFile); } 
		else if(strncmp(Command, "illum", 5) == 0) {
			char value[8] = {0};
			READ_NEXT_WORD(value);
			CurrMat->IllumMode = String_ToI32(value);
		} else {
			Log(Log_Error, "Unknown MTL directive %s", Command);
			SKIP_TO_NEXT_LINE();
		}
	}

	free(Buffer);
}

struct FaceVertex {
	i32 PosId;
	i32 UVId;
	i32 NormalId;

	bool32 HasUV;
	bool32 HasNormal;
};
DEF_ARRAY(FaceVert, struct FaceVertex);
DECL_ARRAY(FaceVert, struct FaceVertex);

DEF_ARRAY(Face, struct Array_FaceVert);
DECL_ARRAY(Face, struct Array_FaceVert);

struct Object {
	WObj_Material *Material;

	char Name[256];
	struct Array_Face Faces;
};
 DEF_ARRAY(Obj, struct Object);
DECL_ARRAY(Obj, struct Object);


void Object_Init(struct Object *o) {
	memset(o, 0, sizeof(struct Object));
}
void Object_Free(struct Object *o) {
	if(!o) return;
	for(u32 j = 0; j < o->Faces.Size; j++)
		Array_FaceVert_Free(o->Faces.Data + j);
	Array_Face_Free(&o->Faces);
}

DEF_ARRAY(Vec2, Vec2);
DEF_ARRAY(Vec3, Vec3);

DECL_ARRAY(Vec2, Vec2);
DECL_ARRAY(Vec3, Vec3);

// TODO: Fix memory leaks
WObj_Library *WObj_FromFile(const char *filename) {
	u8 *Buffer = malloc(Megabytes(1));
	u32 Size;
	if(!File_ReadToBuffer(filename, Buffer, Megabytes(1), &Size)) {
		return NULL;
	}

	WObj_Library *res = malloc(sizeof(WObj_Library));

	struct Object *CurrObject = NULL;

	struct Array_Vec3 Positions;
	struct Array_Vec3 Normals;
	struct Array_Vec2 UVs;

	// TODO: Figure out what this is supposed to do.
	// bool32 SmoothingEnabled = 0;

	struct Array_WMat Materials; 
	struct Array_Obj Objects;

	u32 i = 0;
	while(i < Size) {
		if(Buffer[i] == '#') { SKIP_TO_NEXT_LINE(); }

		char Command[64] = {0};
		READ_NEXT_WORD(Command);

		if(strncmp(Command, "o", 1) == 0) {
			struct Object obj;
			Object_Init(&obj);
			READ_NEXT_WORD(obj.Name);
			Array_Obj_Push(&Objects, &obj);
			CurrObject = Objects.Data + (Objects.Size - 1);
		} else if(strncmp(Command, "mtllib", 6) == 0) {
			char MtlFilename[256] = {0};
			strncpy(MtlFilename, filename, 256);
			char *Dir = strrchr(MtlFilename, '/');
			READ_NEXT_WORD(Dir + 1);
			WObj_ReadMtl(MtlFilename, &Materials);
		} else if(strncmp(Command, "usemtl", 6) == 0) {
			char MaterialName[256] = {0};
			READ_NEXT_WORD(MaterialName);

			WObj_Material *FoundMaterial = NULL;
			for(u32 i = 0; i < Materials.Size; i++) {
				WObj_Material *mat = Materials.Data + i;
				if(strcmp(mat->Name, MaterialName) == 0) {
					FoundMaterial = mat;
					break;
				}
			}

			if(!FoundMaterial)
				Log(Log_Error, "OBJ with nonexistent material \"%s\"",
						MaterialName);
			else
				CurrObject->Material = FoundMaterial;

		} else if(strncmp(Command, "v\0", 2) == 0) {
			Vec3 pos;
			char x[32] = {0};
			char y[32] = {0};
			char z[32] = {0};
			READ_NEXT_WORD(x);
			READ_NEXT_WORD(y);
			READ_NEXT_WORD(z);
			pos.x = atof(x);
			pos.y = atof(y);
			pos.z = atof(z);

			Array_Vec3_Push(&Positions, &pos);
		} else if(strncmp(Command, "vt", 2) == 0) {
			char u[32] = {0};
			char v[32] = {0};
			READ_NEXT_WORD(u);
			READ_NEXT_WORD(v);

			Vec2 UV = V2(atof(u), atof(v));
			Array_Vec2_Push(&UVs, &UV);
		} else if(strncmp(Command, "vn", 2) == 0) {
			char x[32] = {0};
			char y[32] = {0};
			char z[32] = {0};
			READ_NEXT_WORD(x);
			READ_NEXT_WORD(y);
			READ_NEXT_WORD(z);

			Vec3 Normal = V3(atof(x), atof(y), atof(z));
			Array_Vec3_Push(&Normals, &Normal);
		} else if(strncmp(Command, "f", 1) == 0) {
			struct Array_FaceVert FaceVerts;

			while(!Char_IsNewline(Buffer[i])) {
				struct FaceVertex FV;
				FV.HasUV = FV.HasNormal = 0;

				char Vertex[32] = {0};
				READ_NEXT_WORD(Vertex);

				i32 j = -1, q = 0;
				// First pass, read position index
				while(Vertex[++j] && Vertex[j] != '/');
				FV.PosId = String_ToI32_N(Vertex, j) - 1;

				// There's no point in going further, skip to end.
				if(Vertex[j] != '/') { goto face_vertex_end; }

				// Second pass, check to see if there's a UV index.
				q = j;
				while(Vertex[++j] && Vertex[j] != '/');

				if(q + 1 != j) {
					q++;
					FV.HasUV = 1;
					FV.UVId = String_ToI32_N(Vertex + q, j - q) - 1;
				}

				// There's no point in going further, skip to end.
				if(Vertex[j] != '/') { goto face_vertex_end; }

				// Third and final pass, check for normals.
				q = ++j;
				while(Vertex[++j]);

				FV.HasNormal = 1;
				FV.NormalId = String_ToI32_N(Vertex + q, j - q) - 1;

face_vertex_end:
				if(FaceVerts.Size == 3) {
					Log(ERR, "OBJ file \"%s\" has a non-triangulated face.", filename);
				}
				Array_FaceVert_Push(&FaceVerts, &FV);
			}

			Array_Face_Push(&CurrObject->Faces, &FaceVerts);
		} else if(strncmp(Command, "s", 1) == 0) {
			char word[64] = {0};
			READ_NEXT_WORD(word);
			// SmoothingEnabled = (strncmp(word, "on", 2) == 0);
		} else {
			Log(Log_Error, "Unknown OBJ directive %s.", Command);
			SKIP_TO_NEXT_LINE();
		}
	}
	free(Buffer);

	// Stage 2:
	// Take all the vertices with IDs and convert them to vertices with values.
	// If a vertex already exists, add an index to it.

	struct Array_WVert Vertices;
	struct Array_u32 Indices;

	struct Array_WObj FinalObjects;

	for(u32 i = 0; i < Objects.Size; i++) {
		struct Object *obj = Objects.Data + i;
		u32 NextIndex = 0;

		for(u32 i = 0; i < obj->Faces.Size; i++) {
			struct Array_FaceVert FVs = obj->Faces.Data[i];

			for(u32 j = 0; j < FVs.Size; j++) {
				struct FaceVertex v = FVs.Data[j];

				i32 FoundVertexId = -1;
				// Check to see if this vertex exists.
				for(u32 q = 0; q < j; q++) {
					struct FaceVertex v2 = FVs.Data[q];
					if(v.PosId == v2.PosId) {
						FoundVertexId = q;
						break;
					}
				}

				// If it doesn't, add it and the next index.
				if(FoundVertexId == -1) {
					WObj_Vertex vtx;

					vtx.Position = Positions.Data[v.PosId];
					if(v.HasUV) vtx.UV = UVs.Data[v.UVId];
					if(v.HasNormal) vtx.Normal = Normals.Data[v.NormalId];

					Array_WVert_Push(&Vertices, &vtx);
					Array_u32_Push(&Indices, &NextIndex);
					NextIndex++;
				}
				// Else, just add an index to the original.
				else {
					Array_u32_Push(&Indices, &NextIndex);
				}
			}
		}
		Array_WVert_SizeToFit(&Vertices);
		Array_u32_SizeToFit(&Indices);

		WObj_Object Object;
		Object.Name = malloc(strlen(obj->Name));
		strcpy(Object.Name, obj->Name);
		Object.Material = obj->Material;
		Object.NumVertices = Vertices.Size;
		Object.NumIndices = Indices.Size;
		Object.Vertices = Vertices.Data;
		Object.Indices = Indices.Data;

		Array_WObj_Push(&FinalObjects, &Object);

		Vertices.Data = NULL;
		Vertices.Size = 0;
		Vertices.Capacity = 0;

		Indices.Data = NULL;
		Indices.Size = 0;
		Indices.Capacity = 0;
	}

	Array_WObj_SizeToFit(&FinalObjects);
	Array_WMat_SizeToFit(&Materials);

	res->NumObjects = FinalObjects.Size;
	res->NumMaterials = Materials.Size;
	res->Materials = Materials.Data;
	res->Objects = FinalObjects.Data;

	Array_Vec3_Free(&Positions);
	Array_Vec2_Free(&UVs);
	Array_Vec3_Free(&Normals);

	for(u32 i = 0; i < Objects.Size; i++)
		Object_Free(Objects.Data + i);
	Array_Obj_Free(&Objects);

	return res;
}
