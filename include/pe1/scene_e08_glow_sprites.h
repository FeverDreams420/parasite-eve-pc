#ifndef PE1_SCENE_E08_GLOW_SPRITES_H
#define PE1_SCENE_E08_GLOW_SPRITES_H

#include "pe1/gte_short_vector.h"

typedef struct SceneE08GlowSprites {
    GteShortVector position[4]; /* 0x00 */
    s16 scale[4];               /* 0x20 */
    u16 depth[4];               /* 0x28 */
    u8 cell[4];                 /* 0x30 */
    u8 clut[4];                 /* 0x34 */
    u8 frame[4];                /* 0x38: two frames per cell */
    u8 active[4];               /* 0x3C */
} SceneE08GlowSprites;

PE1_STATIC_ASSERT(PE1_OFFSETOF(SceneE08GlowSprites, scale) == 0x20, scene_e08_glow_scale_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(SceneE08GlowSprites, active) == 0x3C, scene_e08_glow_active_offset);
PE1_STATIC_ASSERT(sizeof(SceneE08GlowSprites) == 0x40, scene_e08_glow_size);

#endif
