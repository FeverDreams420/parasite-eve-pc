#include "common.h"
#include "pe1/field_actor.h"
#include "pe1/field_vector_math.h"
#include "pe1/fieldfx_task.h"
#include "pe1/gte_types.h"
#include "pe1/vector_types.h"

typedef struct {
    char pad0[0x8];
    void *target;
    char padC[0x8];
    s16 counter;
    char pad16[0x2];
    s8 value;
} Func800DFB20State;

typedef struct {
    char pad0[0xF];
    u8 limit;
    char pad10[0x4];
    int value;
    char pad18[0x2];
    u16 cursor;
} Func800DFB20Target;

void func_800DFB20(Func800DFB20State *state)
{
    Func800DFB20Target *target = state->target;

    if (state->counter != 0 && target->cursor >= target->limit) {
        target->value = state->value;
        if (state->counter != -1) {
            state->counter--;
        }
    }
}

/* Wait condition for a field object: nonzero while the object is still busy.
 * A pending one-shot flag (0x80000 or 0x40000 at +0x98, selected by the
 * state bits) is consumed and reported once; otherwise the object waits while
 * its child is mid-animation. */
int FieldFx_IsOwnerBusy(FieldFxTaskOwner *arg0)
{
  FieldFxTaskContext *state;
  FieldActor *obj;
  u32 flags;
  u32 masked;
  FieldActorState *child;
  u32 word;
  u32 byte;
  int phase;
  byte = arg0->tasks.flags;
  obj = arg0->prefix.actor;
  state = &arg0->tasks;
  word = 4;
  if ((byte & word) && (obj->mode < 2))
  {
    return 1;
  }
  if (state->flags & 2)
  {
    flags = obj->flags;
    if (flags & 0x80000)
    {
      masked = flags & 0xFFF7FFFF;
      obj->flags = masked;
      return 1;
    }
  }
  if (state->flags & 1)
  {
    flags = obj->flags;
    if (flags & 0x40000)
    {
      masked = flags & 0xFFFBFFFF;
 do { obj->flags = masked; return 1; } while (0);
    }
  }
  child = obj->state;
  if (child != 0)
  {
    word = child->core_flags;
    if (word & 0x1800)
    {
      return 1;
    }
    phase = (word >> 1) & 7;
    if (phase > 0)
    {
      return 1;
    }
  }
  return 0;
}

int FieldFx_IntSqrt(int value) {
    int result = 0;
    int shift = 0x1E;

    do {
        int threshold = ((result << 2) + 1) << shift;

        result <<= 1;
        if (value >= threshold) {
            value -= threshold;
            result |= 1;
        }
        shift -= 2;
    } while (shift >= 0);

    return result;
}

s32 FieldFx_GetFixedDistance3D(Vec3Fixed *lhs, Vec3Fixed *rhs) {
    s32 dx = (lhs->x - rhs->x) >> 16;
    s32 dx_sq = dx * dx;
    s32 dz = (lhs->z - rhs->z) >> 16;
    s32 dz_sq = dz * dz;
    s32 horizontal;
    s32 dy;
    s32 rhs_y;
    s32 lhs_y;
    s32 distance_sq;
    s32 shift;
    s32 threshold;

    horizontal = 0;
    rhs_y = rhs->y;
    lhs_y = lhs->y;
    shift = 0x1E;
    dy = (lhs_y - rhs_y) >> 16;
    distance_sq = dx_sq + dz_sq;
    do {
        threshold = ((horizontal << 2) + 1) << shift;

        horizontal <<= 1;
        if (distance_sq >= threshold) {
            distance_sq -= threshold;
            horizontal |= 1;
        }
        shift -= 2;
    } while (shift >= 0);

    {
        s32 dy_sq;
        s32 horizontal_sq;
        s32 result;
        s32 shift2;
        s32 distance_sq2;

        dy_sq = dy * dy;
        horizontal_sq = horizontal * horizontal;
        distance_sq2 = dy_sq + horizontal_sq;
        result = 0;
        shift2 = 0x1E;
        do {
            threshold = ((result << 2) + 1) << shift2;

            result <<= 1;
            if (distance_sq2 >= threshold) {
                distance_sq2 -= threshold;
                result |= 1;
            }
            shift2 -= 2;
        } while (shift2 >= 0);

        return result;
    }
}

s32 FieldFx_GetShortDistance3D(Pe1Vec3s *lhs, Pe1Vec3s *rhs)
{
  s32 lhs_x = lhs->x;
  s32 rhs_x = rhs->x;
  s32 dx = lhs_x - rhs_x;
  s32 dx_sq = dx * dx;
  s32 lhs_z = lhs->z;
  s32 rhs_z = rhs->z;
  s32 dz = lhs_z - rhs_z;
  s32 dz_sq = dz * dz;
  s32 horizontal;
  short rhs_y;
  s32 lhs_y;
  s32 dy;
  s32 distance_sq;
  s32 shift;
  s32 threshold;
  horizontal = 0;
  rhs_y = rhs->y;
  lhs_y = lhs->y;
  shift = 0x1E;
  dy = lhs_y - rhs_y;
  distance_sq = dx_sq + dz_sq;
  do
  {
    threshold = ((horizontal << 2) + 1) << shift;
    horizontal <<= 1;
    if (distance_sq >= threshold)
    {
      distance_sq -= threshold;
      horizontal |= 1;
    }
    shift -= 2;
  }
  while (shift >= 0);
  {
    s32 dy_sq;
    s32 horizontal_sq;
    s32 result;
    s32 shift2;
    s32 distance_sq2;
    dy_sq = dy * dy;
    horizontal_sq = horizontal * horizontal;
    distance_sq2 = dy_sq + horizontal_sq;
    result = 0;
    shift2 = 0x1E;
    do
    {
      threshold = ((result << 2) + 1) << shift2;
      result <<= 1;
      if (distance_sq2 >= threshold)
      {
        distance_sq2 -= threshold;
        result |= 1;
      }
      shift2 -= 2;
    }
    while (shift2 >= 0);
    return result;
  }
}

s32 FieldFx_GetDistanceXZ(Vec3Fixed *lhs, Vec3Fixed *rhs) {
    s32 dx = (lhs->x - rhs->x) >> 16;
    s32 dx_sq = dx * dx;
    s32 dz = (lhs->z - rhs->z) >> 16;
    s32 dz_sq = dz * dz;
    s32 result;
    s32 shift;
    s32 distance_sq;
    s32 threshold;

    result = 0;
    shift = 0x1E;
    distance_sq = dx_sq + dz_sq;
    do {
        threshold = ((result << 2) + 1) << shift;

        result <<= 1;
        if (distance_sq >= threshold) {
            distance_sq -= threshold;
            result |= 1;
        }
        shift -= 2;
    } while (shift >= 0);

    return result;
}

void FieldFx_FixedVecToLookAngles(Vec3Fixed *from, Vec3Fixed *to, s16 *out) {
    Vec3Fixed *from_reg = from;
    Vec3Fixed *to_reg = to;
    s32 dx = (from_reg->x - to_reg->x) >> 16;
    s32 dx_sq = dx * dx;
    s32 dz = (from_reg->z - to_reg->z) >> 16;
    s32 dz_sq = dz * dz;
    s32 horizontal;
    s32 shift;
    s32 distance_sq;
    s32 threshold;
    s32 angle;

    horizontal = 0;
    shift = 0x1E;
    distance_sq = dx_sq + dz_sq;
    do {
        threshold = ((horizontal << 2) + 1) << shift;

        horizontal <<= 1;
        if (distance_sq >= threshold) {
            distance_sq -= threshold;
            horizontal |= 1;
        }
        shift -= 2;
    } while (shift >= 0);

    angle = ratan2(from_reg->y - to_reg->y, horizontal << 16);
    out[0] = angle & 0xFFF;

    angle = ratan2(to_reg->x - from_reg->x, to_reg->z - from_reg->z);
    out[1] = angle & 0xFFF;
    out[2] = 0;
}

int FieldFx_VecToAngle(Vec3Fixed *from, Vec3Fixed *to) {
    return (s16)ratan2(to->x - from->x, to->z - from->z);
}
