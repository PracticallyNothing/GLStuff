#include "../Common.h"

#include <math.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
// Arrays
//

DECL_ARRAY(u8, u8);
DECL_ARRAY(u16, u16);
DECL_ARRAY(u32, u32);
DECL_ARRAY(u64, u64);

DECL_ARRAY(i8, i8);
DECL_ARRAY(i16, i16);
DECL_ARRAY(i32, i32);
DECL_ARRAY(i64, i64);

DECL_ARRAY(r32, r32);
DECL_ARRAY(r64, r64);

//
// Hashmaps
//

DECL_HASHMAP(r32, r32);
DECL_HASHMAP(r64, r64);

DECL_HASHMAP(u8, u8);
DECL_HASHMAP(u16, u16);
DECL_HASHMAP(u32, u32);
DECL_HASHMAP(u64, u64);

DECL_HASHMAP(i8, i8);
DECL_HASHMAP(i16, i16);
DECL_HASHMAP(i32, i32);
DECL_HASHMAP(i64, i64);

bool8 GL_Initialized = 0;

const r64 Tau        = 6.28318530717958647692;
const r64 Pi         = 3.14159265358979323846;
const r64 Pi_Half    = 1.57079632679489661923;
const r64 Pi_Quarter = 0.78539816339744830962;

r64 DegToRad(r64 degrees) { return degrees * (Pi / 180.0); }
r64 RadToDeg(r64 radians) { return radians * (180.0 / Pi); }

bool8 InRange_I32(i32 value, i32 min, i32 max) { return INRANGE(value, min, max); }
bool8 InRange_R32(r32 value, r32 min, r32 max) { return INRANGE(value, min, max); }
bool8 InRange_R64(r64 value, r64 min, r64 max) { return INRANGE(value, min, max); }

bool8 RangesOverlap_I32(i32 aMin, i32 aMax, i32 bMin, i32 bMax) {
	return RANGESOVERLAP(aMin, aMax, bMin, bMax);
}
bool8 RangesOverlap_R32(r32 aMin, r32 aMax, r32 bMin, r32 bMax) {
	return RANGESOVERLAP(aMin, aMax, bMin, bMax);
}
bool8 RangesOverlap_R64(r64 aMin, r64 aMax, r64 bMin, r64 bMax) {
	return RANGESOVERLAP(aMin, aMax, bMin, bMax);
}

i32 Clamp_I32(i32 value, i32 min, i32 max) { return CLAMP(value, min, max); }
r32 Clamp_R32(r32 value, r32 min, r32 max) { return CLAMP(value, min, max); }
r64 Clamp_R64(r64 value, r64 min, r64 max) { return CLAMP(value, min, max); }

// Thank you,
// https://stackoverflow.com/a/101613
i32 Pow_I32(i32 n, u32 power) {
	i32 res = 1;

	while(1) {
		if(power & 1) res *= n;
		power >>= 1;
		if(!power) break;
		n *= n;
	}

	return res;
}

r32 String_ToR32_N(const char* str, u32 len) {
	char* buf = Allocate(len + 1);
	buf[len]  = '\0';
	strncpy(buf, str, len);

	r32 res = atof(buf);
	Free(buf);
	return res;
}

i32 String_ToI32(const char* str) { return String_ToI32_N(str, strlen(str)); }

i32 String_ToI32_N(const char* str, u32 len) {
	i32 res   = 0;
	bool8 neg = 0;

	// Skip whitespace.
	while(*str == ' ' || *str == '\n' || *str == '\t') {
		++str;
		--len;
	}

	// Check if value is negative.
	if(*str == '-') {
		neg = 1;
		++str;
		--len;
	}

	for(u32 i = 0; i < len; i++) {
		res += (str[i] - '0') * Pow_I32(10, len - 1 - i);
	}
	return (neg ? -res : res);
}

/////////////////////////////////////////////////////////

static u32 File_GetSize(FILE* f) {
	fpos_t CurrPos;
	u32 Size;

	fgetpos(f, &CurrPos);
	fseek(f, 0, SEEK_END);

	Size = ftell(f);

	fsetpos(f, &CurrPos);

	return Size;
}

u8* File_ReadToBuffer_Alloc(const char* filename, u32* size) {
	FILE* File = fopen(filename, "rb");

	if(!File) {
		if(size) *size = 0;
		return NULL;
	}

	u32 Size = File_GetSize(File);
	u8* buf  = Allocate(Size);

	fread(buf, sizeof(u8), Size, File);

	if(size) *size = Size;

	fclose(File);

	return buf;
}

i32 File_ReadToBuffer(const char* filename, u8* buf, u32 bufSize, u32* realSize) {
	FILE* File;
	u32 Size;

	File = fopen(filename, "rb");

	if(!File) {
		Log(ERROR, "File \"%s\" doesn't exist.", filename);
		return 0;
	}

	Size = File_GetSize(File);
	Size = (Size > bufSize ? bufSize : Size);

	fread(buf, sizeof(u8), Size, File);

	if(realSize) *realSize = Size;

	fclose(File);

	return 1;
}

void File_DumpBuffer(const char* filename, const u8* buf, u32 bufSize) {
	FILE* File = fopen(filename, "wb");
	fwrite(buf, sizeof(u8), bufSize, File);
	fclose(File);
}

u64 Bytes(u32 amt) { return amt; }
u64 Kilobytes(u32 amt) { return amt * 1024; }
u64 Megabytes(u32 amt) { return amt * 1024 * 1024; }
u64 Gigabytes(u32 amt) { return amt * 1024 * 1024 * 1024; }

// Thank you,
// https://www.cs-fundamentals.com/tech-interview/c/c-program-to-check-little-and-big-endian-architecture.php
bool8 PC_IsLittleEndian() {
	u32 x = 0x1;

	char* c = (char*) &x;
	return *c;
}

// --- Hashing --- ///

bool8 Hash_Equal(const u128* a, const u128* b) {
	return memcmp(a, b, sizeof(u128)) == 0;
}

u128 Hash_String_MD5(const char* str) {
	return Hash_MD5((const u8*) str, strlen(str));
}

// Thank you,
// https://en.wikipedia.org/wiki/MD5#Pseudocode
// FIXME: Function produces incorrect output.
u128 Hash_MD5(const u8* bytes, u32 length) {
	u32 shiftAmts[64] = {
	    7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, //  0 - 15
	    5, 9,  14, 20, 5, 9,  14, 20, 5, 9,  14, 20, 5, 9,  14, 20, // 16 - 31
	    4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, // 32 - 47
	    6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, // 48 - 63
	};

#if 1
	// Precomputed table.
	u32 K[64] = {
		0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a,
		0xa8304613, 0xfd469501, 0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
		0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821, 0xf61e2562, 0xc040b340,
		0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
		0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8,
		0x676f02d9, 0x8d2a4c8a, 0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
		0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70, 0x289b7ec6, 0xeaa127fa,
		0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
		0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92,
		0xffeff47d, 0x85845dd1, 0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
		0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
	};
#else
	// Generated.
	i32 K[64] = {0};
	for(u32 i = 0; i < 63; i++) K[i] = floor(pow(2, 32) * fabs(sinf(i + 1)));
#endif

	i32 a0 = 0x67452301; // A
	i32 b0 = 0xefcdab89; // B
	i32 c0 = 0x98badcfe; // C
	i32 d0 = 0x10325476; // D

	u32 padding = 0;
	while((length + 1 + padding) % 64 != 56) {
		++padding;
	}

	u32 bufLength = length + 1 + padding + sizeof(u64);

	u8* buf = Allocate(bufLength);
	memcpy(buf, bytes, length);

	// append 0x80
	buf[length] = 0x80;

	// append 0x00 until length % 64 = 56
	memset(buf + length + 1, 0x00, bufLength - length);

	// append original length in bits mod 2^64 to message
	u64 modlen = (length * 8) % ((u64) 1 << 63);
	memcpy(buf + (bufLength - 1 - sizeof(u64)), &modlen, sizeof(u64));
	/*
	Log(INFO, 
	    "Length: %d, Padding: %d, Buffer length: %d, Length*8 mod 2^64: %lu", 
		length, padding, bufLength, modlen);
	*/

	for(u32 i = 0; i < bufLength; i += 64) {
		u32 A = a0, B = b0, C = c0, D = d0;
		u32* M = (u32*) buf + i;

		for(u32 j = 0; j < 64; j++) {
			u32 F, g;
			if(j <= 15) {
				F = (B & C) | ((~B) & D);
				g = j;
			} else if(j <= 31) {
				F = (D & B) | ((~D) & C);
				g = (5 * j + 1) % 16;
			} else if(j <= 47) {
				F = B ^ C ^ D;
				g = (3 * j + 5) % 16;
			} else {
				F = C ^ (B | (~D));
				g = (7 * j) % 16;
			}

			F += A + K[j] + M[g];

			A = D;
			D = C;
			C = B;
			B += (F << shiftAmts[j]) | (F >> (32 - shiftAmts[j]));
		}

		a0 = a0 + A;
		b0 = b0 + B;
		c0 = c0 + C;
		d0 = d0 + D;
	}

	Free(buf);
	u128 res = {.b = {a0, b0, c0, d0}};
	return res;
}

typedef struct Allocation Allocation;
typedef struct Array_Allocation Array_Allocation;

struct Allocation {
	void* Ptr;
	u32 Size;

	char* File;
	char* Function;
	u32 Line;
};

struct Array_Allocation {
	u32 Size;
	u32 Capacity;
	Allocation* Data;
};

void Array_Allocation_Push(Array_Allocation* a, const Allocation* t) {
	if(!t) return;
	if(a->Size == a->Capacity) {
		if(!a->Capacity) a->Capacity = 1;
		a->Capacity *= 2;
		a->Data = realloc(a->Data, sizeof(Allocation) * a->Capacity);
	}
	memcpy(a->Data + a->Size, t, sizeof(Allocation));
	a->Size++;
}
void Array_Allocation_Remove(Array_Allocation* a, u32 idx) {
	if(idx >= a->Size) return;

	memmove(&a->Data[idx], &a->Data[idx + 1], sizeof(Allocation) * (a->Size - idx));

	a->Size--;
}

#ifdef ALLOC_DEBUG

static Array_Allocation Allocs = {0};
static u32 SizesSum            = 0;

static i32 Allocation_FindPtr(void* f) {
	if(!f) return -1;

	for(u32 i = 0; i < Allocs.Size; i++) {
		if(Allocs.Data[i].Ptr == f) return i;
	}

	return -1;
}

#	undef Allocate
#	undef Reallocate
#	undef Free

#	if 0
#		define CHECK_ALLOC_LIMIT(sz)                                         \
			if(SizesSum + sz > Megabytes(50))                                 \
				Log(FATAL,                                                    \
				    "Allocated over maximum limit from [%s:%d %s], exiting.", \
				    __file,                                                   \
				    __line,                                                   \
				    __func);
#	else
#		define CHECK_ALLOC_LIMIT(sz)
#	endif
u32 Alloc_GetTotalSize() { return SizesSum; }

void Alloc_PrintInfo() {
	Log(INFO, "Allocated memory: %.2f kiB (%d bytes)", SizesSum / 1024.0, SizesSum);

	for(u32 i = 0; i < Allocs.Size; i++) {
		Log(INFO,
		    "    -> %d bytes from [%s:%d %s()]",
		    Allocs.Data[i].Size,
		    Allocs.Data[i].File,
		    Allocs.Data[i].Line,
		    Allocs.Data[i].Function);
	}
}

void* Allocate(u32 size, const char* __func, const char* __file, u32 __line) {
	if(size == 0) return NULL;

	CHECK_ALLOC_LIMIT(size);

	Allocation a;
	a.Ptr  = malloc(size);
	a.Size = size;
	a.Line = __line;

	a.File     = malloc(strlen(__file) + 1);
	a.Function = malloc(strlen(__func) + 1);

	a.File[strlen(__file)]     = 0;
	a.Function[strlen(__func)] = 0;

	strcpy(a.File, __file);
	strcpy(a.Function, __func);

	if(!a.Ptr) {
		Log(ERROR, "Allocation with size %d failed.", size);
		Log(ERROR, "  (Called from [%s:%d %s()])", __file, __line, __func);
		return NULL;
	}

	Array_Allocation_Push(&Allocs, &a);
	SizesSum += a.Size;
	return a.Ptr;
}

void* Reallocate(void* ptr, u32 newSize, const char* __func, const char* __file, u32 __line) {
	if(newSize == 0) {
		Free(ptr, __func, __file, __line);
		return NULL;
	}

	if(!ptr) return Allocate(newSize, __func, __file, __line);

	CHECK_ALLOC_LIMIT(newSize);

	i32 i = Allocation_FindPtr(ptr);
	if(i < 0 && ptr) {
		Log(ERROR, "Attempted to reallocate invalid pointer 0x%x.", ptr);
		Log(ERROR, "  (Called from [%s:%d %s()])", __file, __line, __func);
		return NULL;
	}

	void* newPtr = realloc(ptr, newSize);
	if(!newPtr) {
		Log(ERROR, "Reallocation of pointer 0x%x with new size %d bytes failed.",
		    newSize);

		Log(ERROR, "  (Called from [%s:%d %s()])",
		    __file, __line, __func);
		return NULL;
	}

	if(ptr) SizesSum -= Allocs.Data[i].Size;
	SizesSum += newSize;

	Allocs.Data[i].Ptr  = newPtr;
	Allocs.Data[i].Size = newSize;
	Allocs.Data[i].Line = __line;

	Allocs.Data[i].File     = malloc(strlen(__file) + 1);
	Allocs.Data[i].Function = malloc(strlen(__func) + 1);

	Allocs.Data[i].File[strlen(__file)]     = 0;
	Allocs.Data[i].Function[strlen(__func)] = 0;

	strcpy(Allocs.Data[i].File, __file);
	strcpy(Allocs.Data[i].Function, __func);

	return newPtr;
}

void Free(void* ptr, const char* __func, const char* __file, u32 __line) {
	if(!ptr) return;

	i32 i = Allocation_FindPtr(ptr);

	if(i < 0) {
		Log(ERROR, "Attempt to free invalid pointer 0x%x.", ptr);
		Log(ERROR, "  (Called from [%s:%d %s])", __file, __line, __func);
		return;
	}

	SizesSum -= Allocs.Data[i].Size;
	Array_Allocation_Remove(&Allocs, i);
	free(ptr);
}

void Alloc_FreeAll() {
	SizesSum = 0;
	for(u32 i = 0; i < Allocs.Size; ++i) free(Allocs.Data[i].Ptr);
	free(Allocs.Data);

	Allocs.Data     = NULL;
	Allocs.Size     = 0;
	Allocs.Capacity = 0;
}
#endif

void* Allocate(u32 size) { 
	return malloc(size);
}

void* Reallocate(void* ptr, u32 newSize) {
	return realloc(ptr, newSize);
}

void Free(void* ptr) {
	free(ptr);
}

u32 Alloc_GetTotalSize() {
	Log(WARN, "Allocation debugging disabled.", "");
	return 0;
}
void Alloc_PrintInfo() {
	Log(WARN, "Allocation debugging disabled.", "");
}
void Alloc_FreeAll() {
	Log(WARN, "Allocation debugging disabled.", "");
}

// --- Logging --- //

static const char* RESET = "\033[0m";

//static const char *CYAN    = "\033[36m";
static const char* BLUE    = "\033[34m";
//static const char *GREEN   = "\033[32m";
static const char* YELLOW  = "\033[33m";
static const char* RED     = "\033[31m";
static const char* BOLDRED = "\033[31;1m";

#undef Log

bool8 Log_ShortMode = 1;
enum Log_Level Log_Level_Global;

void Log(const char* __func,
         const char* __file,
         u32 __line,
         enum Log_Level level,
         const char* fmt,
         ...) {
	if(level < Log_Level_Global && level != FATAL) {
		return;
	}

	switch(level) {
		case DEBUG:
			fprintf(stdout,
			        Log_ShortMode ? "%sDEBUG%s: " : "%sDEBUG%s [%s:%d %s()]: ",
			        BLUE, RESET, __file, __line, __func);
			break;
		case INFO:
			if(!Log_ShortMode) 
				fprintf(stdout, "INFO [%s:%d %s()]: ", __file, __line, __func);
			break;
		case WARN:
			fprintf(stdout,
			        Log_ShortMode ? "%sWARNING%s: " : "%sWARNING%s [%s:%d %s()]: ",
			        YELLOW, RESET, __file, __line, __func);
			break;
		case ERROR:
			fprintf(stdout,
			        Log_ShortMode ? "%sERROR%s: " : "%sERROR%s [%s:%d %s()]: ",
			        RED, RESET, __file, __line, __func);
			break;
		case FATAL:
			fprintf(stderr, "%sFATAL ERROR%s [%s:%d %s()]: ",
			        BOLDRED, RESET, __file, __line, __func);
			break;
	}

	char msg[512] = {0};

	va_list args;
	va_start(args, fmt);
	vsprintf(msg, fmt, args);
	va_end(args);

	if(level == FATAL) {
		fprintf(stderr, "%s", msg);
		//SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "FATAL ERROR", msg, NULL);
		exit(EXIT_FAILURE);
	} else {
		fprintf(stdout, "%s", msg);
	}

	fprintf(stdout, "\n");
}
