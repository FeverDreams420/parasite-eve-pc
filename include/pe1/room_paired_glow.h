#ifndef PE1_ROOM_PAIRED_GLOW_H
#define PE1_ROOM_PAIRED_GLOW_H

#include "common.h"
#include "pe1/akao_send.h"
#include "pe1/asset_find.h"
#include "pe1/field_model_draw.h"
#include "pe1/field_sprite_state.h"
#include "pe1/psyq_gpu.h"
#include "pe1/psyq_libc.h"
#include "pe1/room_fx.h"
#include "pe1/room_module.h"
#include "pe1/room_interface.h"
#include "pe1/scene_archive.h"

/* The paired glow (src/overlays/room_lib/paired_glow.c). */

/* Per-room data: the glow sprite's record and packet, and the colour table
 * the emitters step through. */
extern RoomFxSpritePacket g_RoomPairedGlowSprite;
extern void *g_RoomPairedGlowPacket;
extern unsigned char g_RoomPairedGlowColorTable[];


int *FieldFx_RegPtr(int index);

void FieldFx_ModelSetTexture(int arg0, int arg1);

extern PmCommand g_RoomPairedGlowHandlers;
extern FieldFxHandler g_RoomInitList__paired[6],g_RoomDrawList__paired[6],g_RoomUpdateList__paired[6];
extern int g_RoomScript__paired[26],g_RoomSpawnLayout__paired[3];
extern void *g_RoomPairedGlowPacket__paired;
extern u8 g_RoomPairedGlowColorTable__paired[24];
#endif
