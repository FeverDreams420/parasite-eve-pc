/*
 * The class table of the rooms that link the room library's actor classes
 * (actor_classes.c): one row of seven methods per class, in the library's
 * slot order (no-op, Init, Configure, no-op, Update, Release, no-op). Class
 * D fills the first two rows; E, B, C and A follow. The table is the first
 * data after the code in every room that links the library this way.
 */

#include "pe1/room_module.h"

#include "room_lib.h"

RoomModuleHandler g_RoomClassTable[6][ROOM_CLASS_METHODS] = {
    ROOMLIB_ACTOR_CLASS_D,
    ROOMLIB_ACTOR_CLASS_D,
    ROOMLIB_ACTOR_CLASS_E,
    ROOMLIB_ACTOR_CLASS_B,
    ROOMLIB_ACTOR_CLASS_C,
    ROOMLIB_ACTOR_CLASS_A,
};
