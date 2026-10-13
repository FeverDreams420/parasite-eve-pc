#ifndef PE1_FIELD_GLOW_LAYERS_H
#define PE1_FIELD_GLOW_LAYERS_H

#include "common.h"
#include "pe1/field_glow_sprite.h"
#include "pe1/field_sprite_state.h"
#include "pe1/gte_types.h"

/* Field engine glow layers: a placed axis matrix turned by fixed spins
 * and drawn as scaled sprites through FieldFx_DrawGlowSprite. Three two-layer
 * variants and one four-layer variant share this record. */
typedef struct FieldGlowLayers {
    /* 0x00 */ u8 pad00[4];
    /* 0x04 */ u16 depth;
    /* 0x06 */ u8 pad06[2];
    /* 0x08 */ s16 x;
    /* 0x0A */ s16 y;
    /* 0x0C */ s16 z;
    /* 0x0E */ u8 pad0E[2];
    /* 0x10 */ GteMatrix matrix;
} FieldGlowLayers;

extern FieldGlowSprite g_GlowEffectBSprite2;

extern GteShortVector g_GlowEffectLayersSpinA;
extern GteShortVector g_GlowLayers1SpinB;
extern GteVector g_GlowLayers1Scale;
extern GteShortVector g_GlowEffectBLayersSpinA;
extern GteShortVector g_GlowEffectBSpinB;
extern GteVector g_GlowEffectBLayersScale;
extern GteShortVector g_GlowEffectCLayersSpinA;
extern GteShortVector g_GlowEffectCLayersSpinB;
extern GteVector g_GlowLayers3Scale;
extern GteShortVector g_FourLayerGlowSpinA;
extern GteShortVector g_FourLayerGlowSpinB;
extern GteShortVector g_FourLayerGlowSpinC;
extern GteShortVector g_FourLayerGlowSpinD;
extern GteVector g_FourLayerGlowScale;

#endif
