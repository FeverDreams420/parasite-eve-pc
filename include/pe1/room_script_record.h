#ifndef PE1_ROOM_SCRIPT_RECORD_H
#define PE1_ROOM_SCRIPT_RECORD_H

#include "common.h"

/*
 * The record a room's scene script reads back through the shared
 * RoomLib_SetTaskVar0. Each room that links that unit (m005, m023, m123)
 * places its own data here with its own layout; the one thing they share
 * is a leading halfword, which is what the symbol names. room_m123 reads
 * that halfword directly (a direct scalar keeps retail's absolute loads,
 * where any record or union view keeps the address in a register); the
 * other rooms take the address and read it through their own view macro.
 */
extern s16 g_RoomScriptRecord;

#endif
