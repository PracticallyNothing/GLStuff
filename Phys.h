#ifndef PHYS_H
#define PHYS_H

#include "Common.h"
#include "Math3D.h"
#include "Transform.h"

struct Ray { Vec3 Start, Dir; };

struct AABB { Vec3 Min, Max; };

extern bool8       AABB_Intersect        (struct AABB a, struct AABB b);
extern struct AABB AABB_Fix              (struct AABB aabb);
extern struct AABB AABB_ApplyTransform3D (struct AABB aabb, Transform3D t);

struct TriHull { Vec3 *TriPoints; u32 NumTris; };

extern Vec3 TriHull_RayIntersect  (struct TriHull hull, struct Ray ray);
extern Vec3 TriHull_Intersect (struct TriHull a, struct TriHull b, Transform3D t);

#endif
