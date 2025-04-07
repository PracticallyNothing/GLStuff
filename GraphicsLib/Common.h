#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <string.h>

#ifndef NULL
#	define NULL 0
#endif

#define SWAP_U8_ARR(a, b, size)              \
	do {                                     \
		for(u32 i = 0; i < (size); i++) {    \
			u8 tmp         = ((u8*) (a))[i]; \
			((u8*) (a))[i] = ((u8*) (b))[i]; \
			((u8*) (b))[i] = tmp;            \
		}                                    \
	} while(0)

typedef uint8_t u8;   // 8-bit unsigned integer
typedef uint16_t u16; // 16-bit unsigned integer
typedef uint32_t u32; // 32-bit unsigned integer
typedef uint64_t u64; // 64-bit unsigned integer

typedef int8_t i8;   // 8-bit signed integer
typedef int16_t i16; // 16-bit signed integer
typedef int32_t i32; // 32-bit signed integer
typedef int64_t i64; // 64-bit signed integer

typedef float r32;  // 32-bit real (floating point) number
typedef double r64; // 64-bit real (floating point) number

typedef i8 bool8;   // 8-bit boolean
typedef i32 bool32; // 32-bit boolean

//
// Memory management
//

u32 Alloc_GetTotalSize();
void Alloc_PrintInfo();
void Alloc_FreeAll();

#ifdef ALLOC_DEBUG
void* Allocate(u32 size, const char* __func, const char* __file, u32 __line);
void* Reallocate(void* ptr, u32 newSize, const char* __func, const char* __file, u32 __line);
void Free(void* ptr, const char* __func, const char* __file, u32 __line);
#	define Allocate(size)           Allocate(size, __func__, __FILE__, __LINE__)
#	define Reallocate(ptr, newSize) Reallocate(ptr, newSize, __func__, __FILE__, __LINE__)
#	define Free(ptr)                Free(ptr, __func__, __FILE__, __LINE__)
#else
void* Allocate(u32 size);                 // Allocate some memory
void* Reallocate(void* ptr, u32 newSize); // Move memory to a bigger/smaller location
void Free(void* ptr);                     // Free allocated memory
#endif

//
// String operations
//

i32 String_ToI32(const char*);
i32 String_ToI32_N(const char*, u32 len);

r32 String_ToR32(const char*);
r32 String_ToR32_N(const char*, u32 len);

//
// Pi, degrees and radians
//

extern const r64 Tau;        // Pi*2
extern const r64 Pi;         // A constant representing pi.
extern const r64 Pi_Half;    // Pi/2
extern const r64 Pi_Quarter; // Pi/4

r64 DegToRad(r64 degrees); // Convert degrees to radians.
r64 RadToDeg(r64 radians); // Convert radians to degrees.

// --- Util. maths --- //
#define MAX(a, b)        ((a) > (b) ? (a) : (b))
#define MAX3(a, b, c)    MAX(MAX(a, b), c)
#define MAX4(a, b, c, d) MAX(MAX(a, b), MAX(c, d))

#define MIN(a, b)        ((a) < (b) ? (a) : (b))
#define MIN3(a, b, c)    MIN(MIN(a, b), c)
#define MIN4(a, b, c, d) MIN(MIN(a, b), MIN(c, d))

#define CLAMP(v, min, max) ((v) <= (min) ? (min) : ((v) >= (max) ? (max) : (v)))

#define INRANGE(v, min, max) (MIN(min, max) <= (v) && (v) <= MAX(min, max))

#define RANGESOVERLAP(aMin, aMax, bMin, bMax) \
	(MIN(aMin, aMax) <= MAX(bMin, bMax) &&    \
     MIN(bMin, bMax) <= MAX(aMin, aMax))

#define TRIEQ(a, b, c)  ((a) == (b) && (a) == (c))
#define TRINEQ(a, b, c) ((a) != (b) && (a) != (c) && (b) != (c))

// Limit an integer between a min and a max value.
i32 Clamp_I32(i32 value, i32 min, i32 max);

// Limit a float between a min and a max value.
r32 Clamp_R32(r32 value, r32 min, r32 max);

// Limit a double between a min and a max value.
r64 Clamp_R64(r64 value, r64 min, r64 max);

bool8 InRange_I32(i32 value, i32 min, i32 max);
bool8 InRange_R32(r32 value, r32 min, r32 max);
bool8 InRange_R64(r64 value, r64 min, r64 max);

bool8 RangesOverlap_I32(i32 aMin, i32 aMax, i32 bMin, i32 bMax);
bool8 RangesOverlap_R32(r32 aMin, r32 aMax, r32 bMin, r32 bMax);
bool8 RangesOverlap_R64(r64 aMin, r64 aMax, r64 bMin, r64 bMax);

//
// Files
//

// Put the contents of a file inside a buffer allocated by the function.
u8* File_ReadToBuffer_Alloc(const char* filename, u32* outSize);

// Put the contents of a file inside a user-allocated buffer.
i32 File_ReadToBuffer(const char* filename, u8* buf, u32 bufSize, u32* realSize);

// Dump the contents of a buffer to a file.
void File_DumpBuffer(const char* filename, const u8* buf, u32 bufSize);

//
// Array
//

#define DEF_ARRAY(name, type)                                          \
	typedef struct Array_##name Array_##name;                          \
	struct Array_##name {                                              \
		u32 Size, Capacity;                                            \
		type* Data;                                                    \
	};                                                                 \
                                                                       \
	void Array_##name##_Prealloc(Array_##name*, u32 numElems);         \
	void Array_##name##_SizeToFit(Array_##name*);                      \
	void Array_##name##_Free(Array_##name*);                           \
                                                                       \
	i32 Array_##name##_Find(const Array_##name*, const type*);         \
	bool8 Array_##name##_Has(const Array_##name*, const type*);        \
                                                                       \
	void Array_##name##_Push(Array_##name*, const type*);              \
	void Array_##name##_PushVal(Array_##name*, const type);            \
	void Array_##name##_Pop(Array_##name*);                            \
	void Array_##name##_PushMany(Array_##name*, const type*, u32 n);   \
                                                                       \
	void Array_##name##_Insert(Array_##name*, u32 idx, const type*);   \
	void Array_##name##_InsertVal(Array_##name*, u32 idx, const type); \
	void Array_##name##_Remove(Array_##name*, u32 idx);                \
                                                                       \
	void Array_##name##_Reverse(Array_##name*);

#define DECL_ARRAY(name, type)                                                     \
	void Array_##name##_Prealloc(Array_##name* a, u32 n) {                         \
		if(!n) {                                                                   \
			Array_##name##_Free(a);                                                \
			return;                                                                \
		}                                                                          \
		a->Data     = Allocate(sizeof(type) * n);                                  \
		a->Size     = 0;                                                           \
		a->Capacity = n;                                                           \
	}                                                                              \
	bool8 Array_##name##_Has(const Array_##name* a, const type* t) {               \
		return Array_##name##_Find(a, t) < 0 ? 0 : 1;                              \
	}                                                                              \
	i32 Array_##name##_Find(const Array_##name* a, const type* t) {                \
		if(!a->Size) return -1;                                                    \
		for(u32 i = 0; i < a->Size; i++) {                                         \
			if(memcmp(&a->Data[i], t, sizeof(type)) == 0) return i;                \
		}                                                                          \
		return -1;                                                                 \
	}                                                                              \
	void Array_##name##_Push(Array_##name* a, const type* t) {                     \
		if(!t) return;                                                             \
		if(a->Size == a->Capacity) {                                               \
			if(!a->Capacity) a->Capacity = 1;                                      \
			a->Capacity *= 2;                                                      \
			a->Data = Reallocate(a->Data, sizeof(type) * a->Capacity);             \
		}                                                                          \
		memcpy(a->Data + a->Size, t, sizeof(type));                                \
		a->Size++;                                                                 \
	}                                                                              \
	void Array_##name##_PushMany(Array_##name* a, const type* t, u32 n) {          \
		if(!t) return;                                                             \
		if(a->Size + n >= a->Capacity) {                                           \
			if(!a->Capacity) a->Capacity = 1;                                      \
			while(a->Size + n <= a->Capacity) a->Capacity *= 2;                    \
			a->Data = Reallocate(a->Data, sizeof(type) * a->Capacity);             \
		}                                                                          \
		memcpy(a->Data + a->Size, t, sizeof(type) * n);                            \
		a->Size += n;                                                              \
	}                                                                              \
	void Array_##name##_PushVal(Array_##name* a, const type t) {                   \
		Array_##name##_Push(a, &t);                                                \
	}                                                                              \
	void Array_##name##_Free(Array_##name* a) {                                    \
		if(a->Data) Free(a->Data);                                                 \
		a->Data     = NULL;                                                        \
		a->Size     = 0;                                                           \
		a->Capacity = 0;                                                           \
	}                                                                              \
	void Array_##name##_Remove(Array_##name* a, u32 idx) {                         \
		if(idx >= a->Size) return;                                                 \
		memmove(&a->Data[idx], &a->Data[idx + 1], sizeof(type) * (a->Size - idx)); \
		a->Size--;                                                                 \
	}                                                                              \
	void Array_##name##_Insert(Array_##name* a, u32 idx, const type* t) {          \
		if(idx >= a->Size || !t) return;                                           \
		if(a->Size == a->Capacity) {                                               \
			if(!a->Capacity) a->Capacity = 1;                                      \
			a->Capacity *= 2;                                                      \
			a->Data = Reallocate(a->Data, sizeof(type) * a->Capacity);             \
		}                                                                          \
		memmove(&a->Data[idx + 1], &a->Data[idx], sizeof(type) * (a->Size - idx)); \
		memcpy(a->Data + idx, t, sizeof(type));                                    \
		a->Size++;                                                                 \
	}                                                                              \
	void Array_##name##_InsertVal(Array_##name* a, u32 idx, const type t) {        \
		Array_##name##_Insert(a, idx, &t);                                         \
	}                                                                              \
	void Array_##name##_Pop(Array_##name* a) {                                     \
		if(!a || !a->Size) return;                                                 \
		a->Size--;                                                                 \
	}                                                                              \
	void Array_##name##_SizeToFit(Array_##name* a) {                               \
		a->Data     = Reallocate(a->Data, a->Size * sizeof(type));                 \
		a->Capacity = a->Size;                                                     \
	}                                                                              \
	void Array_##name##_Reverse(Array_##name* a) {                                 \
		for(u32 i = 0; i < a->Size / 2; ++i) {                                     \
			type tmp                 = a->Data[i];                                 \
			a->Data[i]               = a->Data[a->Size - 1 - i];                   \
			a->Data[a->Size - 1 - i] = tmp;                                        \
		}                                                                          \
	}
DEF_ARRAY(u8, u8);
DEF_ARRAY(u16, u16);
DEF_ARRAY(u32, u32);
DEF_ARRAY(u64, u64);

DEF_ARRAY(i8, i8);
DEF_ARRAY(i16, i16);
DEF_ARRAY(i32, i32);
DEF_ARRAY(i64, i64);

DEF_ARRAY(r32, r32);
DEF_ARRAY(r64, r64);

//
// Hash map
//

// Hash result type
typedef struct u128 u128;
struct u128 {
	union {
		u64 a[2];
		u32 b[4];
	};
};

// Compute the MD5 hash for an array of bytes.
u128 Hash_MD5(const u8* bytes, u32 length);

// Compute the MD5 hash for a NULL-terminated string.
u128 Hash_String_MD5(const char* str);

// Compare two hashes for equality.
bool8 Hash_Equal(const u128* a, const u128* b);

// Create a struct and prototype functions for a hashmap type HashMap_##name.
#define DEF_HASHMAP(name, type)                                                               \
	typedef struct HashMap_##name HashMap_##name;                                             \
	struct HashMap_##name {                                                                   \
		u128* Keys;                                                                           \
		type* Values;                                                                         \
		u32 Size, Capacity;                                                                   \
	};                                                                                        \
                                                                                              \
	void HashMap_##name##_Add(HashMap_##name* map, const u8* key, u32 keyLen, const type* t); \
	void HashMap_##name##_AddVal(HashMap_##name* map,                                         \
	                             const u8* key,                                               \
	                             u32 keyLen,                                                  \
	                             const type t);                                               \
                                                                                              \
	void HashMap_##name##_Remove(HashMap_##name* map, const u8* key, u32 keyLen);             \
                                                                                              \
	type* HashMap_##name##_FindStr(const HashMap_##name* map, const char* str);               \
	type* HashMap_##name##_Find(const HashMap_##name* map, const u8* key, u32 keyLen);        \
	i32 HashMap_##name##_FindIdx(const HashMap_##name* map, const u8* key, u32 keyLen);       \
                                                                                              \
	void HashMap_##name##_Free(HashMap_##name* map);

// Declare all the functions for a hashmap.
#define DECL_HASHMAP(name, type)                                                         \
	void HashMap_##name##_Add(HashMap_##name* map,                                       \
	                          const u8* key,                                             \
	                          u32 keyLen,                                                \
	                          const type* t) {                                           \
		if(!t) return;                                                                   \
		if(map->Size == map->Capacity) {                                                 \
			if(!map->Capacity) map->Capacity = 1;                                        \
			map->Capacity *= 2;                                                          \
			map->Keys   = Reallocate(map->Keys, sizeof(u128) * map->Capacity);           \
			map->Values = Reallocate(map->Values, sizeof(type) * map->Capacity);         \
		}                                                                                \
		u128 k = Hash_MD5(key, keyLen);                                                  \
		memcpy(map->Keys + map->Size, &k, sizeof(u128));                                 \
		memcpy(map->Values + map->Size, t, sizeof(type));                                \
		map->Size++;                                                                     \
	}                                                                                    \
                                                                                         \
	void HashMap_##name##_AddVal(HashMap_##name* map,                                    \
	                             const u8* key,                                          \
	                             u32 keyLen,                                             \
	                             const type t) {                                         \
		HashMap_##name##_Add(map, key, keyLen, &t);                                      \
	}                                                                                    \
                                                                                         \
	void HashMap_##name##_Remove(HashMap_##name* map, const u8* key, u32 keyLen) {       \
		i32 i = HashMap_##name##_FindIdx(map, key, keyLen);                              \
		if(i < 0) return;                                                                \
		memmove(map->Keys + i + 1, map->Keys + i, sizeof(u128) * (map->Size - i));       \
		memmove(map->Values + i + 1, map->Values + i, sizeof(type) * (map->Size - i));   \
		map->Size--;                                                                     \
	}                                                                                    \
                                                                                         \
	type* HashMap_##name##_FindStr(const HashMap_##name* map, const char* str) {         \
		return HashMap_##name##_Find(map, (u8*) str, strlen(str));                       \
	}                                                                                    \
	type* HashMap_##name##_Find(const HashMap_##name* map, const u8* key, u32 keyLen) {  \
		i32 i = HashMap_##name##_FindIdx(map, key, keyLen);                              \
		return (i >= 0 ? map->Values + i : NULL);                                        \
	}                                                                                    \
	i32 HashMap_##name##_FindIdx(const HashMap_##name* map, const u8* key, u32 keyLen) { \
		if(!map->Size) return -1;                                                        \
		u128 k = Hash_MD5(key, keyLen);                                                  \
		for(u32 i = 0; i < map->Size; ++i) {                                             \
			if(memcmp(map->Keys + i, &k, sizeof(u128)) == 0) return i;                   \
		}                                                                                \
		return -1;                                                                       \
	}                                                                                    \
                                                                                         \
	void HashMap_##name##_Free(HashMap_##name* map) {                                    \
		Free(map->Keys);                                                                 \
		Free(map->Values);                                                               \
		map->Keys     = NULL;                                                            \
		map->Values   = NULL;                                                            \
		map->Size     = 0;                                                               \
		map->Capacity = 0;                                                               \
	}

DEF_HASHMAP(r32, r32);
DEF_HASHMAP(r64, r64);

DEF_HASHMAP(u8, u8);
DEF_HASHMAP(u16, u16);
DEF_HASHMAP(u32, u32);
DEF_HASHMAP(u64, u64);

DEF_HASHMAP(i8, i8);
DEF_HASHMAP(i16, i16);
DEF_HASHMAP(i32, i32);
DEF_HASHMAP(i64, i64);

//
// Size units
//

u64 Bytes(u32 amt);
u64 Kilobytes(u32 amt);
u64 Megabytes(u32 amt);
u64 Gigabytes(u32 amt);

//
// Utility
//

// Check if the system is little endian (false means it's big endian).
bool8 PC_IsLittleEndian();

// Comparison function type
// Must return the following:
//   -1 if first item is larger than second
//   0 if both items are equal
//   1 if the second item is larger than the first
typedef i32 (*Util_CompFunc)(const u8*, const u8*);

void Util_Quicksort_i32(i32* arr, u32 size);
void Util_Quicksort_r32(r32* arr, u32 size);
void Util_Quicksort_func(u8* arr, u32 itemSize, u32 arrSize, Util_CompFunc compFunc);

//
// GL Initialized
//

// Has an OpenGL context been initialized yet?
extern bool8 GL_Initialized;

//
// Logging
//

enum Log_Level {
	DEBUG = 0,
	INFO  = 1,
	WARN  = 2,
	ERROR = 3,
	FATAL = 4,
};
extern enum Log_Level Log_Level_Global;

// If true, logs won't contain file and line.
extern bool8 Log_ShortMode; 

void Log(const char* __func,
         const char* __file,
         u32 __line,
         enum Log_Level level,
         const char* fmt,
         ...);

#define Log(level, fmt, ...) \
	Log(__func__, __FILE__, __LINE__, (level), (fmt), __VA_ARGS__)

#endif
