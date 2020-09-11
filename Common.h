#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

#ifndef NULL
#  define NULL 0
#endif

#define SWAP_I8(a, b)  do { i8  tmp = (a); (a) = (b); (b) = tmp; } while(0)
#define SWAP_I16(a, b) do { i16 tmp = (a); (a) = (b); (b) = tmp; } while(0)
#define SWAP_I32(a, b) do { i32 tmp = (a); (a) = (b); (b) = tmp; } while(0)
#define SWAP_I64(a, b) do { i64 tmp = (a); (a) = (b); (b) = tmp; } while(0)

#define SWAP_U8(a, b)  do { u8  tmp = (a); (a) = (b); (b) = tmp; } while(0)
#define SWAP_U16(a, b) do { u16 tmp = (a); (a) = (b); (b) = tmp; } while(0)
#define SWAP_U32(a, b) do { u32 tmp = (a); (a) = (b); (b) = tmp; } while(0)
#define SWAP_U64(a, b) do { u64 tmp = (a); (a) = (b); (b) = tmp; } while(0)

#define SWAP_R32(a, b) do { r32 tmp = (a); (a) = (b); (b) = tmp; } while(0)
#define SWAP_R64(a, b) do { r64 tmp = (a); (a) = (b); (b) = tmp; } while(0)

#define SWAP_U8_ARR(a, b, size)                \
	do {                                       \
		for(i32 i = 0; i < (size); i++) {      \
			u8 tmp = ((u8 *) (a))[i];          \
			((u8 *) (a))[i] = ((u8 *) (b))[i]; \
			((u8 *) (b))[i] = tmp;             \
		}                                      \
	} while(0)

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

// --- Util. maths --- //
#define MAX(a, b)        ((a) > (b) ? (a) : (b))
#define MAX3(a, b, c)    MAX(MAX(a,b), c)
#define MAX4(a, b, c, d) MAX(MAX(a,b), MAX(c,d))

#define MIN(a, b)        ((a) < (b) ? (a) : (b))
#define MIN3(a, b, c)    MIN(MIN(a, b), c)
#define MIN4(a, b, c, d) MIN(MIN(a, b), MIN(c, d))

#define CLAMP(v, min, max) ((v) <= (min) ? (min) : ((v) >= (max) ? (max) : (v)))

#define INRANGE(v, min, max) (MIN(min,max) <= (v) && (v) <= MAX(min,max))

#define RANGESOVERLAP(aMin, aMax, bMin, bMax) \
	(   INRANGE(MIN(aMin, aMax), MIN(bMin, bMax), MAX(bMin, bMax)) \
	 || INRANGE(MAX(aMin, aMax), MIN(bMin, bMax), MAX(bMin, bMax)) \
	 || INRANGE(MIN(bMin, bMax), MIN(aMin, aMax), MAX(aMin, aMax)) \
	 || INRANGE(MAX(bMin, bMax), MIN(aMin, aMax), MAX(aMin, aMax)))

#define TRIEQ(a, b, c)  ((a) == (b) && (a) == (c))
#define TRINEQ(a, b, c) ((a) != (b) && (a) != (c) && (b) != (c))

extern i32 Clamp_I32(i32 value, i32 min, i32 max);
extern r32 Clamp_R32(r32 value, r32 min, r32 max);
extern r64 Clamp_R64(r64 value, r64 min, r64 max);

extern bool8 InRange_I32(i32 value, i32 min, i32 max);
extern bool8 InRange_R32(r32 value, r32 min, r32 max);
extern bool8 InRange_R64(r64 value, r64 min, r64 max);

extern bool8 RangesOverlap_I32(i32 aMin, i32 aMax, i32 bMin, i32 bMax);
extern bool8 RangesOverlap_R32(r32 aMin, r32 aMax, r32 bMin, r32 bMax);
extern bool8 RangesOverlap_R64(r64 aMin, r64 aMax, r64 bMin, r64 bMax);

// --- File operations --- //

/// Put the contents of a file inside a buffer malloc-ed by the function.
extern u8* File_ReadToBuffer_Alloc(const char *filename, u32 *size);

/// Put the contents of a file inside a user-allocated buffer.
extern i32 File_ReadToBuffer(const char *filename, u8 *buf, u32 bufSize, u32 *realSize);

/// Dump the contents of a buffer to a file.
extern void File_DumpBuffer(const char *filename, const u8 *buf, u32 bufSize);

// --- Array --- //
#define DEF_ARRAY(name, type)                                               \
struct Array_##name { u32 Size, Capacity; type *Data; };                    \
                                                                            \
 void Array_##name##_SizeToFit(struct Array_##name *);                      \
 void Array_##name##_Free(struct Array_##name *);                           \
                                                                            \
  i32 Array_##name##_Find(const struct Array_##name *, const type *);       \
bool8 Array_##name##_Has(const struct Array_##name *, const type *);        \
                                                                            \
 void Array_##name##_Push(struct Array_##name *, const type *);             \
 void Array_##name##_PushVal(struct Array_##name *, const type);            \
 void Array_##name##_Pop(struct Array_##name *);                            \
                                                                            \
 void Array_##name##_Insert(struct Array_##name *, u32 idx, const type *);  \
 void Array_##name##_InsertVal(struct Array_##name *, u32 idx, const type); \
 void Array_##name##_Remove(struct Array_##name *, u32 idx);                \
                                                                            \
 void Array_##name##_Reverse(struct Array_##name *);

#define DECL_ARRAY(name, type)                                                \
bool8 Array_##name##_Has(const struct Array_##name *a, const type *t)         \
{                                                                             \
    return Array_##name##_Find(a, t) < 0 ? 0 : 1;                             \
}                                                                             \
i32 Array_##name##_Find(const struct Array_##name *a, const type *t)          \
{                                                                             \
	if(!a->Size) return -1;                                                   \
	for(u32 i = 0; i < a->Size; i++) {                                        \
	    if(memcmp(&a->Data[i], t, sizeof(type)) == 0)                         \
	        return i;                                                         \
    }                                                                         \
	return -1;                                                                \
}                                                                             \
void Array_##name##_Push(struct Array_##name *a, const type *t)               \
{                                                                             \
    if(!t) return;                                                            \
    if(a->Size == a->Capacity) {                                              \
		if(!a->Capacity) a->Capacity = 1;                                     \
	    a->Capacity *= 2;                                                     \
	    a->Data = realloc(a->Data, sizeof(type) * a->Capacity);               \
	}                                                                         \
    memcpy(a->Data + a->Size, t, sizeof(type));                               \
    a->Size++;                                                                \
}                                                                             \
void Array_##name##_PushVal(struct Array_##name *a, const type t) {           \
    Array_##name##_Push(a, &t);                                               \
}                                                                             \
void Array_##name##_Free(struct Array_##name *a) {                            \
	free(a->Data);                                                            \
	a->Data = NULL;                                                           \
	a->Size = 0;                                                              \
	a->Capacity = 0;                                                          \
}                                                                             \
void Array_##name##_Remove(struct Array_##name *a, u32 idx)                   \
{                                                                             \
	if(idx >= a->Size) return;                                                \
	memmove(&a->Data[idx  ],                                                  \
			&a->Data[idx+1],                                                  \
			sizeof(type)*(a->Size-idx));                                      \
	a->Size--;                                                                \
}                                                                             \
void Array_##name##_Insert(struct Array_##name *a, u32 idx, const type* t)    \
{                                                                             \
	if(idx >= a->Size || !t) return;                                          \
    if(a->Size == a->Capacity) {                                              \
		if(!a->Capacity) a->Capacity = 1;                                     \
	    a->Capacity *= 2;                                                     \
	    a->Data = realloc(a->Data, sizeof(type) * a->Capacity);               \
	}                                                                         \
	memmove(&a->Data[idx+1],                                                  \
			&a->Data[idx  ],                                                  \
			sizeof(type)*(a->Size-idx));                                      \
    memcpy(a->Data + idx, t, sizeof(type));                                   \
    a->Size++;                                                                \
}                                                                             \
void Array_##name##_InsertVal(struct Array_##name *a, u32 idx, const type t)  \
{                                                                             \
    Array_##name##_Insert(a, idx, &t);                                        \
}                                                                             \
void Array_##name##_Pop(struct Array_##name *a)                               \
{                                                                             \
    if(!a || !a->Size) return;                                                \
    a->Size--;                                                                \
}                                                                             \
void Array_##name##_SizeToFit(struct Array_##name *a)                         \
{                                                                             \
    a->Data = realloc(a->Data, a->Size * sizeof(type));                       \
    a->Capacity = a->Size;                                                    \
}                                                                             \
void Array_##name##_Reverse(struct Array_##name *a)                           \
{                                                                             \
    for(u32 i = 0; i < a->Size/2; ++i) {                                      \
		type tmp = a->Data[i];                                                \
		a->Data[i] = a->Data[a->Size-1-i];                                    \
		a->Data[a->Size-1-i] = tmp;                                           \
	}                                                                         \
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

// --- Size units --- //

extern u64 Bytes(u32 amt);
extern u64 Kilobytes(u32 amt);
extern u64 Megabytes(u32 amt);
extern u64 Gigabytes(u32 amt);

// --- Utility --- //

extern bool8 PC_IsLittleEndian(void);

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

// --- GL Initialized --- //

/// Has an OpenGL context been initialized yet?
extern bool8 GL_Initialized;

// --- Logging --- //

extern enum Log_Level {
	Log_Debug = 0,
	Log_Info = 1,
	Log_Warning = 2,
	Log_Error = 3,
	Log_Fatal = 4,

	DBG   = Log_Debug,
	INFO  = Log_Info,
	WARN  = Log_Warning,
	ERR   = Log_Error,
	FATAL = Log_Fatal,

	DEBUG   = Log_Debug,
	WARNING = Log_Warning,
	ERROR   = Log_Error,
} Log_Level_Global;

extern void Log(const char *__func, const char *__file, u32 __line,
                enum Log_Level level, const char *fmt, ...);

#define Log(level, fmt, ...) \
	Log(__FUNCTION__, __FILE__, __LINE__, (level), (fmt), __VA_ARGS__)

typedef struct { union {u64 a[2]; u32 b[4]; }; } u128;
u128 Hash_MD5(const u8 *bytes, u32 length);

#endif

