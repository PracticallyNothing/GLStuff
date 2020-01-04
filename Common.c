#include "Common.h"

#include <SDL2/SDL.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

const r64 Tau = 6.28318530717958647692;
const r64 Pi = 3.14159265358979323846;
const r64 Pi_Half = 1.57079632679489661923;
const r64 Pi_Quarter = 0.78539816339744830962;

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
	for(i32 i = 0; i < len; i++) {
		res += (str[i] - '0') * Pow_I32(10, len - 1 - i);
	}
	return res;
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

i32 File_ReadToBuffer(const char *filename, u8 *buf, u32 bufSize,
                      u32 *realSize) {
	FILE *File;
	u32 Size;

	File = fopen(filename, "rb");
	if(!File) { return 0; }

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

Array *Array_Init(u32 itemSize) {
	if(!itemSize) return NULL;

	Array *a = malloc(sizeof(Array));

	a->Data = malloc(itemSize);
	a->ItemSize = itemSize;
	a->ArraySize = 0;
	a->ArrayCapacity = 1;

	return a;
}

Array *Array_Prealloc(u32 itemSize, u32 capacity) {
	if(!capacity) return NULL;

	Array *a = malloc(sizeof(Array));

	a->Data = malloc(itemSize * capacity);
	a->ItemSize = itemSize;
	a->ArraySize = 0;
	a->ArrayCapacity = capacity;

	return a;
}

const r32 Array_GrowthFactor = 2;

void Array_Push(Array *a, const u8 *item) {
	if(a->ArraySize >= a->ArrayCapacity) {
		a->ArrayCapacity *= Array_GrowthFactor;
		a->Data = realloc(a->Data, a->ItemSize * a->ArrayCapacity);
	}
	memcpy(a->Data + a->ItemSize * a->ArraySize, item, a->ItemSize);
	a->ArraySize++;
}

void Array_CopyData(u8 *out, const Array *src) {
	for(u32 i = 0; i < src->ArraySize; i++) {
		u8 *to = out + i * src->ItemSize;
		u8 *from = Array_Get(src, i);
		memcpy(to, from, src->ItemSize);
	}
}

void Array_Copy(Array *out, const Array *src) {
	out->ArraySize = src->ArraySize;
	out->ArrayCapacity = src->ArrayCapacity;
	out->ItemSize = src->ItemSize;

	for(u32 i = 0; i < out->ArraySize; i++) {
		u8 *to = out->Data + i * out->ItemSize;
		u8 *from = src->Data + i * src->ItemSize;
		memcpy(to, from, out->ItemSize);
	}
}

void Array_Clear(Array *a) { a->ArraySize = 0; }
void Array_Reverse(Array *a) {
	for(u32 i = 0; i < a->ArraySize / 2; i++) {
		u8 *A = a->Data + i * a->ItemSize;
		u8 *B = a->Data + (a->ArraySize - i) * a->ItemSize;
		SWAP_U8_ARR(A, B, a->ItemSize);
	}
}

u8 *Array_Get(const Array *a, i32 idx) {
	if(idx < 0 || idx >= a->ArraySize) return NULL;
	return a->Data + idx * a->ItemSize;
}

u8 *Array_GetFirst(const Array *arr) { return Array_Get(arr, 0); }
u8 *Array_GetLast(const Array *arr) {
	return Array_Get(arr, arr->ArraySize - 1);
}

void Array_Pop(Array *a) {
	if(a->ArraySize > 0) a->ArraySize--;
}

void Array_Free(Array *a) {
	free(a->Data);
	free(a);
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

#include <stdarg.h>

const char *CYAN = "\033[36m";
const char *BLUE = "\033[34m";
const char *GREEN = "\033[32m";
const char *YELLOW = "\033[33m";
const char *RED = "\033[31m";
const char *BOLDRED = "\033[31;1m";

#define VA_PRINT                     \
	do {                             \
		va_list args;                \
		va_start(args, fmt);         \
		vfprintf(stdout, fmt, args); \
		va_end(args);                \
	} while(0)

void Log_Debug(const char *fmt, ...) {
	fprintf(stdout, "%sDEBUG%s: ", BLUE, RESET);
	VA_PRINT;
	fprintf(stdout, "\n");
}

void Log_Info(const char *fmt, ...) {
	fprintf(stdout, "INFO: ");
	VA_PRINT;
	fprintf(stdout, "\n");
}

void Log_Warning(const char *fmt, ...) {
	fprintf(stdout, "%sWARNING%s: ", YELLOW, RESET);
	VA_PRINT;
	fprintf(stdout, "\n");
}
void Log_Error(const char *fmt, ...) {
	fprintf(stdout, "%sERROR%s: ", RED, RESET);
	VA_PRINT;
	fprintf(stdout, "\n");
}
void Log_FatalError(const char *fmt, ...) {
	char msg[512] = {0};

	va_list args;
	va_start(args, fmt);
	vsprintf(msg, fmt, args);
	va_end(args);

	fprintf(stderr, "%sFATAL ERROR%s: %s\n", BOLDRED, RESET, msg);
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal error", msg, NULL);
	exit(EXIT_FAILURE);
}

