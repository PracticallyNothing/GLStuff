#include "Common.h"

#include <stdio.h>

const r64 Tau = 6.28318530717958647692;
const r64 Pi = 3.14159265358979323846;
const r64 Pi_Half = 1.57079632679489661923;
const r64 Pi_Quarter = 0.78539816339744830962;

r64 DegToRad(r64 degrees) { return degrees * Pi / 180.0; }
r64 RadToDeg(r64 radians) { return radians * 180.0 / Pi; }

static u32 File_GetSize(FILE *f) {
    fpos_t CurrPos;
    u32 Size;

    fgetpos(f, &CurrPos);
    fseek(f, 0, SEEK_END);
    Size = ftell(f);
    fsetpos(f, &CurrPos);

    return Size;
}

i32 File_ReadToBuffer(const char *filename, u8 *buf, u32 bufSize) {
    FILE *File;
    u32 Size;

    File = fopen(filename, "rb");
    if(!File) { return 0; }

    Size = File_GetSize(File);
    Size = (Size > bufSize ? bufSize : Size);
    fread(buf, sizeof(u8), Size, File);
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
