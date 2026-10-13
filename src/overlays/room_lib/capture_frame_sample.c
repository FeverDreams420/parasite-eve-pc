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

typedef struct RoomCaptureContext {
    unsigned char pad00[8];
    FieldActor *object;
} RoomCaptureContext;

void RoomLib_CaptureFrameSample(RoomCaptureContext *modeOrContext, int x, int y, int z) {
    unsigned int sample;
    FieldActor *object;
    volatile s16 *captureX;

    if ((int)modeOrContext == 1) {
        g_RoomCaptureX = x;
        g_RoomCaptureY = y;
        g_RoomCaptureZ = z;
    } else {
        g_RoomCaptureFrameWord = x;
    }

    modeOrContext = ANIM_OWNER_AS(RoomCaptureContext);
    captureX = &g_RoomCaptureX;
    object = modeOrContext->object;
    sample = (u16)object->render_object.target_x;
    *captureX = sample;
    sample = (unsigned int)modeOrContext->object;
    sample = (u16)((FieldActor *)sample)->render_object.target_y;
    g_RoomCaptureY = sample;
    sample = (unsigned int)modeOrContext->object;
    modeOrContext = (RoomCaptureContext *)(unsigned int)g_RoomFloorY->raw;
    x = (u16)((FieldActor *)sample)->render_object.target_z;
    sample = 0x76c;
    g_RoomCaptureY = (unsigned int)modeOrContext;
    g_RoomCaptureFrameWord = sample;
    g_RoomCaptureZ = x;
}
