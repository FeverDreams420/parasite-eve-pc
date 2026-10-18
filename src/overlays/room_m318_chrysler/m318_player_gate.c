#include "common.h"
#include "pe1/field_actor.h"
#include "pe1/battle.h"
#include "pe1/fieldfx_callback_list.h"
#include "pe1/fieldfx_task_pool.h"

#include "../room_lib/room_lib.h"
#include "m318_player_gate_externs.h"

int RoomM318_PlayerGate(int arg0, void *arg1, intptr_t argument) {
    s32 *arg2 = (void *)argument;
    (void)arg1;
    Combatant *actor;

    if (arg0 != 1) {
        if (arg0 >= 2) {
            return 0;
        }
        if (arg0 != 0) {
            return 0;
        }
        g_FieldFxTaskContext->variables[0] = 0;
        g_FieldFxTaskContext->variables[1] = 0;
        {
            /* Player flags belong to the battle record, also on the host. */
            Combatant *actor0 = (void *)g_PlayerEntity->state;
            s32 mask = 0xEFFFFFFF;
            actor0->stateFlags &= mask;
        }
        return 0;
    } else {
        actor = (void *)g_PlayerEntity->state;
        if (actor->stateFlags & 0x10000000) {
            g_FieldFxTaskContext->variables[0] = 1;
            g_FieldFxTaskContext->variables[1] = 1;
            return 1;
        }
        if (g_FieldFxCurrentAge < *arg2) {
            return 0;
        }
        g_FieldFxTaskContext->variables[0] = 1;
        g_FieldFxTaskContext->variables[1] = 0;
        return 1;
    }
}

/* A second single-argument script entry; the room's first is the shared
 * RoomLib_SetArg1 unit linked just before this one. */
ROOMLIB_SET_ARG1(RoomM318_SetScriptArg2, g_RoomM318ScriptArg2)

intptr_t RoomM318_GetActorParams(int command, intptr_t arg1, intptr_t arg2, intptr_t arg3) {
    (void)command;
    (void)arg1;
    (void)arg2;
    (void)arg3;
    return (intptr_t)g_RoomM318ActorParams;
}
