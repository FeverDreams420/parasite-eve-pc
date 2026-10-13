#ifndef MENU_MEMCARD_LINK_H
#define MENU_MEMCARD_LINK_H

#include "pe1/field_sprite_draw.h"
#include "pe1/field_texture_tables.h"
#include "pe1/fieldfx_callback_list.h"
#include "pe1/fieldfx_calls.h"
#include "pe1/fieldfx_task_pool.h"
#include "pe1/gte.h"

#include "menu_memcard_offset.h"
#include "menu_memcard_pulse.h"

typedef struct MemcardLinkColor { u8 bytes[4]; } MemcardLinkColor;
typedef struct MemcardLinkState { s16 phase, timer, opacity, reserved06; GteRotation rotation; } MemcardLinkState;
typedef struct MemcardLinkChannel { s32 reserved[2]; void *slots; } MemcardLinkChannel;
extern MemcardLinkColor D_801ED850, D_801ED854;
extern struct BattleEntity *D_8009D254;
extern s16 D_801F1D86, D_801F1D88;

s32 MenuCard_LinkEffect(s32 mode, MemcardLinkState *state);

#endif
