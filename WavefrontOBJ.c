#include "WavefrontOBJ.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO: Replace atof() with custom function.

#define READ_NEXT_WORD(tgt)                                                   \
	{                                                                         \
		while(Char_IsNewline(Buffer[i]) || Char_IsWhitespace(Buffer[i])) ++i; \
		if(i >= Size) break;                                                  \
		i32 j = i;                                                            \
		while(!Char_IsNewline(Buffer[++j]) && !Char_IsWhitespace(Buffer[j]))  \
			;                                                                 \
		strncpy(tgt, (char *) Buffer + i, j - i);                             \
		i = j;                                                                \
	}
#define SKIP_TO_NEXT_LINE()                 \
	{                                       \
		while(!Char_IsNewline(Buffer[++i])) \
			;                               \
		++i;                                \
		continue;                           \
	}

bool32 Char_IsWhitespace(char c) { return c == '\t' || c == ' '; }
bool32 Char_IsNewline(char c) { return c == '\r' || c == '\n'; }

bool32 Char_IsLetter(char c) {
	return ('A' >= c && c <= 'Z') || ('a' >= c && c <= 'z');
}
bool32 Char_IsDigit(char c) { return '0' >= c && c <= '9'; }

void WObj_ReadMtl(const char *filename, Array *MaterialsArray) {
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
			READ_NEXT_WORD(NewMaterial.Name);
			Array_Push(MaterialsArray, (u8 *) &NewMaterial);
			CurrMat = (WObj_Material *) Array_GetLast(MaterialsArray);
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
		} else if(strncmp(Command, "map_Ka", 6) == 0) {
			READ_NEXT_WORD(CurrMat->AmbientMapFile);
		} else if(strncmp(Command, "map_Kd", 6) == 0) {
			READ_NEXT_WORD(CurrMat->DiffuseMapFile);
		} else if(strncmp(Command, "map_Ks", 6) == 0) {
			READ_NEXT_WORD(CurrMat->SpecularMapFile);
		} else if(strncmp(Command, "map_Ns", 6) == 0) {
			READ_NEXT_WORD(CurrMat->SpecularExponentMapFile);
		} else if(strncmp(Command, "map_d", 5) == 0) {
			READ_NEXT_WORD(CurrMat->OpacityMapFile);
		} else if(strncmp(Command, "bump", 6) == 0) {
			READ_NEXT_WORD(CurrMat->NormalMapFile);
		} else if(strncmp(Command, "illum", 5) == 0) {
			char value[8] = {0};
			READ_NEXT_WORD(value);
			CurrMat->IllumMode = String_ToI32(value);
		} else {
			Log(Log_Error, "Unknown MTL command %s", Command);
			SKIP_TO_NEXT_LINE();
		}
	}

	free(Buffer);
}

struct FaceVertex {
	i32 PosId;
	bool32 HasUV;
	i32 UVId;
	bool32 HasNormal;
	i32 NormalId;
};
struct Face {
	Array *FaceVertices;
};
struct Object {
	WObj_Material *Material;

	char Name[256];
	Array *Faces;
};

void Object_Init(struct Object *o) {
	memset(o, 0, sizeof(struct Object));
	o->Faces = Array_Init(sizeof(struct Face));
}
void Object_Free(struct Object *o) {
	if(!o) return;
	for(u32 j = 0; j < o->Faces->ArraySize; j++) {
		struct Face *f = (struct Face *) Array_Get(o->Faces, j);
		Array_Free(f->FaceVertices);
	}
	Array_Free(o->Faces);
}

void Face_Init(struct Face *f) {
	f->FaceVertices = Array_Init(sizeof(struct FaceVertex));
}

// TODO: Fix memory leaks
WObj_Library *WObj_FromFile(const char *filename) {
	u8 *Buffer = malloc(Megabytes(1));
	u32 Size;
	if(!File_ReadToBuffer(filename, Buffer, Megabytes(1), &Size)) {
		return NULL;
	}

	WObj_Library *res = malloc(sizeof(WObj_Library));

	struct Object *CurrObject = NULL;

	Array *Positions = NULL;
	Array *Normals = NULL;
	Array *UVs = NULL;

	Positions = Array_Init(sizeof(Vec3));
	Normals = Array_Init(sizeof(Vec3));
	UVs = Array_Init(sizeof(Vec2));

	// TODO: Figure out what this is supposed to do.
	// bool32 SmoothingEnabled = 0;

	Array *Materials = Array_Init(sizeof(WObj_Material));
	Array *Objects = Array_Init(sizeof(struct Object));

	u32 i = 0;
	while(i < Size) {
		if(Buffer[i] == '#') { SKIP_TO_NEXT_LINE(); }

		char Command[64] = {0};
		READ_NEXT_WORD(Command);

		if(strncmp(Command, "o", 1) == 0) {
			struct Object obj;
			Object_Init(&obj);
			READ_NEXT_WORD(obj.Name);
			Array_Push(Objects, (u8 *) &obj);
			CurrObject = (struct Object *) Array_GetLast(Objects);
		} else if(strncmp(Command, "mtllib", 6) == 0) {
			char MtlFilename[256] = {0};
			strncpy(MtlFilename, filename, 256);
			char *Dir = strrchr(MtlFilename, '/');
			READ_NEXT_WORD(Dir + 1);
			WObj_ReadMtl(MtlFilename, Materials);
		} else if(strncmp(Command, "usemtl", 6) == 0) {
			char MaterialName[256] = {0};
			READ_NEXT_WORD(MaterialName);

			WObj_Material *FoundMaterial = NULL;
			for(u32 i = 0; i < Materials->ArraySize; i++) {
				WObj_Material *mat = (WObj_Material *) Array_Get(Materials, i);
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

			Array_Push(Positions, (u8 *) &pos);
		} else if(strncmp(Command, "vt", 2) == 0) {
			Vec2 UV;
			char u[32] = {0};
			char v[32] = {0};

			READ_NEXT_WORD(u);
			READ_NEXT_WORD(v);
			UV.u = atof(u);
			UV.v = atof(v);

			Array_Push(UVs, (u8 *) &UV);
		} else if(strncmp(Command, "vn", 2) == 0) {
			Vec3 normal;
			char x[32] = {0};
			char y[32] = {0};
			char z[32] = {0};

			READ_NEXT_WORD(x);
			READ_NEXT_WORD(y);
			READ_NEXT_WORD(z);
			normal.x = atof(x);
			normal.y = atof(y);
			normal.z = atof(z);

			Array_Push(Normals, (u8 *) &normal);
		} else if(strncmp(Command, "f", 1) == 0) {
			struct Face Face;
			Face.FaceVertices = Array_Init(sizeof(struct FaceVertex));

			while(!Char_IsNewline(Buffer[i])) {
				struct FaceVertex FV;
				FV.HasUV = FV.HasNormal = 0;

				char Vertex[32] = {0};
				READ_NEXT_WORD(Vertex);

				i32 j = -1, q = 0;
				// First pass, read position index
				while(Vertex[++j] && Vertex[j] != '/')
					;
				FV.PosId = String_ToI32_N(Vertex, j) - 1;

				// There's no point in going further, skip to end.
				if(Vertex[j] != '/') { goto face_vertex_end; }

				// Second pass, check to see if there's a UV index.
				q = j;
				while(Vertex[++j] && Vertex[j] != '/')
					;
				if(q + 1 != j) {
					q++;
					FV.HasUV = 1;
					FV.UVId = String_ToI32_N(Vertex + q, j - q) - 1;
				}

				// There's no point in going further, skip to end.
				if(Vertex[j] != '/') { goto face_vertex_end; }

				// Third and final pass, check for normals.
				q = ++j;
				while(Vertex[++j])
					;
				FV.HasNormal = 1;
				FV.NormalId = String_ToI32_N(Vertex + q, j - q) - 1;

			face_vertex_end:
				Array_Push(Face.FaceVertices, (u8 *) &FV);
			}

			Array_Push(CurrObject->Faces, (u8 *) &Face);
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

	Array *Vertices = Array_Init(sizeof(WObj_Vertex));
	Array *Indices = Array_Init(sizeof(u32));

	Array *FinalObjects = Array_Init(sizeof(WObj_Object));

	for(u32 i = 0; i < Objects->ArraySize; i++) {
		struct Object *obj = (struct Object *) Array_Get(Objects, i);
		u32 NextIndex = 0;

		for(u32 i = 0; i < obj->Faces->ArraySize; i++) {
			Array *FVs =
			    ((struct Face *) Array_Get(obj->Faces, i))->FaceVertices;

			for(u32 j = 0; j < FVs->ArraySize; j++) {
				struct FaceVertex *v = (struct FaceVertex *) Array_Get(FVs, j);

				i32 FoundVertexId = -1;
				// Check to see if this vertex exists.
				for(u32 q = 0; q < j; q++) {
					struct FaceVertex *v2 =
					    (struct FaceVertex *) Array_Get(FVs, q);
					if(v->PosId == v2->PosId) {
						FoundVertexId = q;
						break;
					}
				}

				// If it doesn't, add it and the next index.
				if(FoundVertexId == -1) {
					WObj_Vertex vtx;

					vtx.Position = *((Vec3 *) Array_Get(Positions, v->PosId));
					if(v->HasUV) vtx.UV = *((Vec2 *) Array_Get(UVs, v->UVId));
					if(v->HasNormal)
						vtx.Normal =
						    *((Vec3 *) Array_Get(Normals, v->NormalId));

					Array_Push(Vertices, (u8 *) &vtx);
					Array_Push(Indices, (u8 *) &NextIndex);
					NextIndex++;
				}
				// Else, just add an index to the original.
				else {
					Array_Push(Indices, (u8 *) &NextIndex);
				}
			}
		}

		WObj_Object Object;
		Object.Material = obj->Material;
		Object.NumVertices = Vertices->ArraySize;
		Object.NumIndices = Indices->ArraySize;
		Object.Vertices = malloc(Vertices->ArraySize * sizeof(WObj_Vertex));
		Object.Indices = malloc(Indices->ArraySize * sizeof(u32));
		Array_CopyData((u8 *) Object.Vertices, Vertices);
		Array_CopyData((u8 *) Object.Indices, Indices);

		Array_Push(FinalObjects, (u8 *) &Object);

		Array_Clear(Vertices);
		Array_Clear(Indices);
	}

	res->NumObjects = Objects->ArraySize;
	res->NumMaterials = Materials->ArraySize;
	res->Materials = (WObj_Material *) Materials->Data;
	res->Objects = (WObj_Object *) FinalObjects->Data;

	Materials->Data = NULL;
	Array_Free(Materials);

	FinalObjects->Data = NULL;
	Array_Free(FinalObjects);

	Array_Free(Vertices);
	Array_Free(Indices);
	Array_Free(Positions);
	Array_Free(UVs);
	Array_Free(Normals);

	for(u32 i = 0; i < Objects->ArraySize; i++) {
		struct Object *o = (struct Object *) Array_Get(Objects, i);
		Object_Free(o);
	}
	Array_Free(Objects);


	return res;
}
