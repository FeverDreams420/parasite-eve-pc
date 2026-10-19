#include "pe1/field_actor.h"
#include "pe1/gte_types.h"
#include "pe1/room_m023_beacon.h"
#include "pe1/scene_grab_head.h"

#include "../room_lib/room_lib.h"
#include "m273_class_externs.h"
#include "room_m273.h"

void RoomM273_GrabNop0(void) {
}

void RoomM273_GrabInit(PmSlotHeader *raw) {
    RoomSelectionState *obj = (void *)raw;
    GteMatrix rotation;

    obj->callback = RoomLib_GrabAwaitStance;
    obj->started = 0;
    obj->placementChecked = 0;
    obj->verticalTicks = 0;
    obj->activated = 0;
    obj->flag3 = 0xFF;
    RotMatrixYXZ((GteShortVector *)&g_RoomM273GrabInitAngles, &rotation);

    MulMatrix0(&g_RoomM273GrabInitMatrix, &rotation, (GteMatrix *)&obj->matrix);
    /* Preserve the authored grab transform with native pointer alignment. */
    obj->matrix.translation[0] = g_RoomM273GrabInitTranslationX;
    obj->matrix.translation[1] = g_RoomM273GrabInitTranslationY;
    obj->matrix.translation[2] = g_RoomM273GrabInitTranslationZ;
    return;
}

int RoomM273_GrabConfigure(PmSlotHeader *raw, int mode, int event, intptr_t arg3, intptr_t index, intptr_t unused) {
    RoomSelectionState *obj = (void *)raw;
    switch (event) {
    case 0x19:
        if (mode == 1) {
            ((RoomSelectionState *)obj)->signal = (int *)arg3;
            *(int *)arg3 = mode;
        }
        break;
    case 0x1D:
        ((RoomSelectionState *)obj)->phase_countdown = arg3;
        ((RoomSelectionState *)obj)->pending_action = g_RoomM273GrabSelectionByIndex[index];
        break;
    }

    return 0;
}

int RoomM273_GrabNop3(PmSlotHeader *raw) {
    return 0;
}
