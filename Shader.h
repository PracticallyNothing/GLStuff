#ifndef SHADER_H
#define SHADER_H

#include "Common.h"
#include "Math3D.h"
#include "glad/glad.h"

struct Shader {
	GLuint ProgramID;
	GLuint VertexID, FragmentID;
	char *SrcFile;
};
extern struct Shader *Shader_FromFile(const char *file);
extern struct Shader *Shader_FromSrc(const char *vertexSrc,
                                     const char *fragmentSrc,
									 const char* filename);

extern void Shader_Use(struct Shader *);

extern void Shader_Uniform1i(struct Shader *, const char *name, i32);
extern void Shader_Uniform1f(struct Shader *, const char *name, r32);
extern void Shader_Uniform2f(struct Shader *, const char *name, Vec2);
extern void Shader_Uniform3f(struct Shader *, const char *name, Vec3);
extern void Shader_Uniform4f(struct Shader *, const char *name, Vec4);

extern void Shader_Uniform1fv(struct Shader *, const char *name, const r32 *,  u32 count);
extern void Shader_Uniform2fv(struct Shader *, const char *name, const Vec2 *, u32 count);
extern void Shader_Uniform3fv(struct Shader *, const char *name, const Vec3 *, u32 count);
extern void Shader_Uniform4fv(struct Shader *, const char *name, const Vec4 *, u32 count);

extern void Shader_UniformMat2(struct Shader *, const char *name, const Mat2);
extern void Shader_UniformMat3(struct Shader *, const char *name, const Mat3);
extern void Shader_UniformMat4(struct Shader *, const char *name, const Mat4);

extern void Shader_Free(struct Shader *);

extern void Shader_Reload(struct Shader *);

#endif
