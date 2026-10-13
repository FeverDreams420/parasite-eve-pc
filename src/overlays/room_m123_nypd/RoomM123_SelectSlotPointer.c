#include "RoomM123_SelectSlotPointer_externs.h"

void **RoomM123_SelectSlotPointer(int unused, unsigned int index) {
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

    return &g_RoomM123SelectedColorTrack;
}
