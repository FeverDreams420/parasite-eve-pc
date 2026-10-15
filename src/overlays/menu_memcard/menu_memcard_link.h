#ifndef MENU_MEMCARD_LINK_H
#define MENU_MEMCARD_LINK_H

#include "pe1/field_sprite_draw.h"
#include "pe1/field_texture_tables.h"
#include "pe1/fieldfx_callback_list.h"
#include "pe1/fieldfx_calls.h"
#include "pe1/fieldfx_task_pool.h"
#include "pe1/gte.h"

#include "menu_memcard_offset.h"
#include "pe1/field_history_trail.h"
#include "menu_memcard_pulse.h"

typedef struct MemcardLinkColor { u8 bytes[4]; } MemcardLinkColor;
typedef struct MemcardLinkState { s16 phase, timer, opacity, reserved06; FieldTrailPair history[17]; } MemcardLinkState;
PE1_STATIC_ASSERT(sizeof(MemcardLinkState)==0x118,memcard_link_state_size);
extern MemcardLinkColor g_MenuCardLinkColor1, g_MenuCardLinkColor2;
#include "pe1/player_entity.h"
extern s16 g_MenuCardLinkJointA, g_MenuCardLinkJointB;

s32 MenuCard_LinkEffect(int mode, void *state, intptr_t argument);

#endif
