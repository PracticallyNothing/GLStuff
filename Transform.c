#include "Transform.h"

#include <math.h>

void Transform2D_Mat3(Transform2D t, Mat3 out) {
	Mat3 translate, rotate, scale, parent;

	Mat3_Identity(translate);
	Mat3_Identity(rotate);
	Mat3_Identity(scale);
	Mat3_Identity(parent);

	// Translation
	translate[2] = t.Position.x;
	translate[5] = t.Position.y;

	// Rotation
	rotate[0] = cosf(t.Rotation);
	rotate[1] = -sinf(t.Rotation);
	rotate[3] = sinf(t.Rotation);
	rotate[4] = cosf(t.Rotation);

	// Scale
	scale[0] = t.Scale.x;
	scale[4] = t.Scale.y;

	// Parent
	if(t.Parent && t.Parent != &t) Transform2D_Mat3(*t.Parent, parent);

	Mat3_MultMat(rotate, scale);
	Mat3_MultMat(translate, rotate);
	Mat3_MultMat(parent, translate);
	Mat3_Copy(out, parent);
}

void Transform3D_Mat4(Transform3D t, Mat4 out) {
	Mat4 translate, rotate, scale, parent;

	Mat4_Identity(translate);
	Mat4_Identity(rotate);
	Mat4_Identity(scale);
	Mat4_Identity(parent);

	// Translation
	Mat4_Translate(translate, t.Position);

	// Rotation
	Mat4_RotateQuat(rotate, t.Rotation);

	// Scale
	Mat4_Scale(scale, t.Scale);

	// Parent matrix
	if(t.Parent && t.Parent != &t) Transform3D_Mat4(*t.Parent, parent);

	Mat4_MultMat(rotate, scale);
	Mat4_MultMat(translate, rotate);
	Mat4_MultMat(parent, translate);
	Mat4_Copy(out, parent);
}
