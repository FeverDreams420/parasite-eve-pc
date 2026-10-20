/*
 * Registration and start methods of scene_e08's first module, between the
 * shared RoomLib_Spawn6 and RoomLib_CloseTarget in its class. Unlike
 * RoomLib_RegisterDrawList and RoomLib_RegisterPairedTables they act
 * whatever the field engine's status.
 */

#include "../room_lib/room_lib.h"
#include "scene_e08_modules.h"

int SceneE08_RegisterDrawList(PmSlotHeader *object) {
    FieldFx_Register((FieldFxObject *)object, g_RoomDrawList);
    return 0;
}

int SceneE08_StartModule(PmSlotHeader *object) {
    int result = FieldFx_UpdateSlots((FieldFxObject *)object, g_RoomUpdateList);
    result |= FieldFx_RunScript((FieldFxObject *)object, g_RoomInitList, g_SceneE08FadeSpawnLayout);
    if (result == -1) {
        RoomLib_CloseTarget(object);
    }
    return 0;
}
