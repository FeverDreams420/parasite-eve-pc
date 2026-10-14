#ifndef PE1_FIXED_POINT_H
#define PE1_FIXED_POINT_H

#include "common.h"

/* Signed 16.16 world coordinates, with the little-endian halfword view
 * used by the geometry and actor-rendering code. */
typedef union Pe1Fixed16_16 {
    s32 fixed;
    struct { u16 frac; s16 integer; } parts;
} Pe1Fixed16_16;

/* FieldActor retains its scalar coordinate interface. Read the same
 * signed high halfword without converting the full fixed-point word. */
static inline s16 Pe1Fixed_Integer(const s32 *value)
{
    return ((const Pe1Fixed16_16 *)value)->parts.integer;
}

/* 16.16 product: the middle 32 bits of the signed 64-bit a * b
 * (src/main/math/fixed_mul.c). */
int Math_FixedMul(int a, int b);

/* The same integer half read through a plain halfword pointer. GCC 2.7
 * marks a member access as an aggregate memory reference, which lets the
 * scheduler move it across unrelated global stores; some callers read the
 * halfword as an ordinary scalar, which keeps it ordered against them. */
static inline s16 Pe1Fixed_IntegerHalf(const Pe1Fixed16_16 *value)
{
    const s16 *integer = &value->parts.integer;

    return *integer;
}

PE1_STATIC_ASSERT(sizeof(Pe1Fixed16_16) == 4, fixed_16_16_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(Pe1Fixed16_16, parts.integer) == 2,
                  fixed_16_16_integer_offset);

int Math_FixedDivide(int a, int b);
int Math_FixedRoundToInt(int value);
int Math_FixedRoundToByte(int value);
unsigned int Math_SqrtApprox3(int x, int y, int z);

#endif
