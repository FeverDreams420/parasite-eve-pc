/*
 * The class table of room_m063 and m083, which link only the window class
 * (window_class.c): a single row. The table is the first data after the
 * code.
 */

#include "pe1/room_module.h"
#include "pe1/room_window.h"

RoomModuleHandler g_RoomClassTable[1][ROOM_CLASS_METHODS] = {
    ROOMLIB_WINDOW_CLASS,
};
