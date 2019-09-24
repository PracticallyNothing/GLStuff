#ifndef MATRIX_H
#define MATRIX_H

#include "Common.h"
#include "Quaternion.h"
#include "Vector.h"

typedef r32 MatValue_t;

typedef MatValue_t Mat2[4];
typedef MatValue_t Mat3[9];
typedef MatValue_t Mat4[16];

// Identity
// Transpose
// Inverse
// Determinant
// Addition & Subtraction
// Multiplication & Division
//   mat */ scalar
//   mat */ mat
//   mat * vector (TODO)
// Perspective & orthographic projections (TODO)
// Translate, Rotate & Scale (TODO)

/// Generate a 2x2 identity matrix.
extern void Mat2_Identity(Mat2 matrix);
/// Generate a 3x3 identity matrix.
extern void Mat3_Identity(Mat3 matrix);
/// Generate a 4x4 identity matrix.
extern void Mat4_Identity(Mat4 matrix);

/// Get the inverse of a 2x2 matrix or return 0 on failure.
extern i32 Mat2_Inverse(Mat2 matrix);
/// Get the inverse of a 3x3 matrix or return 0 on failure.
extern i32 Mat3_Inverse(Mat3 matrix);
/// Get the inverse of a 4x4 matrix or return 0 on failure.
extern i32 Mat4_Inverse(Mat4 matrix);
/// Write out the inverse of a 4x4 matrix or return 0 on failure.
extern i32 Mat2_Inverse_Out(const Mat2 matrix, Mat2 out);
/// Write out the inverse of a 4x4 matrix or return 0 on failure.
extern i32 Mat3_Inverse_Out(const Mat3 matrix, Mat3 out);
/// Write out the inverse of a 4x4 matrix or return 0 on failure.
extern i32 Mat4_Inverse_Out(const Mat4 matrix, Mat4 out);

extern void Mat2_Transpose(Mat2 matrix);
extern void Mat3_Transpose(Mat3 matrix);
extern void Mat4_Transpose(Mat4 matrix);

extern r32 Mat2_Determinant(const Mat2 matrix);
extern r32 Mat3_Determinant(const Mat3 matrix);
extern r32 Mat4_Determinant(const Mat4 matrix);

extern void Mat2_Neg(Mat2 matrix);
extern void Mat3_Neg(Mat3 matrix);
extern void Mat4_Neg(Mat4 matrix);
extern void Mat2_Neg_Out(const Mat2 matrix, Mat2 out);
extern void Mat3_Neg_Out(const Mat3 matrix, Mat3 out);
extern void Mat4_Neg_Out(const Mat4 matrix, Mat4 out);

extern void Mat2_Add(Mat2 a, const Mat2 b);
extern void Mat3_Add(Mat3 a, const Mat3 b);
extern void Mat4_Add(Mat4 a, const Mat4 b);
extern void Mat2_Add_Out(const Mat2 a, const Mat2 b, Mat2 out);
extern void Mat3_Add_Out(const Mat3 a, const Mat3 b, Mat3 out);
extern void Mat4_Add_Out(const Mat4 a, const Mat4 b, Mat4 out);

extern void Mat2_Sub(Mat2 a, const Mat2 b);
extern void Mat3_Sub(Mat3 a, const Mat3 b);
extern void Mat4_Sub(Mat4 a, const Mat4 b);
extern void Mat2_Sub_Out(const Mat2 a, const Mat2 b, Mat2 out);
extern void Mat3_Sub_Out(const Mat3 a, const Mat3 b, Mat3 out);
extern void Mat4_Sub_Out(const Mat4 a, const Mat4 b, Mat4 out);

extern void Mat2_MulScal(Mat2 matrix, const r32 scalar);
extern void Mat3_MulScal(Mat3 matrix, const r32 scalar);
extern void Mat4_MulScal(Mat4 matrix, const r32 scalar);
extern void Mat2_MulScal_Out(const Mat2 matrix, const r32 scalar, Mat2 out);
extern void Mat3_MulScal_Out(const Mat3 matrix, const r32 scalar, Mat3 out);
extern void Mat4_MulScal_Out(const Mat4 matrix, const r32 scalar, Mat4 out);

extern void Mat2_MulMat(Mat2 a, const Mat2 b);
extern void Mat3_MulMat(Mat3 a, const Mat3 b);
extern void Mat4_MulMat(Mat4 a, const Mat4 b);
extern void Mat2_MulMat_Out(const Mat2 a, const Mat2 b, Mat2 out);
extern void Mat3_MulMat_Out(const Mat3 a, const Mat3 b, Mat3 out);
extern void Mat4_MulMat_Out(const Mat4 a, const Mat4 b, Mat4 out);

extern void Mat2_DivScal(Mat2 matrix, const r32 scalar);
extern void Mat3_DivScal(Mat3 matrix, const r32 scalar);
extern void Mat4_DivScal(Mat4 matrix, const r32 scalar);
extern void Mat2_DivScal_Out(const Mat2 matrix, const r32 scalar, Mat2 out);
extern void Mat3_DivScal_Out(const Mat3 matrix, const r32 scalar, Mat3 out);
extern void Mat4_DivScal_Out(const Mat4 matrix, const r32 scalar, Mat4 out);

extern i32 Mat2_DivMat(Mat2 a, const Mat2 b);
extern i32 Mat3_DivMat(Mat3 a, const Mat3 b);
extern i32 Mat4_DivMat(Mat4 a, const Mat4 b);
extern i32 Mat2_DivMat_Out(const Mat2 a, const Mat2 b, Mat2 out);
extern i32 Mat3_DivMat_Out(const Mat3 a, const Mat3 b, Mat3 out);
extern i32 Mat4_DivMat_Out(const Mat4 a, const Mat4 b, Mat4 out);

extern void Mat4_Translate(Mat4 out, Vec3 amt);
extern void Mat4_RotateVec(Mat4 out, Vec3 amt);
extern void Mat4_RotateQuat(Mat4 out, Quat quat);
extern void Mat4_Scale(Mat4 out, Vec3 amt);

extern void Mat4_OrthoProj(Mat4 out, r32 left, r32 right, r32 top, r32 bottom,
                           r32 zNear, r32 zFar);
extern void Mat4_RectProj(Mat4 out, r32 fov, r32 aspect, r32 zNear, r32 zFar);

#endif
