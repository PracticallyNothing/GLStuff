#include "Camera.h"

#include <stdlib.h>

// Thank you,
// http://ogldev.atspace.co.uk/www/tutorial13/tutorial13.html
void Camera_Mat4(Camera c, Mat4 out_view, Mat4 out_proj) {
	Mat4 Translation, Rotation;

	Mat4_Identity(Translation);
	Mat4_Identity(Rotation);

	Mat4_Translate(Translation, Vec3_Neg(c.Position));

	Vec3 tgt = Vec3_Sub(c.Target, c.Position);

	Vec3 N = Vec3_Norm(tgt);
	Vec3 U = Vec3_Norm(Vec3_Cross(c.Up, tgt));
	Vec3 V = Vec3_Cross(N, U);

	// printf("%.2f %.2f %.2f\n", N.x, N.y, N.z);

	// N points towards the target point
	// U points to the right of the camera
	// V points towards the sky

	Rotation[0] = U.x;
	Rotation[1] = U.y;
	Rotation[2] = U.z;

	Rotation[4] = V.x;
	Rotation[5] = V.y;
	Rotation[6] = V.z;

	Rotation[8] = N.x;
	Rotation[9] = N.y;
	Rotation[10] = N.z;

	Mat4_MultMat(Rotation, Translation);
	Mat4_Copy(out_view, Rotation);

	switch(c.Mode) {
		case CameraMode_Orthographic:
			Mat4_OrthoProj(out_proj, 0, c.ScreenWidth, 0, c.ScreenHeight,
			               c.ZNear, c.ZFar);
			break;
		case CameraMode_Perspective:
			Mat4_RectProj(out_proj, c.VerticalFoV, c.AspectRatio, c.ZNear,
			              c.ZFar);
			break;

		case CameraMode_NumModes: exit(EXIT_FAILURE);
	}
}
