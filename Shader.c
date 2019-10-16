#include "Shader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Shader *Shader_FromFiles(const char *vertexFile, const char *fragmentFile) {
	Shader *Res;
	char *VertexSrc, *FragmentSrc;

	Res = NULL;
	VertexSrc = calloc(1, Megabytes(1));
	FragmentSrc = calloc(1, Megabytes(1));

	if(!File_ReadToBuffer(vertexFile, (u8 *) VertexSrc, Megabytes(1), NULL) ||
	   !File_ReadToBuffer(fragmentFile, (u8 *) FragmentSrc, Megabytes(1),
	                      NULL)) {
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

static GLuint _Shader_GenShader(GLenum type, const char *src) {
	u32 Shader;
	i32 ShaderOK;

	Shader = glCreateShader(type);
	glShaderSource(Shader, 1, &src, NULL);
	glCompileShader(Shader);

	glGetShaderiv(Shader, GL_COMPILE_STATUS, &ShaderOK);
	if(ShaderOK != GL_TRUE) {
		i32 LogLength;
		char *Log;

		glGetShaderiv(Shader, GL_INFO_LOG_LENGTH, &LogLength);
		Log = malloc(sizeof(char) * LogLength);
		glGetShaderInfoLog(Shader, LogLength, NULL, Log);

		// TODO: Need better way to report errors.
		printf("%s shader compilation failed.\n%s\n",
		       (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment"), Log);
		free(Log);
	}

	return Shader;
}

GLuint _Shader_Link(Shader *s) {
	GLuint ShaderProgram;
	i32 ShaderProgramOK;

	ShaderProgram = glCreateProgram();
	glAttachShader(ShaderProgram, s->VertexID);
	glAttachShader(ShaderProgram, s->FragmentID);
	glLinkProgram(ShaderProgram);

	glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &ShaderProgramOK);
	if(ShaderProgramOK != GL_TRUE) {
		i32 LogLength;
		char *Log;

		glGetProgramiv(ShaderProgram, GL_INFO_LOG_LENGTH, &LogLength);
		Log = malloc(sizeof(char) * LogLength);
		glGetProgramInfoLog(ShaderProgram, LogLength, NULL, Log);

		// TODO: Need better way to report errors.
		printf("Shader program linking failed.\n%s\n", Log);
		free(Log);
	}
	return ShaderProgram;
}

Shader *Shader_FromSrc(const char *vertexSrc, const char *fragmentSrc) {
	Shader *Res = malloc(sizeof(Shader));
	memset(Res, 0, sizeof(Shader));
	Res->VertexFile = NULL;
	Res->FragmentFile = NULL;

	Res->VertexID = _Shader_GenShader(GL_VERTEX_SHADER, vertexSrc);
	Res->FragmentID = _Shader_GenShader(GL_FRAGMENT_SHADER, fragmentSrc);
	Res->ProgramID = _Shader_Link(Res);

	return Res;
}

void Shader_Use(Shader *s) { glUseProgram(s->ProgramID); }

void Shader_Uniform1i(Shader *s, const char *name, i32 v) {
	glUniform1i(glGetUniformLocation(s->ProgramID, name), v);
}

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
	glUniform2fv(glGetUniformLocation(s->ProgramID, name), count, (r32 *) v);
}
void Shader_Uniform3fv(Shader *s, const char *name, const Vec3 *v, u32 count) {
	glUniform3fv(glGetUniformLocation(s->ProgramID, name), count, (r32 *) v);
}
void Shader_Uniform4fv(Shader *s, const char *name, const Vec4 *v, u32 count) {
	glUniform4fv(glGetUniformLocation(s->ProgramID, name), count, (r32 *) v);
}

void Shader_UniformMat2(Shader *s, const char *name, const Mat2 m) {
	glUniformMatrix2fv(glGetUniformLocation(s->ProgramID, name), 1, GL_TRUE, m);
}
void Shader_UniformMat3(Shader *s, const char *name, const Mat3 m) {
	glUniformMatrix3fv(glGetUniformLocation(s->ProgramID, name), 1, GL_TRUE, m);
}
void Shader_UniformMat4(Shader *s, const char *name, const Mat4 m) {
	i32 Loc = glGetUniformLocation(s->ProgramID, name);
	glUniformMatrix4fv(Loc, 1, GL_TRUE, m);
}

void Shader_Free(Shader *s) {
	glDeleteShader(s->VertexID);
	glDeleteShader(s->FragmentID);
	glDeleteProgram(s->ProgramID);
	free(s);
}
