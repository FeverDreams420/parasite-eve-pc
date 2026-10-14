#include "common.h"
#include "pe1/script_expression.h"
#include "pe1/fixed_point.h"
#include "pe1/battle_runtime.h"
#include "pe1/boot.h"
#include "pe1/field_actor.h"
#include "pe1/field_collision.h"
#include "pe1/field_movement.h"
#include "pe1/geom_state.h"
#include "pe1/gte.h"
#include "pe1/player_entity.h"
#include "pe1/scene_flags.h"

/* Script value opcodes: expression evaluation, current-actor vector get/set
 * and pad/flag mask tests. Contiguous default-profile handlers between the
 * node pool and the node actions; the expression divide and remainder
 * carry ASPSX's divide checks. */



int Script_SetEntityVec3(int **args) {
    int **argp = args;

    switch (*argp[0]) {
    case 0:
        {
            FieldActor *state = g_CurrentEntity;

            state->pos_x = *argp[1];
            state->pos_y = *argp[2];
            state->pos_z = *argp[3];
            Entity_FindFloor((struct BattleEntity *)state);
        }
        g_CurrentEntity->base_x = g_CurrentEntity->pos_x;
        g_CurrentEntity->base_y = g_CurrentEntity->pos_y;
        g_CurrentEntity->base_z = g_CurrentEntity->pos_z;
        if (g_CurrentEntity == g_PlayerEntity) {
            int *flags = &g_GeomScroll.state.flags;

            *flags |= 0x80;
        }
        break;
    case 1:
        g_CurrentEntity->base_x = *argp[1];
        g_CurrentEntity->base_y = *argp[2];
        g_CurrentEntity->base_z = *argp[3];
        break;
    case 2:
        g_CurrentEntity->motion_x = *argp[1];
        g_CurrentEntity->motion_y = *argp[2];
        g_CurrentEntity->motion_z = *argp[3];
        break;
    case 3:
        g_CurrentEntity->accel_x = *argp[1];
        g_CurrentEntity->accel_y = *argp[2];
        g_CurrentEntity->accel_z = *argp[3];
        break;
    case 4:
        g_CurrentEntity->gravity_x = *argp[1];
        g_CurrentEntity->gravity_y = *argp[2];
        g_CurrentEntity->gravity_z = *argp[3];
        break;
    case 5:
        g_CurrentEntity->rot_x = *argp[1];
        g_CurrentEntity->rot_y = *argp[2];
        g_CurrentEntity->rot_z = *argp[3];
        break;
    case 6:
        g_CurrentEntity->delta_x = *argp[1];
        g_CurrentEntity->delta_y = *argp[2];
        g_CurrentEntity->delta_z = *argp[3];
        break;
    }
    return 1;
}

int Script_GetEntityVec3(int **args) {
    switch (*args[0]) {
    case 0:
        *args[1] = g_CurrentEntity->pos_x;
        *args[2] = g_CurrentEntity->pos_y;
        *args[3] = g_CurrentEntity->pos_z;
        break;
    case 1:
        *args[1] = g_CurrentEntity->base_x;
        *args[2] = g_CurrentEntity->base_y;
        *args[3] = g_CurrentEntity->base_z;
        break;
    case 2:
        *args[1] = g_CurrentEntity->motion_x;
        *args[2] = g_CurrentEntity->motion_y;
        *args[3] = g_CurrentEntity->motion_z;
        break;
    case 3:
        *args[1] = g_CurrentEntity->accel_x;
        *args[2] = g_CurrentEntity->accel_y;
        *args[3] = g_CurrentEntity->accel_z;
        break;
    case 4:
        *args[1] = g_CurrentEntity->gravity_x;
        *args[2] = g_CurrentEntity->gravity_y;
        *args[3] = g_CurrentEntity->gravity_z;
        break;
    case 5:
        *args[1] = g_CurrentEntity->rot_x;
        *args[2] = g_CurrentEntity->rot_y;
        *args[3] = g_CurrentEntity->rot_z;
        break;
    case 6:
        *args[1] = g_CurrentEntity->delta_x;
        *args[2] = g_CurrentEntity->delta_y;
        *args[3] = g_CurrentEntity->delta_z;
        break;
    }
    return 1;
}
int Script_DispatchCmd(int **args) {
    int mode;

    mode = *args[0];
    switch (mode) {
    case 0: {
        u32 mask;
        u32 state;
        state = g_FieldPadBits;
        mask = *args[1];
        if ((state & mask) == mask) {
            *args[2] = 1;
        } else {
            *args[2] = 0;
        }
        break;
    }
    case 1: {
        u32 mask;
        u32 state;
        state = g_FieldPadPressed;
        mask = *args[1];
        if ((state & mask) == mask) {
            *args[2] = 1;
        } else {
            *args[2] = 0;
        }
        break;
    }
    case 2: {
        u32 mask;
        u32 state;
        state = g_FieldPadReleased;
        mask = *args[1];
        if ((state & mask) == mask) {
            *args[2] = 1;
        } else {
            *args[2] = 0;
        }
        break;
    }
    case 3: {
        u32 mask;
        u32 state;
        int index;

        state = g_FieldPadBits;
        mask = (u32)*args[1];
        state &= mask;
        if (state == mask) {
            gte_ldlzcs(mask);
            index = 31;
            if (state != 0x80000000) {
                gte_stlzcr(args[1]);
                index -= *args[1];
            }
            *args[2] = g_FieldPadHeldFrameCounts[index];
        } else {
            *args[2] = 0;
        }
        break;
    }
    }
    return 1;
}

/* Keep original 32-bit word arithmetic without host signed-overflow UB. */
static s32 signed_word(u32 value)
{
    return value <= INT32_MAX ? (s32)value : -1 - (s32)(UINT32_MAX - value);
}
static s32 shift_right(s32 value, unsigned shift)
{
    s64 divisor = (s64)1 << shift;
    return value >= 0 ? (s32)(value / divisor) : (s32)(-1 - (-1 - (s64)value) / divisor);
}

int Script_EvalExpr(int **args) {
    switch (*args[0]) {
    case 0:
        *args[1] = signed_word((u32)*args[2] + (u32)*args[3]);
        break;
    case 1:
        *args[1] = signed_word((u32)*args[2] - (u32)*args[3]);
        break;
    case 2:
        *args[1] = *args[2] | *args[3];
        break;
    case 3:
        *args[1] = *args[2] & *args[3];
        break;
    case 4:
        *args[1] = *args[2] ^ *args[3];
        break;
    case 5:
        *args[1] = (*args[2] || *args[3]);
        break;
    case 6:
        *args[1] = (*args[2] && *args[3]);
        break;
    case 7:
        *args[1] = !*args[2];
        break;
    case 8:
        *args[1] = ~*args[2];
        break;
    case 9:
        *args[1] = *args[2] > *args[3];
        break;
    case 10:
        *args[1] = *args[2] < *args[3];
        break;
    case 11:
        *args[1] = *args[2] == *args[3];
        break;
    case 12:
        *args[1] = *args[2] >= *args[3];
        break;
    case 13:
        *args[1] = *args[2] <= *args[3];
        break;
    case 14:
        *args[1] = *args[2] != *args[3];
        break;
    case 15:
        *args[1] = signed_word((u32)*args[2] * (u32)*args[3]);
        break;
    case 16:
        *args[1] = (*args[2] == INT32_MIN && *args[3] == -1) ? INT32_MIN : *args[2] / *args[3];
        break;
    case 17:
        *args[1] = signed_word((u32)*args[2] << ((u32)*args[3] & 31));
        break;
    case 18:
        *args[1] = shift_right(*args[2], (u32)*args[3] & 31);
        break;
    case 19:
        *args[1] = *args[2];
        break;
    case 20:
        *args[1] = Math_FixedMul(*args[2], *args[3]);
        break;
    case 21:
        *args[1] = Math_FixedDivide(*args[2], *args[3]);
        break;
    case 22:
        *args[1] = (*args[2] == INT32_MIN && *args[3] == -1) ? 0 : *args[2] % *args[3];
        break;
    case 23:
        *args[1] = signed_word(0u - (u32)*args[2]);
        break;
    }
    return 1;
}
