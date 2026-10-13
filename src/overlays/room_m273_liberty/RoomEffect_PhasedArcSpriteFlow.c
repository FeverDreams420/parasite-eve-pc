#include "common.h"
#include "pe1/field_sprite_draw.h"
#include "pe1/fieldfx_callback_list.h"
#include "pe1/fieldfx_task_pool.h"
#include "pe1/gte_sine_table.h"
#include "pe1/scene_fx.h"

#include "room_m273_arc.h"
#include "room_m273_effects.h"
#include "room_m273_sweep.h"
#include "RoomEffect_PhasedArcSpriteFlow_externs.h"
#include "RoomEffect_PhasedArcSpriteFlow_externs_1.h"
#include "RoomEffect_PhasedArcSpriteFlow_externs_2.h"
#include "RoomEffect_PhasedArcSpriteFlow_externs_3.h"

typedef RoomM273PointerPoolEffect Effect;
typedef struct { unsigned char unknown[58]; unsigned short value; } Object;

int RoomM273_HandRingBandCallback(int mode,Effect *effect) {
    GteRotation parameters;
    /* Layout only: purpose of these eight original frame bytes is unknown. */
    int stack_pad[2];
    if(mode==1) {
        if(g_FieldFxCurrentAge>=16) return 1;
    } else if(mode==2) {
        int frame=g_FieldFxCurrentAge-1;
        unsigned int sizeOffset=((unsigned int)(u8)frame<<8)&0x3F00;
        unsigned int shadeOffset=((unsigned int)frame<<9)&0x3E00;
        int size=rcossin_tbl[sizeOffset/4].part.sin*2+4096;
        int shade=(short)rcossin_tbl[shadeOffset/4].word>>5;
        parameters.x=0;
        parameters.y=((Object *)ANIM_OWNER_AS(RoomM273EffectStateContext)->state.mode)->value;
        parameters.z=(unsigned int)frame<<8;
        parameters.flags=1;
        FieldFx_DrawRingBand(effect->position,32,96,8,&parameters,size,size,
            (struct RenderColor *)D_8019ACDC,(struct RenderColor *)D_8019ACE0,shade,1);
    }
    return 0;
}

/* Arc spawn-state helper, callback, and pool controller. */

void *RoomM273_SweepConfigure(int unused, int value, int flag) {
    D_8019AF56 = value;
    D_8019AF58 = value;
    value++;
    D_8019AF5C = value * 6;
    D_8019AF5A = flag;
    D_8019AF64 = flag != 0 ? 0x52 : 0xE;

    return &g_RoomM273SweepPoint;
}

s32 RoomM273_SweepArcEmitter(s32 arg0) {
    char *obj;

    if (arg0 != 1) {
        if (arg0 >= 2) {
            return 0;
        }
        if (arg0 != 0) {
            return 0;
        }
        return FieldFx_InitCallbackList(ANIM_POOL_AS(RoomM273PoolContext)->pool, 8, 5,
                             (FieldFxCallbackListCallback)RoomM273_SweepArcCallback);
    }

    if (D_8019AF6B != 0) {
        return 2;
    }
    if ((g_FieldFxCurrentAge & 3) != 0) {
        return 0;
    }

    obj = FieldFx_AcquireCallbackSlot(ANIM_POOL_AS(RoomM273PoolContext)->pool);
    if (obj != 0) {
        *(u16 *)(obj + 0) = g_RoomM273SweepPoint.x;
        *(u16 *)(obj + 2) = D_8019AEFE;
        *(u16 *)(obj + 4) = D_8019AF00;
        *(u16 *)(obj + 6) = 0;
    }

    return 0;
}

short *RoomM273_SwayConfigure(void *arg0, short arg1) {
    short *ptr = &D_8019AF9A;

    *ptr = arg1;
    return ptr - 0x17;
}

s32 RoomM273_SweepArcCallback(s32 mode, RoomM273ArcCallbackState *state) {
    s32 day;
    s32 rotationValue;
    s32 packed;
    s32 result;
    RoomM273ArcCallbackState *callState;
    s32 count;
    volatile GteRotation rotation;

    if (mode == 1) {
        if (g_FieldFxCurrentAge >= 0x10) {
            result = 1;
            return result;
        }
        state->value -= state->step;
        state->step++;
    } else {
        if (mode != 2) {
        result = 0;
        return result;
        }
        {
        rotation.x = 0;
        rotation.y = 0;
        rotation.flags = 0;
        callState = state;
        day = g_FieldFxCurrentAge - 1;
        count = 0x10;
        rotationValue = day << 7;
        rotation.z = rotationValue;
        packed = rcossin_tbl[(day << 6) & 0xFC0].word;
        FieldFx_DrawStarFan((GteShortVector *)callState, 0x100, 0x100, count, (GteRotation *)&rotation,
        ((packed << 16) >> 16) + 0x400,
        ((packed << 16) >> 16) + 0x400,
        (struct RenderColor *)g_RoomM273BlackColor, (struct RenderColor *)D_8019AD7C, packed >> 21, 1);
        }
    }
return_zero:
    result = 0;

epilogue:
    return result;
}
