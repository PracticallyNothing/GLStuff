#ifndef SHADER_H
#define SHADER_H

#include "Common.h"
#include "Math3D.h"
#include "glad/glad.h"

typedef struct Shader Shader;

struct Shader {
	GLuint ProgramID;
	GLuint VertexID, FragmentID;
	char *SrcFile;
};

/// Load a shader from a given file. 
/// The vertex/fragment shader begins with a @vert/@frag line and ends with an @@ line.
Shader *Shader_FromFile(const char *file);

/// Create a shader from two source strings.
/// The filename is used for shader reloading and should be NULL.
Shader *Shader_FromSrc(const char *vertexSrc,
                                     const char *fragmentSrc,
									 const char* filename);

void Shader_Use(Shader *);

void Shader_Uniform1i(Shader *, const char *name, i32);

void Shader_Uniform1f(Shader *, const char *name, r32);
void Shader_Uniform2f(Shader *, const char *name, Vec2);
void Shader_Uniform3f(Shader *, const char *name, Vec3);
void Shader_Uniform4f(Shader *, const char *name, Vec4);

void Shader_Uniform1fv(Shader *, const char *name, const r32 *,  u32 count);
void Shader_Uniform2fv(Shader *, const char *name, const Vec2 *, u32 count);
void Shader_Uniform3fv(Shader *, const char *name, const Vec3 *, u32 count);
void Shader_Uniform4fv(Shader *, const char *name, const Vec4 *, u32 count);

void Shader_UniformMat2(Shader *, const char *name, const Mat2);
void Shader_UniformMat3(Shader *, const char *name, const Mat3);
void Shader_UniformMat4(Shader *, const char *name, const Mat4);

void Shader_Free(Shader *);

void Shader_Reload(Shader *);

#endif
