#ifndef PE1_FIELD_FLASH_BURST_H
#define PE1_FIELD_FLASH_BURST_H

#include "common.h"
#include "pe1/field_effect_pool.h"
#include "pe1/field_tile.h"
#include "pe1/gte_types.h"
#include "pe1/player_entity.h"
#include "pe1/psyq_gpu.h"
#include "pe1/render_object.h"

/* Field engine flash burst (FieldFx_FlashBurst): two screen flashes, a model
 * swelling on the actor's point 19 and a star fan, ring band and shape
 * quad at the anchor. */
typedef struct FieldFlashBurst {
    /* 0x00 */ GteShortVector anchor;
    /* 0x08 */ s16 point[3];
} FieldFlashBurst;

/* Model drawn by the burst and its colour track. */
extern u8 *g_FieldFxModel83ACD504;
extern u8 D_800E20CC[];

#endif /* PE1_FIELD_FLASH_BURST_H */
