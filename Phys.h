#ifndef PHYS_H
#define PHYS_H

#include "Common.h"
#include "Math3D.h"
#include "Transform.h"

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

#endif
