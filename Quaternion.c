#include "Quaternion.h"

#include <assert.h>
#include <float.h>
#include <math.h>

Quat Quat_Identity() { return (Quat){0, 0, 0, 1}; }
Quat Quat_RotAxis(Vec3 axis, r32 angle) {
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

		Vec4 _a = Vec4_MulScal(&a, sin(angle * amt));
		Vec4 _b = Vec4_MulScal(&b, sin(angle * (1 - amt)));
		Vec4 res = Vec4_Add(&_a, &_b);
		Vec4_DivScal_Self(&res, sin(angle));
		return res;
	}
}
Quat Quat_Lerp(Quat a, Quat b, r32 amt) {
	assert(amt >= 0);
	assert(amt <= 1);
	Vec4_MulScal_Self(&a, amt);
	Vec4_MulScal_Self(&b, 1 - amt);
	return Vec4_Add(&a, &b);
}

Quat Quat_Slerp(Quat a, Quat b, r32 amt) {
	r32 cosTheta = Quat_Dot(a, b);
	if(cosTheta < 0) {
		Vec4_Neg_Self(&a);
		cosTheta = -cosTheta;
	}
	return Quat_Mix(a, b, amt);
}
Quat Quat_Conjugate(Quat q) {
	Vec4_Neg_Self(&q);
	q.w = -q.w;
	return q;
}
Quat Quat_Inverse(Quat q) {
	Quat conj = Quat_Conjugate(q);
	return Vec4_DivScal(&conj, Quat_Dot(q, q));
}
r32 Quat_Dot(Quat a, Quat b) { return Vec4_Dot(&a, &b); }
Quat Quat_Norm(Quat q) {
	r32 len = Vec4_Len(&q);
	if(len <= 0) return (Quat){0, 0, 0, 1};
	return Vec4_Norm(&q);
}

Quat Quat_Mul(Quat a, Quat b) {
	r32 x, y, z, w;

	x = b.w * a.x + b.x * a.w + b.y * a.z - b.z * a.y;
	y = b.w * a.y + b.y * a.w + b.z * a.x - b.x * a.z;
	z = b.w * a.z + b.z * a.w + b.x * a.y - b.y * a.x;
	w = b.w * a.w - b.x * a.x - b.y * a.y - b.z * a.z;

	return (Quat){x, y, z, w};
}
