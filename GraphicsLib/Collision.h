#ifndef COLLISION_H
#define COLLISION_H

#include "Common.h"
#include "Math3D.h"

typedef struct Plane Plane;
typedef struct Ray Ray;
typedef struct OBB OBB;
typedef struct AABB AABB;
typedef struct TriHull TriHull;

//
// Plane
//

struct Plane {
	Vec3 Normal;
	r32 Distance;
};

enum Plane_HalfSpace {
	Plane_HalfSpace_Front,
	Plane_HalfSpace_On,
	Plane_HalfSpace_Back,
};

enum Plane_HalfSpace Plane_HalfSpaceTest(Plane plane, Vec3 point);

//
// AABB + OBB
//

// Axis-aligned bounding box
struct AABB {
	Vec3 Min;
	Vec3 Max;
};

bool8 AABB_Intersect(AABB a, AABB b);
AABB AABB_Fix(AABB aabb);

// Create a fitting AABB around a transformed one.
AABB AABB_ApplyTransform3D(AABB aabb, Transform3D t);

// Generate a sum AABB around two others.
AABB AABB_Add(AABB a, AABB b);

// Oriented bounding box
struct OBB {
	AABB aabb;
	Transform3D* transform;
};

// Check if two oriented bounding boxes intersect.
bool8 OBB_Intersect(OBB a, OBB b);

// Check if an OBB intersects with an AABB.
bool8 OBB_AABB_Intersect(OBB a, AABB b);

#endif
