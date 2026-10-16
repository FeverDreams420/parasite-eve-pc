/* Private declarations of the motion particle set, shared by its three
 * units (motion_particles.c, motion_particles_2.c, motion_particles_3.c). */
#ifndef ROOM_LIB_MOTION_PARTICLES_H
#define ROOM_LIB_MOTION_PARTICLES_H

#include "room_lib.h"

typedef RoomMotionInitState RoomMotionContext;


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


PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomMotionContext, allocation) == sizeof(void *) + 0x68,
                  room_motion_context_asset_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomMotionState, velocity) == 0xC0,
                  room_motion_velocity_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomMotionState, scale) == 0x140,
                  room_motion_scale_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomMotionState, lookup) == 0x160,
                  room_motion_lookup_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomMotionState, depth) == 0x180,
                  room_motion_depth_offset);


/* Field engine calls of the motion particle set. */

void FieldFx_DrawTexturedStrip(FieldTexturedStrip *arg0);
int FieldFx_UpdateStripCollision(char *obj, void *values, int *result);

typedef struct RoomAnchoredSpriteState {
    u8 pad00[0x10];
    s16 scale;
    u16 depth;
    u8 sourceData[0x100];
    FieldRingGeometry ring;
} RoomAnchoredSpriteState;

typedef struct RoomParticleRenderPairState {
    u8 pad00[0x78];
    FieldChainRecord first, second;
} RoomParticleRenderPairState;

int RoomEffect_MotionParticlesInit(void *,void *,void *);
int RoomEffect_MotionParticlesNop(void *,void *,void *);
int RoomEffect_MotionParticlesTick(void *,void *,void *);
int RoomEffect_MotionParticlesAnchor(void *,void *,void *);
int RoomEffect_DrawAnchoredSpriteQuad(void *,void *,void *);
int RoomEffect_UpdateTimedFade(void *,void *,void *);
int RoomEffect_InitSpritePair(void *,void *,void *);
int RoomEffect_DrawAnchoredSpritePair(void *,void *,void *);
int RoomEffect_StepSpritePair(void *,void *,void *);
int RoomEffect_SeedMotionParticles(void *,void *,void *);
int RoomEffect_DrawMotionParticles(void *,void *,void *);
int RoomEffect_UpdateParticleMotion(void *,void *,void *);
int RoomEffect_InitParticleRenderPair(void *,void *,void *);
int RoomEffect_DrawParticleRenderPair(void *,void *,void *);
int RoomEffect_UpdateParticleRenderPair(void *,void *,void *);
#endif
