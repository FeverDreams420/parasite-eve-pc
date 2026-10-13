/*
 * The class table of room_m358, m384 and m387, which link the window class
 * (window_class.c) directly after the room library's actor classes
 * (actor_classes.c): the six rows of actor_class_table.c, then the window
 * class. The table is the first data after the code.
 */

#include "pe1/room_module.h"
#include "pe1/room_window.h"

#include "room_lib.h"

RoomModuleHandler g_RoomClassTable[7][ROOM_CLASS_METHODS] = {
    ROOMLIB_ACTOR_CLASS_D,
    ROOMLIB_ACTOR_CLASS_D,
    ROOMLIB_ACTOR_CLASS_E,
    ROOMLIB_ACTOR_CLASS_B,
    ROOMLIB_ACTOR_CLASS_C,
    ROOMLIB_ACTOR_CLASS_A,
    ROOMLIB_WINDOW_CLASS,
};
