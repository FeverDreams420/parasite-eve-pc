/*
 * The three transform particles: their module class and the particles.
 *
 * The class is the plant, spawn, registration, start and close methods the
 * class table lists in front of the particles. They are the room module
 * class's methods (pe1/room_module.h) under the module's names, so that
 * room_m393, m396 and m401 can link them in front of the falling sprite
 * burst's class, which uses the shared methods. room_m065, m085, m087,
 * m391, m393, m394, m395, m396, m399, m401 and m402 link the class and the
 * particles directly after each other; the script and the lists are each
 * room's own data (pe1/room_three_particle_class.h).
 */

#include "common.h"
#include "pe1/field_glow_sprite.h"
#include "pe1/field_script_context.h"
#include "pe1/field_sprite_state.h"
#include "pe1/fieldfx_calls.h"
#include "pe1/floor_polygon.h"
#include "pe1/gte.h"
#include "pe1/gte_short_vector.h"
#include "pe1/gte_types.h"
#include "pe1/pe_image.h"
#include "pe1/psyq_libc.h"
#include "pe1/room_debris_triplet.h"
#include "pe1/room_effect_state.h"
#include "pe1/room_floor.h"
#include "pe1/room_fx.h"
#include "pe1/room_three_particle_class.h"

#include "room_lib.h"

ROOMLIB_PLANT_TABLE(RoomLib_ThreeParticlePlantScript, g_RoomThreeParticleScript)

ROOMLIB_SPAWN6(RoomLib_ThreeParticleSpawn6)

ROOMLIB_REGISTER_TABLE(RoomLib_ThreeParticleRegister, g_RoomThreeParticleDrawList)

int RoomLib_ThreeParticleStart(void *o) {
    int result;

    if ((unsigned int)FieldFx_GetStatus(o) >= 2) {
        result = FieldFx_UpdateSlots(o, g_RoomThreeParticleUpdateList);
        result |= FieldFx_RunScript(o, g_RoomThreeParticleInitList,
                                g_RoomThreeParticleSpawnLayout);
    } else {
        result = -1;
    }

    if (result == -1) {
        RoomLib_ThreeParticleClose(o);
    }

    return 0;
}

ROOMLIB_CLOSE_TARGET(RoomLib_ThreeParticleClose)

/*
 * Three transform particles: a sprite class whose Init seeds the sprite and
 * floor-shadow packet templates, whose effect owner places three particles,
 * draws them with their shadows and lets them fall as debris. The eight
 * functions, from the two no-ops after RoomLib_ThreeParticleClose to the
 * debris update, are the same run in eleven Central Park and Chrysler
 * rooms; the unit's rodata is the draw pass's rotation seed, the eight
 * bytes after each room's twelve-byte rodata header.
 */

/* Rotation seed of the draw pass, copied into its stack frame. */
static const RoomFxSeed8 s_ThreeParticleSeed = {
    {0x04, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};

/* The effect owner state: the owner's actor, a copy of its view matrix
 * (the translation is where the particles start), the reload timer, the
 * landed flag and the sprite asset whose low byte selects the page. */
typedef struct RoomDebrisClock {
    struct RoomDebrisActor *actor;    /* 0x00 */
    union {
        RoomEffectWords8 words;
        GteMatrix matrix;
    } view;                           /* 0x04 */
    unsigned short reload;            /* 0x24 */
    s16 cooldown;                     /* 0x26 */
    s16 landedFlag;                   /* 0x28 */
    s16 pad2A;
    union {
        int index;
        u8 page;
    } asset;                          /* 0x2C */
} RoomDebrisClock;

/* The owner's actor: its view matrices. */
typedef struct RoomDebrisActor {
    u8 pad000[0x238];
    RoomEffectWords8 *views;          /* 0x238 */
} RoomDebrisActor;

typedef struct RoomDebrisEntity {
    u8 pad00[0x8];
    RoomDebrisActor *actor;           /* 0x08 */
} RoomDebrisEntity;

int RoomLib_ThreeParticleNop0(void) {
    return 0;
}

int RoomLib_ThreeParticleNop1(void) {
    return 0;
}

void RoomLib_InitThreeParticlePackets(RoomDebrisEntity *arg0, void *arg1, RoomDebrisClock *state) {
    RoomEffectWords8 *src;
    RoomDebrisActor *table;

    FieldFx_SetScriptContext(state);
    state->asset.index = Asset_SearchByKeyType(0xA);
    table = arg0->actor;
    state->actor = table;
    src = table->views;
    state->view.words = *src;
    state->cooldown = 0;
    state->landedFlag = 0;
    state->reload = *FieldFx_VarPtr(6);

    RoomLib_ThreeParticlePacket.offset = -0x64;
    RoomLib_ThreeParticleFloorPacket.code = 4;
    RoomLib_ThreeParticleFloorPacket.mode = 1;
    RoomLib_ThreeParticlePacket.code = 0;
    RoomLib_ThreeParticlePacket.mode = 0;
    RoomLib_ThreeParticlePacket.depth = 0x80;
    RoomLib_ThreeParticlePacket.r = 0x80;
    RoomLib_ThreeParticlePacket.g = 0x80;
    RoomLib_ThreeParticlePacket.b = 0x80;
    RoomLib_ThreeParticlePacket.zero = 0;
    RoomLib_ThreeParticleFloorPacket.offset = 0x32;
    RoomLib_ThreeParticleFloorPacket.depth = 0x80;
    RoomLib_ThreeParticleFloorPacket.r = 0x80;
    RoomLib_ThreeParticleFloorPacket.g = 0x80;
    RoomLib_ThreeParticleFloorPacket.b = 0x80;
    RoomLib_ThreeParticleFloorPacket.zero = 0;
}

void RoomLib_ThreeParticleNop3(void) {
}

void RoomLib_UpdateThreeParticleTimer(int arg0, char *arg1, char *arg2) {
    RoomDebrisClock *state = (RoomDebrisClock *)arg2;

    if (state->cooldown != 0) {
        state->cooldown--;
    }

    if (state->landedFlag == 1) {
        int timer = state->cooldown;

        state->landedFlag = 0;
        if (timer == 0) {
            state->cooldown = state->reload;
            func_800C6C18(arg0);
        }
    }

    if (FieldFx_IsLastSlotHalted() == 1) {
        arg1[1] = 2;
    }
}

typedef GteShortVector RoomInitThreeVec;

typedef struct RoomInitThreeStack {
    RoomInitThreeVec projectionSeed;
    RoomInitThreeVec firstSeed;
    RoomInitThreeVec secondSeed;
    RoomSpriteMatrix matrices[2];
} RoomInitThreeStack;

int *FieldFx_RegPtr(int index);
int FieldFx_PlaySoundAtMatrixPosition(void *object, int message, void *state);

/* Retail marks this function handwritten; only its COP2 windows remain ASM. */
void RoomLib_InitThreeTransformParticles(void *owner, void *unused,
                                         RoomDebrisTriplet *state) {
    RoomInitThreeStack stack;
    RoomSpriteMatrix *firstMatrix;
    RoomSpriteMatrix *secondMatrix;
    RoomInitThreeVec *seed;
    void *callOwner;
    RoomDebrisClock *clock;
    int i;
    RoomSpriteMatrix *projectionMatrix;
    int parameterIndex;
    RoomDebrisClock *clockResult;
    intptr_t *value;
    RoomDebrisTriplet *particles;
    int randomValue;

    callOwner = owner;
    particles = state;
    clockResult = FieldFx_GetScriptContext();
    parameterIndex = 3;
    clock = clockResult;
        stack.firstSeed.x = 0;
    value = FieldFx_VarPtr(parameterIndex);
    stack.firstSeed.y = -*value;
    stack.firstSeed.z = 0;
    seed = &stack.firstSeed;
    firstMatrix = &stack.matrices[0];
    RotMatrixYXZ(seed, firstMatrix);

    parameterIndex = 3;
    stack.secondSeed.x = 0;
    value = FieldFx_VarPtr(parameterIndex);
    stack.secondSeed.y = *value;
    stack.secondSeed.z = 0;
    seed = &stack.secondSeed;
    secondMatrix = &stack.matrices[1];
    RotMatrixYXZ(seed, secondMatrix);

    gte_ldrotmatrix(firstMatrix);
    gte_ldrtir12_matrix_column(&clock->view.matrix.m[0][0]);
    gte_stir123_column(firstMatrix);
    gte_ldrtir12_matrix_column(&clock->view.matrix.m[0][1]);
    gte_stir123_column_at(&stack.matrices[0].m[0][1]);
    gte_ldrtir12_matrix_column(&clock->view.matrix.m[0][2]);
    gte_stir123_column_at(&stack.matrices[0].m[0][2]);
    gte_ldtransmatrix(firstMatrix);
    {
        s32 *translation = clock->view.matrix.t;
        gte_ldv0_word3(translation);
        gte_cop2_hazard_slot();
        gte_cop2_hazard_slot();
        gte_rtv0tr_sf0();
        gte_swc2_9_0(stack.matrices[0].t);
        gte_swc2_10_4(stack.matrices[0].t);
        gte_swc2_11_8(stack.matrices[0].t);

        gte_ldrotmatrix(secondMatrix);
        gte_ldrtir12_matrix_column(&clock->view.matrix.m[0][0]);
        gte_stir123_column(secondMatrix);
        gte_ldrtir12_matrix_column(&clock->view.matrix.m[0][1]);
        gte_stir123_column_at(&stack.matrices[1].m[0][1]);
        gte_ldrtir12_matrix_column(&clock->view.matrix.m[0][2]);
        gte_stir123_column_at(&stack.matrices[1].m[0][2]);
        gte_ldtransmatrix(secondMatrix);
        gte_ldv0_word3(translation);
        gte_cop2_hazard_slot();
        gte_cop2_hazard_slot();
        gte_rtv0tr_sf0();
        gte_swc2_9_0(stack.matrices[1].t);
        gte_swc2_10_4(stack.matrices[1].t);
        gte_swc2_11_8(stack.matrices[1].t);
    }

    i = 0;
    for (; i < 3; i++) {
        randomValue = rand();
        particles->size[i] = 0;
        particles->countdown[i] = (randomValue % 10) + 1;
        particles->life[i] = *FieldFx_VarPtr(2);
        particles->position[i].x = clock->view.matrix.t[0] << 16;
        particles->position[i].y = clock->view.matrix.t[1] << 16;
        particles->position[i].z = clock->view.matrix.t[2] << 16;
        particles->rotation[i].x = 0;
        particles->rotation[i].y = 0;
        particles->rotation[i].z = 0;
        particles->swing[i] = 0;
        particles->mode[i] = 1;
        particles->active[i] = 0;
        particles->landed[i] = 0;
    }

    particles->remaining = 3;
    particles->speed = *FieldFx_VarPtr(0);
    particles->acceleration = *FieldFx_VarPtr(1);
    particles->swingLimit = *FieldFx_VarPtr(5);
    particles->keepAlive = *FieldFx_VarPtr(4);
    particles->reserved08 = *FieldFx_RegPtr(1);

    stack.projectionSeed.x = 0;
    stack.projectionSeed.y = 0;
    stack.projectionSeed.z = 0x1000;
    ApplyMatrixSV(&stack.matrices[0], &stack.projectionSeed,
                  (GteShortVector *)&particles->velocity[0]);
    projectionMatrix = &clock->view.matrix;
    ApplyMatrixSV(projectionMatrix, &stack.projectionSeed,
                  (GteShortVector *)&particles->velocity[1]);
    ApplyMatrixSV(&stack.matrices[1], &stack.projectionSeed,
                  (GteShortVector *)&particles->velocity[2]);
    FieldFx_PlaySoundAtMatrixPosition(callOwner, 0x573, projectionMatrix);
}

typedef GteShortVector RoomDrawThreeVec;

typedef struct RoomDrawThreeStack {
    RoomSpriteMatrix primaryMatrix;
    RoomSpriteMatrix secondaryMatrix;
    RoomFxSeed8 seed;
    RoomDrawThreeVec position;
    RoomFxVec4 scale;
    RoomFxVec4 sourceScale;
    RoomFxVec4 floorScale;
    RoomFxVec4 sourceFloorScale;
    RoomFxControl *args;
} RoomDrawThreeStack;

void FieldFx_SetSpriteColorMode(int depth);

void RoomLib_DrawThreeTransformParticles(void *unused, RoomFxControl *args, RoomDebrisTriplet *state) {
    RoomDrawThreeStack stack;
    unsigned int i;
    RoomFxVec4 *sourceScale;
    RoomSpriteMatrix *secondaryMatrix;
    s16 scaleValue;
    void *arg0;
    RoomSpriteMatrix *inputArg;
    int frame;
    RoomFxControl *callArgs;
    RoomDebrisClock *clock;
    RoomDebrisTriplet *drawState;
    int computedScale;
    int alpha;
    int baseScale;

    drawState = state;
    stack.args = args;
    clock = FieldFx_GetScriptContext();
    stack.seed = s_ThreeParticleSeed;
    i = 0;
    sourceScale = &stack.sourceScale;
    secondaryMatrix = &stack.secondaryMatrix;
    FieldFx_SetSpriteTexturePreset(clock->asset.page);
    FieldFx_SetSpriteColorMode(0x10);

    for (; i < 3; i++) {
        if (drawState->active[i] >= 0) {
            arg0 = &drawState->rotation[i];
            RotMatrixYXZ(arg0, &stack.primaryMatrix);
            memset(sourceScale, 0, 0x10);
            stack.sourceScale.x = drawState->swing[i] >> 1;
            stack.sourceScale.y = drawState->swing[i] >> 1;
            stack.sourceScale.z = drawState->swing[i] >> 1;
            stack.scale = stack.sourceScale;
            ScaleMatrix(&stack.primaryMatrix, &stack.scale);

            stack.primaryMatrix.t[0] = (drawState->position[i].x >> 16) +
                                       drawState->jitter[0];
            stack.primaryMatrix.t[1] = (drawState->position[i].y >> 16) +
                                       drawState->jitter[1];
            stack.primaryMatrix.t[2] = (drawState->position[i].z >> 16) +
                                       drawState->jitter[2];
            callArgs = stack.args;
            alpha = drawState->size[i];
            arg0 = RoomLib_ThreeParticleTable;
            RoomLib_ThreeParticleAlpha = alpha;
            frame = callArgs->frame;
            FieldFx_InterpolateColorRamp(arg0, frame, (unsigned char *)&RoomLib_ThreeParticlePacket);
            FieldFx_SetSpriteCellSize(0x40, 0x40);
            FieldFx_SetSpriteBlendMode(2);
            FieldFx_DrawGlowSprite((FieldGlowSprite *)&RoomLib_ThreeParticlePacket,
                          &stack.primaryMatrix, 1);

            if (drawState->reserved08 == 1) {
                callArgs = stack.args;
                computedScale = rsin(callArgs->frame << 7) >> 3;
                baseScale = drawState->phase[i];
                computedScale += baseScale;
                scaleValue = computedScale;
                if (drawState->swing[i] < (s16)computedScale) {
                    scaleValue = drawState->swing[i];
                }

                stack.position = drawState->rotation[i];
                arg0 = &stack.position;
                inputArg = &stack.primaryMatrix;
                scaleValue = scaleValue >> 1;
                stack.position.z += drawState->life[i] << 4;
                RotMatrixYXZ(arg0, inputArg);
                memset(sourceScale, 0, 0x10);
                stack.sourceScale.x = scaleValue;
                stack.sourceScale.y = scaleValue;
                stack.sourceScale.z = scaleValue;
                ScaleMatrix(&stack.primaryMatrix, sourceScale);
                stack.primaryMatrix.t[0] = (drawState->position[i].x >> 16) +
                                           drawState->jitter[0];
                stack.primaryMatrix.t[1] = (drawState->position[i].y >> 16) +
                                           drawState->jitter[1];
                stack.primaryMatrix.t[2] = (drawState->position[i].z >> 16) +
                                           drawState->jitter[2];
                FieldFx_DrawGlowSprite((FieldGlowSprite *)&RoomLib_ThreeParticlePacket,
                              &stack.primaryMatrix, 1);

                RotMatrix(&stack.seed.vector, secondaryMatrix);
                memset(&stack.sourceFloorScale, 0, 0x10);
                stack.sourceFloorScale.x = drawState->swing[i];
                stack.sourceFloorScale.y = drawState->swing[i];
                stack.sourceFloorScale.z = drawState->swing[i];
                stack.floorScale = stack.sourceFloorScale;
                ScaleMatrix(secondaryMatrix, &stack.floorScale);
                computedScale = (drawState->position[i].x >> 16);
                baseScale = g_RoomFloorY->y;
                stack.secondaryMatrix.t[0] = computedScale;
                stack.secondaryMatrix.t[1] = baseScale;
                computedScale = (drawState->position[i].z >> 16);

                stack.secondaryMatrix.t[2] = computedScale;
                alpha = drawState->size[i];
                RoomLib_ThreeParticleFloorPacket.depth = (unsigned int)alpha >> 1;
                FieldFx_SetSpriteCellSize(0x20, 0x20);
                FieldFx_SetSpriteBlendMode(3);
                FieldFx_DrawGlowSprite((FieldGlowSprite *)&RoomLib_ThreeParticleFloorPacket,
                              secondaryMatrix, 0);
            }
        }
    }
}

typedef struct RoomDebrisOwner {
    u8 reserved00;
    u8 phase;                     /* 0x01 */
    s16 angle;                    /* 0x02 */
} RoomDebrisOwner;

PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomDebrisTriplet, position) == 0x3C,
                  room_debris_triplet_position_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomDebrisTriplet, velocity) == 0x9C,
                  room_debris_triplet_velocity_offset);

void RoomLib_UpdateDebrisTriplet(void *unused, RoomDebrisOwner *owner,
                                        RoomDebrisTriplet *state) {
    s16 point[4];
    RoomDebrisClock *clock;
    unsigned int i;
    int step;

    clock = FieldFx_GetScriptContext();
    for (i = 0; i < 3; i++) {
        if (state->countdown[i] != 0) {
            state->countdown[i]--;
            if (state->countdown[i] == 0) state->active[i] = 0;
        }
        if (state->active[i] == 0) {
            if (state->size[i] < 0x80) {
                state->size[i] += 4;
            } else {
                state->active[i] = 1;
            }
        }
        if (state->mode[i] == 1) {
            state->position[i].x += (state->velocity[i].x * state->speed) >> 4;
            state->position[i].y += (state->velocity[i].y * state->speed) >> 4;
            state->position[i].z += (state->velocity[i].z * state->speed) >> 4;
        }
        if (state->swing[i] < state->swingLimit) {
            state->swing[i] += rand() % 80 + 20;
            state->phase[i] += rand() % 40 + 40;
            state->position[i].y += -0xA0000;
        }
        state->swing[i] += rsin(owner->angle << 5) >> 7;
        if (state->life[i] < 0x14) {
            if (state->size[i] >= 7) state->size[i] -= 6;
        }
        if (state->landed[i] == 0) {
            if (!Geo_PointInPoly((state->position[i].x >> 16) << 16,
                               (state->position[i].z >> 16) << 16,
                               g_FloorPolygonVertices, g_FloorPolygonVertexCount)) {
                state->landed[i] = 1;
                state->mode[i] = 0;
                if (state->keepAlive == 0) state->life[i] = 0x14;
            }
        }
        state->life[i]--;
        if (state->life[i] == 0) {
            state->active[i] = -1;
            state->remaining--;
        }
        point[0] = (state->position[i].x >> 16);
        point[1] = (state->position[i].y >> 16);
        point[2] = (state->position[i].z >> 16);
        state->jitter[0] = rand() % 11 - 5;
        state->jitter[1] = rand() % 11 - 5;
        state->jitter[2] = rand() % 11 - 5;
        if (FieldFx_IsPlayerWithinRadius(point, state->swing[i] >> 4)) {
            if (state->landed[i] == 0) {
                clock->landedFlag = 1;
                state->life[i] = 0x14;
                state->mode[i] = 0;
            }
            state->landed[i] = 1;
        }
    }
    state->speed += state->acceleration;
    if (state->speed < 0) state->speed = 0;
    if (state->remaining == 0) owner->phase = 2;
}
