/*
 * Room module class, slot 2: forwards a spawn request to the field engine.
 * The same function in every room that links it (pe1/room_module.h).
 */

#include "pe1/room_module.h"

#include "pe1/fieldfx_state.h"

int RoomLib_Spawn6(PmSlotHeader *slot, int b, int c, intptr_t d, intptr_t e, intptr_t f)
{
    FieldFx_Spawn6((intptr_t)slot, b, c, d, (int)e, (int)f);
    return 0;
}
