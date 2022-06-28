#include "Math3D.h"

#include <assert.h>
#include <float.h>
#include <math.h>
#include <string.h>

// --- Vector operations --- //

Vec2 V2(r32 x, r32 y) { return (Vec2) V2C(x, y); }
Vec3 V3(r32 x, r32 y, r32 z) { return (Vec3) V3C(x, y, z); }
Vec4 V4(r32 x, r32 y, r32 z, r32 w) { return (Vec4) V4C(x, y, z, w); }

Vec2 Vec2_Neg(Vec2 v) { return V2(-v.x, -v.y); }
Vec3 Vec3_Neg(Vec3 v) { return V3(-v.x, -v.y, -v.z); }
Vec4 Vec4_Neg(Vec4 v) { return V4(-v.x, -v.y, -v.z, -v.w); }

Vec2 Vec2_Add(Vec2 a, Vec2 b) { return V2(a.x + b.x, a.y + b.y); }
Vec3 Vec3_Add(Vec3 a, Vec3 b) { return V3(a.x + b.x, a.y + b.y, a.z + b.z); }
Vec4 Vec4_Add(Vec4 a, Vec4 b) { return V4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w); }

Vec2 Vec2_Sub(Vec2 a, Vec2 b) { return V2(a.x - b.x, a.y - b.y); }
Vec3 Vec3_Sub(Vec3 a, Vec3 b) { return V3(a.x - b.x, a.y - b.y, a.z - b.z); }
Vec4 Vec4_Sub(Vec4 a, Vec4 b) { return V4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w); }

Vec2 Vec2_Center(Vec2 a, Vec2 b) { return Vec2_DivScal(Vec2_Add(a, b), 2); }
Vec3 Vec3_Center(Vec3 a, Vec3 b) { return Vec3_DivScal(Vec3_Add(a, b), 2); }
Vec4 Vec4_Center(Vec4 a, Vec4 b) { return Vec4_DivScal(Vec4_Add(a, b), 2); }

Vec2 Vec2_TriCenter(Vec2 a, Vec2 b, Vec2 c) {
	return Vec2_DivScal(Vec2_Add(a, Vec2_Add(b, c)), 3);
}
Vec3 Vec3_TriCenter(Vec3 a, Vec3 b, Vec3 c) {
	return Vec3_DivScal(Vec3_Add(a, Vec3_Add(b, c)), 3);
}
Vec4 Vec4_TriCenter(Vec4 a, Vec4 b, Vec4 c) {
	return Vec4_DivScal(Vec4_Add(a, Vec4_Add(b, c)), 3);
}

Vec2 Vec2_QuadCenter(Vec2 a, Vec2 b, Vec2 c, Vec2 d) {
	return Vec2_DivScal(Vec2_Add(Vec2_Add(a, b), Vec2_Add(c, d)), 4);
}
Vec3 Vec3_QuadCenter(Vec3 a, Vec3 b, Vec3 c, Vec3 d) {
	return Vec3_DivScal(Vec3_Add(Vec3_Add(a, b), Vec3_Add(c, d)), 4);
}
Vec4 Vec4_QuadCenter(Vec4 a, Vec4 b, Vec4 c, Vec4 d) {
	return Vec4_DivScal(Vec4_Add(Vec4_Add(a, b), Vec4_Add(c, d)), 4);
}

Vec2 Vec2_MultScal(Vec2 v, r32 s) { return V2(v.x * s, v.y * s); }
Vec3 Vec3_MultScal(Vec3 v, r32 s) { return V3(v.x * s, v.y * s, v.z * s); }
Vec4 Vec4_MultScal(Vec4 v, r32 s) { return V4(v.x * s, v.y * s, v.z * s, v.w * s); }

Vec2 Vec2_DivScal(Vec2 v, r32 s) { return V2(v.x / s, v.y / s); }
Vec3 Vec3_DivScal(Vec3 v, r32 s) { return V3(v.x / s, v.y / s, v.z / s); }
Vec4 Vec4_DivScal(Vec4 v, r32 s) { return V4(v.x / s, v.y / s, v.z / s, v.w / s); }

Vec2 Vec2_MultVec(Vec2 a, Vec2 b) { return V2(a.x * b.x, a.y * b.y); }
Vec3 Vec3_MultVec(Vec3 a, Vec3 b) { return V3(a.x * b.x, a.y * b.y, a.z * b.z); }
Vec4 Vec4_MultVec(Vec4 a, Vec4 b) { return V4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w); }

Vec2 Vec2_DivVec(Vec2 a, Vec2 b) { return V2(a.x / b.x, a.y / b.y); }
Vec3 Vec3_DivVec(Vec3 a, Vec3 b) { return V3(a.x / b.x, a.y / b.y, a.z / b.z); }
Vec4 Vec4_DivVec(Vec4 a, Vec4 b) { return V4(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w); }

r32 Vec2_Len2(Vec2 v) { return v.x * v.x + v.y * v.y; }
r32 Vec3_Len2(Vec3 v) { return v.x * v.x + v.y * v.y + v.z * v.z; }
r32 Vec4_Len2(Vec4 v) { return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w; }

r32 Vec2_Len(Vec2 v) { return sqrtf(Vec2_Len2(v)); }
r32 Vec3_Len(Vec3 v) { return sqrtf(Vec3_Len2(v)); }
r32 Vec4_Len(Vec4 v) { return sqrtf(Vec4_Len2(v)); }

Vec2 Vec2_Norm(Vec2 v) {
	r32 l = Vec2_Len(v);
	return Vec2_DivScal(v, l);
}
Vec3 Vec3_Norm(Vec3 v) {
	r32 l = Vec3_Len(v);
	return Vec3_DivScal(v, l);
}
Vec4 Vec4_Norm(Vec4 v) {
	r32 l = Vec4_Len(v);
	return Vec4_DivScal(v, l);
}

r32 Vec2_SumValues(Vec2 v) { return v.x + v.y; }
r32 Vec3_SumValues(Vec3 v) { return v.x + v.y + v.z; }
r32 Vec4_SumValues(Vec4 v) { return v.x + v.y + v.z + v.w; }

r32 Vec2_Dot(Vec2 a, Vec2 b) { return Vec2_SumValues(Vec2_MultVec(a, b)); }
r32 Vec3_Dot(Vec3 a, Vec3 b) { return Vec3_SumValues(Vec3_MultVec(a, b)); }
r32 Vec4_Dot(Vec4 a, Vec4 b) { return Vec4_SumValues(Vec4_MultVec(a, b)); }

r32 Vec2_Cross(Vec2 a, Vec2 b) { return a.x * b.y - a.y * b.x; }

Vec3 Vec3_Cross(Vec3 a, Vec3 b) {
	return V3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

// --- Matrix operations --- //

void Mat2_Identity(Mat2 m) {
	memset(m, 0, sizeof(Mat2));
	m[0] = m[2] = 1;
}
void Mat3_Identity(Mat3 m) {
	memset(m, 0, sizeof(Mat3));
	m[0] = m[4] = m[8] = 1;
}
void Mat4_Identity(Mat4 m) {
	memset(m, 0, sizeof(Mat4));
	m[0] = m[5] = m[10] = m[15] = 1;
}

void Mat2_FromMat3(Mat2 out, const Mat3 m) {
	for(i32 y = 0; y < 2; y++)
		for(i32 x = 0; x < 2; x++) out[x + y * 2] = m[x + y * 3];
}
void Mat2_FromMat4(Mat2 out, const Mat4 m) {
	for(i32 y = 0; y < 2; y++)
		for(i32 x = 0; x < 2; x++) out[x + y * 2] = m[x + y * 4];
}
void Mat3_FromMat4(Mat3 out, const Mat4 m) {
	for(i32 y = 0; y < 3; y++)
		for(i32 x = 0; x < 3; x++) out[x + y * 3] = m[x + y * 4];
}

void Mat2_Copy(Mat2 out, const Mat2 m) {
	for(i32 i = 0; i < 4; ++i) out[i] = m[i];
}
void Mat3_Copy(Mat3 out, const Mat3 m) {
	for(i32 i = 0; i < 9; ++i) out[i] = m[i];
}
void Mat4_Copy(Mat4 out, const Mat4 m) {
	for(i32 i = 0; i < 16; ++i) out[i] = m[i];
}

void Mat2_Neg(Mat2 m) {
	for(i32 i = 0; i < 4; ++i) m[i] = -m[i];
}
void Mat3_Neg(Mat3 m) {
	for(i32 i = 0; i < 9; ++i) m[i] = -m[i];
}
void Mat4_Neg(Mat4 m) {
	for(i32 i = 0; i < 16; ++i) m[i] = -m[i];
}

void Mat2_Add(Mat2 a, const Mat2 b) {
	for(i32 i = 0; i < 4; ++i) a[i] += b[i];
}
void Mat3_Add(Mat3 a, const Mat3 b) {
	for(i32 i = 0; i < 9; ++i) a[i] += b[i];
}
void Mat4_Add(Mat4 a, const Mat4 b) {
	for(i32 i = 0; i < 16; ++i) a[i] += b[i];
}

void Mat2_Sub(Mat2 a, const Mat2 b) {
	for(i32 i = 0; i < 4; ++i) a[i] -= b[i];
}
void Mat3_Sub(Mat3 a, const Mat3 b) {
	for(i32 i = 0; i < 9; ++i) a[i] -= b[i];
}
void Mat4_Sub(Mat4 a, const Mat4 b) {
	for(i32 i = 0; i < 16; ++i) a[i] -= b[i];
}

void Mat2_MultScal(Mat2 a, r32 s) {
	for(i32 i = 0; i < 4; ++i) a[i] *= s;
}
void Mat3_MultScal(Mat3 a, r32 s) {
	for(i32 i = 0; i < 9; ++i) a[i] *= s;
}
void Mat4_MultScal(Mat4 a, r32 s) {
	for(i32 i = 0; i < 16; ++i) a[i] *= s;
}

void Mat2_DivScal(Mat2 a, r32 s) {
	for(i32 i = 0; i < 4; ++i) a[i] /= s;
}
void Mat3_DivScal(Mat3 a, r32 s) {
	for(i32 i = 0; i < 9; ++i) a[i] /= s;
}
void Mat4_DivScal(Mat4 a, r32 s) {
	for(i32 i = 0; i < 16; ++i) a[i] /= s;
}

extern Vec2 Mat2_MultVec2(const Mat2 m, Vec2 v) {
	return V2(v.x * m[0] + v.y * m[1], v.x * m[2] + v.y * m[3]);
}
extern Vec3 Mat3_MultVec3(const Mat3 m, Vec3 v) {
	return V3(v.x * m[0] + v.y * m[1] + v.z * m[2],
	          v.x * m[3] + v.y * m[4] + v.z * m[5],
	          v.x * m[6] + v.y * m[7] + v.z * m[8]);
}
extern Vec4 Mat4_MultVec4(const Mat4 m, Vec4 v) {
	return V4(v.x * m[0] + v.y * m[1] + v.z * m[2] + v.w * m[3],
	          v.x * m[4] + v.y * m[5] + v.z * m[6] + v.w * m[7],
	          v.x * m[8] + v.y * m[9] + v.z * m[10] + v.w * m[11],
	          v.x * m[12] + v.y * m[13] + v.z * m[14] + v.w * m[15]);
}

r32 Mat2_Determinant(const Mat2 m) {
	r32 A = m[0], B = m[1], C = m[2], D = m[3];
	return A * D - B * C;
}

r32 Mat3_Determinant(const Mat3 m) {
	r32 A = m[0], B = m[1], C = m[2], D = m[3], E = m[4], F = m[5], G = m[6], H = m[7],
	    I = m[8];

	return A * (E * I - F * H) - B * (D * I - F * G) + C * (D * H - E * G);
}

r32 Mat4_Determinant(const Mat4 m) {
	// Thank you, Excel and Libreoffice Calc.

	r32 A = m[0], B = m[1], C = m[2], D = m[3], E = m[4], F = m[5], G = m[6], H = m[7],
	    I = m[8], J = m[9], K = m[10], L = m[11], M = m[12], N = m[13], O = m[14], P = m[15];

	r32 Det[4] = {
	    F * (K * P - O * L) - G * (J * P - N * L) + H * (J * O - N * K),
	    E * (K * P - O * L) - G * (I * P - M * L) + H * (I * O - M * K),
	    E * (J * P - N * L) - F * (I * P - M * L) + H * (I * N - M * J),
	    E * (J * O - N * K) - F * (I * O - M * K) + G * (I * N - M * J),
	};

	return A * Det[0] - B * Det[1] + C * Det[2] - D * Det[3];
}

bool8 Mat2_Inverse(Mat2 out, const Mat2 m) {
	if(!out || !m) return 0;

	r32 Det = Mat2_Determinant(m);

	if(Det == 0.0f) return 0;

	Mat2 minor;

	for(u32 i = 0; i < 2; i++)
		for(u32 j = 0; j < 2; j++) minor[j * 2 + i] = Mat2_Minor(m, i, j);

	Mat2_MultScal(minor, 1.0 / Det);
	Mat2_Copy(out, minor);

	return 1;
}

bool8 Mat3_Inverse(Mat3 out, const Mat3 m) {
	r32 Det = Mat3_Determinant(m);

	if(Det == 0.0f) return 0;

	Mat3 minor;

	for(u32 i = 0; i < 3; i++)
		for(u32 j = 0; j < 3; j++) minor[j * 3 + i] = Mat3_Minor(m, i, j);

	Mat3_MultScal(minor, 1.0 / Det);
	Mat3_Copy(out, minor);
	return 1;
}

bool8 Mat4_Inverse(Mat4 out, const Mat4 m) {
	r32 Det = Mat4_Determinant(m);

	if(Det == 0.0f) return 0;

	Mat3 minor;

	for(u32 i = 0; i < 4; i++)
		for(u32 j = 0; j < 4; j++) minor[j * 4 + i] = Mat4_Minor(m, i, j);

	Mat4_MultScal(minor, 1.0 / Det);
	Mat4_Copy(out, minor);
	return 1;
}

static void _Swap(r32* a, r32* b) {
	if(a == b) return;
	r32 tmp = *a;
	*a      = *b;
	*b      = tmp;
}

void Mat2_Transpose(Mat2 m) { _Swap(&m[1], &m[2]); }
void Mat3_Transpose(Mat3 m) {
	for(i32 y = 0; y < 2; y++)
		for(i32 x = 0; x < y; x++) _Swap(&m[x + y * 3], &m[y + x * 3]);
}
void Mat4_Transpose(Mat4 m) {
	for(i32 y = 0; y < 3; y++)
		for(i32 x = 0; x < y; x++) _Swap(&m[x + y * 4], &m[y + x * 4]);
}

void Mat4_OrthoProj(Mat4 out, r32 left, r32 right, r32 top, r32 bottom, r32 zNear, r32 zFar) {
	Mat4_Identity(out);
	r32 x  = right - left;
	r32 y  = top - bottom;
	r32 z  = zFar - zNear;
	out[0] = 2.0 / x;
	out[3] = -((right + left) / x);

	out[5] = 2.0 / y;
	out[7] = -((top + bottom) / y);

	out[10] = 2.0 / z;
	out[11] = -((zFar + zNear) / z);
}

void Mat2_MultMat(Mat2 a, const Mat2 b) {
	Mat2 out;
	out[0] = a[0] * b[0] + a[1] * b[2];
	out[1] = a[0] * b[1] + a[1] * b[3];
	out[2] = a[2] * b[0] + a[3] * b[2];
	out[3] = a[2] * b[1] + a[3] * b[3];
	Mat2_Copy(a, out);
}

void Mat3_MultMat(Mat3 a, const Mat3 b) {
	Mat3 out;
	out[0] = a[0] * b[0] + a[1] * b[3] + a[2] * b[6];
	out[1] = a[0] * b[1] + a[1] * b[4] + a[2] * b[7];
	out[2] = a[0] * b[2] + a[1] * b[5] + a[2] * b[8];

	out[3] = a[3] * b[0] + a[4] * b[3] + a[5] * b[6];
	out[4] = a[3] * b[1] + a[4] * b[4] + a[5] * b[7];
	out[5] = a[3] * b[2] + a[4] * b[5] + a[5] * b[8];

	out[6] = a[6] * b[0] + a[7] * b[3] + a[8] * b[6];
	out[7] = a[6] * b[1] + a[7] * b[4] + a[8] * b[7];
	out[8] = a[6] * b[2] + a[7] * b[5] + a[8] * b[8];
	Mat3_Copy(a, out);
}

void Mat4_MultMat(Mat4 a, const Mat4 b) {
	Mat4 out;
	out[0] = a[0] * b[0] + a[1] * b[4] + a[2] * b[8] + a[3] * b[12];
	out[1] = a[0] * b[1] + a[1] * b[5] + a[2] * b[9] + a[3] * b[13];
	out[2] = a[0] * b[2] + a[1] * b[6] + a[2] * b[10] + a[3] * b[14];
	out[3] = a[0] * b[3] + a[1] * b[7] + a[2] * b[11] + a[3] * b[15];

	out[4] = a[4] * b[0] + a[5] * b[4] + a[6] * b[8] + a[7] * b[12];
	out[5] = a[4] * b[1] + a[5] * b[5] + a[6] * b[9] + a[7] * b[13];
	out[6] = a[4] * b[2] + a[5] * b[6] + a[6] * b[10] + a[7] * b[14];
	out[7] = a[4] * b[3] + a[5] * b[7] + a[6] * b[11] + a[7] * b[15];

	out[8]  = a[8] * b[0] + a[9] * b[4] + a[10] * b[8] + a[11] * b[12];
	out[9]  = a[8] * b[1] + a[9] * b[5] + a[10] * b[9] + a[11] * b[13];
	out[10] = a[8] * b[2] + a[9] * b[6] + a[10] * b[10] + a[11] * b[14];
	out[11] = a[8] * b[3] + a[9] * b[7] + a[10] * b[11] + a[11] * b[15];

	out[12] = a[12] * b[0] + a[13] * b[4] + a[14] * b[8] + a[15] * b[12];
	out[13] = a[12] * b[1] + a[13] * b[5] + a[14] * b[9] + a[15] * b[13];
	out[14] = a[12] * b[2] + a[13] * b[6] + a[14] * b[10] + a[15] * b[14];
	out[15] = a[12] * b[3] + a[13] * b[7] + a[14] * b[11] + a[15] * b[15];
	Mat4_Copy(a, out);
}

r32 Mat2_Minor(const Mat2 a, u32 i, u32 j) { return ((i + j) % 2 ? -1 : 1) * a[2 * i + j]; }
r32 Mat3_Minor(const Mat3 a, u32 i, u32 j) {
	if(i > 2 || j > 2) {
		Log(WARN, "3x3 Matrix minor with incorrect coordinates (i: %d, j: %d)", i, j);
		return 0;
	}

	Mat2 b;

	for(u32 iA = 0, iB = 0; iA < 3 && iB < 2; iA++) {
		if(iA == i) continue;

		for(u32 jA = 0, jB = 0; jA < 3 && jB < 2; jA++) {
			if(jA == j) continue;

			b[iB * 2 + jB] = a[iA * 3 + jB];
			jB++;
		}

		iB++;
	}

	return ((i + j) % 2 ? -1 : 1) * Mat2_Determinant(b);
}

r32 Mat4_Minor(const Mat4 a, u32 i, u32 j) {
	if(i > 3 || j > 3) {
		Log(WARN, "4x4 Matrix minor with incorrect coordinates (i: %d, j: %d)", i, j);
		return 0;
	}

	Mat3 b;

	for(u32 iA = 0, iB = 0; iA < 4 && iB < 3; iA++) {
		if(iA == i) continue;

		for(u32 jA = 0, jB = 0; jA < 4 && jB < 3; jA++) {
			if(jA == j) continue;

			b[iB * 3 + jB] = a[iA * 4 + jB];
			jB++;
		}

		iB++;
	}

	return ((i + j) % 2 ? -1 : 1) * Mat3_Determinant(b);
}

bool8 Mat2_DivMat(Mat2 a, const Mat2 b) {
	Mat2 inverse;
	if(!Mat2_Inverse(inverse, b)) return 0;

	Mat2_MultMat(a, inverse);
	return 1;
}

bool8 Mat3_DivMat(Mat3 a, const Mat3 b) {
	Mat3 inverse;
	if(!Mat3_Inverse(inverse, b)) return 0;

	Mat3_MultMat(a, inverse);
	return 1;
}

bool8 Mat4_DivMat(Mat4 a, const Mat4 b) {
	Mat4 inverse;
	if(!Mat4_Inverse(inverse, b)) return 0;

	Mat4_MultMat(a, inverse);
	return 1;
}

// Thank you,
// http://ogldev.atspace.co.uk/www/tutorial12/tutorial12.html
// and
// OpenGL Maths (glm) library source code
void Mat4_RectProj(Mat4 out, r32 verticalFoV, r32 aspect, r32 zNear, r32 zFar) {
	Mat4_Identity(out);

	const r32 tanHalfFov = tan(verticalFoV / 2.0);

	out[0]  = 1.0 / (aspect * tanHalfFov);
	out[5]  = 1.0 / tanHalfFov;
	out[10] = (-zNear - zFar) / (zNear - zFar);
	out[11] = (2.0 * zFar * zNear) / (zNear - zFar);
	out[14] = 1.0;
	out[15] = 0;
}

void Mat4_Translate(Mat4 out, Vec3 amt) {
	out[3]  = amt.x;
	out[7]  = amt.y;
	out[11] = amt.z;
}

void Mat4_Scale(Mat4 out, Vec3 amt) {
	out[0]  = amt.x;
	out[5]  = amt.y;
	out[10] = amt.z;
}

// Thank you,
// http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToMatrix/index.htm
void Mat4_RotateQuat(Mat4 out, Quat q) {
	Mat4_Identity(out);
	out[0] = 1 - 2 * q.y * q.y - 2 * q.z * q.z;
	out[1] = 2 * q.x * q.y - 2 * q.z * q.w;
	out[2] = 2 * q.x * q.z + 2 * q.y * q.w;

	out[4] = 2 * q.x * q.y + 2 * q.z * q.w;
	out[5] = 1 - 2 * q.x * q.x - 2 * q.z * q.z;
	out[6] = 2 * q.y * q.z - 2 * q.x * q.w;

	out[8]  = 2 * q.x * q.z - 2 * q.y * q.w;
	out[9]  = 2 * q.y * q.z + 2 * q.x * q.w;
	out[10] = 1 - 2 * q.x * q.x - 2 * q.y * q.y;
}

// --- Quaternion operations --- //

const Quat Quat_Identity = {.x = 0, .y = 0, .z = 0, .w = 1};

Quat Quat_RotAxis(Vec3 axis, r32 angle) {
	axis = Vec3_Norm(axis);
	return (Quat){.x = axis.x * sin(angle / 2),
	              .y = axis.y * sin(angle / 2),
	              .z = axis.z * sin(angle / 2),
	              .w = cos(angle / 2)};
}

static r32 _Mix(r32 a, r32 b, r32 t) { return t * a + (1 - t) * b; }

Quat Quat_Mix(Quat a, Quat b, r32 amt) {
	r32 cosTheta = Quat_Dot(a, b);

	if(cosTheta > 1 - FLT_EPSILON) {
		return (Quat){.x = _Mix(a.x, b.x, amt),
		              .y = _Mix(a.y, b.y, amt),
		              .z = _Mix(a.z, b.z, amt),
		              .w = _Mix(a.w, b.w, amt)};
	} else {
		r32 angle = acos(cosTheta);

		Vec4 _a  = Vec4_MultScal(a, sin(angle * amt));
		Vec4 _b  = Vec4_MultScal(b, sin(angle * (1 - amt)));
		Vec4 res = Vec4_Add(_a, _b);
		res      = Vec4_DivScal(res, sin(angle));
		return res;
	}
}
Quat Quat_Lerp(Quat a, Quat b, r32 amt) {
	assert(amt >= 0);
	assert(amt <= 1);

	a = Vec4_MultScal(a, amt);
	b = Vec4_MultScal(b, 1 - amt);
	return Vec4_Add(a, b);
}

Quat Quat_Slerp(Quat a, Quat b, r32 amt) {
	r32 cosTheta = Quat_Dot(a, b);
	if(cosTheta < 0) {
		a        = Vec4_Neg(a);
		cosTheta = -cosTheta;
	}
	return Quat_Mix(a, b, amt);
}
Quat Quat_Conjugate(Quat q) {
	q   = Vec4_Neg(q);
	q.w = -q.w;
	return q;
}
Quat Quat_Inverse(Quat q) {
	Quat conj = Quat_Conjugate(q);
	return Vec4_DivScal(conj, Quat_Dot(q, q));
}
r32 Quat_Dot(Quat a, Quat b) { return Vec4_Dot(a, b); }
Quat Quat_Norm(Quat q) {
	r32 len = Vec4_Len(q);
	if(len <= 0) return (Quat){.x = 0, .y = 0, .z = 0, .w = 1};
	return Vec4_Norm(q);
}

Quat Quat_Mult(Quat a, Quat b) {
	r32 x, y, z, w;

	x = b.w * a.x + b.x * a.w + b.y * a.z - b.z * a.y;
	y = b.w * a.y + b.y * a.w + b.z * a.x - b.x * a.z;
	z = b.w * a.z + b.z * a.w + b.x * a.y - b.y * a.x;
	w = b.w * a.w - b.x * a.x - b.y * a.y - b.z * a.z;

	return (Quat){.x = x, .y = y, .z = z, .w = w};
}

static i32 HexToInt(char c) {
	if(c >= '0' && c <= '9')
		return c - '0';
	else if(c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	else if(c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	else
		return -1;
}

RGB HexToRGB(const char str[6]) {
	Vec3 res;
	for(u32 i = 0; i < 3; i++)
		res.d[i] = (HexToInt(str[i * 2]) * 16 + HexToInt(str[i * 2 + 1])) / 255.0;
	return res;
}

RGBA HexToRGBA(const char str[8]) {
	Vec4 res;
	for(u32 i = 0; i < 4; i++)
		res.d[i] = (HexToInt(str[i * 2]) * 16 + HexToInt(str[i * 2 + 1])) / 255.0;
	return res;
}

// Thank you,
// https://www.rapidtables.com/convert/color/hsv-to-rgb.html
RGB HSVToRGB(Vec3 hsv) {
	r32 c = hsv.y * hsv.z;
	r32 x = c * (1 - fabs(fmod(hsv.x / (Pi / 3), 2) - 1));
	r32 m = hsv.z - c;

	r32 H = hsv.x / (Pi / 3);

	Vec3 rgb;

	if(H < 1.0)
		rgb = V3(c, x, 0);
	else if(H < 2.0)
		rgb = V3(x, c, 0);
	else if(H < 3.0)
		rgb = V3(0, c, x);
	else if(H < 4.0)
		rgb = V3(0, x, c);
	else if(H < 5.0)
		rgb = V3(x, 0, c);
	else
		rgb = V3(c, 0, x);

	return Vec3_Add(rgb, V3(m, m, m));
}

// Thank you,
// https://www.rapidtables.com/convert/color/rgb-to-hsv.html
Vec3 RGBToHSV(RGB rgb) {
	r32 cMax =
	    (rgb.r > rgb.g ? (rgb.r > rgb.b ? rgb.r : rgb.b) : (rgb.g > rgb.b ? rgb.g : rgb.b));

	r32 cMin =
	    (rgb.r < rgb.g ? (rgb.r < rgb.b ? rgb.r : rgb.b) : (rgb.g < rgb.b ? rgb.g : rgb.b));

	r32 delta = cMax - cMin;

	Vec3 hsv;
	// ---- Calculate H ----
	if(cMax == cMin)
		hsv.x = 0;
	else if(cMax == rgb.r)
		hsv.x = (Pi / 3) * fmod((rgb.g - rgb.b) / delta, 6);
	else if(cMax == rgb.g)
		hsv.x = (Pi / 3) * ((rgb.b - rgb.r) / delta + 2);
	else
		hsv.x = (Pi / 3) * ((rgb.r - rgb.g) / delta + 4);

	// ---- Calculate S ----
	if(cMax == 0)
		hsv.y = 0;
	else
		hsv.y = delta / cMax;

	// ---- Assign V -----
	hsv.z = cMax;

	return hsv;
}

r32 Lerp_Linear(r32 start, r32 end, r32 amt) {
	return start + (end - start) * Clamp_R32(amt, 0, 1);
}

r32 Lerp_Cubic(r32 start, r32 end, r32 amt) {
	return start + (end - start) * Clamp_R32(amt * amt, 0, 1);
}

// https://wikimedia.org/api/rest_v1/media/math/render/svg/504c44ca5c5f1da2b6cb1702ad9d1afa27cc1ee0
r32 Lerp_Bezier(r32 start, r32 end, r32 amt, Vec2 P1, Vec2 P2) {
	Vec2 P0 = V2(0, start), P3 = V2(1, end);
	P1.y = start + (end - start) * P1.y;
	P2.y = start + (end - start) * P2.y;

	r32 t = Clamp_R32(amt, 0, 1);

	P0 = Vec2_MultScal(P0, powf(1 - t, 3));
	P1 = Vec2_MultScal(P1, 3.0f * powf(1 - t, 2) * t);
	P2 = Vec2_MultScal(P2, 3.0f * (1 - t) * t * t);
	P3 = Vec2_MultScal(P3, t * t * t);

	return Vec2_Add(Vec2_Add(P0, P1), Vec2_Add(P2, P3)).y;
}

// https://www.w3schools.com/cssref/css3_pr_transition-timing-function.asp
r32 Lerp_EaseInOut(r32 start, r32 end, r32 amt) {
	return Lerp_Bezier(start, end, amt, V2(0.25, 0.1), V2(0.25, 1));
}

r32 Lerp_Spring(r32 start, r32 end, r32 amt) {
	return Lerp_Bezier(start, end, amt / 2, V2(0.25, -0.5), V2(0.60, 2.5));
}

r32 Triangle_Area(Vec3 A, Vec3 B, Vec3 C) {
	r32 a = Vec3_Len(Vec3_Sub(B, C)), b = Vec3_Len(Vec3_Sub(A, C)),
	    c = Vec3_Len(Vec3_Sub(A, B));

	r32 p = (a + b + c) / 2;

	return sqrtf(p * (p - a) * (p - b) * (p - c));
}

r32 Triangle_AreaAxisAligned(Vec3 A, Vec3 B, Vec3 C, enum Triangle_TargetAxis axis) {
	Vec2 a, b, c;
	switch(axis) {
		case Triangle_TargetAxis_XY:
			a = A.xy;
			b = B.xy;
			c = C.xy;
			break;
		case Triangle_TargetAxis_XZ:
			a = V2(A.x, A.z);
			b = V2(B.x, B.z);
			c = V2(C.x, C.z);
			break;
		case Triangle_TargetAxis_YZ:
			a = V2(A.y, A.z);
			b = V2(B.y, B.z);
			c = V2(C.y, C.z);
			break;
	}

	// Thank you,
	// https://www.gamedev.net/forums/topic.asp?topic_id=295943
	Mat3 m = {a.x, a.y, 1, b.x, b.y, 1, c.x, c.y, 1};

	return Mat3_Determinant(m) / 2.0f;
}

Vec3 Triangle_GetNormal(Vec3 a, Vec3 b, Vec3 c) {
	Vec3 ba = Vec3_Sub(a, b);
	Vec3 ca = Vec3_Sub(a, c);
	return Vec3_Norm(Vec3_Cross(ba, ca));
}

// Thank you,
// https://en.wikipedia.org/wiki/Circumscribed_circle#Cartesian_coordinates_from_cross-_and_dot-products
Vec3 Triangle_CircumsphereCenter(Vec3 a, Vec3 b, Vec3 c) {
	r32 div = 2 * Vec3_Len2(Vec3_Cross(Vec3_Sub(a, b), Vec3_Sub(b, c)));

	r32 alpha = (Vec3_Len2(Vec3_Sub(b, c)) * Vec3_Dot(Vec3_Sub(a, b), Vec3_Sub(a, c))) / div;
	r32 beta  = (Vec3_Len2(Vec3_Sub(a, c)) * Vec3_Dot(Vec3_Sub(b, a), Vec3_Sub(b, c))) / div;
	r32 gamma = (Vec3_Len2(Vec3_Sub(a, b)) * Vec3_Dot(Vec3_Sub(c, a), Vec3_Sub(c, b))) / div;

	Vec3_MultScal(a, alpha);
	Vec3_MultScal(b, beta);
	Vec3_MultScal(c, gamma);

	return Vec3_Add(a, Vec3_Add(b, c));
}

r32 Triangle_CircumsphereRadius(Vec3 a, Vec3 b, Vec3 c) {
	r32 abc = sqrtf(Vec3_Len2(Vec3_Sub(b, a)) * Vec3_Len2(Vec3_Sub(c, a)) *
	                Vec3_Len2(Vec3_Sub(c, b)));

	r32 area = Triangle_Area(a, b, c);

	return abc / (4 * area);
}

const Transform2D Transform2D_Default = {
    .Position = V2C(0, 0), .Rotation = 0, .Scale = V2C(1, 1)};
const Transform3D Transform3D_Default = {
    .Position = V3C(0, 0, 0), .Rotation = V4C(0, 0, 0, 1), .Scale = V3C(1, 1, 1)};

void Transform2D_Mat3(Transform2D t, Mat3 out) {
	Mat3 translate, rotate, scale;

	Mat3_Identity(translate);
	Mat3_Identity(rotate);
	Mat3_Identity(scale);

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

	Mat3_MultMat(rotate, scale);
	Mat3_MultMat(translate, rotate);
	Mat3_Copy(out, translate);
}

void Transform3D_Mat4(Transform3D t, Mat4 out) {
	Mat4 translate, rotate, scale;

	Mat4_Identity(translate);
	Mat4_Identity(rotate);
	Mat4_Identity(scale);

	// Translation
	Mat4_Translate(translate, t.Position);

	// Rotation
	Mat4_RotateQuat(rotate, t.Rotation);

	// Scale
	Mat4_Scale(scale, t.Scale);

	Mat4_MultMat(rotate, scale);
	Mat4_MultMat(translate, rotate);
	Mat4_Copy(out, translate);
}

Transform3D Mat4_Decompose(Mat4 mat) {
	Transform3D res;

	// Get the translation, which is in the last column
	res.Position = V3(mat[3], mat[7], mat[11]);

	// Get the scale, which is the lengths of the three column vectors
	r32 x     = Vec3_Len(V3(mat[0], mat[4], mat[8]));
	r32 y     = Vec3_Len(V3(mat[1], mat[5], mat[9]));
	r32 z     = Vec3_Len(V3(mat[2], mat[6], mat[10]));
	res.Scale = V3(x, y, z);

	Mat4 copy;
	Mat4_Copy(copy, mat);

	copy[3]  = 0;
	copy[7]  = 0;
	copy[11] = 0;

	copy[0] /= x;
	copy[4] /= x;
	copy[8] /= x;

	copy[1] /= y;
	copy[5] /= y;
	copy[9] /= y;

	copy[2] /= z;
	copy[6] /= z;
	copy[10] /= z;

	// Get the rotation, which is complicated

	// Thank you,
	// https://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/
	float trace = mat[0] + mat[5] + mat[10];
	if(trace > 0) {
		float s = 0.5f / sqrtf(trace + 1.0f);
		res.Rotation =
		    V4((mat[9] - mat[6]) * s, (mat[2] - mat[8]) * s, (mat[4] - mat[1]) * s, 0.25f / s);
	} else {
		if(mat[0] > mat[5] && mat[0] > mat[10]) {
			float s      = 2.0f * sqrtf(1.0f + mat[0] - mat[5] - mat[10]);
			res.Rotation = V4(0.25f * s,
			                  (mat[1] + mat[4]) / s,
			                  (mat[2] + mat[8]) / s,
			                  (mat[9] - mat[6]) / s);
		} else if(mat[5] > mat[10]) {
			float s      = 2.0f * sqrtf(1.0f + mat[5] - mat[0] - mat[10]);
			res.Rotation = V4((mat[1] + mat[4]) / s,
			                  0.25f * s,
			                  (mat[6] + mat[9]) / s,
			                  (mat[2] - mat[8]) / s);
		} else {
			float s      = 2.0f * sqrtf(1.0f + mat[10] - mat[0] - mat[5]);
			res.Rotation = V4((mat[2] + mat[8]) / s,
			                  (mat[6] + mat[9]) / s,
			                  0.25f * s,
			                  (mat[4] - mat[1]) / s);
		}
	}

	return res;
}
