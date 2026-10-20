#ifndef PE1_SCENE_RING_SPRITES_H
#define PE1_SCENE_RING_SPRITES_H

#include "common.h"
#include "pe1/field_glow_sprite.h"
#include "pe1/field_ring_geometry.h"
#include "pe1/field_script_context.h"
#include "pe1/field_sprite_state.h"
#include "pe1/gte_types.h"
#include "pe1/psyq_libc.h"

/* Scene e08 ring sprites: an animated flash sprite (first 12 ticks), a
 * steady glow sprite, and two shaded rings scaled by the state's size. */
typedef struct SceneRingSpritesOwner {
    /* 0x00 */ u8 pad00[sizeof(void *) + 0x20];
    /* 0x24 */ u8 mode;
} SceneRingSpritesOwner;

typedef struct SceneRingSpritesState {
    /* 0x00 */ s16 x;
    /* 0x02 */ s16 y;
    /* 0x04 */ s16 z;
    /* 0x06 */ u8 pad06[2];
    /* 0x08 */ s16 size;
    /* 0x0A */ s16 depth;
    /* 0x0C */ u8 ticks;
    /* 0x0D */ u8 reserved0D;  /* set to 156 by the init, never read */
} SceneRingSpritesState;

/* Shaded ring parameter block drawn by FieldFx_DrawShadedRing. */
typedef FieldRingGeometry SceneRingShade;

extern GteRotation g_SceneE08RingSpritesRotation;
extern GteVector g_SceneE08RingSpritesFlashScale;
extern GteVector g_SceneE08RingSpritesGlowScale;
extern FieldGlowSprite g_SceneE08RingSpritesFlashSprite;
extern FieldGlowSprite g_SceneE08RingSpritesGlowSprite;
extern SceneRingShade g_SceneE08RingSpritesRings[2];

#endif
