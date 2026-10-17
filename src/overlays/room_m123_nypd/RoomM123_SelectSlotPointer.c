#include "RoomM123_SelectSlotPointer_externs.h"

intptr_t RoomM123_SelectSlotPointer(int unused, intptr_t value, intptr_t b, intptr_t c) {
    u32 index=(u32)value;
    if (index >= 2) {
        index = 0;
    }

    switch (index) {
    case 0:
        g_RoomM123SelectedColorTrack = g_RoomM123PulseColorTrack0;
        break;
    case 1:
        g_RoomM123SelectedColorTrack = g_RoomM123PulseColorTrack1;
        break;
    }

    return (intptr_t)&g_RoomM123SelectedColorTrack;
}
