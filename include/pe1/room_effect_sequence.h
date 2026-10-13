#ifndef PE1_ROOM_EFFECT_SEQUENCE_H
#define PE1_ROOM_EFFECT_SEQUENCE_H

#include "common.h"
#include "pe1/asset_find.h"
#include "pe1/field_script_context.h"
#include "pe1/field_sprite_state.h"
#include "pe1/psyq_libc.h"
#include "pe1/room_floor.h"
#include "pe1/room_interface.h"
#include "pe1/scene_archive.h"

/* The effect sequence (src/overlays/room_lib/effect_sequence.c):
 * the room data it fills and reads (the same addresses in all five rooms
 * that link it) and the main code it calls. */

/* The 0xC used bytes of each 0x10-byte sprite preset. The third
 * preset's trailing word is a separate model resource (g_EffectSequenceModel). */
typedef struct RoomEffectSequenceSprite {
    u8 r, g, b, reserved03;
    u8 cell, clut, flip, reserved07;
    s16 offset;
    u16 depth;
} RoomEffectSequenceSprite;
PE1_STATIC_ASSERT(sizeof(RoomEffectSequenceSprite) == 0xC, effect_sequence_sprite_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomEffectSequenceSprite, depth) == 0xA, effect_sequence_sprite_depth);
extern RoomEffectSequenceSprite g_EffectSequenceGlowSprite, g_EffectSequenceGlowSpriteSecondary, g_EffectSequenceFinalGlowSprite, g_EffectSequenceSparkleSprite;
extern void *g_EffectSequenceModel;

extern int *FieldFx_VarPtr(int index);
extern int *FieldFx_RegPtr(int index);
/* The two colour ramps the variant glows fade through (FieldFx_InterpolateColorRamp
 * records r, g, b, duration): lime for resource selector -1, amber otherwise. */
extern char g_EffectSequenceAmberGlowRamp;
extern char g_EffectSequenceLimeGlowRamp;

/* The module's class methods (slots 3 to 5). */
int RoomEffect_EffectSequenceRegister(void *o);
int RoomEffect_EffectSequenceStart(void *o);
s32 RoomEffect_EffectSequenceNop5(void *o);

#endif
