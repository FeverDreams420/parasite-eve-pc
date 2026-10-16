/*
 * Room module class, slots 3 and 4, in the modules that their
 * lists only while the field engine runs the object (status 3) rather than
 * from status 2 on (class_register_draw_list.c, class_register_tables.c).
 * room_m034, room_m174 and room_m383 (the glow orb rooms) link both,
 * directly after each other, so they are one unit. The lists are the
 * room's (pe1/room_module.h).
 */

#include "pe1/room_module.h"

#include "pe1/fieldfx_state.h"

int RoomLib_RegisterDrawListActive(PmSlotHeader *slot)
{
    if (FieldFx_GetStatus(slot) == 3)
        FieldFx_Register((FieldFxObject *)slot, g_RoomDrawList);
    return 0;
}

/*
 * Room module class, slot 4, in the modules that start only while the field
 * engine runs the object (status 3) rather than from status 2 on
 * (RoomLib_RegisterPairedTables): registers the module's update handlers
 * and spawns its objects; closes the module when either call fails or the
 * object is not running. The same function in every room that links it;
 * the lists are the room's (pe1/room_module.h).
 */

int RoomLib_RegisterPairedTablesActive(PmSlotHeader *slot)
{
    int result;
    if (FieldFx_GetStatus(slot) == 3) {
        result = FieldFx_UpdateSlots((FieldFxObject *)slot, g_RoomUpdateList);
        result |= FieldFx_RunScript((FieldFxObject *)slot, g_RoomInitList, (u16 *)g_RoomSpawnLayout);
    } else {
        result = -1;
    }
    if (result == -1)
        RoomLib_CloseTarget(slot);
    return 0;
}
