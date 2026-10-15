#ifndef MENU_MEMCARD_TRAIL_H
#define MENU_MEMCARD_TRAIL_H

#include "pe1/field_texture_tables.h"
#include "pe1/fieldfx_callback_list.h"

#include "menu_memcard_link.h"

typedef struct MemcardTrailState { s16 phase, timer, attachment, reserved06; } MemcardTrailState;
extern u8 g_MenuCardTrailColorTrack[];
extern u8 g_MenuCardSineTrailColorTrack[];

s32 MenuCard_TrailEffect(int mode, void *state, intptr_t argument);
s32 MenuCard_SineTrailEffect(int mode, void *state, intptr_t argument);

#endif
