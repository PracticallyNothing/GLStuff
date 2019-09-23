#ifndef QUATERNION_H
#define QUATERNION_H

#include "Common.h"
#include "Vector.h"

typedef Vec4 Quat;

extern Quat Quat_Identity();
extern Quat Quat_RotAxis(Vec3 axis, r32 amt);
extern Quat Quat_Mix(Quat a, Quat b, r32 amt);
extern Quat Quat_Lerp(Quat a, Quat b, r32 amt);
extern Quat Quat_Slerp(Quat a, Quat b, r32 amt);
extern Quat Quat_Conjugate(Quat q);
extern r32 Quat_Dot(Quat a, Quat b);
extern Quat Quat_Mul(Quat a, Quat b);
extern Quat Quat_Norm(Quat q);

#endif
