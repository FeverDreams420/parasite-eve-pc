#ifndef PE1_FIELD_GLOW_SPRITE_H
#define PE1_FIELD_GLOW_SPRITE_H

#include "common.h"
#include "pe1/gte_types.h"

/* Sprite parameter block (0x10 bytes) drawn by FieldFx_DrawGlowSprite: colour,
 * texture cell (16x16 grid), CLUT slot, flip flags (1 = U, 2 = V), depth
 * offset and the brightness the colour is scaled by. */
typedef struct FieldGlowSprite {
    /* 0x00 */ u8 r;
    /* 0x01 */ u8 g;
    /* 0x02 */ u8 b;
    /* 0x03 */ u8 pad03;
    /* 0x04 */ u8 cell;
    /* 0x05 */ u8 clut;
    /* 0x06 */ u8 flip;
    /* 0x07 */ u8 pad07;
    /* 0x08 */ s16 offset;
    /* 0x0A */ u16 depth;
    /* 0x0C */ u8 pad0C[4];
} FieldGlowSprite;

/* Shared sprite presets initialized by GlowEffect_PlantScript. */
extern FieldGlowSprite g_GlowEffectSprite0;
extern FieldGlowSprite g_GlowEffectSprite1;
extern FieldGlowSprite g_GlowEffectSprite2;
extern FieldGlowSprite g_GlowEffectSprite3;

/* Presets shared by emitter setup and the glow renderers. */
extern FieldGlowSprite g_GlowEffectBSprite0, g_GlowEffectBSprite1;
extern FieldGlowSprite g_GlowEffectCSprite0, g_GlowEffectCSprite1, g_FourLayerGlowSprite0, g_FourLayerGlowSprite1, g_FourLayerGlowSprite2, g_FourLayerGlowSprite3;

/* Burst presets shared by setup and scaled glow renderers. */
extern FieldGlowSprite g_BurstEffectSprite1, g_BurstEffectSprite0, g_BurstEffectSprite2, g_BurstEffectSprite3;

/* Draws one scaled glow sprite at the matrix translation (engine/
 * fieldfx/sprite_render.c). */
void FieldFx_DrawGlowSprite(FieldGlowSprite *sprite, GteMatrix *matrix, u8 mode);

#endif
