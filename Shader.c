#include "Shader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Shader *Shader_FromFile(const char *file) {
	struct Shader *Res = NULL;
	u32 bufSize = Kilobytes(16);
	char *src = calloc(1, bufSize);

	if(!File_ReadToBuffer(file, (u8 *) src, bufSize, NULL)) { goto end; }

	char *vertSrc, *fragSrc;

	{ /* Read vertex segment */
		char *start = strstr(src, "@vert") + strlen("@vert"),
		     *end = strstr(start, "@@");

		u32 len = end - start;
		vertSrc = calloc(len + 1, sizeof(char));
		strncpy(vertSrc, start, len);
	}

	{ /* Read fragment segment */
		char *start = strstr(src, "@frag") + strlen("@frag"),
		     *end = strstr(start, "@@");

		u32 len = end - start;
		fragSrc = calloc(len + 1, sizeof(char));
		strncpy(fragSrc, start, len);
	}

	Res = Shader_FromSrc(vertSrc, fragSrc);
	free(vertSrc);
	free(fragSrc);
end:
	free(src);
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
		char *LogStr;

		glGetShaderiv(Shader, GL_INFO_LOG_LENGTH, &LogLength);
		LogStr = malloc(sizeof(char) * LogLength);
		glGetShaderInfoLog(Shader, LogLength, NULL, LogStr);

		Log(Log_Error, "%s shader compilation failed.\n%s",
		    (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment"), LogStr);
		free(LogStr);
	}

	return Shader;
}

GLuint _Shader_Link(struct Shader *s) {
	GLuint ShaderProgram;
	i32 ShaderProgramOK;

	ShaderProgram = glCreateProgram();
	glAttachShader(ShaderProgram, s->VertexID);
	glAttachShader(ShaderProgram, s->FragmentID);
	glLinkProgram(ShaderProgram);

	glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &ShaderProgramOK);
	if(ShaderProgramOK != GL_TRUE) {
		i32 LogLength;
		char *LogStr;

		glGetProgramiv(ShaderProgram, GL_INFO_LOG_LENGTH, &LogLength);
		LogStr = malloc(sizeof(char) * LogLength);
		glGetProgramInfoLog(ShaderProgram, LogLength, NULL, LogStr);

		Log(Log_Error, "Shader program linking failed.\n%s", LogStr);
		free(LogStr);
	}
	return ShaderProgram;
}

struct Shader *Shader_FromSrc(const char *vertexSrc, const char *fragmentSrc) {
	struct Shader *Res = malloc(sizeof(struct Shader));
	memset(Res, 0, sizeof(struct Shader));
	Res->VertexFile = NULL;
	Res->FragmentFile = NULL;

	Res->VertexID = _Shader_GenShader(GL_VERTEX_SHADER, vertexSrc);
	Res->FragmentID = _Shader_GenShader(GL_FRAGMENT_SHADER, fragmentSrc);
	Res->ProgramID = _Shader_Link(Res);

	return Res;
}

void Shader_Use(struct Shader *s) { glUseProgram(s->ProgramID); }

void Shader_Uniform1i(struct Shader *s, const char *name, i32 v) {
	glUniform1i(glGetUniformLocation(s->ProgramID, name), v);
}

void Shader_Uniform1f(struct Shader *s, const char *name, r32 v) {
	glUniform1f(glGetUniformLocation(s->ProgramID, name), v);
}
void Shader_Uniform2f(struct Shader *s, const char *name, Vec2 v) {
	glUniform2f(glGetUniformLocation(s->ProgramID, name), v.x, v.y);
}
void Shader_Uniform3f(struct Shader *s, const char *name, Vec3 v) {
	glUniform3f(glGetUniformLocation(s->ProgramID, name), v.x, v.y, v.z);
}
void Shader_Uniform4f(struct Shader *s, const char *name, Vec4 v) {
	glUniform4f(glGetUniformLocation(s->ProgramID, name), v.x, v.y, v.z, v.w);
}

void Shader_Uniform1fv(struct Shader *s, const char *name, const r32 *v,
                       u32 count) {
	glUniform1fv(glGetUniformLocation(s->ProgramID, name), count, v);
}
void Shader_Uniform2fv(struct Shader *s, const char *name, const Vec2 *v,
                       u32 count) {
	glUniform2fv(glGetUniformLocation(s->ProgramID, name), count, (r32 *) v);
}
void Shader_Uniform3fv(struct Shader *s, const char *name, const Vec3 *v,
                       u32 count) {
	glUniform3fv(glGetUniformLocation(s->ProgramID, name), count, (r32 *) v);
}
void Shader_Uniform4fv(struct Shader *s, const char *name, const Vec4 *v,
                       u32 count) {
	glUniform4fv(glGetUniformLocation(s->ProgramID, name), count, (r32 *) v);
}

void Shader_UniformMat2(struct Shader *s, const char *name, const Mat2 m) {
	glUniformMatrix2fv(glGetUniformLocation(s->ProgramID, name), 1, GL_TRUE, m);
}
void Shader_UniformMat3(struct Shader *s, const char *name, const Mat3 m) {
	glUniformMatrix3fv(glGetUniformLocation(s->ProgramID, name), 1, GL_TRUE, m);
}
void Shader_UniformMat4(struct Shader *s, const char *name, const Mat4 m) {
	i32 Loc = glGetUniformLocation(s->ProgramID, name);
	glUniformMatrix4fv(Loc, 1, GL_TRUE, m);
}

void Shader_Free(struct Shader *s) {
	glDeleteShader(s->VertexID);
	glDeleteShader(s->FragmentID);
	glDeleteProgram(s->ProgramID);
	free(s);
}
