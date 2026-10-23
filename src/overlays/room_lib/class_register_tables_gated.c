/*
 * Room module class, slot 4, in the modules that start only once their
 * link is past variant 1 or its target's flag at +0xAC is set: from field
 * engine status 2 on, registers the module's update handlers and spawns its
 * objects when the gate is open; closes the module when either call fails
 * or the engine is not running. The lists are the room's
 * (pe1/room_module.h).
 */

#include "pe1/room_module.h"

#include "room_lib.h"

int RoomLib_RegisterPairedTablesGated(PmSlotHeader *o) {
    RoomLink *entry;
    int result = 0;

    if ((unsigned int)FieldFx_GetStatus(o) >= 2) {
        entry = ((RoomEnt *)o)->link;
        if (entry->variant >= 2 || entry->target->startFlag != 0) {
            result = FieldFx_UpdateSlots((FieldFxObject *)o, g_RoomUpdateList);
            result |= FieldFx_RunScript((FieldFxObject *)o, g_RoomInitList, (u16 *)g_RoomSpawnLayout);
        }
    } else {
        result = -1;
    }

    if (result == -1) {
        RoomLib_CloseTarget(o);
    }

    return 0;
}
