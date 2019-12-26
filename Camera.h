#ifndef CAMERA_H
#define CAMERA_H

#include "Common.h"
#include "Math3D.h"

typedef struct Camera Camera;

enum CameraMode {
	CameraMode_Orthographic,
	CameraMode_Perspective,

	CameraMode_NumModes,
};

struct Camera {
	// Generic properties
	Vec3 Position;
	Vec3 Target;
	Vec3 Up;
	r32 ZNear, ZFar;

	enum CameraMode Mode;

	// Orthographic specific:
	i32 ScreenWidth;
	i32 ScreenHeight;
	// Perspective specific:
	r32 VerticalFoV;
	r32 AspectRatio;
};

extern void Camera_Mat4(Camera c, Mat4 out_view, Mat4 out_proj);

#endif
