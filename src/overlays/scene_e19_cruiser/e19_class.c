#include "common.h"
#include "pe1/pm_types.h"
#include "pe1/gte_types.h"
#include "pe1/room_m023_beacon.h"
#include "pe1/scene_grab_head.h"
#include "pe1/scene_grab_trigger.h"

#include "e19_class_externs.h"

void SceneE19_GrabNop0(void) {
}

void SceneE19_GrabInit(PmSlotHeader *raw) {
    SceneGrabTrigger *obj=(void *)raw;
    GteMatrix rotation;
    obj->control.callback = RoomLib_GrabAwaitStance;
    obj->control.started = 0;
    obj->control.grabbed = 0;
    obj->flag3 = 0xFF;
    RotMatrixYXZ((GteShortVector *)g_SceneE19GrabAngles, &rotation);
    MulMatrix0((GteMatrix *)g_SceneE19GrabIdentityMatrix, &rotation, (GteMatrix *)&obj->control.player_matrix);
    /* player_matrix.translation, stored as plain words to keep retail order. */
    obj->control.player_matrix.translation[0] = g_SceneE19GrabPlayerTranslation[0];
    obj->control.player_matrix.translation[1] = g_SceneE19GrabPlayerTranslation[1];
    obj->control.player_matrix.translation[2] = g_SceneE19GrabPlayerTranslation[2];
    return;
}

int SceneE19_GrabConfigure(PmSlotHeader *raw, int mode, int event, intptr_t arg3, intptr_t arg4, intptr_t arg5) {
    SceneGrabTrigger *obj=(void *)raw;
    (void)arg4; (void)arg5;
    if (event == 0x19 && mode == 1) {
        obj->control.signal = (int *)arg3;
        *(int *)arg3 = mode;
    }

    return 0;
}

int SceneE19_GrabNop3(PmSlotHeader *raw) {
    (void)raw;
    return 0;
}
