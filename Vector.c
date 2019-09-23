#include "Vector.h"

#include <math.h>

static Vec2 Vec2_Default() { return (Vec2){0, 0}; }
static Vec3 Vec3_Default() { return (Vec3){0, 0, 0}; }
static Vec4 Vec4_Default() { return (Vec4){0, 0, 0, 0}; }

Vec3 Vec2_To_Vec3(const Vec2 *v, r32 z) {
	Vec3 res;
	res.x = v->x;
	res.y = v->y;
	res.z = z;
	return res;
}

Vec4 Vec2_To_Vec4(const Vec2 *v, r32 z, r32 w) {
	Vec4 res;
	res.x = v->x;
	res.y = v->y;
	res.z = z;
	res.w = w;
	return res;
}
Vec4 Vec3_To_Vec4(const Vec3 *v, r32 w) {
	Vec4 res;
	res.x = v->x;
	res.y = v->y;
	res.z = v->z;
	res.w = w;
	return res;
}

Vec3 Vec4_To_Vec3(const Vec4 *v) {
	Vec3 res;
	res.x = v->x;
	res.y = v->y;
	res.z = v->z;
	return res;
}
Vec2 Vec4_To_Vec2(const Vec4 *v) {
	Vec2 res;
	res.x = v->x;
	res.y = v->y;
	return res;
}
Vec2 Vec3_To_Vec2(const Vec3 *v) {
	Vec2 res;
	res.x = v->x;
	res.y = v->y;
	return res;
}

Vec2 Vec2_Neg(const Vec2 *v) { return (Vec2){-v->x, -v->y}; }
Vec3 Vec3_Neg(const Vec3 *v) { return (Vec3){-v->x, -v->y, -v->z}; }
Vec4 Vec4_Neg(const Vec4 *v) { return (Vec4){-v->x, -v->y, -v->z, -v->w}; }
void Vec2_Neg_Self(Vec2 *v) {
	v->x = -v->x;
	v->y = -v->y;
}
void Vec3_Neg_Self(Vec3 *v) {
	v->x = -v->x;
	v->y = -v->y;
	v->z = -v->z;
}
void Vec4_Neg_Self(Vec4 *v) {
	v->x = -v->x;
	v->y = -v->y;
	v->z = -v->z;
	v->w = -v->w;
}

Vec2 Vec2_Add(const Vec2 *a, const Vec2 *b) {
	Vec2 res;
	res.x = a->x + b->x;
	res.y = a->y + b->y;
	return res;
}
Vec3 Vec3_Add(const Vec3 *a, const Vec3 *b) {
	Vec3 res;
	res.x = a->x + b->x;
	res.y = a->y + b->y;
	res.z = a->z + b->z;
	return res;
}
Vec4 Vec4_Add(const Vec4 *a, const Vec4 *b) {
	Vec4 res;
	res.x = a->x + b->x;
	res.y = a->y + b->y;
	res.z = a->z + b->z;
	res.w = a->w + b->w;
	return res;
}
void Vec2_Add_Self(Vec2 *a, const Vec2 *b) {
	a->x += b->x;
	a->y += b->y;
}
void Vec3_Add_Self(Vec3 *a, const Vec3 *b) {
	a->x += b->x;
	a->y += b->y;
	a->z += b->z;
}
void Vec4_Add_Self(Vec4 *a, const Vec4 *b) {
	a->x += b->x;
	a->y += b->y;
	a->z += b->z;
	a->w += b->w;
}

Vec2 Vec2_Sub(const Vec2 *a, const Vec2 *b) {
	return (Vec2){a->x - b->x, a->y - b->y};
}
Vec3 Vec3_Sub(const Vec3 *a, const Vec3 *b) {
	return (Vec3){a->x - b->x, a->y - b->y, a->z - b->z};
}
Vec4 Vec4_Sub(const Vec4 *a, const Vec4 *b) {
	return (Vec4){a->x - b->x, a->y - b->y, a->z - b->z, a->w - b->w};
}
void Vec2_Sub_Self(Vec2 *a, const Vec2 *b) {
	a->x -= b->x;
	a->y -= b->y;
}
void Vec3_Sub_Self(Vec3 *a, const Vec3 *b) {
	a->x -= b->x;
	a->y -= b->y;
	a->z -= b->z;
}
void Vec4_Sub_Self(Vec4 *a, const Vec4 *b) {
	a->x -= b->x;
	a->y -= b->y;
	a->z -= b->z;
	a->w -= b->w;
}

Vec2 Vec2_MulScal(const Vec2 *v, r32 scalar) {
	return (Vec2){v->x * scalar, v->y * scalar};
}
Vec3 Vec3_MulScal(const Vec3 *v, r32 scalar) {
	return (Vec3){v->x * scalar, v->y * scalar, v->z * scalar};
}
Vec4 Vec4_MulScal(const Vec4 *v, r32 scalar) {
	return (Vec4){v->x * scalar, v->y * scalar, v->z * scalar, v->w * scalar};
}
void Vec2_MulScal_Self(Vec2 *v, r32 scalar) {
	v->x *= scalar;
	v->y *= scalar;
}
void Vec3_MulScal_Self(Vec3 *v, r32 scalar) {
	v->x *= scalar;
	v->y *= scalar;
	v->z *= scalar;
}
void Vec4_MulScal_Self(Vec4 *v, r32 scalar) {
	v->x *= scalar;
	v->y *= scalar;
	v->z *= scalar;
	v->w *= scalar;
}

Vec2 Vec2_DivScal(const Vec2 *v, r32 scalar) {
	Vec2 res;
	res.x = v->x / scalar;
	res.y = v->y / scalar;
	return res;
}
Vec3 Vec3_DivScal(const Vec3 *v, r32 scalar) {
	Vec3 res;
	res.x = v->x / scalar;
	res.y = v->y / scalar;
	res.z = v->z / scalar;
	return res;
}
Vec4 Vec4_DivScal(const Vec4 *v, r32 scalar) {
	Vec4 res;
	res.x = v->x / scalar;
	res.y = v->y / scalar;
	res.z = v->z / scalar;
	res.w = v->w / scalar;
	return res;
}
void Vec2_DivScal_Self(Vec2 *v, r32 scalar) {
	v->x /= scalar;
	v->y /= scalar;
}
void Vec3_DivScal_Self(Vec3 *v, r32 scalar) {
	v->x /= scalar;
	v->y /= scalar;
	v->z /= scalar;
}
void Vec4_DivScal_Self(Vec4 *v, r32 scalar) {
	v->x /= scalar;
	v->y /= scalar;
	v->z /= scalar;
	v->w /= scalar;
}

Vec2 Vec2_MulVec(const Vec2 *a, const Vec2 *b) {
	return (Vec2){a->x * b->x, a->y * b->y};
}
Vec3 Vec3_MulVec(const Vec3 *a, const Vec3 *b) {
	return (Vec3){a->x * b->x, a->y * b->y, a->z * b->z};
}
Vec4 Vec4_MulVec(const Vec4 *a, const Vec4 *b) {
	return (Vec4){a->x * b->x, a->y * b->y, a->z * b->z, a->w * b->w};
}
void Vec2_MulVec_Self(Vec2 *a, const Vec2 *b) {
	a->x *= b->x;
	a->y *= b->y;
}
void Vec3_MulVec_Self(Vec3 *a, const Vec3 *b) {
	a->x *= b->x;
	a->y *= b->y;
	a->z *= b->z;
}
void Vec4_MulVec_Self(Vec4 *a, const Vec4 *b) {
	a->x *= b->x;
	a->y *= b->y;
	a->z *= b->z;
	a->w *= b->w;
}

Vec2 Vec2_DivVec(const Vec2 *a, const Vec2 *b) {
	return (Vec2){a->x / b->x, a->y / b->y};
}
Vec3 Vec3_DivVec(const Vec3 *a, const Vec3 *b) {
	return (Vec3){a->x / b->x, a->y / b->y, a->z / b->z};
}
Vec4 Vec4_DivVec(const Vec4 *a, const Vec4 *b) {
	return (Vec4){a->x / b->x, a->y / b->y, a->z / b->z, a->w / b->w};
}
void Vec2_DivVec_Self(Vec2 *a, const Vec2 *b) {}
void Vec3_DivVec_Self(Vec3 *a, const Vec3 *b) {}
void Vec4_DivVec_Self(Vec4 *a, const Vec4 *b) {}

r32 Vec2_Len2(const Vec2 *v) { return v->x * v->x + v->y * v->y; }
r32 Vec3_Len2(const Vec3 *v) { return v->x * v->x + v->y * v->y + v->z * v->z; }
r32 Vec4_Len2(const Vec4 *v) {
	return v->x * v->x + v->y * v->y + v->z * v->z * v->w * v->w;
}

r32 Vec2_Len(const Vec2 *v) { return sqrtf(Vec2_Len2(v)); }
r32 Vec3_Len(const Vec3 *v) { return sqrtf(Vec3_Len2(v)); }
r32 Vec4_Len(const Vec4 *v) { return sqrtf(Vec4_Len2(v)); }

Vec2 Vec2_Norm(const Vec2 *v) { return Vec2_DivScal(v, Vec2_Len(v)); }
Vec3 Vec3_Norm(const Vec3 *v) { return Vec3_DivScal(v, Vec3_Len(v)); }
Vec4 Vec4_Norm(const Vec4 *v) { return Vec4_DivScal(v, Vec4_Len(v)); }
void Vec2_Norm_Self(Vec2 *v) { *v = Vec2_Norm(v); }
void Vec3_Norm_Self(Vec3 *v) { *v = Vec3_Norm(v); }
void Vec4_Norm_Self(Vec4 *v) { *v = Vec4_Norm(v); }

r32 Vec2_Dot(const Vec2 *a, const Vec2 *b) { return a->x * b->x + a->y * b->y; }
r32 Vec3_Dot(const Vec3 *a, const Vec3 *b) {
	return a->x * b->x + a->y * b->y + a->z * b->z;
}
r32 Vec4_Dot(const Vec4 *a, const Vec4 *b) {
	return a->x * b->x + a->y * b->y + a->z * b->z + a->w * b->w;
}

r32 Vec2_Cross(const Vec2 *a, const Vec2 *b) {
	return a->x * b->y - a->y * b->x;
}
Vec3 Vec3_Cross(const Vec3 *a, const Vec3 *b) {
	return (Vec3){a->y * b->z - a->z * b->y, a->z * b->x - a->x * b->z,
	              a->x * b->y - a->y * b->x};
}
