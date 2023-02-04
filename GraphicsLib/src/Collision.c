#include "../Collision.h"

#include <float.h>

// To get a point on a plane:
//   Multiply the normal by the distance from origin
//
// To get a distance from origin for a plane
//   Calculate the dot product of a point on the plane and the normal

// Thank you,
// http://www.miguelcasillas.com/?p=43
enum Plane_HalfSpace Plane_HalfSpaceTest(Plane p, Vec3 point) {
	// Point on the plane
	Vec3 pp = Vec3_MultScal(p.Normal, p.Distance);

	Vec3 norm = Vec3_Norm(p.Normal);
	Vec3 v    = Vec3_Sub(point, pp);
	r32 dist  = Vec3_Dot(norm, v);

	if(dist > FLT_EPSILON)
		return Plane_HalfSpace_Front;
	else if(dist < -FLT_EPSILON)
		return Plane_HalfSpace_Back;
	else
		return Plane_HalfSpace_On;
}

/// Checks if box b is inside a.
/// DOES NOT check the reverse.
static bool8 AABB_CheckInside(AABB a, AABB b)
{
	a = AABB_Fix(a);
	b = AABB_Fix(b);

	return InRange_R32(b.Min.x, a.Min.x, a.Max.x)
		&& InRange_R32(b.Max.x, a.Min.x, a.Max.x)

		&& InRange_R32(b.Min.y, a.Min.y, a.Max.y)
		&& InRange_R32(b.Max.y, a.Min.y, a.Max.y)

		&& InRange_R32(b.Min.z, a.Min.z, a.Max.z)
		&& InRange_R32(b.Max.z, a.Min.z, a.Max.z);
}

bool8 AABB_CheckCollision(AABB a, AABB b)
{
	a = AABB_Fix(a);
	b = AABB_Fix(b);

	return RangesOverlap_R32(a.Min.x, a.Max.x, b.Min.x, b.Max.x)
		&& RangesOverlap_R32(a.Min.y, a.Max.y, b.Min.y, b.Max.y)
		&& RangesOverlap_R32(a.Min.z, a.Max.z, b.Min.z, b.Max.z);
}

AABB AABB_Fix(AABB aabb)
{
	return (AABB) {
		.Min = V3C(MIN(aabb.Min.x, aabb.Max.x),
				   MIN(aabb.Min.y, aabb.Max.y),
				   MIN(aabb.Min.z, aabb.Max.z)),

		.Max = V3C(MAX(aabb.Min.x, aabb.Max.x),
	               MAX(aabb.Min.y, aabb.Max.y),
                   MAX(aabb.Min.z, aabb.Max.z))
	};
}

AABB AABB_Add(AABB a, AABB b)
{
	return (AABB) {
		.Min = V3C(MIN(a.Min.x, b.Min.x),
				   MIN(a.Min.y, b.Min.y),
				   MIN(a.Min.z, b.Min.z)),

		.Max = V3C(MAX(a.Max.x, b.Max.x),
				   MAX(a.Max.y, b.Max.y),
				   MAX(a.Max.z, b.Max.z)),
	};
}

AABB AABB_ApplyTransform3D(AABB aabb, Transform3D t)
{
	Mat4 model;
	Transform3D_Mat4(t, model);

	AABB res;
	res.Min = Mat4_MultVec4(model, V4_V3(aabb.Min, 1)).xyz;
	res.Max = Mat4_MultVec4(model, V4_V3(aabb.Max, 1)).xyz;
	return AABB_Fix(res);
}
