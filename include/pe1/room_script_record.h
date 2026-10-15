#ifndef PE1_ROOM_SCRIPT_RECORD_H
#define PE1_ROOM_SCRIPT_RECORD_H

#include "common.h"
#include "pe1/room_interface.h"

/* Each original overlay supplies its complete room-local record. Legacy
 * scalar accesses name its first halfword; vector views share the same storage.
 * PE_OVERLAY qualifies the backing symbol when several rooms are linked. */
extern s16 g_RoomScriptRecordWords[];
#define g_RoomScriptRecord (g_RoomScriptRecordWords[0])
intptr_t RoomLib_SetTaskVar0(int mode, intptr_t a, intptr_t b, intptr_t c);

#endif
