#include "JSON.h"
#include "Common.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

DECL_ARRAY(Hash, u128);
DECL_ARRAY(String, char *);
DECL_ARRAY(JSON_Value, struct JSON_Value);
DECL_HASHMAP(JSON_Value, struct JSON_Value);

struct JSON_Value
JSON_FromFile(const char* filename)
{
	Log(INFO, "Reading JSON file \"%s\".", filename);
	u32 size;
	u8* buf = File_ReadToBuffer_Alloc(filename, &size);
	struct JSON_Value res = JSON_FromString_N((char*) buf, size);
	Free(buf);
	return res;
}

struct JSON_Value 
JSON_FromString(const char* str)
{
	return JSON_FromString_N(str, strlen(str));
}

static bool8 
Char_OneOf(char c, const char* chars)
{
	if(!chars) return 0;

	while(*chars) {
		if(c == *chars) return 1;
		++chars;
	}
	return 0;
}

enum TokenType {
	Token_EOF = 0,

	Token_Comma, // ,
	Token_Colon, // :

	Token_BeginObject, // {
	Token_EndObject,   // }
	
	Token_BeginArray,  // [
	Token_EndArray,    // ]

	Token_Null,    // null
	Token_Number,  // 0 || 1.0012
	Token_String,  // "asdf"
	Token_Bool_True, // true
	Token_Bool_False, // false
	
	Token_TotalTypes,
};

const char *TokenType_Strings[Token_TotalTypes+1] = {
	"Token_EOF",

	"Token_Comma",
	"Token_Colon",

	"Token_BeginObject",
	"Token_EndObject",
	
	"Token_BeginArray",
	"Token_EndArray",

	"Token_Null",
	"Token_Number",
	"Token_String",
	"Token_Bool_True",
	"Token_Bool_False",
	
	"Token_TotalTypes"
};

struct Token {
	enum TokenType Type;
	const char* Start;
	const char* End;
};

DEF_ARRAY(Token, struct Token);
DECL_ARRAY(Token, struct Token);

static void PrintToken(const struct Token *t)
{
	printf("%s", TokenType_Strings[t->Type]);
	switch(t->Type) {
		case Token_String: printf("(\"%.*s\")", (i32) (t->End - t->Start), t->Start); break;
		case Token_Number: printf("(%.2f)", String_ToR32_N(t->Start, t->End - t->Start)); break;
		default: break;
	}
	printf(" ");
}

static struct Array_Token 
JSON_ToTokens(const char* str, u32 len) 
{
	const char* p = str;

	struct Array_Token tokens = {
		.Size = 0,
		.Capacity = 0,
		.Data = NULL
	};

	while(p != str+len)
	{
		enum TokenType type = Token_EOF;
		const char *start = 0, *end = 0;

		// Eat whitespace
		while(p < str+len && Char_OneOf(*p, " \t\n\r")) ++p;

		     if(*p == '{') { type = Token_BeginObject; ++p;}
		else if(*p == '}') { type = Token_EndObject;   ++p;} 
		else if(*p == '[') { type = Token_BeginArray;  ++p;} 
		else if(*p == ']') { type = Token_EndArray;    ++p;}
		else if(*p == ':') { type = Token_Colon;       ++p;} 
		else if(*p == ',') { type = Token_Comma;       ++p;} 
		else if(*p == '\"') { 
			++p;
			const char* q = p;
			while(q < str+len && *q != '\"') q += (*q == '\\' ? 2 : 1);
			if(q == str+len)
			{
				Log(ERROR, "JSON with unclosed string.", "");
			}

			type = Token_String;
			start = p;
			end = q;

			p = q+1;
		} 
		else if(*p == '-' || ('0' <= *p && *p <= '9')) {
			const char* q = p;
			bool8 gotDot = 0;

			while(q < str+len && !Char_OneOf(*q, " \t\n\r{}[]:,\""))
			{
				if(*q == '.') {
					if(gotDot) { 
						Log(ERROR, "JSON number with too many decimal points.", ""); 
						Array_Token_Free(&tokens);
						return (struct Array_Token) {0};
					}
					else 
						gotDot = 1;
				}
				++q;
			}
			type = Token_Number;
			start = p;
			end = q;

			p = q;
		}
		else if(strncmp(p, "null",  4) == 0) { type = Token_Null;       p+=4;} 
		else if(strncmp(p, "true",  4) == 0) { type = Token_Bool_True;  p+=4;} 
		else if(strncmp(p, "false", 5) == 0) { type = Token_Bool_False; p+=5;} 
		else { while(p < str+len && !Char_OneOf(*p, " \t\n\r{}[]:,\"")) ++p; }

		// Allocate more space for tokens

		struct Token t = {
			.Type = type,
			.Start = start,
			.End = end
		};
		//PrintToken(&t);
		Array_Token_Push(&tokens, &t);
	}

	// Push final Token_EOF
	struct Token t = { .Type = Token_EOF };
	Array_Token_Push(&tokens, &t);
	Array_Token_SizeToFit(&tokens);
	return tokens;
}

const struct JSON_Value Error = { .Type = JSON_Error };

static struct JSON_Value JSON_ParsePrimitive(const struct Token **curr);
static struct JSON_Value JSON_ParseArray(const struct Token **curr);
static struct JSON_Value JSON_ParseObject(const struct Token **curr);

static struct JSON_Value 
JSON_ParsePrimitive(const struct Token **curr) 
{
	struct JSON_Value res = { .Type = JSON_Error };

	switch((*curr)->Type) {
		case Token_String: {
			res.Type = JSON_String;

			u32 len = (*curr)->End - (*curr)->Start; 

			res.String = Allocate(len + 1);
			strncpy(res.String, (*curr)->Start, len);
			res.String[len] = '\0';

			break; 
		}
		case Token_Number: {
			res.Type = JSON_Number;
			res.Number = String_ToR32_N((*curr)->Start, (*curr)->End - (*curr)->Start);
			break;
		}
		case Token_Null:       res.Type = JSON_Null; break;
		case Token_Bool_True:  res.Type = JSON_Boolean; res.Boolean = 1; break;
		case Token_Bool_False: res.Type = JSON_Boolean; res.Boolean = 0; break;

		default: {
			Log(ERROR, "Token %s is not a primitive.", TokenType_Strings[(*curr)->Type]); 
			break;
		}
	}

	(*curr)++;
	return res;
}

static struct JSON_Value
JSON_ParseArray(const struct Token **curr) {
	(*curr)++;

	struct JSON_Value array;
	array.Type = JSON_Array;
	array.Array = (struct Array_JSON_Value) {0};

	while((*curr)->Type != Token_EndArray && (*curr)->Type != Token_EOF)
	{
		struct JSON_Value v;
		switch((*curr)->Type) {
			case Token_BeginArray:
				v = JSON_ParseArray(curr);
				break;
			case Token_BeginObject:
				v = JSON_ParseObject(curr);
				break;
			case Token_Bool_True:
			case Token_Bool_False:
			case Token_Null:
			case Token_Number:
			case Token_String:
				v = JSON_ParsePrimitive(curr);
				break;
			default:
				v = Error;
				break;
		}
		if(v.Type == JSON_Error)
			goto ParseArray_error;

		Array_JSON_Value_Push(&array.Array, &v);

		if((*curr)->Type != Token_Comma && (*curr)->Type != Token_EndArray) {
			Log(ERROR, "JSON Array, expected ',' or ']', got: ", "");
			PrintToken(*curr-2);
			PrintToken(*curr-1);
			PrintToken(*curr);
			PrintToken(*curr+1);
			PrintToken(*curr+2);
			printf("\n");
			goto ParseArray_error;
		}

		if((*curr)->Type == Token_Comma)
			(*curr)++;
	}

	if((*curr)->Type == Token_EOF) {
		Log(ERROR, "JSON Array, reached EOF without closing the array.", "");
		PrintToken(*curr-4);
		PrintToken(*curr-3);
		PrintToken(*curr-2);
		PrintToken(*curr-1);
		PrintToken(*curr);
		printf("\n");
		goto ParseArray_error;
	}

	(*curr)++;
	return array;

ParseArray_error:
	JSON_Free(&array);
	return Error;
}

static struct JSON_Value
JSON_ParseObject(const struct Token **curr) {
	(*curr)++;

	struct JSON_Value object;
	object.Type = JSON_Object;
	object.Object.Map        = (struct HashMap_JSON_Value) {0};
	object.Object.StringKeys = (struct Array_String) {0};

	while((*curr)->Type != Token_EndObject && (*curr)->Type != Token_EOF)
	{
		char *Key;
		u32  KeyLen;

		if((*curr)->Type == Token_String) {
			KeyLen = (*curr)->End - (*curr)->Start;

			Key = Allocate(KeyLen+1);
			Key[KeyLen] = '\0';
			strncpy(Key, (*curr)->Start, KeyLen);

			(*curr)++;
		} else {
			Log(ERROR, "JSON Object, expected string, got %s.", TokenType_Strings[(*curr)->Type]);
			PrintToken(*curr-2);
			PrintToken(*curr-1);
			PrintToken(*curr);
			PrintToken(*curr+1);
			PrintToken(*curr+2);
			printf("\n");
			goto ParseObject_error;
		}

		if((*curr)->Type == Token_Colon) {
			(*curr)++;
		} else {
			Free(Key);
			Log(ERROR, "JSON Object, expected ':', got %s.", TokenType_Strings[(*curr)->Type]);
			PrintToken(*curr-2);
			PrintToken(*curr-1);
			PrintToken(*curr);
			PrintToken(*curr+1);
			PrintToken(*curr+2);
			printf("\n");
			goto ParseObject_error;
		}

		struct JSON_Value v;
		switch((*curr)->Type) {
			case Token_BeginArray:
				v = JSON_ParseArray(curr);
				break;
			case Token_BeginObject:
				v = JSON_ParseObject(curr);
				break;
			case Token_Bool_True:
			case Token_Bool_False:
			case Token_Null:
			case Token_Number:
			case Token_String:
				v = JSON_ParsePrimitive(curr);
				break;
			default:
				v = Error;
				break;
		}
		if(v.Type == JSON_Error) {
			Free(Key);
			goto ParseObject_error;
		}

		i32 i = HashMap_JSON_Value_FindIdx(&object.Object.Map, (u8*) Key, KeyLen);
		if(i >= 0) {
			Log(WARNING, "JSON Object, key-value pair with already existing key \"%s\", replacing value.", Key);
			object.Object.Map.Values[i] = v;
		} else {
			HashMap_JSON_Value_Add(&object.Object.Map, (u8*) Key, KeyLen, &v);
			Array_String_PushVal(&object.Object.StringKeys, Key);
		}

		if((*curr)->Type != Token_Comma && (*curr)->Type != Token_EndObject) {
			Free(Key);
			Log(ERROR, "JSON Object, expected ',' or '}', got %s", TokenType_Strings[(*curr)->Type]);
			PrintToken(*curr-2);
			PrintToken(*curr-1);
			PrintToken(*curr);
			PrintToken(*curr+1);
			PrintToken(*curr+2);
			printf("\n");
			goto ParseObject_error;
		}

		if((*curr)->Type == Token_Comma)
			(*curr)++;
	}

	if((*curr)->Type == Token_EOF) {
		Log(ERROR, "JSON Object, reached EOF without closing the object.", "");
		goto ParseObject_error;
	}

	(*curr)++;
	return object;
ParseObject_error:
	JSON_Free(&object);
	return Error;
}

struct JSON_Value 
JSON_FromString_N(const char* str, u32 len)
{
	if(!str || !len) {
		Log(ERROR, "Null string or zero length.", "");
		return Error;
	}

	struct Array_Token tokens = JSON_ToTokens(str, len);

	if(!tokens.Data) {
		Log(ERROR, "No tokens parsed.", "");
		return Error;
	}

	const struct Token *currToken = tokens.Data;
	struct JSON_Value res;

	switch(currToken->Type) {
		case Token_BeginArray:  
			res = JSON_ParseArray(&currToken);
			break;
		case Token_BeginObject: 
			res = JSON_ParseObject(&currToken);
			break;
		case Token_Null: 
		case Token_String:
		case Token_Number:
		case Token_Bool_True:
		case Token_Bool_False:
			res = JSON_ParsePrimitive(&currToken);
			goto end;
		default: 
			goto error;
	}

	if(res.Type == JSON_Error)
		goto error;

end:
	if(currToken->Type != Token_EOF)
		Log(WARNING, "There are still unparsed JSON tokens.", "");
	Array_Token_Free(&tokens);
	return res;
error:
	Log(ERROR, "JSON parsing error.", "");
	Array_Token_Free(&tokens);
	return Error;
}

void JSON_Free(struct JSON_Value *v)
{
	switch(v->Type) {
		case JSON_Array:
			for(u32 i = 0; i < v->Array.Size; ++i)
				JSON_Free(v->Array.Data + i);
			Array_JSON_Value_Free(&v->Array);

			break;

		case JSON_Object:
			for(u32 i = 0; i < v->Object.Map.Size; ++i) {
				Free(v->Object.StringKeys.Data[i]);
				JSON_Free(v->Object.Map.Values + i);
			}

			Array_String_Free(&v->Object.StringKeys);
			HashMap_JSON_Value_Free(&v->Object.Map);

			break;
		case JSON_String:
			Free(v->String);
			break;

		default: 
			break;
	}
}

struct JSON_Value* JSON_ObjectFind(const struct JSON_Value* v, const char* str)
{
	if(!v || !str) return NULL;
	return HashMap_JSON_Value_Find(&v->Object.Map, (u8*) str, strlen(str));
}
