#include "../GLTF.h"
#include "../Common.h"
#include "../JSON.h"

#include <stdlib.h>
#include <assert.h>

GLTF_Asset* GLTF_LoadFile(const char* filename) {
	JSON_Value v = JSON_FromFile(filename);

	if (v.Type == JSON_Error) {
		Log(ERROR, "[GLTF] Error reading %s.", filename);
		return NULL;
	}

	if(v.Type != JSON_Object) {
		Log(ERROR, "[GLTF] %s doesn't seem to contain valid GLTF.", filename);
		return NULL;
	}

	GLTF_Asset* res = malloc(sizeof(GLTF_Asset));

	for(u32 i = 0; i < v.Object.StringKeys.Size; i++) {
		const char* key = v.Object.StringKeys.Data[i];
		const JSON_Value* val = &v.Object.Map.Values[i];

		if(strcmp(key, "asset"))
		{
			assert(val->Type == JSON_Object);
		}
		else if(strcmp(key, "scene"))
		{
			assert(val->Type == JSON_Number);
			res->firstScene = &res->scenes[(i32) val->Number];
		}
		else if(strcmp(key, "scenes"))
		{
			assert(val->Type == JSON_Array);
			
			for (u32 j = 0; j < val->Array.Size; j++)
			{
				//JSON_Value* scene = &val->Array.Data[j];
			}
		}
	}


	JSON_Free(&v);
	return res;
}
