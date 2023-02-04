#ifndef PHYS_H
#define PHYS_H

#include "Common.h"
#include "Math3D.h"

typedef struct Plane        Plane;
typedef struct Intersection Intersection;
typedef struct Ray          Ray;
typedef struct AABB         AABB;
typedef struct TriHull      TriHull;

struct Plane {
	Vec3 Normal;

	bool8 IsPointPlane; // True means Point is set, false means Distance is set.

	union {
		Vec3 Point;   // Point on the plane.
		r32 Distance; // Distance from origin
	};
};

Plane Plane_ChangeType(Plane p); // Change the type of the plane according to IsPointPlane.

enum HSRes {
	HS_Front, // The point is in front of the plane
	HS_On,    // The point is on the plane
	HS_Back   // The point is behind the plane
};

// Test if point is part of a plane formed from a normal and a point on it.
enum HSRes HalfSpaceTest(Vec3 planeNormal, Vec3 planePoint, Vec3 point);

struct Intersection {
	bool8 Occurred;
	Vec3 Point;
};

struct Ray {
	Vec3 Start;
	Vec3 Dir;
};

struct AABB { 
	Vec3 Min;
	Vec3 Max;
};

bool8 AABB_Intersect        (AABB a, AABB b);
AABB  AABB_Fix              (AABB aabb);
AABB  AABB_ApplyTransform3D (AABB aabb, Transform3D t);
AABB  AABB_Add              (AABB a, AABB b); // Generate a sum AABB around two others.

struct TriHull { 
	Vec3 *TriPoints;
	u32 NumTris;
	Transform3D *Transform;
};

Intersection TriTri_Intersect(const Vec3 a[3], const Vec3 b[3]);
Intersection TriHull_RayIntersect(TriHull hull, Ray ray);
Intersection TriHull_Intersect(TriHull a, TriHull b);

typedef struct PhysObject      PhysObject;
typedef struct PhysWorld       PhysWorld;
typedef struct PhysWorld_Cache PhysWorld_Cache;

struct PhysObject {
	enum {
		/// Static scenery objects, only used to collide against.
		PhysObject_Static,

		/// Any non-static objects standing on this
		/// platform will be dragged along with it.
		PhysObject_MovingPlaftorm,

		/// 
		PhysObject_RigidBody,
	} Type;

	Vec3 Velocity;

	r32 Mass;
	
	// Float from 0.0 to 1.0 of how much an object 
	// will bounce when colliding with another.
	r32 Bounciness;

	Transform3D Transform;

	AABB    AABB;
	TriHull Hull;
};

DEF_ARRAY(PhysObject, PhysObject);

struct PhysWorld {
	r32 Gravity;

	Array_PhysObject Objects;

	PhysWorld_Cache *_cache;
};

void        PhysWorld_Init(PhysWorld* world);
PhysObject* PhysWorld_RayCollide(const PhysWorld* world, Ray ray);
void        PhysWorld_Update(PhysWorld* world, r32 dt);

#endif
