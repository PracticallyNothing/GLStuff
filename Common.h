#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

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

// --- Pi, degrees and radians --- //

extern const r64 Tau;
extern const r64 Pi;
extern const r64 Pi_Half;
extern const r64 Pi_Quarter;

extern r64 DegToRad(r64 degrees);
extern r64 RadToDeg(r64 radians);

// --- File operations --- //

extern i32 File_ReadToBuffer(const char *filename, u8 *buf, u32 bufSize);
extern void File_DumpBuffer(const char *filename, const u8 *buf, u32 bufSize);

// --- Size units --- //

extern u64 Bytes(u32 amt);
extern u64 Kilobytes(u32 amt);
extern u64 Megabytes(u32 amt);
extern u64 Gigabytes(u32 amt);

// --- Utility --- //

extern i32 PC_IsLittleEndian(void);
extern void PC_PrintVideoDriverInfo(void);

#endif
