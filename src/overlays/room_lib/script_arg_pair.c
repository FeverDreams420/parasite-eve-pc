/*
 * The scene script's two single-argument entries of the spark barrage
 * rooms (room_m188, room_m390): each stores its argument in its own word of
 * the room's data and returns where it went. Linked right after the
 * barrage module (spark_barrage.c).
 */

#include "pe1/room_module.h"

#include "room_lib.h"

ROOMLIB_SET_ARG1(RoomLib_SetArgA, g_RoomScriptArgA)

ROOMLIB_SET_ARG1(RoomLib_SetArgB, g_RoomScriptArgB)
