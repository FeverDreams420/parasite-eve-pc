/*
 * Room module class, slot 1: plants the module's command program into the
 * field engine slot. The same function in every room that links it; the
 * program is the room's g_RoomScript (pe1/room_module.h).
 */

#include "pe1/room_module.h"

#include "room_lib.h"

void RoomLib_PlantScript(PmSlotHeader *object) {
    *FieldFx_GetSlot((FieldFxObject *)object) = g_RoomScript;
}
