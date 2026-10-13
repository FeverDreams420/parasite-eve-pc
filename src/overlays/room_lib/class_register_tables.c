/*
 * Room module class, slot 4: once the field engine is running, registers the
 * module's update handlers and spawns its objects from the init list and the
 * spawn layout; closes the module when either call fails or the engine is
 * not running. The same function in every room that links it; the lists are
 * the room's (pe1/room_module.h).
 */

#include "pe1/room_module.h"

#include "room_lib.h"

int RoomLib_RegisterPairedTables(void *o) {
    int result;

    if ((unsigned int)FieldFx_GetStatus(o) >= 2) {
        result = FieldFx_UpdateSlots(o, g_RoomUpdateList);
        result |= FieldFx_RunScript(o, g_RoomInitList, g_RoomSpawnLayout);
    } else {
        result = -1;
    }

    if (result == -1) {
        RoomLib_CloseTarget(o);
    }

    return 0;
}
