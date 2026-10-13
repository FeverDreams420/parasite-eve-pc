#ifndef PE1_FIELD_EFFECT_POOL_H
#define PE1_FIELD_EFFECT_POOL_H

#include "common.h"
#include "pe1/field_actor.h"
#include "pe1/field_model_draw.h"
#include "pe1/fieldfx_callback_list.h"
#include "pe1/fieldfx_task_pool.h"
#include "pe1/render_object.h"
#include "pe1/room_floor.h"

/* Effect pool declarations for field engine emitters that read the room
 * render Y override as a one-field record, so its load stays below the
 * in-struct position stores like retail. fieldfx_task.h declares the same word
 * as a plain s16 for its other users, so these units include this header
 * instead of fieldfx_task.h. */
typedef FieldFxTaskSlot FieldEffectPool;
typedef FieldFxObjectPrefix FieldEffectOwner;

typedef int (*FieldEffectCallback)(int mode, void *state);

extern u8 *g_FieldFxModel6DEAE684;

/* Effect model used by the memory-card spin ring. */
extern u8 *g_FieldFxGlintModel;

#endif
