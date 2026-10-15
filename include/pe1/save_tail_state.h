#ifndef PE1_SAVE_TAIL_STATE_H
#define PE1_SAVE_TAIL_STATE_H

#include "common.h"
#include "pe1/battle_modifiers.h"
#include "pe1/battle_saved_state.h"
#include "pe1/entity_allocation.h"
#include "pe1/game_state.h"
#include "pe1/save_blob.h"
#include "pe1/scene_dispatch.h"

/* Save tail cursor: Save_SerializeTail/Save_DeserializeTail copy each
 * record below through it and advance it by the record size. */
extern u8 *g_SaveIoCursor;

#endif
