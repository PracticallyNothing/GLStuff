#include "Math3D.h"

#include <assert.h>
#include <float.h>
#include <math.h>
#include <string.h>

// --- Vector operations --- //

Vec2 Vec2_Neg(Vec2 v) { return V2(-v.x, -v.y); }
Vec3 Vec3_Neg(Vec3 v) { return V3(-v.x, -v.y, -v.z); }
Vec4 Vec4_Neg(Vec4 v) { return V4(-v.x, -v.y, -v.z, -v.w); }

Vec2 Vec2_Add(Vec2 a, Vec2 b) { return V2(a.x + b.x, a.y + b.y); }
Vec3 Vec3_Add(Vec3 a, Vec3 b) { return V3(a.x + b.x, a.y + b.y, a.z + b.z); }
Vec4 Vec4_Add(Vec4 a, Vec4 b) {
	return V4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

Vec2 Vec2_Sub(Vec2 a, Vec2 b) { return V2(a.x - b.x, a.y - b.y); }
Vec3 Vec3_Sub(Vec3 a, Vec3 b) { return V3(a.x - b.x, a.y - b.y, a.z - b.z); }
Vec4 Vec4_Sub(Vec4 a, Vec4 b) {
	return V4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}

Vec2 Vec2_MultScal(Vec2 v, r32 s) { return V2(v.x * s, v.y * s); }
Vec3 Vec3_MultScal(Vec3 v, r32 s) { return V3(v.x * s, v.y * s, v.z * s); }
Vec4 Vec4_MultScal(Vec4 v, r32 s) {
	return V4(v.x * s, v.y * s, v.z * s, v.w * s);
}

Vec2 Vec2_DivScal(Vec2 v, r32 s) { return V2(v.x / s, v.y / s); }
Vec3 Vec3_DivScal(Vec3 v, r32 s) { return V3(v.x / s, v.y / s, v.z / s); }
Vec4 Vec4_DivScal(Vec4 v, r32 s) {
	return V4(v.x / s, v.y / s, v.z / s, v.w / s);
}

Vec2 Vec2_MultVec(Vec2 a, Vec2 b) { return V2(a.x * b.x, a.y * b.y); }
Vec3 Vec3_MultVec(Vec3 a, Vec3 b) {
	return V3(a.x * b.x, a.y * b.y, a.z * b.z);
}
Vec4 Vec4_MultVec(Vec4 a, Vec4 b) {
	return V4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}

Vec2 Vec2_DivVec(Vec2 a, Vec2 b) { return V2(a.x / b.x, a.y / b.y); }
Vec3 Vec3_DivVec(Vec3 a, Vec3 b) { return V3(a.x / b.x, a.y / b.y, a.z / b.z); }
Vec4 Vec4_DivVec(Vec4 a, Vec4 b) {
	return V4(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w);
}

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

r32 Vec2_Dot(Vec2 a, Vec2 b) { return Vec2_Len2(Vec2_MultVec(a, b)); }
r32 Vec3_Dot(Vec3 a, Vec3 b) { return Vec3_Len2(Vec3_MultVec(a, b)); }
r32 Vec4_Dot(Vec4 a, Vec4 b) { return Vec4_Len2(Vec4_MultVec(a, b)); }

r32 Vec2_Cross(Vec2 a, Vec2 b) { return a.x * b.y - a.y * b.x; }
Vec3 Vec3_Cross(Vec3 a, Vec3 b) {
	return V3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z,
	          a.x * b.y - a.y * b.x);
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
	r32 A = m[0], B = m[1], C = m[2], D = m[3], E = m[4], F = m[5], G = m[6],
	    H = m[7], I = m[8];

	return A * (E * I - F * H) - B * (D * I - F * G) + C * (D * H - E * G);
}

r32 Mat4_Determinant(const Mat4 m) {
	// Thank you, Excel and Libreoffice Calc.

	r32 A = m[0], B = m[1], C = m[2], D = m[3], E = m[4], F = m[5], G = m[6],
	    H = m[7], I = m[8], J = m[9], K = m[10], L = m[11], M = m[12],
	    N = m[13], O = m[14], P = m[15];

	r32 Det[4] = {
	    F * (K * P - O * L) - G * (J * P - N * L) + H * (J * O - N * K),
	    E * (K * P - O * L) - G * (I * P - M * L) + H * (I * O - M * K),
	    E * (J * P - N * L) - F * (I * P - M * L) + H * (I * N - M * J),
	    E * (J * O - N * K) - F * (I * O - M * K) + G * (I * N - M * J),
	};

	return A * Det[0] - B * Det[1] + C * Det[2] - D * Det[3];
}

i32 Mat2_Inverse(Mat2 m) {
	r32 Det = Mat2_Determinant(m);

	if(Det == 0.0f) return 0;

	Mat2_MultScal(m, 1.0 / Det);
	return 1;
}

i32 Mat3_Inverse(Mat3 m) {
	r32 Det = Mat3_Determinant(m);

	if(Det == 0.0f) return 0;

	Mat3_MultScal(m, 1.0 / Det);
	return 1;
}

i32 Mat4_Inverse(Mat4 m) {
	r32 Det = Mat4_Determinant(m);

	if(Det == 0.0f) return 0;

	Mat4_MultScal(m, 1.0 / Det);
	return 1;
}

static void _Swap(r32 *a, r32 *b) {
	if(a == b) return;
	r32 tmp = *a;
	*a = *b;
	*b = tmp;
}

void Mat2_Transpose(Mat2 m) {
	for(i32 y = 0; y < 1; y++)
		for(i32 x = 0; x < 1; x++) _Swap(m + (x + y * 2), m + (y + x * 2));
}
void Mat3_Transpose(Mat3 m) {
	for(i32 y = 0; y < 2; y++)
		for(i32 x = 0; x < 2; x++) _Swap(m + (x + y * 3), m + (y + x * 3));
}
void Mat4_Transpose(Mat4 m) {
	for(i32 y = 0; y < 3; y++)
		for(i32 x = 0; x < 3; x++) _Swap(m + (x + y * 4), m + (y + x * 4));
}

void Mat4_OrthoProj(Mat4 out, r32 left, r32 right, r32 top, r32 bottom,
                    r32 zNear, r32 zFar) {
	Mat4_Identity(out);
	r32 x = right - left;
	r32 y = top - bottom;
	r32 z = zFar - zNear;
	out[0] = 2.0 / x;
	out[3] = (x > 0 ? -1 : 1);
	out[5] = 2.0 / y;
	out[7] = (y > 0 ? -1 : 1);
	out[10] = 2.0 / z;
	out[11] = (z > 0 ? -1 : 1);
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

	out[8] = a[8] * b[0] + a[9] * b[4] + a[10] * b[8] + a[11] * b[12];
	out[9] = a[8] * b[1] + a[9] * b[5] + a[10] * b[9] + a[11] * b[13];
	out[10] = a[8] * b[2] + a[9] * b[6] + a[10] * b[10] + a[11] * b[14];
	out[11] = a[8] * b[3] + a[9] * b[7] + a[10] * b[11] + a[11] * b[15];

	out[12] = a[12] * b[0] + a[13] * b[4] + a[14] * b[8] + a[15] * b[12];
	out[13] = a[12] * b[1] + a[13] * b[5] + a[14] * b[9] + a[15] * b[13];
	out[14] = a[12] * b[2] + a[13] * b[6] + a[14] * b[10] + a[15] * b[14];
	out[15] = a[12] * b[3] + a[13] * b[7] + a[14] * b[11] + a[15] * b[15];
	Mat4_Copy(a, out);
}

i32 Mat2_DivMat(Mat2 a, const Mat2 b) {
	Mat2 inverse;
	Mat2_Copy(inverse, b);

	if(!Mat2_Inverse(inverse)) return 0;

	Mat2_MultMat(a, inverse);
	return 1;
}

i32 Mat3_DivMat(Mat3 a, const Mat3 b) {
	Mat3 inverse;
	Mat3_Copy(inverse, b);

	if(!Mat3_Inverse(inverse)) return 0;

	Mat3_MultMat(a, inverse);
	return 1;
}

i32 Mat4_DivMat(Mat4 a, const Mat4 b) {
	Mat4 inverse;
	Mat4_Copy(inverse, b);

	if(!Mat4_Inverse(inverse)) return 0;

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

	out[0] = 1.0 / (aspect * tanHalfFov);
	out[5] = 1.0 / tanHalfFov;
	out[10] = (-zNear - zFar) / (zNear - zFar);
	out[11] = (2.0 * zFar * zNear) / (zNear - zFar);
	out[14] = 1.0;
	out[15] = 0;
}

void Mat4_Translate(Mat4 out, Vec3 amt) {
	out[3] = amt.x;
	out[7] = amt.y;
	out[11] = amt.z;
}

void Mat4_Scale(Mat4 out, Vec3 amt) {
	out[0] = amt.x;
	out[5] = amt.y;
	out[10] = amt.z;
}

void Mat4_RotateVec(Mat4 out, Vec3 amt) {}

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

	out[8] = 2 * q.x * q.z - 2 * q.y * q.w;
	out[9] = 2 * q.y * q.z + 2 * q.x * q.w;
	out[10] = 1 - 2 * q.x * q.x - 2 * q.y * q.y;
}

// --- Quaternion operations --- //

const Quat Quat_Identity = {.x = 0, .y = 0, .z = 0, .w = 1};

Quat Quat_RotAxis(Vec3 axis, r32 angle) {
	axis = Vec3_Norm(axis);
	return (Quat){axis.x * sin(angle / 2), axis.y * sin(angle / 2),
	              axis.z * sin(angle / 2), cos(angle / 2)};
}

static r32 _Mix(r32 a, r32 b, r32 t) { return t * a + (1 - t) * b; }

Quat Quat_Mix(Quat a, Quat b, r32 amt) {
	r32 cosTheta = Quat_Dot(a, b);

	if(cosTheta > 1 - FLT_EPSILON) {
		return (Quat){_Mix(a.x, b.x, amt), _Mix(a.y, b.y, amt),
		              _Mix(a.z, b.z, amt), _Mix(a.w, b.w, amt)};
	} else {
		r32 angle = acos(cosTheta);

		Vec4 _a = Vec4_MultScal(a, sin(angle * amt));
		Vec4 _b = Vec4_MultScal(b, sin(angle * (1 - amt)));
		Vec4 res = Vec4_Add(_a, _b);
		res = Vec4_DivScal(res, sin(angle));
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
		a = Vec4_Neg(a);
		cosTheta = -cosTheta;
	}
	return Quat_Mix(a, b, amt);
}
Quat Quat_Conjugate(Quat q) {
	q = Vec4_Neg(q);
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
	if(len <= 0) return (Quat){0, 0, 0, 1};
	return Vec4_Norm(q);
}

Quat Quat_Mult(Quat a, Quat b) {
	r32 x, y, z, w;

	x = b.w * a.x + b.x * a.w + b.y * a.z - b.z * a.y;
	y = b.w * a.y + b.y * a.w + b.z * a.x - b.x * a.z;
	z = b.w * a.z + b.z * a.w + b.x * a.y - b.y * a.x;
	w = b.w * a.w - b.x * a.x - b.y * a.y - b.z * a.z;

	return (Quat){x, y, z, w};
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
	return V3((HexToInt(str[0]) * 16 + HexToInt(str[1])) / 255.0,
	          (HexToInt(str[2]) * 16 + HexToInt(str[3])) / 255.0,
	          (HexToInt(str[4]) * 16 + HexToInt(str[5])) / 255.0);
}

RGBA HexToRGBA(const char str[8]) {
	return V4((HexToInt(str[0]) * 16 + HexToInt(str[1])) / 255.0,
	          (HexToInt(str[2]) * 16 + HexToInt(str[3])) / 255.0,
	          (HexToInt(str[4]) * 16 + HexToInt(str[5])) / 255.0,
	          (HexToInt(str[6]) * 16 + HexToInt(str[7])) / 255.0);
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
