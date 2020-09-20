#ifndef JSON_H
#define JSON_H

#include "Common.h"

DEF_ARRAY(Hash, u128);
DEF_ARRAY(String, char *);
DEF_ARRAY(JSON_Value, struct JSON_Value);

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
			u32                     NumItems;
			struct Array_Hash       Keys;
			struct Array_String     StringKeys;
			struct Array_JSON_Value Values;
		} Object;
	};
};

extern struct JSON_Value JSON_FromString(const char* str);
extern struct JSON_Value JSON_FromString_N(const char* str, u32 len);
extern struct JSON_Value JSON_FromFile(const char* filename);
extern void JSON_Free(struct JSON_Value *v);

#endif
