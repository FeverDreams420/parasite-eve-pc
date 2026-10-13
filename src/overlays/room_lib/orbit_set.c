/* CC1_FLAGS: -fno-strength-reduce */

/*
 * The orbit effect set: its module class and the effects it runs.
 *
 * The class is the seven methods the class table lists in front of the
 * set. The registration and the start method act only while the field
 * engine runs the object (status 3). room_m187 links the class as its only
 * module; room_m186, m385, m388 and m389 link it after the twelve-element
 * effect's module. The script and the lists are each room's own data
 * (pe1/room_orbit_set.h).
 *
 * The class and the set follow each other in all five rooms, with the
 * set's seeds as the unit's only rodata, so they are one unit. The set's
 * loops need -fno-strength-reduce; the class methods compile the same
 * either way.
 */

#include "pe1/field_actor.h"
#include "pe1/room_orbit_set.h"

#include "render_layouts.h"
#include "room_lib.h"

ROOMLIB_RETURN_ZERO(RoomEffect_OrbitSetNop0)

ROOMLIB_PLANT_TABLE(RoomEffect_OrbitSetPlantScript, g_RoomOrbitSetScript)

ROOMLIB_SPAWN6(RoomEffect_OrbitSetSpawn6)

ROOMLIB_REGISTER_TABLE_AT3(RoomEffect_OrbitSetRegister, g_RoomOrbitSetDrawList)

ROOMLIB_START_AT3(RoomEffect_OrbitSetStart, g_RoomOrbitSetUpdateList,
                  g_RoomOrbitSetInitList, g_RoomOrbitSetSpawnLayout,
                  RoomEffect_OrbitSetClose)

ROOMLIB_CLOSE_TARGET(RoomEffect_OrbitSetClose)

ROOMLIB_RETURN_ZERO(RoomEffect_OrbitSetNop6)

/*
 * The orbit effect set: eight particles that orbit outwards from the actor,
 * a burst of eight sprites that fall to the floor over their shadows, and a
 * sprite that drifts away from the actor. Each effect is an init, draw and
 * update handler; the set's own init fills the four sprite records.
 *
 * room_m186, m187, m385, m388 and m389 link these twelve functions in this
 * order, with the same 0x20 bytes of read-only seeds; this unit is that
 * object, compiled into each of them. It starts after the no-op that ends
 * the room prefix. The four sprite records live in each room's own data.
 */

/* Copies the owner's eight words into the state, allocates the set's
 * buffer and fills the four sprite records. The register pins and launders
 * reproduce retail's assignment of the shared constants. */
/* The script object the set belongs to and its owner actor. */
typedef struct RoomOrbitSetEntity {
    u8 pad00[0x8];
    FieldActor *owner;            /* 0x08 */
} RoomOrbitSetEntity;

void RoomEffect_OrbitSetInit(RoomOrbitSetEntity *object, void *unused, RoomOrbitSetState *state) {
    FieldActor *owner;
    void *allocation;
    RenderMatrix *source;
    RenderMatrix *matrixArg;
    register void *callObject asm("$4");
    register int value asm("$2");
    u16 value128a;
    u8 value128b;
    int value32;
    int packetValue;
    register int valueNeg42 asm("$5");

    FieldFx_SetScriptContext(state);
    owner = object->owner;
    state->owner = owner;
    source = owner->render_object.matrices;
    state->matrix = *source;
    allocation = Asset_SearchByKeyType(0x24);

    callObject = object;
    state->asset.allocation = allocation;
    value = -0x29;
    value128a = 0x80;
    value128b = 0x80;
    value32 = 0x20;
    packetValue = 1;
    valueNeg42 = -0x2A;
    PE1_COMPILER_LAUNDER(value128a);
    PE1_COMPILER_LAUNDER(value128b);
    PE1_COMPILER_LAUNDER(value32);
    PE1_COMPILER_LAUNDER(packetValue);
    g_RoomOrbitParticlePacket.offset = value;
    value = 0x60;
    g_RoomOrbitBurstPacket.r = value;
    value = 0x40;
    asm volatile("" : "=r"(state) : "0"(state), "r"(value));
    matrixArg = &state->matrix;
    PE1_COMPILER_USE(matrixArg);
    g_RoomOrbitBurstPacket.offset = valueNeg42;
    g_RoomOrbitShadowPacket.offset = valueNeg42;
    g_RoomOrbitSpritePacket.offset = valueNeg42;
    g_RoomOrbitParticlePacket.depth = value128a;
    g_RoomOrbitBurstPacket.depth = value128a;
    g_RoomOrbitSpritePacket.depth = value128a;
    g_RoomOrbitParticlePacket.code = 0;
    g_RoomOrbitParticlePacket.mode = 0;
    g_RoomOrbitParticlePacket.r = value128b;
    g_RoomOrbitParticlePacket.g = value128b;
    g_RoomOrbitParticlePacket.b = value128b;
    g_RoomOrbitParticlePacket.zero = 0;
    g_RoomOrbitBurstPacket.code = value32;
    g_RoomOrbitBurstPacket.mode = packetValue;
    g_RoomOrbitBurstPacket.g = value;
    g_RoomOrbitBurstPacket.b = value;
    g_RoomOrbitBurstPacket.zero = 0;
    g_RoomOrbitShadowPacket.code = value32;
    g_RoomOrbitShadowPacket.mode = packetValue;
    g_RoomOrbitShadowPacket.depth = 0;
    g_RoomOrbitShadowPacket.r = 0;
    g_RoomOrbitShadowPacket.g = 0;
    g_RoomOrbitShadowPacket.b = 0;
    g_RoomOrbitShadowPacket.zero = 0;
    packetValue = value128b;
    g_RoomOrbitSpritePacket.code = 0;
    g_RoomOrbitSpritePacket.mode = 0;
    g_RoomOrbitSpritePacket.r = packetValue;
    g_RoomOrbitSpritePacket.g = value128b;
    g_RoomOrbitSpritePacket.b = value128b;
    g_RoomOrbitSpritePacket.zero = 0;
    FieldFx_PlaySoundAtMatrixPosition(callObject, 0x592, matrixArg);
}

/* The set itself draws nothing; each effect draws its own sprites. */
void RoomEffect_OrbitSetDraw(void) {
}

void RoomEffect_OrbitSetUpdate(void *arg0, char *event) {
    if (FieldFx_IsLastSlotHalted() == 1) {
        event[1] = 2;
    }
}

void RoomEffect_InitOrbitParticles(void *unused, void *unused2, RoomOrbitParticleState *state) {
    RoomOrbitSetState *clock = FieldFx_GetScriptContext();
    unsigned int i;
    RoomOrbitParticleLaneView *particle;

    i = 0;
    particle = (RoomOrbitParticleLaneView *)state;
    while (i < 8) {
        int angle;

        *(u16 *)&particle->position.x = clock->matrix.translation[0];
        particle->position.y = (u16)g_RoomFloorY->y;
        particle->position.z = clock->matrix.translation[2];

        angle = rand() % 0x1000;
        particle->velocity.x = rsin(angle);
        angle = rand() % 0x1000;
        particle->velocity.z = rcos(angle);
        particle->velocity.angle = -(rand() % 8000 + 4000);
        i++;
        particle = (RoomOrbitParticleLaneView *)(&particle->position + 1);
    }

    state->decay = 0x80;
    state->height = 0x400;
    state->radiusStep = -30;
    state->intensity = 0;
    state->radius = 0x258;
}

void RoomEffect_DrawOrbitParticles(
    void *unused0, void *unused1, RoomOrbitParticleState *fx) {
    RoomSpriteMatrix matrix;
    RoomFxVec4 scratchScale;
    RoomFxVec4 scale;
    RoomOrbitSetState *owner;
    unsigned short *depthSlot;
    short *position;
    unsigned int i;

    owner = FieldFx_GetScriptContext();

    i = 0;
    depthSlot = &g_RoomOrbitParticlePacket.depth;
    FieldFx_SetSpriteTexturePreset(owner->asset.page);
    FieldFx_SetSpriteCellSize(0x20, 0x20);
    FieldFx_SetSpriteColorMode(0x10);
    FieldFx_SetSpriteBlendMode(2);

    matrix.m[2][2] = 0x1000;
    matrix.m[1][1] = 0x1000;
    matrix.m[0][0] = 0x1000;
    matrix.t[2] = 0;
    matrix.t[1] = 0;
    matrix.t[0] = 0;
    matrix.m[2][1] = 0;
    matrix.m[2][0] = 0;
    matrix.m[1][2] = 0;
    matrix.m[1][0] = 0;
    matrix.m[0][2] = 0;
    matrix.m[0][1] = 0;

    memset(&scale, 0, 0x10);
    scale.x = (s16)fx->height;
    scale.y = (s16)fx->height;
    scale.z = 0x400;
    scratchScale = scale;
    ScaleMatrix(&matrix, &scratchScale);

    position = &fx->position[0].x;
    while (i < 8) {
        matrix.t[0] = position[0];
        matrix.t[1] = position[1];
        matrix.t[2] = position[2];
        *depthSlot = fx->decay >> 1;
        *((unsigned char *)depthSlot - 6) = fx->frame << 1;
        FieldFx_DrawGlowSprite((FieldGlowSprite *)((char *)depthSlot - 0xA), &matrix, 1);
        i++;
        position += 4;
    }
}

void RoomEffect_UpdateOrbitParticles(
    void *unused, u8 *control, RoomOrbitParticleState *state) {
    unsigned int i;
    RoomOrbitParticleLaneView *lane;
    u8 intensity;
    u16 height;

    state->radius += state->radiusStep;
    if (state->decay >= 9) {
        state->decay -= 8;
    }

    i = 0;
    lane = (RoomOrbitParticleLaneView *)state;
    while (i < 8) {
        s32 xDelta = (lane->velocity.x * state->radius) >> 16;

        lane->position.y += (s8)(lane->velocity.angle >> 8);
        lane->position.x += xDelta;
        lane->position.z += (lane->velocity.z * state->radius) >> 16;
        lane->velocity.angle += 0x258;
        i++;
        lane = (RoomOrbitParticleLaneView *)(&lane->position + 1);
    }

    height = state->height + 0x46;
    intensity = state->intensity + 6;
    state->intensity = intensity;
    intensity >>= 4;
    state->height = height;
    state->frame = intensity + 1;
    if (intensity == 7) {
        state->frame = 0;
        control[1] = 2;
    }
}

void RoomEffect_InitOrbitBurst(
    void *unused0, void *unused1, RoomOrbitBurstState *state) {
    RoomLibOrbitView *view;
    short *particle;
    unsigned int i;

    view = FieldFx_GetScriptContext();
    i = 0;
    particle = (short *)state;
    while (i < 8) {
        particle[0] = view->baseX;
        particle[1] = g_RoomFloorY->y;
        particle[2] = view->baseZ;
        particle[0x20] = rand() % 0x2000 - 0x1000;
        particle[0x22] = rand() % 0x2000 - 0x1000;
        particle[0x21] = -(rand() % 8000 + 8000);
        particle[0x40] = 0;
        particle[0x41] = 0;
        particle[0x42] = rand() % 0x1000;
        state->active[i] = 1;
        state->frame[i] = rand() % 3;
        i++;
        particle += 4;
    }

    state->depth = 0x80;
    state->scale = 0x200;
    state->phaseStep = 0xC8;
    state->count = 8;
}

void RoomEffect_DrawOrbitBurst(
    void *unused0, void *unused1, RoomOrbitBurstState *state) {
    RoomSpriteMatrix matrix;
    RoomFxVec4 scratchScale;
    RoomFxVec4 scale;
    RoomOrbitSetState *owner;
    RoomOrbitBurstVector *position;
    unsigned char *configA;
    unsigned char *configB;
    unsigned int i;

    owner = FieldFx_GetScriptContext();

    i = 0;
    configA = &g_RoomOrbitBurstPacket.code;
    configB = &g_RoomOrbitShadowPacket.code;
    PE1_COMPILER_USE(configB);
    position = state->position;
    FieldFx_SetSpriteTexturePreset(owner->asset.page);
    FieldFx_SetSpriteCellSize(0x20, 0x20);
    FieldFx_SetSpriteColorMode(0x10);
    FieldFx_SetSpriteBlendMode(2);

    while (i < 8) {
        if (state->active[i] == 1) {
            RotMatrix(&state->secondary[i], &matrix);
            memset(&scale, 0, 0x10);
            scale.x = state->scale;
            scale.y = state->scale;
            scale.z = 0x400;
            scratchScale = scale;
            ScaleMatrix(&matrix, &scratchScale);

            matrix.t[0] = position->x;
            matrix.t[1] = position->y;
            matrix.t[2] = position->z;
            *configA = state->frame[i] * 2 + 0x20;
            FieldFx_DrawGlowSprite((FieldGlowSprite *)(configA - 4), &matrix, 1);

            matrix.t[0] = position->x;
            matrix.t[1] = g_RoomFloorY->y;
            matrix.t[2] = position->z;
            *configB = state->frame[i] * 2 + 0x20;
            FieldFx_DrawGlowSprite((FieldGlowSprite *)(configB - 4), &matrix, 0);
        }
        i++;
        position++;
    }
}

/* One burst sprite's lane through the parallel position, velocity and
 * spin arrays, read as unsigned halfwords. */
typedef struct RoomOrbitBurstLaneView {
    unsigned short position[4];   /* 0x00 */
    unsigned char pad08[0x38];
    unsigned short velocity[4];   /* 0x40 */
    unsigned char pad48[0x38];
    unsigned short secondary[4];  /* 0x80 */
} RoomOrbitBurstLaneView;

void RoomEffect_UpdateOrbitBurst(
    void *unused, unsigned char *control, RoomOrbitBurstState *state) {
    unsigned char *particle;
    unsigned int i;

    if ((short)state->depth >= 9) {
        state->depth -= 8;
    }

    i = 0;
    particle = (unsigned char *)state;
    while (i < 8) {
        if (state->active[i] == 1) {
            RoomOrbitBurstLaneView *lane = (RoomOrbitBurstLaneView *)particle;
            int xStep =
                (signed char)(lane->velocity[0] >> 8);
            int yStep =
                (signed char)(lane->velocity[1] >> 8);
            int zStep =
                (signed char)(lane->velocity[2] >> 8);

            lane->position[0] += xStep;
            lane->position[1] += yStep;
            lane->position[2] += zStep;
            lane->velocity[1] += 0x190;
            lane->secondary[2] +=
                rand() % 0x200;
            if ((short)lane->position[1] > g_RoomFloorY->y) {
                state->count--;
                state->active[i] = 0;
            }
        }
        i++;
        particle += 8;
    }

    if (state->count == 0) {
        control[1] = 2;
    }
}

/* Sprite and floor-point offsets from the actor, for the two sides. */
static const RoomFxSeed8 s_OrbitSpriteSeed0 = {
    { 0xB8, 0x0B, 0x00, 0x00, 0x70, 0x17, 0x00, 0x00 }
};
static const RoomFxSeed8 s_OrbitSpriteSeed1 = {
    { 0xC8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
};
static const RoomFxSeed8 s_OrbitSpriteSeed2 = {
    { 0x48, 0xF4, 0x00, 0x00, 0x70, 0x17, 0x00, 0x00 }
};
static const RoomFxSeed8 s_OrbitSpriteSeed3 = {
    { 0x38, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
};

void RoomEffect_InitOrbitSprite(
    void *unused0, void *unused1, RoomOrbitSpriteFxParams *state) {
    RoomFxSeed8 seed0;
    RoomFxSeed8 seed1;
    RoomFxSeed8 seed2;
    RoomFxSeed8 seed3;
    unsigned short position[4];
    RoomLibOrbitView *view;

    view = FieldFx_GetScriptContext();
    seed0 = s_OrbitSpriteSeed0;
    seed1 = s_OrbitSpriteSeed1;
    seed2 = s_OrbitSpriteSeed2;
    seed3 = s_OrbitSpriteSeed3;

    if (*FieldFx_RegPtr(1) == 0) {
        ApplyMatrixSV((GteMatrix *)view->transform, &seed0.vector,
                      &state->velocity);
        ApplyMatrixSV((GteMatrix *)view->transform, &seed1.vector,
                      (GteShortVector *)position);
    } else {
        ApplyMatrixSV((GteMatrix *)view->transform, &seed2.vector,
                      &state->velocity);
        ApplyMatrixSV((GteMatrix *)view->transform, &seed3.vector,
                      (GteShortVector *)position);
    }

    state->x = position[0] + view->baseX;
    state->y = g_RoomFloorY->y;
    state->z = position[2] + view->baseZ;
    state->depth = 0x80;
    state->scale = 0x800;
    state->phase = 0x258;
    state->alpha = 0;
}

void RoomEffect_DrawOrbitSprite(
    void *unused, void *unused2, RoomOrbitSpriteFxParams *fx) {
    RoomSpriteMatrix matrix;
    RoomFxVec4 scratchScale;
    RoomFxVec4 scale;
    RoomOrbitSetState *owner;
    u8 *drawSlot;

    owner = FieldFx_GetScriptContext();
    FieldFx_SetSpriteTexturePreset(owner->asset.page);
    FieldFx_SetSpriteCellSize(0x20, 0x20);
    FieldFx_SetSpriteColorMode(0x10);
    FieldFx_SetSpriteBlendMode(2);

    matrix.m[2][2] = 0x1000;
    matrix.m[1][1] = 0x1000;
    matrix.m[0][0] = 0x1000;
    matrix.t[0] = matrix.t[1] = matrix.t[2] = 0;
    matrix.m[0][1] = matrix.m[0][2] = matrix.m[1][0] =
        matrix.m[1][2] = matrix.m[2][0] = matrix.m[2][1] = 0;

    memset(&scale, 0, 0x10);
    scale.x = fx->scale;
    scale.y = fx->scale;
    scale.z = 0x400;
    scratchScale = scale;
    ScaleMatrix(&matrix, &scratchScale);

    drawSlot = &g_RoomOrbitSpritePacket.code;
    matrix.t[0] = fx->x;
    matrix.t[1] = fx->y;
    matrix.t[2] = fx->z;
    *drawSlot = ((s16)fx->alpha >> 1) << 1;
    g_RoomOrbitSpritePacket.depth = fx->depth;
    FieldFx_DrawGlowSprite((FieldGlowSprite *)(drawSlot - 4), &matrix, 1);
}

void RoomEffect_UpdateOrbitSprite(unsigned char *arg0, RoomFxControl *signal, RoomOrbitSpriteFxParams *state) {
    RoomOrbitSpriteFxParams *p = state;
    unsigned short counter = state->alpha;

    state->alpha = counter + 1;
    if ((short)counter >= 16) {
        state->alpha = 15;
    }

    if ((s16)state->depth >= 9) {
        state->depth -= 8;
    }
    if (state->phase >= 21) {
        state->phase -= 20;
    }

    p->scale -= 0x64;
    p->x += (p->velocity.x * p->phase) >> 16;
    p->z += (p->velocity.z * p->phase) >> 16;

    if (signal->frame >= 21) {
        signal->state = 2;
    }
}
