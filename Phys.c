#include "Phys.h"

#include <math.h>
#include <stdlib.h>
#include "Common.h"
#include "Math3D.h"
#include "Transform.h"

bool8 AABB_CheckCollision(struct AABB a, struct AABB b)
{
	a = AABB_Fix(a);
	b = AABB_Fix(b);

	return RangesOverlap_R32(a.Min.x, a.Max.x, b.Min.x, b.Max.x)
		&& RangesOverlap_R32(a.Min.y, a.Max.y, b.Min.y, b.Max.y)
		&& RangesOverlap_R32(a.Min.z, a.Max.z, b.Min.z, b.Max.z);
}

struct AABB AABB_Fix(struct AABB aabb)
{
	return (struct AABB) {
		.Min = V3C(MIN(aabb.Min.x, aabb.Max.x),
				   MIN(aabb.Min.y, aabb.Max.y),
				   MIN(aabb.Min.z, aabb.Max.z)),

		.Max = V3C(MAX(aabb.Min.x, aabb.Max.x),
	               MAX(aabb.Min.y, aabb.Max.y),
                   MAX(aabb.Min.z, aabb.Max.z))
	};
}

struct AABB 
AABB_ApplyTransform3D (struct AABB aabb, Transform3D t)
{
	Mat4 model;
	Transform3D_Mat4(t, model);

	struct AABB res; 
	res.Min = Mat4_MultVec4(model, V4_V3(aabb.Min, 1)).xyz;
	res.Max = Mat4_MultVec4(model, V4_V3(aabb.Max, 1)).xyz;
	return AABB_Fix(res);
}

/// Function for debugging purposes.
static bool8 GetSign_R32(r32 s) { return signbit(s); }

// Thank you,
// https://stackoverflow.com/a/2049593
static r32 
sign(Vec2 p1, Vec2 p2, Vec2 p3) { return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y); }

static bool8 
Triangle_PointInside(const Vec2 t[3], Vec2 p)
{
    r32 d1, d2, d3;
    bool8 has_neg, has_pos;

    d1 = sign(p, t[1], t[2]);
    d2 = sign(p, t[2], t[3]);
    d3 = sign(p, t[3], t[1]);

    has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

    return !(has_neg && has_pos);
}

// Thank you,
// https://web.stanford.edu/class/cs277/resources/papers/Moller1997b.pdf
static struct Intersection
TriTri_Intersect(const Vec3 Tri1[3], const Vec3 Tri2[3])
{
	const r32 epsilon = 1e-8;

	Vec3 N2 = Vec3_Cross(Vec3_Sub(Tri2[1], Tri2[0]), Vec3_Sub(Tri2[2], Tri2[0]));
	r32 d2 = Vec3_Dot(Vec3_Neg(N2), Tri2[0]);
	r32 dist1[3] = {0};


	for(u32 i = 0; i < 3; i++) {
		dist1[i] = Vec3_Dot(N2, Tri1[i]) + d2;
		if(fabs(dist1[i]) < epsilon) dist1[i] = 0;
	}

	if(dist1[0] != 0 && dist1[1] != 0 && dist1[2] != 0 &&
	   TRIEQ(GetSign_R32(dist1[0]), GetSign_R32(dist1[1]), GetSign_R32(dist1[2])))
	{
		Log(DEBUG, "Early reject 1. dist1[%.2f, %.2f, %.2f]", dist1[0], dist1[1], dist1[2]);
		return (struct Intersection) { .Occurred = 0 };
	}

	Vec3 N1 = Vec3_Cross(Vec3_Sub(Tri1[1], Tri1[0]), Vec3_Sub(Tri1[2], Tri1[0]));
	r32 d1 = Vec3_Dot(Vec3_Neg(N1), Tri1[0]);
	r32	dist2[3] = {0};

	for(u32 i = 0; i < 3; i++) {
		dist2[i] = Vec3_Dot(N1, Tri2[i]) + d1;
		if(fabs(dist2[i]) < epsilon) dist2[i] = 0;
	}

	if(dist2[0] != 0 && dist2[1] != 0 && dist2[2] != 0 &&
	   TRIEQ(GetSign_R32(dist2[0]), GetSign_R32(dist2[1]), GetSign_R32(dist2[2])))
	{
		Log(DEBUG, "Early reject 2. dist2[%.2f, %.2f, %.2f]", dist2[0], dist2[1], dist2[2]);
		return (struct Intersection) { .Occurred = 0 };
	}

	if(dist2[0] == 0 && TRIEQ(dist2[0], dist2[1], dist2[2])) {
		// The triangles are co-planar.
		Log(DEBUG, "Triangles are co-planar.", "");
		
		//
		// TODO: This is most likely horrible code, but the document says that the triangles 
		//       should be "projected onto the axis-aligned plane where the areas of the 
		//       triangles are maximized" and I don't know maths well enough to do this better.
		//
		//       In theory, this should be very rare, so hopefully this can't have too much of
		//       an effect on performance.
		//
		r32 areaXY = Triangle_AreaAxisAligned(Tri1[0], Tri1[1], Tri1[2], Triangle_TargetAxis_XY)
			       + Triangle_AreaAxisAligned(Tri2[0], Tri2[1], Tri2[2], Triangle_TargetAxis_XY);
		r32 areaXZ = Triangle_AreaAxisAligned(Tri1[0], Tri1[1], Tri1[2], Triangle_TargetAxis_XZ)
			       + Triangle_AreaAxisAligned(Tri2[0], Tri2[1], Tri2[2], Triangle_TargetAxis_XZ);
		r32 areaYZ = Triangle_AreaAxisAligned(Tri1[0], Tri1[1], Tri1[2], Triangle_TargetAxis_YZ)
			       + Triangle_AreaAxisAligned(Tri2[0], Tri2[1], Tri2[2], Triangle_TargetAxis_YZ);

		r32 max = MAX3(areaXY, areaXZ, areaYZ);

		Vec2 tri1[3];
		Vec2 tri2[3];

		if(max == areaXY) {
			tri1[0] = Tri1[0].xy;
			tri1[1] = Tri1[1].xy;
			tri1[2] = Tri1[2].xy;

			tri2[0] = Tri2[0].xy;
			tri2[1] = Tri2[1].xy;
			tri2[2] = Tri2[2].xy;
		} else if(max == areaXZ) {
			tri1[0] = V2(Tri1[0].x, Tri1[0].z);
			tri1[1] = V2(Tri1[1].x, Tri1[0].z);
			tri1[2] = V2(Tri1[2].x, Tri1[0].z);

			tri2[0] = V2(Tri2[0].x, Tri2[0].z);
			tri2[1] = V2(Tri2[1].x, Tri2[0].z);
			tri2[2] = V2(Tri2[2].x, Tri2[0].z);
		} else if(max == areaYZ) {
			tri1[0] = V2(Tri1[0].y, Tri1[0].z);
			tri1[1] = V2(Tri1[1].y, Tri1[0].z);
			tri1[2] = V2(Tri1[2].y, Tri1[0].z);

			tri2[0] = V2(Tri2[0].y, Tri2[0].z);
			tri2[1] = V2(Tri2[1].y, Tri2[0].z);
			tri2[2] = V2(Tri2[2].y, Tri2[0].z);
		}

		// Thank you,
		// https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection#Given_two_Points_on_each_line
		// Do line-line intersections for every edge of the two triangles.
		for(u32 i = 0; i < 3; i++) {
			Vec2 a1 = tri1[i];
			Vec2 a2 = tri1[(i+1)%3];

			for(u32 j = 0; j < 3; j++) {
				Vec2 b1 = tri2[j];
				Vec2 b2 = tri2[(j+1)%3];

				r32 t = -((a1.x - b1.x)*(b1.y - b2.y) - (a1.y-b1.y)*(b1.x-b2.x))
					   / ((a1.x - a2.x)*(b1.y - b2.y) - (a1.y-a2.y)*(b1.x-b2.x));

				r32 u = -((a1.x - a2.x)*(a1.y - b1.y) - (a1.y-a2.y)*(a1.x-b1.x))
					   / ((a1.x - a2.x)*(b1.y - b2.y) - (a1.y-a2.y)*(b1.x-b2.x));

				if(InRange_R32(t, 0, 1) && InRange_R32(u, 0, 1))
					return (struct Intersection) { 
						.Occurred = 1,
						.Point = Vec3_Add(Tri1[i], Vec3_MultScal(Vec3_Sub(Tri1[(i+1)%3], Tri1[i]), t)) 
					};
			}
		}

		// If the line-line tests fail, check if one of the triangles is fully within the other.
		bool8 triInside = 1;
		for(u32 i = 0; i < 3; i++) {
			if(!Triangle_PointInside(tri1, tri2[i])) {
				triInside = 0;
				break;
			}
		}
		if(triInside)
			return (struct Intersection) {
				.Occurred = 1,
				.Point = Vec3_TriCenter(Tri2[0], Tri2[1], Tri2[2])
			};

		triInside = 1;
		for(u32 i = 0; i < 3; i++) {
			if(!Triangle_PointInside(tri2, tri1[i])) {
				triInside = 0;
				break;
			}
		}

		return (struct Intersection) {
			.Occurred = 1,
			.Point = Vec3_TriCenter(Tri1[0], Tri1[1], Tri1[2])
		};
	} else {
		// The triangles are not co-planar.
		Log(DEBUG, "Triangles are NOT co-planar.", "");

		// Thank you,
		// https://stackoverflow.com/a/32410473
		Vec3 D = Vec3_Cross(N1, N2);
		r32 det = Vec3_Len2(D);

		Vec3 O = Vec3_DivScal(Vec3_Add(
					Vec3_MultScal(Vec3_Cross(D, N2), d1),
					Vec3_MultScal(Vec3_Cross(N1, D), d2)), det);
		r32 pv1[3] = {
			Vec3_Dot(D, Vec3_Sub(Tri1[0], O)),
			Vec3_Dot(D, Vec3_Sub(Tri1[1], O)),
			Vec3_Dot(D, Vec3_Sub(Tri1[2], O))
		};
		r32 t1 = pv1[0] + (pv1[1] - pv1[0]) * (dist1[0] / (dist1[0] - dist1[1]));
		r32 t2 = pv1[1] + (pv1[2] - pv1[1]) * (dist1[1] / (dist1[1] - dist1[2]));

		r32 pv2[3] = {
			Vec3_Dot(D, Vec3_Sub(Tri2[0], O)),
			Vec3_Dot(D, Vec3_Sub(Tri2[1], O)),
			Vec3_Dot(D, Vec3_Sub(Tri2[2], O))
		};
		r32 t3 = pv2[0] + (pv2[1] - pv2[0]) * (dist2[0] / (dist2[0] - dist2[1]));
		r32 t4 = pv2[1] + (pv2[2] - pv2[1]) * (dist2[1] / (dist2[1] - dist2[2]));
		Log(DEBUG, "Ranges: t1 = %12.2f | t3 = %12.2f", t1, t3);
		Log(DEBUG, "        t2 = %12.2f | t4 = %12.2f", t2, t4);

		bool8 Occurred = RangesOverlap_R32(t1, t2, t3, t4);

		// TODO: Check the case where the collision Occurred again, it looks wrong...
		if(!Occurred)
			return (struct Intersection) { .Occurred = 0 };
		else
			return (struct Intersection) { .Occurred = 1, .Point = O };
	}
}

struct Intersection 
TriHull_Intersect(struct TriHull a, struct TriHull b)
{
	// TODO: Optimizations go here.
	
	for(u32 i = 0; i < a.NumTris; i++) {
		for(u32 j = 0; j < b.NumTris; j++) {
			Vec3 triA[3] = { a.TriPoints[i*3], a.TriPoints[i*3+1], a.TriPoints[i*3+2] };
			Vec3 triB[3] = { b.TriPoints[j*3], b.TriPoints[j*3+1], b.TriPoints[j*3+2] };

			if(a.Transform)
			{
				Mat4 m; Transform3D_Mat4(*a.Transform, m);
				for(u32 p = 0; p < 3; p++) triA[p] = Mat4_MultVec4(m, V4_V3(triA[p], 1)).xyz;
			}

			if(b.Transform)
			{
				Mat4 m; Transform3D_Mat4(*b.Transform, m);
				for(u32 p = 0; p < 3; p++) triB[p] = Mat4_MultVec4(m, V4_V3(triB[p], 1)).xyz;
			}

			struct Intersection res = TriTri_Intersect(triA, triB);

			if(res.Occurred) {
				Log(INFO, "Triangles #%d and #%d intersect at Point (%.2f, %.2f, %.2f)!", 
					      i, j, res.Point.x, res.Point.y, res.Point.z);
				return res;
			}
		}
	}

	return (struct Intersection) { .Occurred = 0 };
}

// Thank you,
// https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
static struct Intersection
TriRay_Intersect(Vec3 *tri, struct Ray ray)
{
	const r32 epsilon = 1e-8;

	Vec3 ab = Vec3_Sub(tri[1], tri[0]);
	Vec3 ac = Vec3_Sub(tri[2], tri[0]);

	Vec3 h = Vec3_Cross(ray.Dir, ac);
	r32 a = Vec3_Dot(ab, h);

	if(InRange_R32(a, -epsilon, epsilon))
		return (struct Intersection) { .Occurred = 0 };

	r32 f = 1.0/a;
	Vec3 s = Vec3_Sub(ray.Start, tri[0]);
	r32 u = f * Vec3_Dot(s, h);

	if(!InRange_R32(u, 0.0, 1.0))
		return (struct Intersection) { .Occurred = 0 };

	Vec3 q = Vec3_Cross(s, ab);
	r32 v = f * Vec3_Dot(ray.Dir, q);
	if(!InRange_R32(v, 0.0, 1.0 - u))
		return (struct Intersection) { .Occurred = 0 };

	r32 t = f * Vec3_Dot(ac, q);

	if(t < epsilon)
		return (struct Intersection) { .Occurred = 0 };

	return (struct Intersection) {
		.Occurred = 1,
		.Point = Vec3_Add(ray.Start, Vec3_MultScal(ray.Dir, t)) 
	};
}

struct Intersection 
TriHull_RayIntersect(struct TriHull hull, struct Ray ray)
{
	// TODO: Optimizations go here?
	
	for(u32 i = 0; i < hull.NumTris; i++) {
		Vec3 tri[3] = { hull.TriPoints[i*3], hull.TriPoints[i*3+1], hull.TriPoints[i*3+2] };

		if(hull.Transform)
		{
			Mat4 m; Transform3D_Mat4(*hull.Transform, m);
			for(u32 p = 0; p < 3; p++) tri[p] = Mat4_MultVec4(m, V4_V3(tri[p], 1)).xyz;
		}

		struct Intersection res = TriRay_Intersect(tri, ray);

		if(res.Occurred) {
			Log(INFO, "Triangle #%d and ray intersect at Point (%.2f, %.2f, %.2f)!", 
					i, res.Point.x, res.Point.y, res.Point.z);
			return res;
		}
	}

	return (struct Intersection) { .Occurred = 0 };
}
