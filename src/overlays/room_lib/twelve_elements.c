/*
 * The twelve-element effect: a sprite class whose Init seeds two packet
 * templates, whose effect owner lays out twelve elements, draws each with
 * its floor shadow and ticks their counters. The eight functions, from the
 * two no-ops after RoomLib_CloseTarget to the counter tick in front of the
 * room library, are the same run in thirteen sewer, subway and Chrysler
 * rooms, and the unit's rodata (the three seeds) sits in front of the
 * library's jump tables in every one of them.
 */

#include "common.h"
#include "pe1/room_module.h"
#include "pe1/field_glow_sprite.h"
#include "pe1/field_sprite_state.h"
#include "pe1/field_script_context.h"
#include "pe1/fieldfx_calls.h"
#include "pe1/floor_polygon.h"
#include "pe1/gte.h"
#include "pe1/psyq_libc.h"
#include "pe1/room_floor.h"
#include "pe1/room_fx.h"

#include "room_lib.h"

/* Seeds copied into the stack frames; file-scope so the all-zero rotation
 * stays a rodata copy. */
static const RoomFxSeed8 s_TwelveEffectInitSeed = {{0, 0, 0, 0, 0, 0, 0, 0}};
static const RoomFxSeed8 s_TwelveEffectDrawSeed = {{0x00, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
static const RoomFxVec4 s_TwelveEffectShadowScale = {0x1000, 0x2000, 0x1000, 0};

void RoomLib_TwelveEffectNop0(void) {
}

void RoomLib_TwelveEffectNop1(void) {
}

/* Init: binds the state to the owner's matrix words and sets up the two
 * packet templates the draw pass fills (primary sprite, floor shadow). */
int RoomLib_InitTwelveEffectPackets(void *object, void *unused, void *data) {
    RoomEnt *obj = object;
    RoomLibFxMatrixState *state = data;
    RoomLink *owner;

    FieldFx_SetScriptContext(state);
    state->asset.index = Asset_SearchByKeyType(0xB);
    owner = obj->link;
    state->link = owner;
    memcpy(&state->matrix, owner->p238, sizeof(state->matrix));
    state->reload = 0x28;
    state->cooldown = 0;
    state->hit = 0;
    RoomLib_TwelveEffectPrimaryPacket.code = 4;
    RoomLib_TwelveEffectPrimaryPacket.mode = 1;
    RoomLib_TwelveEffectPrimaryPacket.offset = 0;
    RoomLib_TwelveEffectPrimaryPacket.depth = 0x80;
    RoomLib_TwelveEffectPrimaryPacket.r = 0x80;
    RoomLib_TwelveEffectPrimaryPacket.g = 0x80;
    RoomLib_TwelveEffectPrimaryPacket.b = 0x80;
    RoomLib_TwelveEffectPrimaryPacket.zero = 0;
    RoomLib_TwelveEffectSecondaryPacket.code = 8;
    RoomLib_TwelveEffectSecondaryPacket.mode = 2;
    RoomLib_TwelveEffectSecondaryPacket.offset = 0;
    RoomLib_TwelveEffectSecondaryPacket.depth = 0x30;
    RoomLib_TwelveEffectSecondaryPacket.r = 0x80;
    RoomLib_TwelveEffectSecondaryPacket.g = 0x80;
    RoomLib_TwelveEffectSecondaryPacket.b = 0x80;
    RoomLib_TwelveEffectSecondaryPacket.zero = 0;
    return 0;
}

int RoomLib_TwelveEffectNop3(void *object, void *control, void *data) {
    return 0;
}

int RoomLib_UpdateTwelveElementTimer(void *arg0, void *control, void *data) {
    char *arg1 = control;
    RoomLibFxMatrixState *state = data;

    if (state->cooldown != 0) {
        state->cooldown--;
    }

    if (state->hit == 1) {
        int timer = state->cooldown;

        state->hit = 0;
        if (timer == 0) {
            state->cooldown = state->reload;
            func_800C6C18(arg0);
        }
    }

    if (FieldFx_IsLastSlotHalted() == 1) {
        arg1[1] = 2;
    }
    return 0;
}

/* The element records followed, past the spawn work area, by the handle
 * of the effect the init hands to the field engine. */
typedef struct RoomTwelveEffectWork {
    RoomLibTick12Rec rec;
    u8 padBC[0x60];
    int effectHandle;             /* 0x11C */
} RoomTwelveEffectWork;

typedef struct RoomTwelveEffectStack {
    RoomFxSeed8 seed;
    RoomSpriteMatrix sourceMatrix;
    RoomSpriteMatrix localMatrix;
} RoomTwelveEffectStack;

intptr_t *FieldFx_VarPtr(int index);
int FieldFx_PlaySoundAtMatrixPosition(void *object, int message, void *state);

/* Retail marks this function handwritten; only its COP2 windows remain ASM. */
int RoomLib_InitTwelveElementEffect(void *owner, void *unused, void *state) {
    RoomTwelveEffectStack stack = {0};
    void *effectOwner;
    RoomLibTick12Rec *workState;
    int spacing;
    RoomSpriteMatrix *localMatrix;
    unsigned int i;
    int depth;
    int initialValue;
    RoomLibFxMatrixState *clock;
    RoomSpriteMatrix *sourceMatrix;

    effectOwner = owner;
    workState = (RoomLibTick12Rec *)state;
    clock = FieldFx_GetScriptContext();
    stack.seed = s_TwelveEffectInitSeed;
    stack.sourceMatrix = clock->matrix.matrix;

    workState->repeatCount = 0xC;
    workState->frameStep = *FieldFx_VarPtr(0);
    workState->timerLimit = *FieldFx_VarPtr(1);
    workState->initialPhase = *FieldFx_VarPtr(2);
    spacing = *FieldFx_VarPtr(3);
    *(s16 *)&stack.seed = *FieldFx_VarPtr(4);
    workState->phaseStep = *FieldFx_VarPtr(5);
    workState->phaseLimit = *FieldFx_VarPtr(6);

    localMatrix = &stack.localMatrix;
    localMatrix->m[2][2] = 0x1000;
    localMatrix->m[1][1] = 0x1000;
    localMatrix->m[0][0] = 0x1000;
    localMatrix->t[2] = 0;
    localMatrix->t[1] = 0;
    localMatrix->t[0] = 0;
    localMatrix->m[2][1] = 0;
    localMatrix->m[2][0] = 0;
    localMatrix->m[1][2] = 0;
    localMatrix->m[1][0] = 0;
    localMatrix->m[0][2] = 0;
    localMatrix->m[0][1] = 0;
    RotMatrix((GteShortVector *)&stack.seed, (GteMatrix *)localMatrix);

    sourceMatrix = &stack.sourceMatrix;
    gte_ldrotmatrix(sourceMatrix);
    gte_ldrtir12_matrix_column(&localMatrix->m[0][0]);
    gte_stir123_column(sourceMatrix);
    gte_ldrtir12_matrix_column(&localMatrix->m[0][1]);
    gte_stir123_column_at(&stack.sourceMatrix.m[0][1]);
    gte_ldrtir12_matrix_column(&localMatrix->m[0][2]);
    gte_stir123_column_at(&stack.sourceMatrix.m[0][2]);
    gte_ldtransmatrix(sourceMatrix);
    gte_ldv0_word3_at(localMatrix->t);
    gte_cop2_hazard_slot();
    gte_cop2_hazard_slot();
    gte_rtv0tr_sf0();
    gte_swc2_9_0(stack.sourceMatrix.t);
    gte_swc2_10_4(stack.sourceMatrix.t);
    gte_swc2_11_8(stack.sourceMatrix.t);

    workState->matrix = stack.sourceMatrix;
    i = 0;
    initialValue = 0x64;
    depth = 0;
    for (; i < 12; i++) {
        workState->state[i] = 0;
        workState->timer[i] = 0;
        workState->phase[i] = initialValue;
        workState->position[i].z = depth;
        depth += spacing;
        workState->position[i].x = 0;
        workState->position[i].y = 0;
    }
    ((RoomTwelveEffectWork *)workState)->effectHandle =
        FieldFx_PlaySoundAtMatrixPosition(effectOwner, 0x58F, &stack.sourceMatrix);
    return 0;
}

typedef struct RoomTwelveEffectDrawStack {
    RoomSpriteMatrix primaryMatrix;
    RoomSpriteMatrix secondaryMatrix;
    RoomFxSeed8 seed;
    s16 collision[4];
    GteShortVector point;
    RoomSpriteMatrix localMatrix;
    RoomFxVec4 scale;
    RoomFxVec4 sourceScale;
} RoomTwelveEffectDrawStack;

void ApplyMatrix(RoomSpriteMatrix *matrix, void *position, int *translation);
int FieldFx_IsPointInSpriteQuad(GteShortVector *point, GteMatrix *matrix);

/* Retail marks this function handwritten; only its COP2 windows remain ASM. */
int RoomLib_DrawTwelveElementEffect(void *unused0, void *unused1, void *state) {
    RoomTwelveEffectDrawStack stack = {0};
    RoomLibTick12Rec *workState;
    RoomLibFxMatrixState *clock;
    unsigned int i;
    RoomSpriteMatrix *primaryMatrix;
    RoomFxVec4 *sourceScale;
    RoomSpriteMatrix *secondaryMatrix;
    int pointX, pointY, pointZ;

    workState = (RoomLibTick12Rec *)state;
    clock = FieldFx_GetScriptContext();
    stack.seed = s_TwelveEffectDrawSeed;
    stack.localMatrix.m[2][2] = 0x1000;
    stack.localMatrix.m[1][1] = 0x1000;
    stack.localMatrix.m[0][0] = 0x1000;
    stack.localMatrix.t[2] = 0;
    stack.localMatrix.t[1] = 0;
    stack.localMatrix.t[0] = 0;
    stack.localMatrix.m[2][1] = 0;
    stack.localMatrix.m[2][0] = 0;
    stack.localMatrix.m[1][2] = 0;
    stack.localMatrix.m[1][0] = 0;
    stack.localMatrix.m[0][2] = 0;
    stack.localMatrix.m[0][1] = 0;
    RotMatrix((GteShortVector *)&stack.seed, (GteMatrix *)&stack.localMatrix);

    i = 0;
    primaryMatrix = &stack.primaryMatrix;
    sourceScale = &stack.sourceScale;
    secondaryMatrix = &stack.secondaryMatrix;
    FieldFx_SetSpriteTexturePreset(clock->asset.page);
    FieldFx_SetSpriteCellSize(0x40, 0x40);
    FieldFx_SetSpriteColorMode(0x10);
    FieldFx_SetSpriteBlendMode(2);

    for (; i < 12; i++) {
        if (workState->state[i] == 1) {
            stack.primaryMatrix = workState->matrix;
            ApplyMatrix(primaryMatrix, &workState->position[i],
                          stack.primaryMatrix.t);

            memset(sourceScale, 0, 0x10);
            stack.sourceScale.x = (s16)workState->phase[i];
            stack.sourceScale.y = (s16)workState->phase[i];
            stack.sourceScale.z = (s16)workState->phase[i];
            stack.scale = stack.sourceScale;
            ScaleMatrix((GteMatrix *)primaryMatrix, (GteVector *)&stack.scale);

            stack.primaryMatrix.t[0] += clock->matrix.matrix.t[0];
            stack.primaryMatrix.t[1] += clock->matrix.matrix.t[1];
            stack.primaryMatrix.t[2] += clock->matrix.matrix.t[2];
            FieldFx_InterpolateColorRamp(RoomLib_TwelveEffectTable,
                          workState->timer[i],
                          (unsigned char *)&RoomLib_TwelveEffectPrimaryPacket);

            stack.secondaryMatrix = stack.primaryMatrix;
            stack.sourceScale = s_TwelveEffectShadowScale;
            ScaleMatrix((GteMatrix *)secondaryMatrix, (GteVector *)sourceScale);

            gte_ldrotmatrix(secondaryMatrix);
            gte_ldrtir12_matrix_column(&stack.localMatrix.m[0][0]);
            gte_stir123_column(secondaryMatrix);
            gte_ldrtir12_matrix_column(&stack.localMatrix.m[0][1]);
            gte_stir123_column_at(&stack.secondaryMatrix.m[0][1]);
            gte_ldrtir12_matrix_column(&stack.localMatrix.m[0][2]);
            gte_stir123_column_at(&stack.secondaryMatrix.m[0][2]);
            gte_ldtransmatrix(secondaryMatrix);
            gte_ldv0_word3_at(stack.localMatrix.t);
            gte_cop2_hazard_slot();
            gte_cop2_hazard_slot();
            gte_rtv0tr_sf0();
            gte_swc2_9_0(stack.secondaryMatrix.t);
            gte_swc2_10_4(stack.secondaryMatrix.t);
            gte_swc2_11_8(stack.secondaryMatrix.t);

            FieldFx_DrawGlowSprite((FieldGlowSprite *)&RoomLib_TwelveEffectPrimaryPacket,
                          primaryMatrix, 0);
            stack.secondaryMatrix.t[1] = g_RoomFloorY->y;
            FieldFx_InterpolateColorRamp(RoomLib_TwelveEffectTable,
                          workState->timer[i],
                          (unsigned char *)&RoomLib_TwelveEffectSecondaryPacket);
            FieldFx_DrawGlowSprite((FieldGlowSprite *)&RoomLib_TwelveEffectSecondaryPacket,
                          secondaryMatrix, 0);

            stack.collision[0] = stack.secondaryMatrix.t[0];
            stack.collision[1] = 0;
            stack.collision[2] = stack.secondaryMatrix.t[2];
            if (Geo_PointInPoly((s32)((u32)stack.secondaryMatrix.t[0] << 16),
                              (s32)((u32)stack.secondaryMatrix.t[2] << 16),
                              g_FloorPolygonVertices, g_FloorPolygonVertexCount) == 0) {
                workState->state[i] = -1;
            }

            pointX = (s16)(g_PlayerEntity->pos_x >> 16);
            stack.point.x = pointX;
            pointY = (s16)(g_PlayerEntity->pos_y >> 16);
            stack.point.y = pointY;
            pointZ = (s16)(g_PlayerEntity->pos_z >> 16);
            stack.point.z = pointZ;
            if (FieldFx_IsPointInSpriteQuad(&stack.point, secondaryMatrix) != 0) {
                clock->hit = 1;
            }
        }
    }
    return 0;
}

int RoomLib_TickTwelveElementCounters(void *owner, void *control, void *data) {
    u8 *signal = control;
    RoomLibTick12Rec *rec = data;
    for (unsigned i = 0; i < 12; ++i) {
        int state = rec->state[i];
        if (state != -1) {
            if (state == 1) {
                rec->phase[i] += rec->phaseStep;
                if (rec->phaseLimit < (s16)rec->phase[i])
                    rec->phase[i] = rec->phaseLimit;
            }
            rec->position[i].z -= rec->frameStep;
            if ((s16)rec->position[i].z < 10) rec->state[i] = 1;
        }
        rec->timer[i] = (s8)(u8)(rec->timer[i] + 1);
        if (rec->timer[i] == rec->timerLimit) {
            --rec->repeatCount;
            if ((s16)rec->repeatCount == 0) signal[1] = 2;
        }
    }
    return 0;
}

/* Complete USA M167 descriptor, lists, script and colour track. */
static FieldFxHandler s_TwelveInit[] = {RoomLib_InitTwelveEffectPackets,
    RoomLib_InitTwelveElementEffect, (FieldFxHandler)(intptr_t)-1};
static FieldFxHandler s_TwelveDraw[] = {RoomLib_TwelveEffectNop3,
    RoomLib_DrawTwelveElementEffect, (FieldFxHandler)(intptr_t)-1};
static FieldFxHandler s_TwelveUpdate[] = {RoomLib_UpdateTwelveElementTimer,
    RoomLib_TickTwelveElementCounters, (FieldFxHandler)(intptr_t)-1};
static u16 s_TwelveSpawn[] = {sizeof(RoomLibFxMatrixState), sizeof(RoomTwelveEffectWork), 65535, 0};
static int s_TwelveScript[] = {0x2000d, 0x10000, 0x10001, -1};
RoomFxSpritePacket RoomLib_TwelveEffectPrimaryPacket, RoomLib_TwelveEffectSecondaryPacket;
u8 RoomLib_TwelveEffectTable[60] = {
    0,0,0,10, 128,128,64,5, 128,255,64,5, 255,128,64,5,
    128,128,64,10, 128,128,64,5, 128,255,64,5, 255,128,64,5,
    128,128,64,10, 128,128,64,5, 128,255,64,5, 255,128,64,5,
    128,128,64,10, 0,0,64,255, 0,0,0,255
};
static void TwelvePlant(PmSlotHeader *object) {
    *FieldFx_GetSlot((FieldFxObject *)object) = s_TwelveScript;
}
static int TwelveSpawn(PmSlotHeader *object, int b, int c, intptr_t d, intptr_t e, intptr_t f) {
    FieldFx_Spawn6((intptr_t)object, b, c, d, (int)e, (int)f);
    return 0;
}
static int TwelveRegister(PmSlotHeader *object) {
    if ((unsigned)FieldFx_GetStatus(object) >= 2)
        FieldFx_Register((FieldFxObject *)object, s_TwelveDraw);
    return 0;
}
static int TwelveClose(PmSlotHeader *object) {
    object->state = 4;
    if ((unsigned)FieldFx_GetStatus(object) >= 2) {
        FieldActor *actor = ((FieldFxObjectPrefix *)object)->actor;
        actor->state->core_flags &= 0xC0FFFFFFu;
        *actor->state->action_state = 4;
    }
    return 0;
}
static int TwelveStart(PmSlotHeader *object) {
    int result = -1;
    if ((unsigned)FieldFx_GetStatus(object) >= 2) {
        result = FieldFx_UpdateSlots((FieldFxObject *)object, s_TwelveUpdate);
        result |= FieldFx_RunScript((FieldFxObject *)object, s_TwelveInit, s_TwelveSpawn);
    }
    if (result == -1) TwelveClose(object);
    return 0;
}
PmCommand g_RoomTwelveElementHandlers = {RoomLib_TwelveEffectNop1, TwelvePlant,
    TwelveSpawn, TwelveRegister, TwelveStart, TwelveClose, RoomLib_TwelveEffectNop0};
