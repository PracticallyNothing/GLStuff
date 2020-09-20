#ifndef MATH3D_H
#define MATH3D_H

#include "Common.h"

// Vectors
typedef struct Vec2_t Vec2;
typedef struct Vec3_t Vec3;
typedef struct Vec4_t Vec4;

struct Vec2_t {
	union {
		r32 d[2];
		struct { r32 x, y; };
		struct { r32 u, v; };
	};
};

struct Vec3_t {
	union {
		r32 d[3];
		struct { r32 x, y, z; };
		struct { r32 r, g, b; };
		struct { r32 u, v, w; };
		
		Vec2 xy;
		Vec2 rg;
	};
};
struct Vec4_t {
	union {
		r32 d[4];
		struct { r32 x, y, z, w; };
		struct { r32 r, g, b, a; };

		Vec2 xy;
		Vec3 xyz;

		Vec2 rg;
		Vec3 rgb;
	};
};

typedef Vec3 RGB;
typedef Vec4 RGBA;

typedef r32 MatValue_t;
typedef MatValue_t Mat2[4];
typedef MatValue_t Mat3[9];
typedef MatValue_t Mat4[16];

typedef Vec4 Quat;

// --- Vector operations --- //

extern Vec2 V2(r32 x, r32 y);
extern Vec3 V3(r32 x, r32 y, r32 z);
extern Vec4 V4(r32 x, r32 y, r32 z, r32 w);

#define V2C(X, Y)       { .x = (X), .y = (Y) }
#define V3C(X, Y, Z)    { .x = (X), .y = (Y), .z = (Z) }
#define V4C(X, Y, Z, W) { .x = (X), .y = (Y), .z = (Z), .w = (W) }

#define V3_V2(v, z)    V3((v).x, (v).y, (z))
#define V4_V3(v, w)    V4((v).x, (v).y, (v).z, (w))
#define V4_V2(v, z, w) V4((v).x, (v).y, (z), (w))

extern Vec2 Vec2_Neg(Vec2 v);
extern Vec3 Vec3_Neg(Vec3 v);
extern Vec4 Vec4_Neg(Vec4 v);

extern Vec2 Vec2_Add(Vec2 a, Vec2 b);
extern Vec3 Vec3_Add(Vec3 a, Vec3 b);
extern Vec4 Vec4_Add(Vec4 a, Vec4 b);

extern Vec2 Vec2_Sub(Vec2 a, Vec2 b);
extern Vec3 Vec3_Sub(Vec3 a, Vec3 b);
extern Vec4 Vec4_Sub(Vec4 a, Vec4 b);

extern Vec2 Vec2_Center(Vec2 a, Vec2 b);
extern Vec3 Vec3_Center(Vec3 a, Vec3 b);
extern Vec4 Vec4_Center(Vec4 a, Vec4 b);

extern Vec2 Vec2_TriCenter(Vec2 a, Vec2 b, Vec2 c);
extern Vec3 Vec3_TriCenter(Vec3 a, Vec3 b, Vec3 c);
extern Vec4 Vec4_TriCenter(Vec4 a, Vec4 b, Vec4 c);

extern Vec2 Vec2_QuadCenter(Vec2 a, Vec2 b, Vec2 c, Vec2 d);
extern Vec3 Vec3_QuadCenter(Vec3 a, Vec3 b, Vec3 c, Vec3 d);
extern Vec4 Vec4_QuadCenter(Vec4 a, Vec4 b, Vec4 c, Vec4 d);

extern Vec2 Vec2_MultScal(Vec2 v, r32 scalar);
extern Vec3 Vec3_MultScal(Vec3 v, r32 scalar);
extern Vec4 Vec4_MultScal(Vec4 v, r32 scalar);

extern Vec2 Vec2_DivScal(Vec2 v, r32 scalar);
extern Vec3 Vec3_DivScal(Vec3 v, r32 scalar);
extern Vec4 Vec4_DivScal(Vec4 v, r32 scalar);

extern Vec2 Vec2_MultVec(Vec2 a, Vec2 b);
extern Vec3 Vec3_MultVec(Vec3 a, Vec3 b);
extern Vec4 Vec4_MultVec(Vec4 a, Vec4 b);

extern Vec2 Vec2_DivVec(Vec2 a, Vec2 b);
extern Vec3 Vec3_DivVec(Vec3 a, Vec3 b);
extern Vec4 Vec4_DivVec(Vec4 a, Vec4 b);

extern r32 Vec2_Len2(Vec2 v);
extern r32 Vec3_Len2(Vec3 v);
extern r32 Vec4_Len2(Vec4 v);

extern r32 Vec2_Len(Vec2 v);
extern r32 Vec3_Len(Vec3 v);
extern r32 Vec4_Len(Vec4 v);

extern r32 Vec2_SumValues(Vec2 v);
extern r32 Vec3_SumValues(Vec3 v);
extern r32 Vec4_SumValues(Vec4 v);

extern Vec2 Vec2_Norm(Vec2 v);
extern Vec3 Vec3_Norm(Vec3 v);
extern Vec4 Vec4_Norm(Vec4 v);

extern r32 Vec2_Dot(Vec2 a, Vec2 b);
extern r32 Vec3_Dot(Vec3 a, Vec3 b);
extern r32 Vec4_Dot(Vec4 a, Vec4 b);

extern r32 Vec2_Cross(Vec2 a, Vec2 b);
extern Vec3 Vec3_Cross(Vec3 a, Vec3 b);

// --- Matrix operations --- //

extern void Mat2_Identity(Mat2 matrix);
extern void Mat3_Identity(Mat3 matrix);
extern void Mat4_Identity(Mat4 matrix);

extern void Mat2_Copy(Mat2 out, const Mat2 m);
extern void Mat3_Copy(Mat3 out, const Mat3 m);
extern void Mat4_Copy(Mat4 out, const Mat4 m);

extern i32 Mat2_Inverse(Mat2 matrix);
extern i32 Mat3_Inverse(Mat3 matrix);
extern i32 Mat4_Inverse(Mat4 matrix);

extern void Mat2_Transpose(Mat2 matrix);
extern void Mat3_Transpose(Mat3 matrix);
extern void Mat4_Transpose(Mat4 matrix);

extern r32 Mat2_Determinant(const Mat2 matrix);
extern r32 Mat3_Determinant(const Mat3 matrix);
extern r32 Mat4_Determinant(const Mat4 matrix);

extern void Mat2_Neg(Mat2 matrix);
extern void Mat3_Neg(Mat3 matrix);
extern void Mat4_Neg(Mat4 matrix);

extern void Mat2_Add(Mat2 a, const Mat2 b);
extern void Mat3_Add(Mat3 a, const Mat3 b);
extern void Mat4_Add(Mat4 a, const Mat4 b);

extern void Mat2_Sub(Mat2 a, const Mat2 b);
extern void Mat3_Sub(Mat3 a, const Mat3 b);
extern void Mat4_Sub(Mat4 a, const Mat4 b);

extern void Mat2_MultScal(Mat2 matrix, r32 scalar);
extern void Mat3_MultScal(Mat3 matrix, r32 scalar);
extern void Mat4_MultScal(Mat4 matrix, r32 scalar);

extern void Mat2_MultMat(Mat2 a, const Mat2 b);
extern void Mat3_MultMat(Mat3 a, const Mat3 b);
extern void Mat4_MultMat(Mat4 a, const Mat4 b);

extern void Mat2_DivScal(Mat2 matrix, r32 scalar);
extern void Mat3_DivScal(Mat3 matrix, r32 scalar);
extern void Mat4_DivScal(Mat4 matrix, r32 scalar);

extern Vec2 Mat2_MultVec2(const Mat2 m, Vec2 v);
extern Vec3 Mat3_MultVec3(const Mat3 m, Vec3 v);
extern Vec4 Mat4_MultVec4(const Mat4 m, Vec4 v);

extern i32 Mat2_DivMat(Mat2 a, const Mat2 b);
extern i32 Mat3_DivMat(Mat3 a, const Mat3 b);
extern i32 Mat4_DivMat(Mat4 a, const Mat4 b);

extern void Mat2_RotateAngle(Mat2 out, r32 angle);
extern void Mat3_RotateAngle(Mat3 out, r32 angle);

extern void Mat4_Translate(Mat4 out, Vec3 amt);
extern void Mat4_RotateVec(Mat4 out, Vec3 amt);
extern void Mat4_RotateQuat(Mat4 out, Quat quat);
extern void Mat4_Scale(Mat4 out, Vec3 amt);

extern void Mat4_OrthoProj(Mat4 out, r32 left, r32 right, r32 top, r32 bottom,
                           r32 zNear, r32 zFar);

extern void Mat4_RectProj(Mat4 out, r32 fov, r32 aspect, r32 zNear, r32 zFar);

extern void Mat2_FromMat3(Mat2 out, const Mat3 m);
extern void Mat2_FromMat4(Mat2 out, const Mat4 m);
extern void Mat3_FromMat4(Mat3 out, const Mat4 m);

// --- Quaternion operations --- //

extern const Quat Quat_Identity;

extern Quat Quat_RotAxis(Vec3 axis, r32 amt);
extern Quat Quat_Mix(Quat a, Quat b, r32 amt);
extern Quat Quat_Lerp(Quat a, Quat b, r32 amt);
extern Quat Quat_Slerp(Quat a, Quat b, r32 amt);
extern Quat Quat_Conjugate(Quat q);
extern r32 Quat_Dot(Quat a, Quat b);
extern Quat Quat_Mult(Quat a, Quat b);
extern Quat Quat_Norm(Quat q);

// --- Extra color utilities --- //
extern RGB  HexToRGB(const char str[6]);
extern RGBA HexToRGBA(const char str[8]);

/** Converts an HSV vector to a clamped (0.0 to 1.0) RGB vector. */
extern RGB HSVToRGB(Vec3 HSV);

/** Converts a clamped (0.0 to 1.0) RGB vector to an HSV vector. */
extern Vec3 RGBToHSV(RGB RGB);

// --- Value Interpolation --- //

extern r32 Lerp_Linear(r32 start, r32 end, r32 amt);
extern r32 Lerp_Cubic(r32 start, r32 end, r32 amt);
extern r32 Lerp_Bezier(r32 start, r32 end, r32 amt, Vec2 P1, Vec2 P2);
extern r32 Lerp_EaseInOut(r32 start, r32 end, r32 amt);
extern r32 Lerp_Spring(r32 start, r32 end, r32 amt);

// --- ------------------ --- //

// --- Triangles --- //

enum Triangle_TargetAxis {
	Triangle_TargetAxis_XY,
	Triangle_TargetAxis_XZ,
	Triangle_TargetAxis_YZ
};

extern r32  Triangle_Area(Vec3 a, Vec3 b, Vec3 c);
extern r32  Triangle_AreaAxisAligned(Vec3 a, Vec3 b, Vec3 c, enum Triangle_TargetAxis axis);
extern Vec3 Triangle_GetNormal(Vec3 a, Vec3 b, Vec3 c);

extern Vec3 Triangle_CircumsphereCenter(Vec3 a, Vec3 b, Vec3 c);
extern r32  Triangle_CircumsphereRadius(Vec3 a, Vec3 b, Vec3 c);

// --- ------------------ --- //

// --- Transforms --- //
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

extern const Transform2D Transform2D_Default;
extern const Transform3D Transform3D_Default;

extern void Transform2D_Mat3(Transform2D t, Mat3 out);
extern void Transform3D_Mat4(Transform3D t, Mat4 out);
// --- ------------------ --- //

#endif
