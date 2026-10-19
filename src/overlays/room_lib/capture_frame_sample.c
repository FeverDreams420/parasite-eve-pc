/*
 * Fills the room's capture block (g_RoomCaptureX, Y, Z and g_RoomCaptureFrameWord) from the field
 * engine's current object: its halfwords at +0x268 and +0x26C, the floor
 * height and a fixed 0x76C. room_m156, room_m291 and room_m380 (the +2
 * variants) link it after RoomLib_SlotSet; this unit is that function,
 * compiled into each of them.
 */

#include "common.h"
#include "pe1/field_actor.h"
#include "pe1/fieldfx_task_pool.h"
#include "pe1/room_floor.h"
#include "pe1/room_spark_attacks.h"

RoomSparkRingParams g_RoomSparkCapture;

intptr_t RoomLib_CaptureFrameSample(int mode, intptr_t x, intptr_t y, intptr_t z) {
    FieldActor *object = g_FieldFxOwner->actor;
    if (mode == 1) {
        g_RoomCaptureX = (s16)x;
        g_RoomCaptureY = (s16)y;
        g_RoomCaptureZ = (s16)z;
    } else {
        g_RoomCaptureFrameWord = (s32)x;
    }
    g_RoomCaptureX = object->render_object.target_x;
    g_RoomCaptureY = object->render_object.target_y;
    g_RoomCaptureY = (s16)g_RoomFloorY->raw;
    g_RoomCaptureZ = object->render_object.target_z;
    g_RoomCaptureFrameWord = 0x76C;
    return (intptr_t)&g_RoomSparkCapture;
}
