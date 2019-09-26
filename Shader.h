#ifndef SHADER_H
#define SHADER_H

#include "Common.h"
#include "Math3D.h"
#include "glad_Core-33/include/glad/glad.h"

struct Shader_t {
    GLuint ProgramID;

    GLuint VertexID, FragmentID;
    const char *VertexFile, *FragmentFile;
};
typedef struct Shader_t Shader;

extern Shader *Shader_FromFiles(const char *vertexFile, const char *fragmentFile);
extern Shader *Shader_FromSrc(const char *vertexSrc, const char *fragmentSrc);

extern void Shader_Use(Shader *);

extern void Shader_Uniform1f(Shader *, const char *name, r32);
extern void Shader_Uniform2f(Shader *, const char *name, Vec2);
extern void Shader_Uniform3f(Shader *, const char *name, Vec3);
extern void Shader_Uniform4f(Shader *, const char *name, Vec4);

extern void Shader_Uniform1fv(Shader *, const char *name, const r32 *, u32 count);
extern void Shader_Uniform2fv(Shader *, const char *name, const Vec2 *, u32 count);
extern void Shader_Uniform3fv(Shader *, const char *name, const Vec3 *, u32 count);
extern void Shader_Uniform4fv(Shader *, const char *name, const Vec4 *, u32 count);

extern void Shader_UniformMat2(Shader *, const char *name, const Mat2);
extern void Shader_UniformMat3(Shader *, const char *name, const Mat3);
extern void Shader_UniformMat4(Shader *, const char *name, const Mat4);

extern void Shader_Free(Shader *);

#endif
