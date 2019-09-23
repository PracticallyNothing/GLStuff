#include "Matrix.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

void Mat2_Identity(Mat2 matrix) {
	memset(matrix, 0, sizeof(Mat2));
	matrix[0] = 1;
	matrix[3] = 1;
}

void Mat3_Identity(Mat3 matrix) {
	memset(matrix, 0, sizeof(Mat3));
	matrix[0] = 1;
	matrix[4] = 1;
	matrix[8] = 1;
}

void Mat4_Identity(Mat4 matrix) {
	memset(matrix, 0, sizeof(Mat4));
	matrix[0] = 1;
	matrix[5] = 1;
	matrix[10] = 1;
	matrix[15] = 1;
}

r32 Mat2_Determinant(const Mat2 m) {
	// ad - bc
	r32 A = m[0], B = m[1], C = m[2], D = m[3];
	return A * D - B * C;
}

r32 Mat3_Determinant(const Mat3 m) {
	// a(ei − fh) − b(di − fg) + c(dh − eg)

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

i32 Mat2_Inverse(Mat2 matrix) { return Mat2_Inverse_Out(matrix, matrix); }
i32 Mat3_Inverse(Mat3 matrix) { return Mat3_Inverse_Out(matrix, matrix); }
i32 Mat4_Inverse(Mat4 matrix) { return Mat4_Inverse_Out(matrix, matrix); }
i32 Mat2_Inverse_Out(const Mat2 matrix, Mat2 out) {
	r32 Det = Mat2_Determinant(matrix);
	// If the determinant is 0, there is no inverse.
	if(Det == 0.0f) return 0;
	Mat2_MulScal_Out(matrix, 1.0 / Det, out);
	return 1;
}
i32 Mat3_Inverse_Out(const Mat3 matrix, Mat3 out) {
	r32 Det = Mat3_Determinant(matrix);
	// If the determinant is 0, there is no inverse.
	if(Det == 0.0f) return 0;
	Mat3_MulScal_Out(matrix, 1.0 / Det, out);
	return 1;
}
i32 Mat4_Inverse_Out(const Mat4 matrix, Mat4 out) {
	r32 Det = Mat4_Determinant(matrix);
	// If the determinant is 0, there is no inverse.
	if(Det == 0.0f) return 0;
	Mat4_MulScal_Out(matrix, 1.0 / Det, out);
	return 1;
}

static void _Swap(r32 *a, r32 *b) {
	r32 tmp = *a;
	*a = *b;
	*b = tmp;
}

void Mat2_Transpose(Mat2 m) { _Swap(m + 1, m + 2); }
void Mat3_Transpose(Mat3 m) {
	_Swap(m + 1, m + 3);
	_Swap(m + 2, m + 6);
	_Swap(m + 5, m + 8);
}
void Mat4_Transpose(Mat4 m) {
	_Swap(m + 1, m + 4);
	_Swap(m + 2, m + 8);
	_Swap(m + 3, m + 12);
	_Swap(m + 6, m + 9);
	_Swap(m + 7, m + 13);
	_Swap(m + 11, m + 14);
}

void Mat2_Neg(Mat2 matrix) { Mat2_Neg_Out(matrix, matrix); }
void Mat3_Neg(Mat3 matrix) { Mat3_Neg_Out(matrix, matrix); }
void Mat4_Neg(Mat4 matrix) { Mat4_Neg_Out(matrix, matrix); }
void Mat2_Neg_Out(const Mat2 m, Mat2 out) {
	out[0] = -m[0];
	out[1] = -m[1];
	out[2] = -m[2];
	out[3] = -m[3];
}
void Mat3_Neg_Out(const Mat3 m, Mat3 out) {
	out[0] = -m[0];
	out[1] = -m[1];
	out[2] = -m[2];
	out[3] = -m[3];
	out[4] = -m[4];
	out[5] = -m[5];
	out[6] = -m[6];
	out[7] = -m[7];
	out[8] = -m[8];
}
void Mat4_Neg_Out(const Mat4 m, Mat4 out) {
	out[0] = -m[0];
	out[1] = -m[1];
	out[2] = -m[2];
	out[3] = -m[3];
	out[4] = -m[4];
	out[5] = -m[5];
	out[6] = -m[6];
	out[7] = -m[7];
	out[8] = -m[8];
	out[9] = -m[9];
	out[10] = -m[10];
	out[11] = -m[11];
	out[12] = -m[12];
	out[13] = -m[13];
	out[14] = -m[14];
	out[15] = -m[15];
}

void Mat2_Add(Mat2 a, const Mat2 b) { Mat2_Add_Out(a, b, a); }
void Mat3_Add(Mat3 a, const Mat3 b) { Mat3_Add_Out(a, b, a); }
void Mat4_Add(Mat4 a, const Mat4 b) { Mat4_Add_Out(a, b, a); }
void Mat2_Add_Out(const Mat2 a, const Mat2 b, Mat2 out) { for(i32 i = 0; i < 4; i++) out[i] = a[i] + b[i]; }
void Mat3_Add_Out(const Mat3 a, const Mat3 b, Mat3 out) { for(i32 i = 0; i < 9; i++) out[i] = a[i] + b[i]; }
void Mat4_Add_Out(const Mat4 a, const Mat4 b, Mat4 out) { for(i32 i = 0; i < 16; i++) out[i] = a[i] + b[i]; }

void Mat2_Sub(Mat2 a, const Mat2 b) { Mat2_Sub_Out(a, b, a); }
void Mat3_Sub(Mat3 a, const Mat3 b) { Mat3_Sub_Out(a, b, a); }
void Mat4_Sub(Mat4 a, const Mat4 b) { Mat4_Sub_Out(a, b, a); }
void Mat2_Sub_Out(const Mat2 a, const Mat2 b, Mat2 out) { for(i32 i = 0; i < 4; i++) out[i] = a[i] - b[i]; }
void Mat3_Sub_Out(const Mat3 a, const Mat3 b, Mat3 out) { for(i32 i = 0; i < 9; i++) out[i] = a[i] - b[i]; }
void Mat4_Sub_Out(const Mat4 a, const Mat4 b, Mat4 out) { for(i32 i = 0; i < 16; i++) out[i] = a[i] + b[i]; }

void Mat2_MulScal(Mat2 matrix, const r32 scalar) { Mat2_MulScal_Out(matrix, scalar, matrix); }
void Mat3_MulScal(Mat3 matrix, const r32 scalar) { Mat3_MulScal_Out(matrix, scalar, matrix); }
void Mat4_MulScal(Mat4 matrix, const r32 scalar) { Mat4_MulScal_Out(matrix, scalar, matrix); }
void Mat2_MulScal_Out(const Mat2 a, r32 scalar, Mat2 out) { for(i32 i = 0; i < 4; i++) out[i] = a[i] * scalar; }
void Mat3_MulScal_Out(const Mat3 a, r32 scalar, Mat3 out) { for(i32 i = 0; i < 9; i++) out[i] = a[i] * scalar; }
void Mat4_MulScal_Out(const Mat4 a, r32 scalar, Mat4 out) { for(i32 i = 0; i < 16; i++) out[i] = a[i] * scalar; }

void Mat2_DivScal(Mat2 matrix, const r32 scalar) { Mat2_DivScal_Out(matrix, scalar, matrix); }
void Mat3_DivScal(Mat3 matrix, const r32 scalar) { Mat3_DivScal_Out(matrix, scalar, matrix); }
void Mat4_DivScal(Mat4 matrix, const r32 scalar) { Mat4_DivScal_Out(matrix, scalar, matrix); }
void Mat2_DivScal_Out(const Mat2 a, r32 scalar, Mat2 out) { for(i32 i = 0; i < 4; i++) out[i] = a[i] / scalar; }
void Mat3_DivScal_Out(const Mat3 a, r32 scalar, Mat3 out) { for(i32 i = 0; i < 9; i++) out[i] = a[i] / scalar; }
void Mat4_DivScal_Out(const Mat4 a, r32 scalar, Mat4 out) { for(i32 i = 0; i < 16; i++) out[i] = a[i] / scalar; }

void Mat2_MulMat(Mat2 a, const Mat2 b) {
	Mat2 *tmp = malloc(sizeof(Mat2));
	Mat2_MulMat_Out(a, b, *tmp);
	memcpy(a, tmp, sizeof(Mat2));
	free(tmp);
}
void Mat3_MulMat(Mat3 a, const Mat3 b) {
	Mat3 *tmp = malloc(sizeof(Mat3));
	Mat3_MulMat_Out(a, b, *tmp);
	memcpy(a, tmp, sizeof(Mat3));
	free(tmp);
}
void Mat4_MulMat(Mat4 a, const Mat4 b) {
	Mat4 *tmp = malloc(sizeof(Mat4));
	Mat4_MulMat_Out(a, b, *tmp);
	memcpy(a, tmp, sizeof(Mat4));
	free(tmp);
}
void Mat2_MulMat_Out(const Mat2 a, const Mat2 b, Mat2 out) {
	out[0] = a[0] * b[0] + a[1] * b[2];
	out[1] = a[0] * b[1] + a[1] * b[3];
	out[2] = a[2] * b[0] + a[3] * b[2];
	out[3] = a[2] * b[1] + a[3] * b[3];
}
void Mat3_MulMat_Out(const Mat3 a, const Mat3 b, Mat3 out) {
	out[0] = a[0] * b[0] + a[1] * b[3] + a[2] * b[6];
	out[1] = a[0] * b[1] + a[1] * b[4] + a[2] * b[7];
	out[2] = a[0] * b[2] + a[1] * b[5] + a[2] * b[8];

	out[3] = a[3] * b[0] + a[4] * b[3] + a[5] * b[6];
	out[4] = a[3] * b[1] + a[4] * b[4] + a[5] * b[7];
	out[5] = a[3] * b[2] + a[4] * b[5] + a[5] * b[8];

	out[6] = a[6] * b[0] + a[7] * b[3] + a[8] * b[6];
	out[7] = a[6] * b[1] + a[7] * b[4] + a[8] * b[7];
	out[8] = a[6] * b[2] + a[7] * b[5] + a[8] * b[8];
}
void Mat4_MulMat_Out(const Mat4 a, const Mat4 b, Mat4 out) {
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
}

i32 Mat2_DivMat(Mat2 a, const Mat2 b) { return Mat2_DivMat_Out(a, b, a); }
i32 Mat3_DivMat(Mat3 a, const Mat3 b) { return Mat3_DivMat_Out(a, b, a); }
i32 Mat4_DivMat(Mat4 a, const Mat4 b) { return Mat4_DivMat_Out(a, b, a); }
i32 Mat2_DivMat_Out(const Mat2 a, const Mat2 b, Mat2 out) {
	Mat2 *inverse = malloc(sizeof(Mat2));
	if(!Mat2_Inverse_Out(b, *inverse)) return 0;
	Mat2_MulMat_Out(a, *inverse, out);
	free(inverse);
	return 1;
}
i32 Mat3_DivMat_Out(const Mat3 a, const Mat3 b, Mat3 out) {
	Mat3 *inverse = malloc(sizeof(Mat3));
	if(!Mat3_Inverse_Out(b, *inverse)) return 0;
	Mat3_MulMat_Out(a, *inverse, out);
	free(inverse);
	return 1;
}
i32 Mat4_DivMat_Out(const Mat4 a, const Mat4 b, Mat4 out) {
	Mat4 *inverse = malloc(sizeof(Mat4));
	if(!Mat4_Inverse_Out(b, *inverse)) return 0;
	Mat4_MulMat_Out(a, *inverse, out);
	free(inverse);
	return 1;
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

void Mat4_RotateVec(Mat4 out, Vec3 amt) {
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

	out[8] = 2 * q.x * q.z - 2 * q.y * q.w;
	out[9] = 2 * q.y * q.z + 2 * q.x * q.w;
	out[10] = 1 - 2 * q.x * q.x - 2 * q.y * q.y;
}
