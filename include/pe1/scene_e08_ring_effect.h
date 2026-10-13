#ifndef PE1_SCENE_E08_RING_EFFECT_H
#define PE1_SCENE_E08_RING_EFFECT_H

#include "common.h"

/* Work area shared by the three-ring init and two-ring draw/update callbacks. */
typedef struct SceneE08RingEffectState {
    u8 reserved00[8];
    s16 rotation[3];
    s16 reserved0E;
    s16 firstScale;
    s16 secondScale;
} SceneE08RingEffectState;

PE1_STATIC_ASSERT(PE1_OFFSETOF(SceneE08RingEffectState, rotation) == 8, scene_e08_ring_rotation_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(SceneE08RingEffectState, firstScale) == 0x10, scene_e08_ring_first_scale_offset);
PE1_STATIC_ASSERT(sizeof(SceneE08RingEffectState) == 0x14, scene_e08_ring_state_size);

#endif
