#ifndef PHYS_H
#define PHYS_H

#include "Common.h"
#include "Math3D.h"

typedef struct Intersection Intersection;
typedef struct Ray          Ray;
typedef struct AABB         AABB;
typedef struct TriHull      TriHull;

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

extern bool8 AABB_Intersect        (AABB a, AABB b);
extern AABB  AABB_Fix              (AABB aabb);
extern AABB  AABB_ApplyTransform3D (AABB aabb, Transform3D t);

struct TriHull { 
	Vec3 *TriPoints;
	u32 NumTris;
	Transform3D *Transform;
};

extern Intersection TriHull_RayIntersect(TriHull hull, Ray ray);
extern Intersection TriHull_Intersect(TriHull a, TriHull b);

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

extern void        PhysWorld_Init(PhysWorld* world);
extern PhysObject* PhysWorld_RayCollide(const PhysWorld* world, Ray ray);
extern void        PhysWorld_Update(PhysWorld* world, r32 dt);

#endif
