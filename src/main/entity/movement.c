#include "pe1/field_actor.h"
#include "pe1/game_state.h"
#include "pe1/player_entity.h"
#include "pe1/scene_flags.h"
#include "pe1/script_queue.h"

extern FieldActor *g_CurrentEntity;
void Entity_DispatchCallbacks(FieldActor *actor);

/* Original word additions wrap on MIPS. Avoid host signed-overflow UB. */
static s32 add_word(s32 lhs, s32 rhs)
{
    u32 value = (u32)lhs + (u32)rhs;
    return value <= INT32_MAX ? (s32)value : -1 - (s32)(UINT32_MAX - value);
}

/* Original frame_update.c movement/callback procedures. */
void Entity_IntegratePositionFull(FieldActor *a)
{
    int tmp;
    int v0;

    a->base_x = a->pos_x;
    a->base_y = a->pos_y;
    a->base_z = a->pos_z;
    a->saved_rot_x = a->rot_x;
    a->saved_rot_y = a->rot_y;
    a->saved_rot_z = a->rot_z;
    Entity_DispatchCallbacks(a);

    if ((g_FieldMoveLock & 1) == 0) {
        v0 = g_PlayerEntity->state->flags;
        tmp = a->mode;
        if ((v0 & 0xC0) == 0x80) {
            tmp = 0x11;
        }
        Scene_CheckFlagBits(a, g_EntityInputRulesByMode, &tmp);
    }

    if (a->flags & 2) {
        a->motion_x = add_word(a->motion_x, a->gravity_x);
        a->motion_y = add_word(a->motion_y, a->gravity_y);
        a->motion_z = add_word(a->motion_z, a->gravity_z);
    }

    a->motion_x = add_word(a->motion_x, a->accel_x);
    a->motion_y = add_word(a->motion_y, a->accel_y);
    a->motion_z = add_word(a->motion_z, a->accel_z);

    a->pos_x = add_word(a->pos_x, a->motion_x);
    a->pos_y = add_word(a->pos_y, a->motion_y);
    a->pos_z = add_word(a->pos_z, a->motion_z);

    a->pos_x = add_word(a->pos_x, a->delta_x);
    a->pos_y = add_word(a->pos_y, a->delta_y);
    a->pos_z = add_word(a->pos_z, a->delta_z);
}

void Entity_IntegratePositionConditional(FieldActor *a)
{
    if (g_GameStateBlock.flags & 0x100) {
        Entity_DispatchCallbacks(a);
        return;
    }

    a->base_x = a->pos_x;
    a->base_y = a->pos_y;
    a->base_z = a->pos_z;
    a->saved_rot_x = a->rot_x;
    a->saved_rot_y = a->rot_y;
    a->saved_rot_z = a->rot_z;
    Entity_DispatchCallbacks(a);

    if (a->flags & 2) {
        a->motion_x = add_word(a->motion_x, a->gravity_x);
        a->motion_y = add_word(a->motion_y, a->gravity_y);
        a->motion_z = add_word(a->motion_z, a->gravity_z);
    }

    a->motion_x = add_word(a->motion_x, a->accel_x);
    a->motion_y = add_word(a->motion_y, a->accel_y);
    a->motion_z = add_word(a->motion_z, a->accel_z);

    a->pos_x = add_word(a->pos_x, a->motion_x);
    a->pos_y = add_word(a->pos_y, a->motion_y);
    a->pos_z = add_word(a->pos_z, a->motion_z);

    a->pos_x = add_word(a->pos_x, a->delta_x);
    a->pos_y = add_word(a->pos_y, a->delta_y);
    a->pos_z = add_word(a->pos_z, a->delta_z);
}

void Entity_DispatchCallbacks(FieldActor *arg0) {
    int i;
    ScriptNode **ptr;
    ScriptNode *callback;

    i = 0;
    ptr = arg0->script_node_lists;
    g_CurrentEntity = arg0;
    do {
        callback = *ptr;
        g_ScriptNodePool = callback;
        if (callback != 0) {
            Script_RunQueue();
        }
        i++;
        ptr++;
    } while ((unsigned int)i < 3U);
}
