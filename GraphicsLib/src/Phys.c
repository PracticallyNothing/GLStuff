#include "../Phys.h"

#include <float.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "../Common.h"
#include "../Math3D.h"

static const r32 epsilon = 1e-8;

/// Function for debugging.
static bool8 GetSign_R32(r32 s) { return signbit(s); }

/*
static bool8
Triangle_PointInside(const Vec3 tri[3], Vec3 point) {
	Vec3 normal = Vec3_Cross(Vec3_Sub(tri[1], tri[0]), Vec3_Sub(tri[2], tri[0]));
	r32 dist = Vec3_Dot(Vec3_Neg(normal), tri[0]);

	normal = Vec3_Norm(normal);

	Vec3 pointOnPlane = Vec3_MultScal(normal, dist);
	return fabs(Vec3_Dot(normal, Vec3_Sub(point, pointOnPlane))) <= epsilon;
}
*/

// Thank you,
// https://web.stanford.edu/class/cs277/resources/papers/Moller1997b.pdf
Intersection TriTri_Intersect(const Vec3 a[3], const Vec3 b[3]) {
	// Compute the normal and the distance for the plane equation of the plane triangle B sits on.
	Vec3 N2 = Vec3_Cross(Vec3_Sub(b[1], b[0]), Vec3_Sub(b[2], b[0]));
	r32 d2 = Vec3_Dot(Vec3_Neg(N2), b[0]);

	// Calculate the distances of the points of triangle A to the plane of triangle B.
	r32 dist1[3] = {0};
	for(u32 i = 0; i < 3; i++) {
		dist1[i] = Vec3_Dot(N2, a[i]) + d2;

		if(fabs(dist1[i]) < epsilon)
			dist1[i] = 0;
	}

	// If they're all non-zero and they all have the same sign, the triangles
	// don't intersect (since the points are on only 1 side of the plane).
	if(dist1[0] != 0 && dist1[1] != 0 && dist1[2] != 0 &&
	   TRIEQ(GetSign_R32(dist1[0]), GetSign_R32(dist1[1]), GetSign_R32(dist1[2])))
	{
		Log(DEBUG, "Early reject 1. dist1[%.2f, %.2f, %.2f]", dist1[0], dist1[1], dist1[2]);
		return (Intersection) { .Occurred = 0 };
	}

	Vec3 N1 = Vec3_Cross(Vec3_Sub(a[1], a[0]), Vec3_Sub(a[2], a[0]));
	r32 d1 = Vec3_Dot(Vec3_Neg(N1), a[0]);
	r32	dist2[3] = {0};

	for(u32 i = 0; i < 3; i++) {
		dist2[i] = Vec3_Dot(N1, b[i]) + d1;
		if(fabs(dist2[i]) < epsilon)
			dist2[i] = 0;
	}
#define NONE_ARE_ZERO(arr) arr[0] != 0 && arr[1] != 0 && arr[2] != 0

	if(NONE_ARE_ZERO(dist2) && TRIEQ(GetSign_R32(dist2[0]), GetSign_R32(dist2[1]), GetSign_R32(dist2[2])))
	{
		Log(DEBUG, "[Phys] Early reject 2. dist2[%.2f, %.2f, %.2f]", dist2[0], dist2[1], dist2[2]);
		return (Intersection) { .Occurred = 0 };
	}

	if(dist2[0] == 0 && TRIEQ(dist2[0], dist2[1], dist2[2])) {
		// The triangles are co-planar.
		Log(DEBUG, "[Phys] Triangles are co-planar.", "");
		
		//
		// TODO: This is most likely horrible code, but the document says that the triangles 
		//       should be "projected onto the axis-aligned plane where the areas of the 
		//       triangles are maximized" and I don't know maths well enough to do this better.
		//
		//       In theory, this should be very rare, so hopefully this can't have too much of
		//       an effect on performance.
		//
		r32 areaXY = Triangle_AreaAxisAligned(a[0], a[1], a[2], Triangle_TargetAxis_XY)
			       + Triangle_AreaAxisAligned(b[0], b[1], b[2], Triangle_TargetAxis_XY);
		r32 areaXZ = Triangle_AreaAxisAligned(a[0], a[1], a[2], Triangle_TargetAxis_XZ)
			       + Triangle_AreaAxisAligned(b[0], b[1], b[2], Triangle_TargetAxis_XZ);
		r32 areaYZ = Triangle_AreaAxisAligned(a[0], a[1], a[2], Triangle_TargetAxis_YZ)
			       + Triangle_AreaAxisAligned(b[0], b[1], b[2], Triangle_TargetAxis_YZ);

		r32 max = MAX3(areaXY, areaXZ, areaYZ);

		Vec2 tri1[3];
		Vec2 tri2[3];

		if(max == areaXY) {
			tri1[0] = a[0].xy;
			tri1[1] = a[1].xy;
			tri1[2] = a[2].xy;

			tri2[0] = b[0].xy;
			tri2[1] = b[1].xy;
			tri2[2] = b[2].xy;
		} else if(max == areaXZ) {
			tri1[0] = V2(a[0].x, a[0].z);
			tri1[1] = V2(a[1].x, a[0].z);
			tri1[2] = V2(a[2].x, a[0].z);

			tri2[0] = V2(b[0].x, b[0].z);
			tri2[1] = V2(b[1].x, b[0].z);
			tri2[2] = V2(b[2].x, b[0].z);
		} else if(max == areaYZ) {
			tri1[0] = V2(a[0].y, a[0].z);
			tri1[1] = V2(a[1].y, a[0].z);
			tri1[2] = V2(a[2].y, a[0].z);

			tri2[0] = V2(b[0].y, b[0].z);
			tri2[1] = V2(b[1].y, b[0].z);
			tri2[2] = V2(b[2].y, b[0].z);
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
					return (Intersection) { 
						.Occurred = 1,
						.Point = Vec3_Add(a[i], Vec3_MultScal(Vec3_Sub(a[(i+1)%3], a[i]), t)) 
					};
			}
		}

		// If the line-line tests fail, check if one of the triangles is fully within the other.
		bool8 triInside = 1;
		for(u32 i = 0; i < 3; i++) {
			//if(!Triangle_PointInside(tri1, tri2[i])) {
				triInside = 0;
				break;
			//}
		}
		if(triInside)
			return (Intersection) {
				.Occurred = 1,
				.Point = Vec3_TriCenter(b[0], b[1], b[2])
			};

		triInside = 1;
		for(u32 i = 0; i < 3; i++) {
			//if(!Triangle_PointInside(tri2, tri1[i])) {
				triInside = 0;
				break;
			//}
		}

		return (Intersection) {
			.Occurred = 1,
			.Point = Vec3_TriCenter(a[0], a[1], a[2])
		};
	} else {
		// The triangles are not co-planar.
		Log(DEBUG, "[Phys] Triangles are NOT co-planar.", "");

		// Thank you,
		// https://stackoverflow.com/a/32410473
		Vec3 D = Vec3_Cross(N1, N2);
		r32 det = Vec3_Len2(D);

		Vec3 O = Vec3_DivScal(Vec3_Add(
					Vec3_MultScal(Vec3_Cross(D, N2), d1),
					Vec3_MultScal(Vec3_Cross(N1, D), d2)), det);
		r32 pv1[3] = {
			Vec3_Dot(D, Vec3_Sub(a[0], O)),
			Vec3_Dot(D, Vec3_Sub(a[1], O)),
			Vec3_Dot(D, Vec3_Sub(a[2], O))
		};
		r32 t1 = pv1[0] + (pv1[1] - pv1[0]) * (dist1[0] / (dist1[0] - dist1[1]));
		r32 t2 = pv1[1] + (pv1[2] - pv1[1]) * (dist1[1] / (dist1[1] - dist1[2]));

		r32 pv2[3] = {
			Vec3_Dot(D, Vec3_Sub(b[0], O)),
			Vec3_Dot(D, Vec3_Sub(b[1], O)),
			Vec3_Dot(D, Vec3_Sub(b[2], O))
		};
		r32 t3 = pv2[0] + (pv2[1] - pv2[0]) * (dist2[0] / (dist2[0] - dist2[1]));
		r32 t4 = pv2[1] + (pv2[2] - pv2[1]) * (dist2[1] / (dist2[1] - dist2[2]));
		Log(DEBUG, "[Phys] Ranges: t1 = %12.2f | t3 = %12.2f", t1, t3);
		Log(DEBUG, "               t2 = %12.2f | t4 = %12.2f", t2, t4);

		bool8 Occurred = RangesOverlap_R32(t1, t2, t3, t4);

		// TODO: Check the case where the collision Occurred again, it looks wrong...
		if(!Occurred)
			return (Intersection) { .Occurred = 0 };
		else
			return (Intersection) { .Occurred = 1, .Point = O };
	}
}

Intersection 
TriHull_Intersect(TriHull a, TriHull b)
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

			Intersection res = TriTri_Intersect(triA, triB);

			if(res.Occurred) {
				Log(INFO, "[Phys] Triangles #%d and #%d intersect at Point (%.2f, %.2f, %.2f)!", 
					      i, j, res.Point.x, res.Point.y, res.Point.z);
				return res;
			}
		}
	}

	return (Intersection) { .Occurred = 0 };
}

// Thank you,
// https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
static Intersection
TriRay_Intersect(Vec3 *tri, Ray ray)
{
	const r32 epsilon = 1e-8;

	Vec3 ab = Vec3_Sub(tri[1], tri[0]);
	Vec3 ac = Vec3_Sub(tri[2], tri[0]);

	Vec3 h = Vec3_Cross(ray.Dir, ac);
	r32 a = Vec3_Dot(ab, h);

	if(InRange_R32(a, -epsilon, epsilon))
		return (Intersection) { .Occurred = 0 };

	r32 f = 1.0/a;
	Vec3 s = Vec3_Sub(ray.Start, tri[0]);
	r32 u = f * Vec3_Dot(s, h);

	if(!InRange_R32(u, 0.0, 1.0))
		return (Intersection) { .Occurred = 0 };

	Vec3 q = Vec3_Cross(s, ab);
	r32 v = f * Vec3_Dot(ray.Dir, q);
	if(!InRange_R32(v, 0.0, 1.0 - u))
		return (Intersection) { .Occurred = 0 };

	r32 t = f * Vec3_Dot(ac, q);

	if(t < epsilon)
		return (Intersection) { .Occurred = 0 };

	return (Intersection) {
		.Occurred = 1,
		.Point = Vec3_Add(ray.Start, Vec3_MultScal(ray.Dir, t)) 
	};
}

Intersection 
TriHull_RayIntersect(TriHull hull, Ray ray)
{
	// TODO: Optimizations go here?
	
	for(u32 i = 0; i < hull.NumTris; i++) {
		Vec3 tri[3] = { 
			hull.TriPoints[i * 3],
			hull.TriPoints[i * 3 + 1],
			hull.TriPoints[i * 3 + 2]
		};

		if(hull.Transform)
		{
			Mat4 m;
			Transform3D_Mat4(*hull.Transform, m);

			for(u32 p = 0; p < 3; p++)
				tri[p] = Mat4_MultVec4(m, V4_V3(tri[p], 1)).xyz;
		}

		Intersection res = TriRay_Intersect(tri, ray);

		if(res.Occurred) {
			Log(INFO, "[Phys] Triangle #%d and ray intersect at Point (%.2f, %.2f, %.2f)!", 
				i, res.Point.x, res.Point.y, res.Point.z);
			return res;
		}
	}

	return (Intersection) { .Occurred = 0 };
}

void PhysWorld_Init(PhysWorld* world)
{
	world->Gravity = 9.8;
	world->Objects = (Array_PhysObject) {0};
}

// TODO: Implement.
PhysObject*
PhysWorld_RayCollide(const PhysWorld* world, Ray ray)
{
	// 2. 
	return NULL;
}

// TODO: Implement
void PhysWorld_Update(PhysWorld* world, r32 dt)
{
	// 1. Split world into octree.
	for(u32 i = 0; i < world->Objects.Size; i++) {
	}
	// 2. Do broadphase collision, mark any potential hits for narrowphase.
	// 3. Do narrowphase collision, actually see if objects collide.
	// 4. Apply forces based on delta time
}
