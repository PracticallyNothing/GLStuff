#ifndef UTILS_H
#define UTILS_H

#include "WavefrontOBJ.h" // For WObj_Object
#include "glad/glad.h"    // For GLuint

typedef struct GPUModel {
	GLuint VAO;
	GLuint VBOs[3];
	GLuint ElementBuffer;
	u32 NumVertices;
	u32 NumIndices;
} GPUModel;

void GPUModel_Render(const GPUModel *model);
void WObj_ToGPUModel(GPUModel *out, const WObj_Object *obj);

#endif
