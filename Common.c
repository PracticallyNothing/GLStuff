#include "Common.h"

#include <SDL.h>
#include <math.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

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

const r64 Tau = 6.28318530717958647692;
const r64 Pi = 3.14159265358979323846;
const r64 Pi_Half = 1.57079632679489661923;
const r64 Pi_Quarter = 0.78539816339744830962;

bool8 GL_Initialized = 0;

r64 DegToRad(r64 degrees) { return degrees * (Pi / 180.0); }
r64 RadToDeg(r64 radians) { return radians * (180.0 / Pi); }

r64 Clamp_R64(r64 value, r64 min, r64 max) {
	if(value < min)
		return min;
	else if(value > max)
		return max;
	else
		return value;
}

r32 Clamp_R32(r32 value, r32 min, r32 max) {
	if(value < min)
		return min;
	else if(value > max)
		return max;
	else
		return value;
}

i32 Clamp_I32(i32 value, i32 min, i32 max) {
	if(value < min)
		return min;
	else if(value > max)
		return max;
	else
		return value;
}

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

i32 String_ToI32(const char *str) { return String_ToI32_N(str, strlen(str)); }
i32 String_ToI32_N(const char *str, u32 len) {
	i32 res = 0;
	bool8 neg = 0;

	// Skip whitespace.
	while(*str == ' ' || *str == '\n' || *str == '\t') {++str; --len;}

	// Check if value is negative.
	if(*str == '-') {
		neg = 1;
		++str;
		--len;
	}

	for(i32 i = 0; i < len; i++) {
		res += (str[i] - '0') * Pow_I32(10, len - 1 - i);
	}
	return (neg ? -res : res);
}

static u32 File_GetSize(FILE *f) {
	fpos_t CurrPos;
	u32 Size;

	fgetpos(f, &CurrPos);
	fseek(f, 0, SEEK_END);
	Size = ftell(f);
	fsetpos(f, &CurrPos);

	return Size;
}

u8* File_ReadToBuffer_Alloc(const char *filename, u32 *size)
{
	FILE *File = fopen(filename, "rb");
	if(!File) {
		Log(Log_Error, "File \"%s\" doesn't exist.", filename);
		if(size) *size = 0;
		return NULL;
	}

	u32 Size = File_GetSize(File);
	u8 *buf = malloc(Size);
	fread(buf, sizeof(u8), Size, File);
	if(size) *size = Size;
	fclose(File);

	return buf;
}

i32 File_ReadToBuffer(const char *filename, u8 *buf, u32 bufSize,
                      u32 *realSize) {
	FILE *File;
	u32 Size;

	File = fopen(filename, "rb");
	if(!File) {
		Log(Log_Error, "File \"%s\" doesn't exist.", filename);
		return 0;
	}

	Size = File_GetSize(File);
	Size = (Size > bufSize ? bufSize : Size);
	fread(buf, sizeof(u8), Size, File);
	if(realSize) *realSize = Size;
	fclose(File);

	return 1;
}


void File_DumpBuffer(const char *filename, const u8 *buf, u32 bufSize) {
	FILE *File = fopen(filename, "wb");
	fwrite(buf, sizeof(u8), bufSize, File);
	fclose(File);
}

u64 Bytes(u32 amt) { return amt; }
u64 Kilobytes(u32 amt) { return amt * 1024; }
u64 Megabytes(u32 amt) { return amt * 1024 * 1024; }
u64 Gigabytes(u32 amt) { return amt * 1024 * 1024 * 1024; }

// Thank you,
// https://www.cs-fundamentals.com/tech-interview/c/c-program-to-check-little-and-big-endian-architecture.php
i32 PC_IsLittleEndian() {
	u32 x = 0x1;
	char *c = (char *) &x;
	return *c;
}

void PC_PrintVideoDriverInfo(void) {
	i32 NumVideoDrivers = SDL_GetNumVideoDrivers();
	if(NumVideoDrivers < 0) {
		printf("SDL_GetNumVideoDrivers() failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	printf("Num video drivers: %d\n", NumVideoDrivers);

	for(i32 i = 0; i < NumVideoDrivers; ++i) {
		printf("#%d: %s\n", i, SDL_GetVideoDriver(i));
	}
}

void PC_GetWindowSize(i32 *w, i32 *h) { SDL_GetWindowSize(NULL, w, h); }

// Breaks if A is NULL/invalid or lo > hi
u32 _QSort_Partition_i32(i32 *A, u32 lo, u32 hi) {
	i32 pivot = A[lo + (hi - lo) / 2];
	i32 i = lo - 1;
	i32 j = hi + 1;

	while(1) {
		do { i++; } while(A[i] < pivot);
		do { j--; } while(A[j] > pivot);
		if(i >= j) return j;
		SWAP_I32(A[i], A[j]);
	}
}

void Util_Quicksort_i32(i32 *arr, u32 size) {
	if(size < 2) return;

	if(size == 2) {
		if(arr[0] > arr[1]) { SWAP_I32(arr[0], arr[1]); }
		return;
	}

	u32 p = _QSort_Partition_i32(arr, 0, size - 1);
	Util_Quicksort_i32(arr, p);
	Util_Quicksort_i32(arr + p + 1, size - p);
}

u32 _QSort_Partition_r32(r32 *A, u32 lo, u32 hi) {
	r32 pivot = A[lo + (hi - lo) / 2];
	i32 i = lo - 1;
	i32 j = hi + 1;

	while(1) {
		do { i++; } while(A[i] < pivot);
		do { j--; } while(A[j] > pivot);
		if(i >= j) return j;
		SWAP_R32(A[i], A[j]);
	}
}

void Util_Quicksort_r32(r32 *arr, u32 size) {
	if(size < 2) return;

	if(size == 2) {
		if(arr[0] > arr[1]) { SWAP_R32(arr[0], arr[1]); }
		return;
	}

	u32 p = _QSort_Partition_r32(arr, 0, size - 1);
	Util_Quicksort_r32(arr, p);
	Util_Quicksort_r32(arr + p + 1, size - p - 1);
}

u32 _QSort_Partition_func(u8 *A, u32 lo, u32 hi, u32 itemSize,
                          Util_CompFunc f) {
	u8 *pivot = A + itemSize * (lo + (hi - lo) / 2);
	i32 i = lo - 1;
	i32 j = hi + 1;

	while(1) {
		do { i++; } while(f(A + itemSize * i, pivot) < 0);
		do { j--; } while(f(A + itemSize * j, pivot) > 0);
		if(i >= j) return j;
		SWAP_U8_ARR(A + itemSize * i, A + itemSize * j, itemSize);
	}
}

void Util_Quicksort_func(u8 *arr, u32 itemSize, u32 arrSize,
                         Util_CompFunc compFunc) {
	if(arrSize < 2) return;

	if(arrSize == 2) {
		if(compFunc(arr, arr + itemSize) > 0)
			SWAP_U8_ARR(arr, arr + itemSize, itemSize);
		return;
	}

	u32 p = _QSort_Partition_func(arr, 0, arrSize - 1, itemSize, compFunc);
	Util_Quicksort_func(arr, itemSize, p, compFunc);
	Util_Quicksort_func(arr + p + 1, itemSize, arrSize - p - 1, compFunc);
}

// --- Logging --- //

const char *RESET = "\033[0m";

const char *CYAN = "\033[36m";
const char *BLUE = "\033[34m";
const char *GREEN = "\033[32m";
const char *YELLOW = "\033[33m";
const char *RED = "\033[31m";
const char *BOLDRED = "\033[31;1m";

#undef Log

enum Log_Level Log_Level;
void Log(const char *__func, const char *__file, u32 __line,
         enum Log_Level level, const char *fmt, ...) {
	if(level < Log_Level && level != Log_Fatal) { return; }

	switch(level) {
		case Log_Debug:
			fprintf(stdout, 
					"%sDEBUG%s [%s:%d %s()]: ",
					BLUE, RESET, 
					__file, __line, __func);
			break;
		case Log_Info:
			fprintf(stdout, 
					"INFO [%s:%d %s()]: ", 
					__file, __line, __func);
			break;
		case Log_Warning:
			fprintf(stdout, 
					"%sWARNING%s [%s:%d %s()]: ", 
					YELLOW, RESET, 
					__file, __line, __func);
			break;
		case Log_Error:
			fprintf(stdout, 
					"%sERROR%s [%s:%d %s()]: ",
					RED, RESET, 
					__file, __line, __func);
			break;

		case Log_Fatal:
			fprintf(stderr, 
					"%sFATAL ERROR%s [%s:%d %s()]: ",
					BOLDRED, RESET,
			        __file, __line, __func);
			break;
	}

	char msg[512] = {0};

	va_list args;
	va_start(args, fmt);
	vsprintf(msg, fmt, args);
	va_end(args);

	if(level == Log_Fatal) {
		fprintf(stderr, "%s", msg);
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, 
			                     "FATAL ERROR", 
								 msg, 
			                     NULL);
		exit(EXIT_FAILURE);
	} else {
		fprintf(stdout, "%s", msg);
	}

	fprintf(stdout, "\n");
}

// --- Hashing --- ///

// Thank you,
// https://en.wikipedia.org/wiki/MD5#Pseudocode

u128 Hash_MD5(const u8 *bytes, u32 length)
{
	u32 shiftAmts[64] = { 
		7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22, //  0 - 15
		5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20, // 16 - 31
		4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23, // 32 - 47
		6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21, // 48 - 63
	};

#if 1
	// Precomputed table.
	u32 K[64] = {
		0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
		0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
		0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
		0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
		0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
		0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
		0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
		0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
		0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
		0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
		0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
		0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
		0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
		0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
		0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
		0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391 
	};
#else
	// Generated.
	i32 K[64] = {0};
	for(u32 i = 0; i < 63; i++)
		K[i] = floor(pow(2, 32) * fabs(sinf(i+1)));
#endif

	i32 a0 = 0x67452301; // A
	i32 b0 = 0xefcdab89; // B
	i32 c0 = 0x98badcfe; // C
	i32 d0 = 0x10325476; // D

	// append 0x80
	// append 0x00 until length % 64 = 56
	// append original length % 33 to message
	
	u32 padding = 0;
	while((length+1+padding) % 64 != 56) { ++padding; }

	u32 bufLength = length + 1 + padding + 1;

	u8 *buf = malloc(bufLength);
	buf[length] = 0x80;
	memset(buf+length+1, 0x00, bufLength-length);
	buf[bufLength-1] = length%33;

	for(u32 i = 0; i < bufLength; i+=64)
	{
		u32 A = a0,
			B = b0,
			C = c0,
			D = d0;
		u32 *M = (u32*) buf+i;

		for(u32 j = 0; j < 64; j++) 
		{
			u32 F, g;
			if(j <= 15) {
				F = (B & C) | ((~B) & D);
				g = j;
			} else if(j <= 31) {
				F = (D & B) | ((~D) & C);
				g = (5*j + 1) % 16;
			} else if(j <= 47) {
				F = B ^ C ^ D;
				g = (3*j + 5) % 16;
			} else {
				F = C ^ (B | (~D));
				g = (7*j) % 16;
			}

			F += A + K[j] + M[g];

			A = D;
			D = C;
			C = B;
			B += (F << shiftAmts[j]) | (F >> (32-shiftAmts[j]));
		}

		a0 = a0 + A;
		b0 = b0 + B;
		c0 = c0 + C;
		d0 = d0 + D;
	}

	u128 res = { .b = {a0, b0, c0, d0} };
	return res;
}
