/*
 * Glow orb module class, slot 1: plants the module's script and sets up the
 * orb's and the flashes' sprite blocks. The same function in room_m034,
 * room_m174 and room_m383, in front of the class methods that precede the
 * glow orb (glow_orb.c); the data is each room's own
 * (pe1/room_glow_orb.h).
 */

#include "pe1/room_glow_orb.h"

void RoomEffect_GlowOrbSetup(PmSlotHeader *object) {
    *FieldFx_GetSlot((FieldFxObject *)object) = g_RoomGlowOrbScript;
    g_RoomGlowOrbSprite.code = 0x20;
    g_RoomGlowOrbSprite.mode = 3;
    g_RoomGlowOrbFlashSprite.code = 0x2B;
    g_RoomGlowOrbFlashSprite.mode = 2;
    g_RoomGlowOrbFlashSprite.depth = 0x80;
    g_RoomGlowOrbSprite.offset = 0;
    g_RoomGlowOrbSprite.depth = 0;
    g_RoomGlowOrbSprite.zero = 0;
    g_RoomGlowOrbFlashSprite.offset = 0;
    g_RoomGlowOrbFlashSprite.r = 0x80;
    g_RoomGlowOrbFlashSprite.g = 0x80;
    g_RoomGlowOrbFlashSprite.b = 0x80;
    g_RoomGlowOrbFlashSprite.zero = 0;
}

/* Original M034 lists at 8018FF34/48/5C, layout at FF70,
 * class descriptor FF7C and 35-word script FF98..90020. */
FieldFxHandler g_RoomInitList[] = {RoomEffect_GlowOrbCaptureOwner, RoomEffect_GlowOrbSpawnerInit,
    RoomEffect_GlowOrbInit, RoomEffect_GlowOrbFlashInit, (FieldFxHandler)(intptr_t)-1};
FieldFxHandler g_RoomDrawList[] = {RoomEffect_GlowOrbControllerDraw, RoomEffect_GlowOrbSpawnerDraw,
    RoomEffect_GlowOrbDraw, RoomEffect_GlowOrbFlashDraw, (FieldFxHandler)(intptr_t)-1};
FieldFxHandler g_RoomUpdateList[] = {RoomEffect_GlowOrbControllerUpdate, RoomEffect_GlowOrbSpawnerUpdate,
    RoomEffect_GlowOrbUpdate, RoomEffect_GlowOrbFlashUpdate, (FieldFxHandler)(intptr_t)-1};
int g_RoomSpawnLayout[] = {0x00040000,0x00180044,0x0000ffff};
int g_RoomGlowOrbScript[] = {
    0x00010000,0x00400001,0x10110000,0x10010001,0x301d0000,0x00140200,
    0x00010002,0x00020003,0x00140100,0x00010002,0x00020003,0x00140000,
    0x00010002,0x00020003,0x0014ff00,0x00010002,0x00020003,0x0014fe00,
    0x00010002,0x300e0000,0x0014fe00,0x00010002,0x00020003,0x0014ff00,
    0x00010002,0x00020003,0x00140000,0x00010002,0x00020003,0x00140100,
    0x00010002,0x00020003,0x00140200,0x00010002,0xffffffff,
};
GteMatrix g_RoomGlowOrbAxes, g_RoomGlowOrbOrigin;
RoomGlowOrbLink *g_RoomGlowOrbOwner;
RoomFxSpritePacket g_RoomGlowOrbSprite, g_RoomGlowOrbFlashSprite;
PmCommand g_RoomGlowOrbHandlers = {RoomEffect_GlowOrbNop0, RoomEffect_GlowOrbSetup,
    RoomLib_Spawn6, RoomLib_RegisterDrawListActive, RoomLib_RegisterPairedTablesActive,
    RoomLib_CloseTarget, RoomEffect_GlowOrbNop6};
