#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "Math3D.h"

typedef struct Transform2D Transform2D;
typedef struct Transform3D Transform3D;

struct Transform2D {
	Vec2 Position;
	r32 Rotation;
	Vec2 Scale;
	Transform2D *Parent;
};

struct Transform3D {
	Vec3 Position;
	Quat Rotation;
	Vec3 Scale;
	Transform3D *Parent;
};

void Transform2D_Mat3(Transform2D t, Mat3 out);
void Transform3D_Mat4(Transform3D t, Mat4 out);

#endif
