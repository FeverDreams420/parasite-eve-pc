/*
 * The scene script's single-argument entry: stores its argument for the
 * room's effects and returns where it went. Linked as a function of its own
 * after an effect module by the rooms that take one word from the script;
 * the word is the room's (g_RoomScriptArg, pe1/room_module.h).
 */

#include "pe1/room_module.h"

#include "room_lib.h"

int g_RoomScriptArg;
ROOMLIB_SET_ARG1(RoomLib_SetArg1, g_RoomScriptArg)
