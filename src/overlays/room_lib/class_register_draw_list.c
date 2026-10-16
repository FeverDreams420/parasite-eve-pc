/*
 * Room module class, slot 3: registers the module's draw handlers with the
 * field engine once it is running. The same function in every room that
 * links it; the list is the room's g_RoomDrawList (pe1/room_module.h).
 */

#include "pe1/room_module.h"

#include "room_lib.h"

int RoomLib_RegisterDrawList(PmSlotHeader *o) {
    if ((unsigned int)FieldFx_GetStatus(o) >= 2)
        FieldFx_Register((FieldFxObject *)o, g_RoomDrawList);
    return 0;
}
