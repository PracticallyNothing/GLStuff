#include "Utils.h"

#include "Common.h"

DEF_ARRAY(Vec2, Vec2);
DEF_ARRAY(Vec3, Vec3);

#define VBO_POS 0
#define VBO_UV 1
#define VBO_NORM 2

void WObj_ToGPUModel(GPUModel *out, const WObj_Object *obj) {
	glGenVertexArrays(1, &out->VAO);
	glBindVertexArray(out->VAO);
	glGenBuffers(3, out->VBOs);
	glGenBuffers(1, &out->ElementBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, out->ElementBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * obj->NumIndices,
	             obj->Indices, GL_STATIC_DRAW);

	out->NumVertices = obj->NumVertices;
	out->NumIndices = obj->NumIndices;

	Array_Vec3 Positions = {0};
	Array_Vec2 UVs = {0};
	Array_Vec3 Normals = {0};

	for(i32 i = 0; i < obj->NumVertices; i++) {
		WObj_Vertex *vtx = &obj->Vertices[i];
		Array_Vec3_Push(&Positions, &vtx->Position);
		Array_Vec2_Push(&UVs, &vtx->UV);
		Array_Vec3_Push(&Normals, &vtx->Normal);
	}

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, out->VBOs[VBO_POS]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vec3) * obj->NumVertices,
	             Positions.Data, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, out->VBOs[VBO_UV]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vec2) * obj->NumVertices, UVs.Data,
	             GL_STATIC_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, out->VBOs[VBO_NORM]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vec3) * obj->NumVertices, Normals.Data,
	             GL_STATIC_DRAW);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	Array_Vec3_Free(&Positions);
	Array_Vec2_Free(&UVs);
	Array_Vec3_Free(&Normals);
}

void GPUModel_Render(const GPUModel *model) {
	glBindVertexArray(model->VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->ElementBuffer);
	glDrawElements(GL_TRIANGLES, model->NumIndices, GL_UNSIGNED_INT, NULL);
}
