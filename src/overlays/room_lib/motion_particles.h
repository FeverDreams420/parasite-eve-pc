/* Private declarations of the motion particle set, shared by its three
 * units (motion_particles.c, motion_particles_2.c, motion_particles_3.c). */
#ifndef ROOM_LIB_MOTION_PARTICLES_H
#define ROOM_LIB_MOTION_PARTICLES_H

#include "room_lib.h"

typedef struct RoomMotionContext {
    RoomLink *link;
    u8 pad04[0x68];
    u8 owner;
} RoomMotionContext;

typedef struct RoomMotionTransform {
    u8 pad00[0x120];
    RoomSpriteMatrix primary;
    u8 pad140[0x60];
    RoomSpriteMatrix secondary;
} RoomMotionTransform;

typedef struct RoomMotionLookup {
    s16 id;
    s16 pad;
} RoomMotionLookup;

typedef struct RoomMotionPoint {
    s16 primaryX;
    s16 primaryY;
    s16 primaryZ;
    s16 pad46;
    s16 secondaryX;
    s16 secondaryY;
    s16 secondaryZ;
    s16 pad4E;
} RoomMotionPoint;

typedef struct RoomMotionScale {
    s16 x;
    s16 y;
} RoomMotionScale;

typedef struct RoomMotionState {
    RoomSpriteMatrix matrix[2];
    RoomMotionPoint particle[8];
    RoomMotionPoint velocity[8];
    RoomMotionScale scale[8];
    RoomMotionLookup lookup[8];
    u16 depth;
    s16 count;
} RoomMotionState;

typedef struct RoomMotionCursor {
    u8 pad00[0x40];
    RoomMotionPoint particle;
} RoomMotionCursor;

PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomMotionContext, owner) == 0x6C,
                  room_motion_context_owner_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomMotionTransform, secondary) == 0x1A0,
                  room_motion_secondary_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomMotionState, velocity) == 0xC0,
                  room_motion_velocity_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomMotionState, scale) == 0x140,
                  room_motion_scale_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomMotionState, lookup) == 0x160,
                  room_motion_lookup_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomMotionState, depth) == 0x180,
                  room_motion_depth_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomMotionCursor, particle.secondaryZ) == 0x4C,
                  room_motion_cursor_secondary_z_offset);

/* The battle actor word (D_8009D254) as the update pair reads it. */
typedef struct RoomSharedGlobalState {
    char pad0[0x2A];
    s16 field2A;
} RoomSharedGlobalState;

PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomSharedGlobalState, field2A) == 0x2A,
                  room_shared_global_state_field2a_offset);
PE1_STATIC_ASSERT(sizeof(RoomSharedGlobalState) == 0x2C,
                  room_shared_global_state_size);

/* Field engine calls of the motion particle set. */
void FieldFx_BuildRingGeometry(void *arg0);
void FieldFx_DrawTexturedStrip(void *arg0);
int FieldFx_UpdateStripCollision(void *obj, s16 *values, int *result);

#endif
