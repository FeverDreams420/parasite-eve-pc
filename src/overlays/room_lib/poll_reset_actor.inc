#include "common.h"
#include "pe1/field_sprite_params.h"
#include "pe1/fieldfx_callback_list.h"
#include "pe1/room_fx.h"
#include "pe1/field_texture_tables.h"

/* The including unit declares the flag (u8), the counter (s16) and the
 * seed (RoomFxSeed8) it names. */
#ifndef ROOMLIB_POLL_RESET_FUNC
#error "ROOMLIB_POLL_RESET_FUNC must name the room entry point"
#endif
#ifndef ROOMLIB_POLL_RESET_CALLBACK
#error "ROOMLIB_POLL_RESET_CALLBACK must name the spawned callback"
#endif
#ifndef ROOMLIB_POLL_RESET_FLAG
#error "ROOMLIB_POLL_RESET_FLAG must name the completion flag"
#endif
#ifndef ROOMLIB_POLL_RESET_COUNTER
#error "ROOMLIB_POLL_RESET_COUNTER must name the pending counter"
#endif
#ifndef ROOMLIB_POLL_RESET_SEED
#error "ROOMLIB_POLL_RESET_SEED must name the copied seed (a RoomFxSeed8 lvalue)"
#endif

#ifndef ROOMLIB_POLL_RESET_POOL_CONTEXT_DECL
#define ROOMLIB_POLL_RESET_POOL_CONTEXT_DECL extern char **ANIM_POOL_AS(RoomM273PoolContext)
#endif
#ifndef ROOMLIB_POLL_RESET_POOL_EXPR
#define ROOMLIB_POLL_RESET_POOL_EXPR ANIM_POOL_AS(RoomM273PoolContext)[2]
#endif
ROOMLIB_POLL_RESET_POOL_CONTEXT_DECL;

int ROOMLIB_POLL_RESET_CALLBACK();

s32 ROOMLIB_POLL_RESET_FUNC(s32 state) {
    RoomFxSeed8 *obj;
    int index;
    u16 size;
    u16 tableValue;
    u16 value2;
    u16 value3;
    u16 value1;

    switch (state) {
    case 0:
        return FieldFx_InitCallbackList(ROOMLIB_POLL_RESET_POOL_EXPR, 8, 4,
                             (FieldFxCallbackListCallback)ROOMLIB_POLL_RESET_CALLBACK);
    case 1:
        if (ROOMLIB_POLL_RESET_FLAG != 0) {
            return 2;
        }
        if (ROOMLIB_POLL_RESET_COUNTER == 0) {
            return 0;
        }
        obj = (RoomFxSeed8 *)FieldFx_AcquireCallbackSlot(ROOMLIB_POLL_RESET_POOL_EXPR);
        if (obj != 0) {
            ROOMLIB_POLL_RESET_COUNTER = 0;
            *obj = ROOMLIB_POLL_RESET_SEED;
        }
        return 0;
    case 2:
        index = g_FieldFxTpageSlot11;
        size = 0x20;
        g_FieldFxSpriteParams.parameter00 = size;
        value2 = 2;
        g_FieldFxSpriteParams.parameter02 = value2;
        g_FieldFxSpriteParams.extent_x = size;
        g_FieldFxSpriteParams.extent_y = size;
        g_FieldFxSpriteParams.extent_x = size;
        g_FieldFxSpriteParams.extent_y = size;
        tableValue = g_FieldFxTpageTable[index];
        value3 = 3;
        g_FieldFxSpriteParams.palette = value3;
        value1 = 1;
        g_FieldFxSpriteParams.parameter06 = value1;
        g_FieldFxSpriteParams.parameter0A = 0;
        g_FieldFxSpriteParams.depth = 0;
        g_FieldFxSpriteParams.tpage = tableValue;
        break;
    }
    return 0;
}

#undef ROOMLIB_POLL_RESET_FUNC
#undef ROOMLIB_POLL_RESET_CALLBACK
#undef ROOMLIB_POLL_RESET_FLAG
#undef ROOMLIB_POLL_RESET_COUNTER
#undef ROOMLIB_POLL_RESET_SEED
#undef ROOMLIB_POLL_RESET_POOL_CONTEXT_DECL
#undef ROOMLIB_POLL_RESET_POOL_EXPR
