#include "Shader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Common.h"

Shader *Shader_FromFile(const char *file) {
	Shader *Res = NULL;
	u32 bufSize;
	char *src = (char *) File_ReadToBuffer_Alloc(file, &bufSize);

	if(!src) { return NULL; }

	char *vertSrc, *fragSrc;

	{ /* Read vertex segment */
		char *start = strstr(src, "@vert") + strlen("@vert"),
		     *end   = strstr(start, "@@");

		u32 len = end - start;
		vertSrc = Allocate((len + 1) * sizeof(char));
		vertSrc[len] = '\0';
		strncpy(vertSrc, start, len);
	}

	{ /* Read fragment segment */
		char *start = strstr(src, "@frag") + strlen("@frag"),
		     *end = strstr(start, "@@");

		u32 len = end - start;
		fragSrc = Allocate((len + 1) * sizeof(char));
		fragSrc[len] = '\0';
		strncpy(fragSrc, start, len);
	}

	Res = Shader_FromSrc(vertSrc, fragSrc, file);
	Free(vertSrc);
	Free(fragSrc);
	Free(src);
	return Res;
}

static GLuint _Shader_GenShader(GLenum type, const char *src, const char* filename) {
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
		LogStr = Allocate(sizeof(char) * LogLength);
		glGetShaderInfoLog(Shader, LogLength, NULL, LogStr);

		Log(ERROR, "%s shader compilation failed.\n%s",
		    (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment"), LogStr);
		Free(LogStr);
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
		char *LogStr;

		glGetProgramiv(ShaderProgram, GL_INFO_LOG_LENGTH, &LogLength);
		LogStr = Allocate(sizeof(char) * LogLength);
		glGetProgramInfoLog(ShaderProgram, LogLength, NULL, LogStr);

		Log(ERROR, "[%s] Shader program linking failed.\n%s", 
			s->SrcFile ? s->SrcFile : "Unknown source", LogStr);
		Free(LogStr);
	}
	return ShaderProgram;
}

Shader *Shader_FromSrc(const char *vertexSrc, const char *fragmentSrc, const char* filename) {
	Shader *Res = Allocate(sizeof(Shader));
	memset(Res, 0, sizeof(Shader));
	Res->SrcFile = NULL;
	if(filename) {
		Res->SrcFile = Allocate(strlen(filename)+1);
		strcpy(Res->SrcFile, filename);
	}

	Res->VertexID = _Shader_GenShader(GL_VERTEX_SHADER, vertexSrc, filename);
	Res->FragmentID = _Shader_GenShader(GL_FRAGMENT_SHADER, fragmentSrc, filename);
	Res->ProgramID = _Shader_Link(Res);

	return Res;
}

const Shader *ActiveShader = NULL;
void Shader_Use(Shader *s) { 
	if(ActiveShader != s) {
		glUseProgram(s->ProgramID); 
		ActiveShader = s;
	}
}

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

void Shader_Uniform1fv(Shader *s, const char *name, const r32 *v,
                       u32 count) {
	glUniform1fv(glGetUniformLocation(s->ProgramID, name), count, v);
}
void Shader_Uniform2fv(Shader *s, const char *name, const Vec2 *v,
                       u32 count) {
	glUniform2fv(glGetUniformLocation(s->ProgramID, name), count, (r32 *) v);
}
void Shader_Uniform3fv(Shader *s, const char *name, const Vec3 *v,
                       u32 count) {
	glUniform3fv(glGetUniformLocation(s->ProgramID, name), count, (r32 *) v);
}
void Shader_Uniform4fv(Shader *s, const char *name, const Vec4 *v,
                       u32 count) {
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
	if(s->SrcFile)
		Free(s->SrcFile);
	Free(s);
}

void Shader_Reload(Shader *s)
{
	if(!s || !s->SrcFile) {
		return;
	}

	glDeleteShader(s->VertexID);
	glDeleteShader(s->FragmentID);
	glDeleteProgram(s->ProgramID);

	Shader *ss = Shader_FromFile(s->SrcFile);
	s->VertexID = ss->VertexID;
	s->FragmentID = ss->FragmentID;
	s->ProgramID = ss->ProgramID;
	Free(ss);

	if(ActiveShader == s)
		Shader_Use(s);
}
