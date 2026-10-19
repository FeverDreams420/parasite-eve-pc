/*
 * Timed spawns: the sprite callback and the controller that spawns it every
 * sixth frame and flashes the scene on frame 7. The two functions follow
 * each other in eight hospital and Chrysler rooms; the unit's rodata is the
 * callback's seed initialiser.
 */

#include "common.h"
#include "pe1/battle.h"
#include "pe1/room_floor_walker.h"
#include "pe1/akao.h"
#include "pe1/field_owner_actor.h"
#include "pe1/field_sprite_draw.h"
#include "pe1/field_sprite_params.h"
#include "pe1/field_texture_tables.h"
#include "pe1/fieldfx_callback_list.h"
#include "pe1/fieldfx_task_pool.h"
#include "pe1/gte.h"
#include "pe1/geom_state.h"
#include "pe1/gte_types.h"
#include "pe1/player_entity.h"
#include "pe1/render_matrix_slot.h"
#include "pe1/room_floor.h"
#include "pe1/room_fx.h"
#include "pe1/scene_archive.h"

#include "room_lib.h"

int RoomLib_UpdateTimedRender(int mode,void *data,intptr_t argument) {
    RoomFxTimedRenderState *state=data;
    RoomFxSeed8 work;
    RoomFxSeed8 seed = {{0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00}};
    RoomFxSeed8 *seedPtr;
    RoomSpriteMatrix **matrixSlot;
    int angle;

    seedPtr = &seed;
    switch (mode) {
    case 1:
        if (state->disabled != 0) {
            return 0;
        }
        state->frame++;
        if ((short)state->frame < 24) {
            break;
        }
        return 1;
    case 2:
        if (state->disabled != 0) {
            return 0;
        }
        matrixSlot = &g_ViewMatrixSlot.value;
        gte_ldrotmatrix(*matrixSlot);
        gte_ldtransmatrix(*matrixSlot);
        FieldFx_SampleColorTrack(&RoomLib_TimedRenderTable, &work,
                      (short)state->frame);
        angle = rsin((s32)((u32)g_FieldFxCurrentAge<<10) / 24);
        FieldFx_DrawRingBand((GteShortVector *)state, 0x578, 0x640, 0x14, (GteRotation *)seedPtr, angle, angle, 0,
                      (struct RenderColor *)&work, 0x80, 1);
        return 0;
    }
ret0:
    return 0;
ret1:
    return 1;
disabled:
    return 0;
}

typedef struct RoomTimedSpawnPosition {
    u16 x;
    u16 y;
    u16 z;
} RoomTimedSpawnPosition;

typedef struct RoomTimedSpawnState {
    s16 x;
    s16 y;
    RoomTimedSpawnPosition position;  /* 0x04 */
    u16 reserved0a;
} RoomTimedSpawnState;

int RoomLib_ControlTimedSpawn(int mode,void *data,intptr_t argument) {
    RoomTimedSpawnState *state=data;
    RoomFxTimedRenderState *actor;
    int tick;
    int value;
    u16 ground;

    switch (mode) {
    case 0:

        FieldFx_GetActorPosition((char *)g_FieldFxOwner->actor, 0, (s16 *)&state->position);
        state->x = 0;
        state->y = 0;

        if (g_FieldFxTaskContext->flags != 0) {
            FieldActor *p=g_FieldFxOwner->actor;
            if(p && p->state) {
                u8 *status=p->state->action_state;
                if(status && *status == 1) {
                    *status = 2;
                }
            }
        }

        if (g_LoadedSceneAssetBlock[0] != 0) {
            value = FieldFx_GetOwnerProgress();
            Akao_SendTableCommand(g_LoadedSceneAssetBlock[0], 0x588, value, 0x80, 0x7F);
        }
        return FieldFx_InitCallbackList(g_FieldFxActiveTaskSlot->end, 0xC, 8,
                             RoomLib_UpdateTimedRender);

        case 1:
    tick = g_FieldFxCurrentAge;
    if (tick < 0x20 && tick % 6 == 0) {
        actor = FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
        if (actor != 0) {
            actor->x = state->position.x;
            actor->y = state->position.y;
            actor->z = state->position.z;
            ground = g_RoomFloorY->y;
            actor->disabled = 0;
            actor->frame = 0;
            actor->y = ground;
        }
    }

    if (g_FieldFxCurrentAge == 7 && g_FieldFxTaskContext->flags != 0) {
        FieldActor *owner=g_FieldFxOwner->actor;
        if(owner && owner->state && (owner->state->core_flags&0x3F000000)==0x01000000) {
            if(g_PlayerEntity && g_PlayerEntity->state)((Combatant *)g_PlayerEntity->state)->stateFlags|=0x4000;
            owner->state->core_flags=(owner->state->core_flags&0xC0FFFFFF)|0xA1000000u;
        }
    }
        if (g_FieldFxCurrentAge >= 8) return 2;
    break;
    case 2:
 {
        int index = g_FieldFxTpageSlot3;
        u16 tpage;
        g_FieldFxSpriteParams.parameter00 = 0x20;
        g_FieldFxSpriteParams.parameter02 = 2;
        g_FieldFxSpriteParams.extent_x = 0x20;
        g_FieldFxSpriteParams.extent_y = 0x20;
        tpage = g_FieldFxTpageTable[index];
        g_FieldFxSpriteParams.palette = 3;
        g_FieldFxSpriteParams.parameter06 = 0;
        g_FieldFxSpriteParams.parameter0A = 0;
        g_FieldFxSpriteParams.depth = 0;
        g_FieldFxSpriteParams.tpage = tpage;
    }

    }
    return 0;
}

u8 RoomLib_TimedRenderTable[32]={0,0,0,0,0,0,0,0,40,120,30,12,0,0,0,0,128,40,12,12,0,0,0,0,0,0,0,0,0,0,0,0};
u16 g_RoomTimedSpawnScript[16]={5,0,0,1,6,102,4,48,0,2,0,0,0,0,0,0};
FieldFxTaskProgram g_RoomTimedSpawnProgram={{{RoomLib_ControlTimedSpawn,NULL,NULL,NULL,NULL,NULL,NULL,NULL},{12,0,0,0,0,0,0,0}},NULL,g_RoomTimedSpawnScript};
