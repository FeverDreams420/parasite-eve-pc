#ifndef SCENE_E08_PARTICLE_SLOTS_H
#define SCENE_E08_PARTICLE_SLOTS_H

#include "common.h"
#include "pe1/gte_types.h"

typedef struct SceneParticleOffset {
    s16 x;
    s16 y;
    s16 z;
    s16 pad06;
} SceneParticleOffset;

typedef struct SceneParticleSlots {
    u8 pad00[8];
    GteShortVector offset[4];
    s32 phase[4];
    s16 verticalOffset[4];
    u8 pad40[8];
    s16 timer[4];
    s16 cycle[4];
    s16 motionRamp;
    s16 elapsedFrames;
} SceneParticleSlots;

typedef struct SceneEmitterSlots {
    SceneParticleOffset position[4];
    s16 field20[4];
    s16 field28[4];
    u8 field30[4];
    u8 field34[4];
    u8 field38[4];
    u8 field3C[4];
    u8 field40[4];
    u8 activeSlots;
} SceneEmitterSlots;

typedef struct SceneMovingVector {
    s16 x;
    s16 y;
    s16 z;
    s16 pad06;
    s16 combinedX;
    s16 combinedY;
    s16 combinedZ;
    s16 pad0E;
    s16 dx;
    s16 dy;
    s16 dz;
    s16 pad16;
} SceneMovingVector;

typedef struct SceneMovingSlots {
    SceneMovingVector vector[16];
    s32 life[16];
    s16 active[16];
    s16 timer[16];
    s16 initialSize[16];
    s16 angle[16];
    s16 range[16];
    s16 radius[16];
    s16 extraRadius[16];
} SceneMovingSlots;

/* One moving slot's life counter, seen from a cursor that steps a word per
 * slot through the start of the work area. */
typedef struct SceneMovingWordLane {
    u8 pad000[0x180];
    s32 life;
} SceneMovingWordLane;

/* One moving slot's halfword fields, seen from a cursor that steps a
 * halfword per slot through the start of the work area. */
typedef struct SceneMovingHalfLane {
    u8 pad000[0x1C0];
    s16 active;
    u8 pad1C2[0x3E];
    s16 initialSize;
    u8 pad202[0x3E];
    s16 range;
} SceneMovingHalfLane;

PE1_STATIC_ASSERT(PE1_OFFSETOF(SceneMovingSlots, life) == 0x180,
                  scene_moving_life_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(SceneMovingSlots, active) == 0x1C0,
                  scene_moving_active_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(SceneMovingSlots, initialSize) == 0x200,
                  scene_moving_initial_size_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(SceneMovingSlots, range) == 0x240,
                  scene_moving_range_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(SceneMovingHalfLane, range) == 0x240,
                  scene_moving_half_lane_range_offset);

typedef struct SceneE08EffectState {
    u8 pad00;
    u8 status;
    s16 elapsedFrames;
} SceneE08EffectState;

typedef SceneE08EffectState SceneMovingState;

PE1_STATIC_ASSERT(PE1_OFFSETOF(SceneE08EffectState, status) == 1,
                  scene_e08_effect_state_status_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(SceneE08EffectState, elapsedFrames) == 2,
                  scene_e08_effect_state_elapsed_frames_offset);
PE1_STATIC_ASSERT(sizeof(SceneE08EffectState) == 4,
                  scene_e08_effect_state_size);

typedef struct SceneParticleCluster {
    SceneParticleOffset position[30];
    s16 scale[30];
    s16 angle[30];
    s16 brightness[30];
    u8 texture[30];
    u8 mode[30];
    u8 counter[30];
    u8 field1FE[30];
    u8 active[30];
    u8 timer[30];
    u8 count;
} SceneParticleCluster;

#endif
