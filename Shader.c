#include "Shader.h"

#include <stdio.h>
#include <stdlib.h>

Shader *Shader_FromFiles(const char *vertexFile, const char *fragmentFile) {
    Shader *Res;
    char *VertexSrc, *FragmentSrc;

    Res = NULL;
    VertexSrc = malloc(Megabytes(1));
    FragmentSrc = malloc(Megabytes(1));

    if(!File_ReadToBuffer(vertexFile, (u8*) VertexSrc, Megabytes(1)) ||
       !File_ReadToBuffer(fragmentFile, (u8*) FragmentSrc, Megabytes(1)))
    {
	goto end;
    }

    Res = Shader_FromSrc(VertexSrc, FragmentSrc);
    Res->VertexFile = vertexFile;
    Res->FragmentFile = fragmentFile;

end:
    free(VertexSrc);
    free(FragmentSrc);
    return Res;
}

Shader *Shader_FromSrc(const char *vertexSrc, const char *fragmentSrc) {
    
    return NULL;
}

void Shader_Use(Shader *s) { glUseProgram(s->ProgramID); }

void Shader_Uniform1f(Shader *s, const char *name, r32 v) {
    glUniform1f(glGetUniformLocation(s->ProgramID, name), v);
}
void Shader_Uniform2f(Shader *s, const char *name, Vec2 v) {
    glUniform2f(glGetUniformLocation(s->ProgramID, name), v.x, v.y);
}
void Shader_Uniform3f(Shader *s, const char *name, Vec3 v) {
    glUniform3f(glGetUniformLocation(s->ProgramID, name), v.x, v.y, v.z);
}
void Shader_Uniform4f(Shader *s, const char *name, Vec4 v) {
    glUniform4f(glGetUniformLocation(s->ProgramID, name), v.x, v.y, v.z, v.w);
}

void Shader_Uniform1fv(Shader *s, const char *name, const r32 *v, u32 count) {
    glUniform1fv(glGetUniformLocation(s->ProgramID, name), count, v);
}
void Shader_Uniform2fv(Shader *s, const char *name, const Vec2 *v, u32 count) {
    glUniform2fv(glGetUniformLocation(s->ProgramID, name), count, (r32*) v);
}
void Shader_Uniform3fv(Shader *s, const char *name, const Vec3 *v, u32 count) {
    glUniform3fv(glGetUniformLocation(s->ProgramID, name), count, (r32*) v);
}
void Shader_Uniform4fv(Shader *s, const char *name, const Vec4 *v, u32 count) {
    glUniform4fv(glGetUniformLocation(s->ProgramID, name), count, (r32*) v);
}

void Shader_UniformMat2(Shader *s, const char *name, const Mat2 m) {
    glUniformMatrix2fv(
	glGetUniformLocation(s->ProgramID, name),
	0, GL_FALSE, m
    );
}
void Shader_UniformMat3(Shader *s, const char *name, const Mat3 m) {
    glUniformMatrix3fv(
	glGetUniformLocation(s->ProgramID, name),
	0, GL_FALSE, m
    );
}
void Shader_UniformMat4(Shader *s, const char *name, const Mat4 m) {
    glUniformMatrix4fv(
	glGetUniformLocation(s->ProgramID, name),
	0, GL_FALSE, m
    );
}

void Shader_Free(Shader *s) {
    glDeleteShader(s->VertexID);
    glDeleteShader(s->FragmentID);
    glDeleteProgram(s->ProgramID);
}
