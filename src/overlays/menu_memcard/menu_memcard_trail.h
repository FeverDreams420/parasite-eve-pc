#ifndef MENU_MEMCARD_TRAIL_H
#define MENU_MEMCARD_TRAIL_H

#include "pe1/field_texture_tables.h"
#include "pe1/fieldfx_callback_list.h"

#include "menu_memcard_link.h"

typedef struct MemcardTrailState { s16 phase, timer, attachment; } MemcardTrailState;
extern u8 D_801F1DE8[];
extern u8 D_801F1E74[];

s32 MenuCard_TrailEffect(s32 mode, MemcardTrailState *state);
s32 MenuCard_SineTrailEffect(s32 mode, MemcardTrailState *state);

#endif
