/*
 * Update slot of a room's own actor class: runs the step callback the class
 * keeps at +0xC. The same code as RoomLib_UpdateHandlerB, linked as a
 * function of its own among the class handlers of room_m256, room_m273 and
 * scene_e19.
 */

#include "room_lib.h"
#include "pe1/scene_grab_head.h"

int RoomLib_UpdateCallback(PmSlotHeader *raw) {
    SceneGrabHead *obj=(void *)raw;
    obj->callback(obj);
    return 0;
}
