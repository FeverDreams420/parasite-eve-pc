/*
 * The scene script's slot entry: mode 1 stores two words in slot idx of the
 * room's slot table, any other mode its position and the frame it was set
 * on. Seventeen rooms link it after their line burst or argument entry;
 * the table is the room's (RoomLib_SlotTable).
 */

#include "room_lib.h"

RoomSlotRec RoomLib_SlotTable[8];

intptr_t RoomLib_SlotSet(int mode, intptr_t idx, intptr_t a, intptr_t b) {
    RoomSlotRec *e = &RoomLib_SlotTable[idx];

    if (mode == 1) {
        e->w8 = a;
        e->wC = b;
    } else {
        /* Stored through a halfword view of the field, which GCC schedules
         * before the frame load as retail has it. */
        *&e->h0 = a;
        e->h2 = g_RoomFloorY->raw;
        e->h4 = b;
    }
    return (intptr_t)e;
}
