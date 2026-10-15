/*
 * Scene script entry of room_m005, room_m023 and room_m123: in mode 1 it
 * sets the first script variable of the running field task, and it always
 * returns the room's record the script reads back. The record is each
 * room's own data with its own layout (m005's dialog anchor, m023's anchor
 * vector, m123's burst parameter); see pe1/room_script_record.h.
 */

#include "pe1/fieldfx_task.h"
#include "pe1/room_script_record.h"

intptr_t RoomLib_SetTaskVar0(int mode, intptr_t a, intptr_t b, intptr_t c) {
    if (mode == 1) {
        g_FieldFxTaskContext->variables[0] = 1;
    }
    return (intptr_t)&g_RoomScriptRecord;
}
