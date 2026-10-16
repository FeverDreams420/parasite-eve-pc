/*
 * Room module class, slot 5: closes the module and, while the field engine
 * is running, its link target. The same function in every room that links
 * it (pe1/room_module.h).
 */

#include "pe1/room_module.h"

#include "pe1/field_actor.h"
#include "pe1/fieldfx_state.h"

int RoomLib_CloseTarget(PmSlotHeader *slot)
{
    slot->state = 4;
    if ((unsigned int)FieldFx_GetStatus(slot) >= 2) {
        FieldActor *actor = ((FieldFxObjectPrefix *)slot)->actor;
        actor->state->core_flags &= 0xC0FFFFFFu;
        *actor->state->action_state = 4;
    }
    return 0;
}
