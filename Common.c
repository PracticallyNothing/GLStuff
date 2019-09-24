#include "Common.h"

const r64 Tau = 6.28318530717958647692;
const r64 Pi = 3.14159265358979323846;
const r64 Pi_Half = 1.57079632679489661923;
const r64 Pi_Quarter = 0.78539816339744830962;

r64 DegToRad(r64 degrees) { return degrees * Pi / 180.0; }
r64 RadToDeg(r64 radians) { return radians * 180.0 / Pi; }
