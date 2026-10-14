#ifndef ROOM_M272_H
#define ROOM_M272_H

#include "pe1/pm.h"

extern struct RECT RoomM272_BlobA, RoomM272_BlobB, RoomM272_BlobC;
extern u16 RoomM272_Rec[320 * 64];
extern int g_RoomM272Phase;
extern PmCommand g_RoomM272Handlers;
void RoomM272_Nop_8018F00C(void);
void RoomM272_Cinematic(PmSlotHeader *slot);
int RoomM272_SetPhase(PmSlotHeader *slot, int cancel, int mode, intptr_t phase, intptr_t b, intptr_t c);
int RoomM272_Nop_8018F188(PmSlotHeader *slot);
int RoomM272_Nop_8018F190(PmSlotHeader *slot);
int RoomM272_Nop_8018F198(PmSlotHeader *slot);
void RoomM272_Nop_8018F1A0(void);
#endif
