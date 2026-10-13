#ifndef PE1_FIELD_VECTOR_MATH_H
#define PE1_FIELD_VECTOR_MATH_H

#include "common.h"

/* World position in 16.16 fixed point; the planar helpers read x and z. */
typedef struct Vec3Fixed {
    s32 x;
    s32 y;
    s32 z;
} Vec3Fixed;

s32 FieldFx_GetFixedDistance3D(Vec3Fixed *lhs, Vec3Fixed *rhs);
/* Planar (x/z) distance between two positions, in whole units. */
s32 FieldFx_GetDistanceXZ(Vec3Fixed *lhs, Vec3Fixed *rhs);
void FieldFx_FixedVecToLookAngles(Vec3Fixed *from, Vec3Fixed *to, s16 *out);
/* Heading from one position to another: ratan2(dx, dz) as a signed
 * halfword. */
int FieldFx_VecToAngle(Vec3Fixed *from, Vec3Fixed *to);
/* Turns a 12-bit angle toward target by at most step. */
int FieldFx_TurnToward(int current, int target, int step);
/* Absolute difference of two 12-bit angles, folded into 0..0x800. */
s32 FieldFx_AngleDistance(s32 lhs, s32 rhs);

#endif /* PE1_FIELD_VECTOR_MATH_H */
