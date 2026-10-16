/*
 * The motion particle set, part 3 of 3: the particle motion update and the render pair
 * (init, draw, update).
 *
 * room_m075, room_m080, room_m082, scene_e09 and scene_e10 link the same
 * fifteen functions in the same order, after the room prelude and before
 * the room library, with the same 0x18 bytes of read-only data. The object
 * remains in its three original source units. Native builds use ordinary
 * C11 and pointer-bearing runtime records. Trails, glyphs and lookup codes live
 * in each room's own data (pe1/room_motion_init.h).
 */

#include "common.h"
#include "pe1/field_script_context.h"
#include "pe1/gte.h"
#include "pe1/room_floor.h"
#include "pe1/room_fx.h"
#include "pe1/field_particle_chain.h"
#include "pe1/field_textured_strip.h"
#include "pe1/room_motion_init.h"

#include "motion_particles.h"
#include "render_layouts.h"
#include "room_lib.h"

int RoomEffect_UpdateParticleMotion(void *owner_data, void *control_data, void *data) {
    RoomStatePair *control = control_data; RoomMotionState *state = data;
    unsigned int i;
    RoomStatePair *controlReg = control;
    RoomMotionState *stateReg = state;
    s16 count;
    int amplitude;
    s16 value;


    i = 0;
    if (stateReg->count != 0) {
        do {
            stateReg->lookup[i].id = rand() % 8;
            stateReg->lookup[i].pad = rand() % 8;
            count = stateReg->count;
            i++;
        } while (i < count);
    }

    if (controlReg->h2 >= 31) {
        amplitude = (s16)stateReg->depth;
        value = amplitude;
        if (amplitude >= 5) {
            stateReg->depth = value - 4;
        } else {
            stateReg->depth = 0;
        }

        i = 0;
        if (stateReg->count != 0) {
            do {
                stateReg->particle[i].primaryX += stateReg->velocity[i].primaryX;
                stateReg->particle[i].primaryY += stateReg->velocity[i].primaryY;
                stateReg->particle[i].primaryZ += stateReg->velocity[i].primaryZ;
                stateReg->particle[i].secondaryX += stateReg->velocity[i].secondaryX;
                stateReg->particle[i].secondaryY += stateReg->velocity[i].secondaryY;
                stateReg->particle[i].secondaryZ += stateReg->velocity[i].secondaryZ;
                i++;
            } while (i < stateReg->count);
        }
    }

    if ((u32)((u16)controlReg->h2 - 1) < 31) {
        stateReg->depth += 4;
    }
    if (controlReg->h2 == 65) {
        controlReg->b1 = 2;
    }
    return 0;
}

void FieldFx_BuildParticleChain(FieldChainRecord *record);
int FieldFx_PlaySoundAtMatrixPosition(void *, int, void *);

int RoomEffect_InitParticleRenderPair(void *owner_data, void *control_data, void *data) {
    void *owner = owner_data; RoomParticleRenderPairState *state = data;
    RoomSpriteMatrix secondMatrix;
    RoomSpriteMatrix firstMatrix;
    void *callOwner;
    RoomParticleRenderPairState *workState;
    RoomMotionInitState *root;

    callOwner = owner;
    workState = state;
    root = FieldFx_GetScriptContext();
    memcpy(&firstMatrix, &root->link->render_object.matrices[0], sizeof(firstMatrix));
    memcpy(&secondMatrix, &root->link->render_object.matrices[0], sizeof(secondMatrix));

    firstMatrix.t[0] = root->link->render_object.matrices[9].translation[0];
    firstMatrix.t[1] = g_RoomFloorY->y;
    firstMatrix.t[2] = root->link->render_object.matrices[9].translation[2];
    secondMatrix.t[0] = root->link->render_object.matrices[13].translation[0];
    secondMatrix.t[1] = g_RoomFloorY->y;
    secondMatrix.t[2] = root->link->render_object.matrices[13].translation[2];

    workState->second.count = 10;
    workState->second.links = (FieldChainLink *)g_RoomMotionTrailB;
    workState->second.field06 = 0;
    workState->second.depth = 400;
    workState->second.length = 300;
    workState->second.field0C = 2;
    workState->second.matrix = secondMatrix;
    workState->second.field10 = 32;
    workState->second.field11 = 1;
    workState->second.bend = 1;
    FieldFx_BuildParticleChain(&workState->second);

    workState->first.count = 10;
    workState->first.links = (FieldChainLink *)g_RoomMotionTrailA;
    workState->first.field06 = 0;
    workState->first.depth = 400;
    workState->first.length = 300;
    workState->first.field0C = 2;
    workState->first.matrix = firstMatrix;
    workState->first.field10 = 32;
    workState->first.field11 = 1;
    workState->first.bend = 1;
    FieldFx_BuildParticleChain(&workState->first);

    FieldFx_PlaySoundAtMatrixPosition(callOwner, 0x576, &firstMatrix);
    return 0;
}

int RoomEffect_DrawParticleRenderPair(void *owner_data, void *control_data, void *data) {
    char *sys = data;
    RoomParticleRenderPairState *sysp = (void *)sys;
    RoomMotionInitState *root;

    root = FieldFx_GetScriptContext();
    FieldFx_SetSpriteTexturePreset(root->allocation & 255);
    FieldFx_SetSpriteCellSize(0x10, 0x40);
    FieldFx_SetSpriteColorMode(0x10);
    FieldFx_SetSpriteBlendMode(2);
    FieldFx_DrawTexturedStrip((FieldTexturedStrip *)&sysp->second);
    FieldFx_DrawTexturedStrip((FieldTexturedStrip *)&sysp->first);
    return 0;
}

int RoomEffect_UpdateParticleRenderPair(void *owner_data, void *control_data, void *data) {
    char *state = control_data; char *sys = data;
    char *statep = state;
    RoomParticleRenderPairState *sysp = (void *)sys;
    int *resultp;
    RoomMotionInitState *root;
    FieldActor *global;
    int value0;
    int value1;
    int value2;
    s16 values[3];
    int result;
    int ret;

    root = FieldFx_GetScriptContext();
    resultp = &result;
    global = (FieldActor *)D_8009D254;
    value0 = (s16)((u32)global->pos_x >> 16);
    values[0] = value0;
    value1 = (s16)((u32)global->pos_x >> 16);
    values[1] = value1;
    value2 = (s16)((u32)global->pos_x >> 16);
    values[2] = value2;

    FieldFx_UpdateStripCollision((char *)&sysp->second, values, resultp);
    ret = FieldFx_UpdateStripCollision((char *)&sysp->first, values, resultp);

    if (result == 1) {
        root->field68 = 1;
    }

    if (ret == 1) {
        statep[1] = 2;
    }
    return 0;
}
