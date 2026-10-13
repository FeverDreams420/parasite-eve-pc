#ifndef PE1_ROOM_M089_SPIN_MODEL_H
#define PE1_ROOM_M089_SPIN_MODEL_H

#include "common.h"
#include "pe1/asset_find.h"
#include "pe1/field_model_draw.h"
#include "pe1/field_texture_tables.h"
#include "pe1/gte_types.h"
#include "pe1/psyq_gpu.h"
#include "pe1/render_object.h"
#include "pe1/render_prim.h"
#include "pe1/scene_archive.h"

/* room_m089 spinning model: a glow sprite swells over the anchor while two
 * copies of a loaded model turn and shrink above it. */
typedef struct RoomM089SpinModel {
    /* 0x00 */ s16 timer;
    /* 0x02 */ s16 stage;
} RoomM089SpinModel;

extern RenderColor g_RoomM089SpinGlowColor;
extern u8 *g_RoomM089CoreModel;

#endif
