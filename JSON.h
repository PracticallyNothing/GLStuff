#ifndef JSON_H
#define JSON_H

#include "Common.h"

typedef struct JSON_Value JSON_Value;

DEF_ARRAY(Hash, u128);
DEF_ARRAY(String, char *);
DEF_ARRAY(JSON_Value, JSON_Value);
DEF_HASHMAP(JSON_Value, JSON_Value);

struct JSON_Value {
	enum {
		JSON_Error = -1,

		JSON_Null,

		JSON_Boolean,
		JSON_Number,
		JSON_String,

		JSON_Object,
		JSON_Array,
	} Type;

	union {
		char* String;
		r32   Number;
		bool8 Boolean;

		struct Array_JSON_Value Array;

		struct {
			Array_String       StringKeys;
			HashMap_JSON_Value Map;
		} Object;
	};
};

JSON_Value* JSON_ObjectFind(const JSON_Value* object, const char* key); // Search an object for a given key.

JSON_Value JSON_FromString(const char* str);            // Parse JSON from a C string.
JSON_Value JSON_FromString_N(const char* str, u32 len); // Parse JSON from a C string with a specfied length.
JSON_Value JSON_FromFile(const char* filename);         // Load a JSON file from disk.

void JSON_Free(JSON_Value *v); // Free an allocated JSON structure.

#endif
