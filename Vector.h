#ifndef VECTOR_H
#define VECTOR_H

#include "Common.h"

typedef struct {
	r32 x, y;
} Vec2;
typedef struct {
	r32 x, y, z;
} Vec3;
typedef struct {
	r32 x, y, z, w;
} Vec4;

extern Vec3 Vec2_To_Vec3(const Vec2 *v, r32 z);
extern Vec4 Vec2_To_Vec4(const Vec2 *v, r32 z, r32 w);
extern Vec4 Vec3_To_Vec4(const Vec3 *v, r32 w);

extern Vec3 Vec4_To_Vec3(const Vec4 *v);
extern Vec2 Vec4_To_Vec2(const Vec4 *v);
extern Vec2 Vec3_To_Vec2(const Vec3 *v);

extern Vec2 Vec2_Neg(const Vec2 *v);
extern Vec3 Vec3_Neg(const Vec3 *v);
extern Vec4 Vec4_Neg(const Vec4 *v);
extern void Vec2_Neg_Self(Vec2 *v);
extern void Vec3_Neg_Self(Vec3 *v);
extern void Vec4_Neg_Self(Vec4 *v);

extern Vec2 Vec2_Add(const Vec2 *a, const Vec2 *b);
extern Vec3 Vec3_Add(const Vec3 *a, const Vec3 *b);
extern Vec4 Vec4_Add(const Vec4 *a, const Vec4 *b);
extern void Vec2_Add_Self(Vec2 *a, const Vec2 *b);
extern void Vec3_Add_Self(Vec3 *a, const Vec3 *b);
extern void Vec4_Add_Self(Vec4 *a, const Vec4 *b);

extern Vec2 Vec2_Sub(const Vec2 *a, const Vec2 *b);
extern Vec3 Vec3_Sub(const Vec3 *a, const Vec3 *b);
extern Vec4 Vec4_Sub(const Vec4 *a, const Vec4 *b);
extern void Vec2_Sub_Self(Vec2 *a, const Vec2 *b);
extern void Vec3_Sub_Self(Vec3 *a, const Vec3 *b);
extern void Vec4_Sub_Self(Vec4 *a, const Vec4 *b);

extern Vec2 Vec2_MulScal(const Vec2 *v, r32 scalar);
extern Vec3 Vec3_MulScal(const Vec3 *v, r32 scalar);
extern Vec4 Vec4_MulScal(const Vec4 *v, r32 scalar);
extern void Vec2_MulScal_Self(Vec2 *v, r32 scalar);
extern void Vec3_MulScal_Self(Vec3 *v, r32 scalar);
extern void Vec4_MulScal_Self(Vec4 *v, r32 scalar);

extern Vec2 Vec2_DivScal(const Vec2 *v, r32 scalar);
extern Vec3 Vec3_DivScal(const Vec3 *v, r32 scalar);
extern Vec4 Vec4_DivScal(const Vec4 *v, r32 scalar);
extern void Vec2_DivScal_Self(Vec2 *v, r32 scalar);
extern void Vec3_DivScal_Self(Vec3 *v, r32 scalar);
extern void Vec4_DivScal_Self(Vec4 *v, r32 scalar);

extern Vec2 Vec2_MulVec(const Vec2 *a, const Vec2 *b);
extern Vec3 Vec3_MulVec(const Vec3 *a, const Vec3 *b);
extern Vec4 Vec4_MulVec(const Vec4 *a, const Vec4 *b);
extern void Vec2_MulVec_Self(Vec2 *a, const Vec2 *b);
extern void Vec3_MulVec_Self(Vec3 *a, const Vec3 *b);
extern void Vec4_MulVec_Self(Vec4 *a, const Vec4 *b);

extern Vec2 Vec2_DivVec(const Vec2 *a, const Vec2 *b);
extern Vec3 Vec3_DivVec(const Vec3 *a, const Vec3 *b);
extern Vec4 Vec4_DivVec(const Vec4 *a, const Vec4 *b);
extern void Vec2_DivVec_Self(Vec2 *a, const Vec2 *b);
extern void Vec3_DivVec_Self(Vec3 *a, const Vec3 *b);
extern void Vec4_DivVec_Self(Vec4 *a, const Vec4 *b);

extern r32 Vec2_Len2(const Vec2 *v);
extern r32 Vec3_Len2(const Vec3 *v);
extern r32 Vec4_Len2(const Vec4 *v);

extern r32 Vec2_Len(const Vec2 *v);
extern r32 Vec3_Len(const Vec3 *v);
extern r32 Vec4_Len(const Vec4 *v);

extern Vec2 Vec2_Norm(const Vec2 *v);
extern Vec3 Vec3_Norm(const Vec3 *v);
extern Vec4 Vec4_Norm(const Vec4 *v);
extern void Vec2_Norm_Self(Vec2 *v);
extern void Vec3_Norm_Self(Vec3 *v);
extern void Vec4_Norm_Self(Vec4 *v);

extern r32 Vec2_Dot(const Vec2 *a, const Vec2 *b);
extern r32 Vec3_Dot(const Vec3 *a, const Vec3 *b);
extern r32 Vec4_Dot(const Vec4 *a, const Vec4 *b);

extern r32 Vec2_Cross(const Vec2 *a, const Vec2 *b);
extern Vec3 Vec3_Cross(const Vec3 *a, const Vec3 *b);
#endif
