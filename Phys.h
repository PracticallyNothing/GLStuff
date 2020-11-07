#ifndef PHYS_H
#define PHYS_H

#include "Common.h"
#include "Math3D.h"

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

extern bool8       AABB_Intersect        (struct AABB a, struct AABB b);
extern struct AABB AABB_Fix              (struct AABB aabb);
extern struct AABB AABB_ApplyTransform3D (struct AABB aabb, Transform3D t);

struct TriHull { 
	Vec3 *TriPoints;
	u32 NumTris;
	Transform3D *Transform;
};

extern struct Intersection TriHull_RayIntersect(struct TriHull hull, struct Ray ray);
extern struct Intersection TriHull_Intersect(struct TriHull a, struct TriHull b);

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

	struct AABB    AABB;
	struct TriHull Hull;
};

DEF_ARRAY(PhysObject, struct PhysObject);

struct PhysWorld {
	r32 Gravity;

	struct Array_PhysObject Objects;

	struct PhysWorld_Cache *_cache;
};

extern void               PhysWorld_Init(struct PhysWorld* world);
extern struct PhysObject* PhysWorld_RayCollide(const struct PhysWorld* world, struct Ray ray);
extern void               PhysWorld_Update(struct PhysWorld* world, r32 dt);

#endif
