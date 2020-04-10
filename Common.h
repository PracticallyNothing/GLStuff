#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

#define SWAP_I8       \
	(a, b) do {       \
		i8 tmp = (a); \
		(a) = (b);    \
		(b) = tmp;    \
	}                 \
	while(0)
#define SWAP_I16(a, b) \
	do {               \
		i16 tmp = (a); \
		(a) = (b);     \
		(b) = tmp;     \
	} while(0)
#define SWAP_I32(a, b) \
	do {               \
		i32 tmp = (a); \
		(a) = (b);     \
		(b) = tmp;     \
	} while(0)
#define SWAP_I64(a, b) \
	do {               \
		i64 tmp = (a); \
		(a) = (b);     \
		(b) = tmp;     \
	} while(0)

#define SWAP_U8       \
	(a, b) do {       \
		u8 tmp = (a); \
		(a) = (b);    \
		(b) = tmp;    \
	}                 \
	while(0)
#define SWAP_U16(a, b) \
	do {               \
		u16 tmp = (a); \
		(a) = (b);     \
		(b) = tmp;     \
	} while(0)
#define SWAP_U32(a, b) \
	do {               \
		u32 tmp = (a); \
		(a) = (b);     \
		(b) = tmp;     \
	} while(0)
#define SWAP_U64(a, b) \
	do {               \
		u64 tmp = (a); \
		(a) = (b);     \
		(b) = tmp;     \
	} while(0)

#define SWAP_R32(a, b) \
	do {               \
		r32 tmp = (a); \
		(a) = (b);     \
		(b) = tmp;     \
	} while(0)
#define SWAP_R64(a, b) \
	do {               \
		r64 tmp = (a); \
		(a) = (b);     \
		(b) = tmp;     \
	} while(0)
#define SWAP_U8_ARR(a, b, size)                \
	do {                                       \
		for(i32 i = 0; i < (size); i++) {      \
			u8 tmp = ((u8 *) (a))[i];          \
			((u8 *) (a))[i] = ((u8 *) (b))[i]; \
			((u8 *) (b))[i] = tmp;             \
		}                                      \
	} while(0)
#ifdef NDEBUG
#	define DEBUG_PRINTF_S(str) \
		do {                    \
		} while(0)
#	define DEBUG_PRINTF(fmt, ...) \
		do {                       \
		} while(0)
#else
#	define DEBUG_PRINTF_S(str) \
		do { printf((str)); } while(0)
#	define DEBUG_PRINTF(fmt, ...) \
		do { printf((fmt), __VA_ARGS__); } while(0)
#endif

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float r32;
typedef double r64;

typedef i8 bool8;
typedef i32 bool32;

// --- String operations --- //

i32 String_ToI32(const char *);
i32 String_ToI32_N(const char *, u32 len);

r32 String_ToR32(const char *);
r32 String_ToR32_N(const char *, u32 len);

// --- Pi, degrees and radians --- //
extern const r64 Tau;
extern const r64 Pi;
extern const r64 Pi_Half;
extern const r64 Pi_Quarter;

extern r64 DegToRad(r64 degrees);
extern r64 RadToDeg(r64 radians);

extern r64 Clamp_R64(r64 value, r64 min, r64 max);
extern r32 Clamp_R32(r32 value, r32 min, r32 max);
extern i32 Clamp_I32(i32 value, i32 min, i32 max);

// --- File operations --- //

extern i32 File_ReadToBuffer(const char *filename, u8 *buf, u32 bufSize,
                             u32 *realSize);
extern void File_DumpBuffer(const char *filename, const u8 *buf, u32 bufSize);

#if 0
// --- Map --- //

typedef struct HashMap {
	u64 *HashedKeys;
	void *Values;
	u32 ValueSize;
	u32 ArraySize;
	u32 ArrayCapacity;
} HashMap;

typedef void (*FreeElem_Func)(void *);

void HashMap_Init(HashMap *hm, u32 itemSize);
void HashMap_Add(HashMap *hm, const char *key, const void *value);
void HashMap_Remove(HashMap *hm, const char *key);
void *HashMap_Find(const HashMap *hm, const char *key);
void HashMap_Free(HashMap *hm);
void HashMap_FreePerElement(HashMap *hm, FreeElem_Func func);
#endif

// --- Array --- //

typedef struct Array {
	u8 *Data;
	u32 ItemSize;
	u32 ArraySize;
	u32 ArrayCapacity;
} Array;

Array *Array_Init(u32 itemSize);
Array *Array_Prealloc(u32 itemSize, u32 capacity);
void Array_Push(Array *, const u8 *);
void Array_Pop(Array *);
void Array_Copy(Array *out, const Array *src);
void Array_CopyData(u8 *out, const Array *src);
u8 *Array_Get(const Array *, i32 idx);
u8 *Array_GetFirst(const Array *arr);
u8 *Array_GetLast(const Array *arr);
void Array_Clear(Array *);
void Array_Reverse(Array *);
void Array_Free(Array *);

// --- Size units --- //

extern u64 Bytes(u32 amt);
extern u64 Kilobytes(u32 amt);
extern u64 Megabytes(u32 amt);
extern u64 Gigabytes(u32 amt);

// --- Utility --- //

extern i32 PC_IsLittleEndian(void);
extern void PC_PrintVideoDriverInfo(void);

// Comparison function type
// Must return the following:
//   -1 if first item is larger than second
//   0 if both items are equal
//   1 if the second item is larger than the first
typedef i32 (*Util_CompFunc)(const u8 *, const u8 *);

extern void Util_Quicksort_i32(i32 *arr, u32 size);
extern void Util_Quicksort_r32(r32 *arr, u32 size);
extern void Util_Quicksort_func(u8 *arr, u32 itemSize, u32 arrSize,
                                Util_CompFunc compFunc);

// --- Logging --- //

extern enum Log_Level {
	Log_Level_Debug = 0,
	Log_Level_Info = 1,
	Log_Level_Warning = 2,
	Log_Level_Error = 3,
} Log_Level;

extern void Log_Debug(const char *fmt, ...);
extern void Log_Info(const char *fmt, ...);
extern void Log_Warning(const char *fmt, ...);
extern void Log_Error(const char *fmt, ...);
extern void Log_FatalError(const char *fmt, ...);

#endif
