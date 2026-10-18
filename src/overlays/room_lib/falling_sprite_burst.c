/*
 * The falling sprite burst: a sprite and its floor shadow fall until they
 * reach the floor, then spawn an impact shimmer, a ground pulse and six
 * particles that fall back to the floor.
 *
 * Ten hospital and Chrysler rooms link the same fifteen functions in this
 * order, with the same 0x38 bytes of read-only seeds; this unit is that
 * object, compiled into each of them. The five sprite records and the spawn
 * tables live in each room's own data.
 */

#include "pe1/field_actor.h"
#include "pe1/render_object.h"
#include "pe1/room_falling_burst.h"
#include "pe1/room_rearm_timer.h"

static const RoomFxSeed8 s_FallingSpritePositionSeed = {
    { 0x00, 0x00, 0x14, 0x00, 0xD8, 0xFF, 0x00, 0x00 }
};
static const RoomFxSeed8 s_FallingSpriteVelocitySeed = {
    { 0x00, 0x00, 0x00, 0x00, 0xFA, 0xFF, 0x00, 0x00 }
};
static const RoomFxVec4 s_FallingShadowScale = { 0x100, 0x100, 0x100, 0 };
static const RoomFxSeed8 s_FloorDecalRotation = {
    { 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
};
static const RoomFxVec4 s_ImpactParticleScale = { 0x190, 0x190, 0x190, 0 };

/* Sets up the burst state and the room's five sprite records. */
int RoomEffect_FallingBurstInit(void *object, void *slot, void *data) {
    void *arg0 = object;
    void *arg2 = data;
    RoomFallingBurstClock *state = arg2;
    RoomFallingBurstEntity *self = arg0;

    FieldFx_SetScriptContext(state);
    state->asset.index = Asset_SearchByKeyType(0x28);
    state->cooldown = 0;
    state->hit = 0;
    state->actor = self->actor;
    state->step = *FieldFx_VarPtr(1);
    state->tick = *FieldFx_VarPtr(2);
    state->reload = *FieldFx_VarPtr(3);

    g_RoomFallingBurstSprite.offset = 0xA;
    g_RoomFallingBurstShimmer.offset = 0x64;
    g_RoomFallingBurstPulse.code = 0x20;
    {
        int value24 = 0x24;
        int value1 = 1;
        int value80arg = 0x80;
        unsigned char value80 = 0x80;
        int value6e = 0x6E;
        g_RoomFallingBurstSprite.code = value24;
        g_RoomFallingBurstSprite.mode = value1;
        g_RoomFallingBurstSprite.depth = value80arg;
        g_RoomFallingBurstSprite.r = value80;
        g_RoomFallingBurstSprite.g = value80;
        g_RoomFallingBurstSprite.b = value80;
        g_RoomFallingBurstSprite.zero = 0;
        g_RoomFallingBurstShimmer.code = 0;
        g_RoomFallingBurstShimmer.mode = 0;
        g_RoomFallingBurstPulse.offset = 0xA0;
        g_RoomFallingBurstShimmer.depth = value80arg;
        g_RoomFallingBurstShimmer.r = value80;
        g_RoomFallingBurstShimmer.g = value80;
        g_RoomFallingBurstShimmer.b = value80;
        g_RoomFallingBurstShimmer.zero = 0;
        g_RoomFallingBurstPulse.mode = value1;
        g_RoomFallingBurstPulse.depth = value80arg;
        g_RoomFallingBurstPulse.r = value80;
        g_RoomFallingBurstPulse.g = value80;
        g_RoomFallingBurstPulse.b = value80;
        g_RoomFallingBurstPulse.zero = 0;
        g_RoomFallingBurstParticle.code = value24;
        g_RoomFallingBurstParticle.mode = value1;
        g_RoomFallingBurstParticle.offset = value6e;
        g_RoomFallingBurstParticle.depth = value80arg;
        g_RoomFallingBurstParticle.r = value80;
        g_RoomFallingBurstParticle.g = value80;
        g_RoomFallingBurstParticle.b = value80;
        g_RoomFallingBurstParticle.zero = 0;
        g_RoomFallingBurstShadow.offset = value6e;
        g_RoomFallingBurstShadow.depth = 0x30;
        g_RoomFallingBurstShadow.code = value24;
        g_RoomFallingBurstShadow.mode = value1;
        g_RoomFallingBurstShadow.r = 0xA0;
        g_RoomFallingBurstShadow.g = 0xA0;
        g_RoomFallingBurstShadow.b = 0xA0;
        g_RoomFallingBurstShadow.zero = 0;
    }
    return 0;
}

int RoomEffect_FallingBurstNop(void *object, void *slot, void *data) {
    return 0;
}

/* Counts the hold timer down and re-sends the burst when it is rearmed; ends
 * the effect when the script reports completion. */
int RoomEffect_FallingBurstTick(void *object, void *slot, void *data) {
    RoomFallingBurstClock *clock=data;
    RoomFallingBurstControl *control=slot;
    if(clock->cooldown) --clock->cooldown;
    if(clock->hit==1) {
        clock->hit=0;
        if(!clock->cooldown) {
            clock->cooldown=clock->reload;
            func_800C6C18(object);
        }
    }
    if(FieldFx_IsLastSlotHalted()==1)control->state=2;
    return 0;
}



/* Places the sprite at the actor's offset and gives it a random sideways
 * drift, or the actor's facing when script 1 is set. */
int RoomEffect_InitFallingSprite(void *object, void *slot, void *data) {
    void *entity = object;
    void *unused = slot;
    RoomFallingSpriteState *state = data;
    RoomFxSeed8 positionSeed;
    GteShortVector position;
    RoomFxSeed8 velocitySeed;
    GteShortVector velocity;
    FieldActor **root;
    RenderMatrix *view;

    root = FieldFx_GetScriptContext();
    positionSeed = s_FallingSpritePositionSeed;
    velocitySeed = s_FallingSpriteVelocitySeed;
    view = &(*root)->render_object.matrices[5];
    ApplyMatrixSV((GteMatrix *)view, &positionSeed.vector,
                  &position);

    state->x = (u16)position.x + view->translation[0];
    state->y = (u16)position.y + view->translation[1];
    state->z = (u16)position.z + view->translation[2];
    state->scale = 0x100;
    state->depth = 0x80;
    state->verticalStep = 0;
    state->active = 0;

    if (*FieldFx_RegPtr(1) == 0) {
        state->velocityX = rand() % 10 - 5;
        state->velocityY = 0;
        state->velocityZ = rand() % 10 - 5;
    } else {
        ApplyMatrixSV((GteMatrix *)view, &velocitySeed.vector,
                      &velocity);
        state->velocityX = (u16)velocity.x * 2;
        state->velocityY = 0;
        state->velocityZ = (u16)velocity.z * 2;
    }

    FieldFx_FindAssetAtShortPosition(entity, 0x583, state);
    return 0;
}

/* Draws the sprite stretched by its fall speed, then its shadow on the
 * floor. */
int RoomEffect_DrawFallingSprite(void *object, void *slot, void *data) {
    void *unused0 = object;
    void *unused1 = slot;
    RoomFallingSpriteState *fx = data;
    RoomSpriteMatrix matrix;
    RoomFxVec4 scratchScale;
    RoomFxVec4 scale;
    RoomFallingBurstClock *owner;

    owner = FieldFx_GetScriptContext();
    FieldFx_SetSpriteTexturePreset(owner->asset.page);
    FieldFx_SetSpriteCellSize(0x10, 0x10);
    FieldFx_SetSpriteColorMode(0x10);
    FieldFx_SetSpriteBlendMode(0);

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
    scale.x = 0x100;
    scale.y = (s16)fx->scale;
    scale.z = 0x1000;
    scratchScale = scale;
    ScaleMatrix(&matrix, &scratchScale);

    g_RoomFallingBurstSprite.depth = fx->depth;
    matrix.t[0] = (s16)fx->x;
    matrix.t[1] = (s16)fx->y - (((s16)fx->scale - 0x100) >> 4);
    matrix.t[2] = (s16)fx->z;
    FieldFx_DrawGlowSprite((FieldGlowSprite *)&g_RoomFallingBurstSprite, &matrix, 1);

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

    scale = s_FallingShadowScale;
    ScaleMatrix(&matrix, &scale);
    FieldFx_SetSpriteBlendMode(2);
    matrix.t[0] = (s16)fx->x;
    matrix.t[1] = g_RoomFloorY->y;
    matrix.t[2] = (s16)fx->z;
    FieldFx_DrawGlowSprite((FieldGlowSprite *)&g_RoomFallingBurstShadow, &matrix, 1);
    return 0;
}


/* Moves the sprite until it reaches the floor; there it shrinks away and,
 * once, spawns the ground pulse (kind 3), the impact shimmer (kind 2) and
 * the six particles (kind 4) at the landing point. */
int RoomEffect_UpdateFallingSprite(void *object, void *slot, void *data) {
    void *entity = object;
    RoomFallingBurstControl *control = slot;
    RoomFallingSpriteState *effect = data;
    void *self = entity;
    RoomFallingSpriteState *state;
    RoomFallingBurstParticles *burst;
    unsigned short *frame;
    unsigned int i;
    int belowFloor;

    belowFloor = (short)effect->y < g_RoomFloorY->y;
    state = effect;
    if (!belowFloor) {
        if ((short)effect->scale >= 0x15) {
            effect->scale -= 0x14;
        } else {
            control->state = 2;
        }

        if (state->active == 0) {
            state->active = 1;

            {
                RoomFallingBurstDecal *shimmer = FieldFx_StartSlot(
                    self, 3, (unsigned short *)g_RoomSpawnLayout,
                    g_RoomInitList);
                if (shimmer != 0) {
                    shimmer->x = state->x;
                    shimmer->y = g_RoomFloorY->y;
                    shimmer->z = state->z;
                }
            }

            {
                RoomFallingBurstDecal *pulse = FieldFx_StartSlot(
                    self, 2, (unsigned short *)g_RoomSpawnLayout,
                    g_RoomInitList);
                if (pulse != 0) {
                    pulse->x = state->x;
                    pulse->y = g_RoomFloorY->y;
                    pulse->z = state->z;
                }
            }

            burst = FieldFx_StartSlot(
                self, 4, (unsigned short *)g_RoomSpawnLayout,
                g_RoomInitList);
            if (burst != 0) {
                i = 0;
                frame = &g_RoomFloorY->raw;
                do {
                    burst->position[i].x = state->x;
                    burst->position[i].y = *frame;
                    burst->position[i].z = state->z;
                    i++;
                } while (i < 6);
            }
        }
    } else {
        RoomFallingSpriteState *motion = state;
        motion->x += motion->velocityX;
        motion->z += motion->velocityZ;
        motion->y += motion->verticalStep >> 8;
        motion->scale += 0x28;
        motion->verticalStep += 0x64;
    }
    return 0;
}

int RoomEffect_InitImpactShimmer(void *object, void *slot, void *data) {
    RoomFallingBurstDecal *c = data;
    c->shimmerScale = 0x258;
    c->shimmerDepth = 0x80;
    c->shimmerAlpha = 0;
    return 0;
}

/* Draws the shimmer flat on the floor. */
int RoomEffect_DrawImpactShimmer(void *object, void *slot, void *data) {
    void *arg0 = object;
    void *arg1 = slot;
    RoomSpriteFxParams *fx = data;
    RoomSpriteMatrix matrix;
    RoomFxSeed8 seed;
    RoomFxVec4 scratch_scale;
    RoomFxVec4 scale;
    RoomFallingBurstClock *owner;

    owner = FieldFx_GetScriptContext();
    seed = s_FloorDecalRotation;
    FieldFx_SetSpriteTexturePreset(owner->asset.page);
    FieldFx_SetSpriteCellSize(0x20, 0x20);
    FieldFx_SetSpriteColorMode(0x10);
    FieldFx_SetSpriteBlendMode(1);
    RotMatrix(&seed.vector, &matrix);

    memset(&scale, 0, 0x10);
    scale.x = fx->scale;
    scale.y = fx->scale;
    scale.z = 0x1000;
    scratch_scale = scale;
    ScaleMatrix(&matrix, &scratch_scale);

    g_RoomFallingBurstShimmer.depth = fx->depth;
    matrix.t[0] = fx->x;
    matrix.t[1] = fx->y;
    matrix.t[2] = fx->z;
    g_RoomFallingBurstShimmer.code = fx->alpha << 1;
    FieldFx_DrawGlowSprite((FieldGlowSprite *)&g_RoomFallingBurstShimmer, &matrix, 0);
    return 0;
}

/* Grows the shimmer; in its last thirty ticks it fades in and ends. */
int RoomEffect_UpdateImpactShimmer(void *object, void *slot, void *data) {
    RoomFallingBurstControl *st = slot;
    RoomFallingBurstDecal *c = data;
    RoomFallingBurstClock *r = FieldFx_GetScriptContext();
    c->shimmerScale += 3;
    if (r->tick - 0x1E < st->endTick) {
        unsigned char v = c->shimmerPhase + 8;
        c->shimmerPhase = v;
        c->shimmerAlpha = v >> 4;
        if (v >> 4 == 8) {
            st->state = 2;
        }
    }
    return 0;
}

int RoomEffect_InitGroundPulse(void *object, void *slot, void *data) {
    RoomFallingBurstDecal *c = data;
    c->pulseScale = 0xC8;
    c->pulseDepth = 0x80;
    return 0;
}

/* Draws the pulse flat on the floor. */
int RoomEffect_DrawGroundPulse(void *object, void *slot, void *data) {
    void *unused0 = object;
    void *unused1 = slot;
    RoomUniformSpriteFxParams *fx = data;
    RoomFxSeed8 seed;
    RoomSpriteMatrix matrix;
    RoomSpriteMatrix *matrixPtr;
    RoomFxVec4 scratchScale;
    RoomFxVec4 scale;
    RoomFallingBurstClock *owner;

    owner = FieldFx_GetScriptContext();
    matrixPtr = &matrix;
    seed = s_FloorDecalRotation;
    FieldFx_SetSpriteTexturePreset(owner->asset.page);
    FieldFx_SetSpriteCellSize(0x40, 0x40);
    FieldFx_SetSpriteColorMode(0x10);
    FieldFx_SetSpriteBlendMode(1);
    RotMatrix(&seed.vector, matrixPtr);

    memset(&scale, 0, 0x10);
    scale.x = fx->scale;
    scale.y = fx->scale;
    scale.z = fx->scale;
    scratchScale = scale;
    ScaleMatrix(matrixPtr, &scratchScale);

    g_RoomFallingBurstPulse.depth = fx->depth;
    matrixPtr->t[0] = fx->x;
    matrixPtr->t[1] = fx->y;
    matrixPtr->t[2] = fx->z;
    FieldFx_DrawGlowSprite((FieldGlowSprite *)&g_RoomFallingBurstPulse, matrixPtr, 0);
    return 0;
}

/* Expands the pulse with the clock, fades it in its last sixty ticks and
 * flags the clock when the pulse touches the player. */
int RoomEffect_UpdateGroundPulse(void *object, void *slot, void *data) {
    RoomFallingBurstControl *state = slot;
    RoomFallingBurstDecal *fx = data;
    RoomFallingBurstClock *clock = FieldFx_GetScriptContext();
    if (clock->tick - 0x3C < state->endTick) {
        if (fx->pulseDepth >= 5) {
            fx->pulseDepth -= 4;
        }
        if (clock->tick - 0x1E < state->endTick &&
            fx->pulseScale > clock->step * 6) {
            fx->pulseScale -= clock->step * 6;
        }
    } else {
        fx->pulseScale += clock->step;
    }
    if (FieldFx_IsPlayerWithinRadius(fx, fx->pulseScale >> 3) != 0) {
        clock->hit = 1;
    }
    if (state->endTick == clock->tick) {
        state->state = 2;
    }
    return 0;
}

/* Launches the six particles upwards with random spreads. */
int RoomEffect_InitImpactParticles(void *object, void *slot, void *data) {
    void *state = slot;
    RoomFallingBurstParticles *fx = data;
    RoomFxVec4 scale;
    unsigned int i;
    fx->matrix.m[2][2] = 0x1000;
    fx->matrix.m[1][1] = 0x1000;
    fx->matrix.m[0][0] = 0x1000;
    fx->matrix.t[2] = 0;
    fx->matrix.t[1] = 0;
    fx->matrix.t[0] = 0;
    fx->matrix.m[2][1] = 0;
    fx->matrix.m[2][0] = 0;
    fx->matrix.m[1][2] = 0;
    fx->matrix.m[1][0] = 0;
    fx->matrix.m[0][2] = 0;
    fx->matrix.m[0][1] = 0;
    scale = s_ImpactParticleScale;
    ScaleMatrix(&fx->matrix, &scale);
    fx->liveCount = 6;
    for (i = 0; i < 6; i++) {
        fx->active[i] = 1;
        fx->depth[i] = 0x80;
        fx->velocity[i].x = rand() % 31 - 15;
        fx->velocity[i].y = -(rand() % 100 + 50);
        fx->velocity[i].z = rand() % 31 - 15;
    }
    return 0;
}

/* Draws each live particle and its dimmer reflection on the floor. */
int RoomEffect_DrawImpactParticles(void *object, void *slot, void *data) {
    void *state = slot;
    RoomFallingBurstParticles *fx = data;
    RoomFallingBurstClock *clock = FieldFx_GetScriptContext();
    unsigned int i;
    FieldFx_SetSpriteTexturePreset(clock->asset.page);
    FieldFx_SetSpriteCellSize(0x10, 0x10);
    FieldFx_SetSpriteColorMode(0x10);
    FieldFx_SetSpriteBlendMode(1);
    for (i = 0; i < 6; i++) {
        if (fx->active[i] == 1) {
            fx->matrix.t[0] = (short)fx->position[i].x;
            fx->matrix.t[1] = (short)fx->position[i].y;
            fx->matrix.t[2] = (short)fx->position[i].z;
            g_RoomFallingBurstParticle.depth = fx->depth[i];
            FieldFx_DrawGlowSprite((FieldGlowSprite *)&g_RoomFallingBurstParticle, &fx->matrix, 1);
            g_RoomFallingBurstParticle.depth = (short)fx->depth[i] >> 2;
            fx->matrix.t[1] = g_RoomFloorY->y;
            FieldFx_DrawGlowSprite((FieldGlowSprite *)&g_RoomFallingBurstParticle, &fx->matrix, 1);
        }
    }
    return 0;
}

/* Per-frame particle step: velocity/16 integration, +4 gravity, kill on
 * floor hit; when the last one dies the effect ends. */
int RoomEffect_UpdateImpactParticles(void *object, void *slot, void *data) {
    unsigned char *st = slot;
    RoomFallingBurstParticles *sys = data;
    int i;
    int grav;
    unsigned short *gp;
    for (i = 0; (unsigned)i < 6; i++) {
        grav = 4;
        if (sys->active[i] == 1) {
            int vx = (short)sys->velocity[i].x >> grav;
            int vy = (short)sys->velocity[i].y >> grav;
            int vz = (short)sys->velocity[i].z >> grav;
            sys->position[i].x += vx;
            sys->position[i].y = sys->position[i].y + vy;
            sys->position[i].z += vz;
            gp = &g_RoomFloorY->raw;
            sys->velocity[i].y += grav;
            if ((short)sys->position[i].y >= (short)*gp) {
                short r;
                sys->active[i] = 0;
                sys->liveCount = (r = sys->liveCount - 1);
                if (r == 0) {
                    st[1] = 2;
                }
            }
        }
    }
    return 0;
}

/* Complete USA M137/M140/M147/M324/M327 module data. The bytecode and
 * handler order are those at 80190e30..80190eec. Native pointer-bearing
 * clock storage grows with the host ABI; every following slot stays aligned. */
RoomFxSpritePacket g_RoomFallingBurstShadow,g_RoomFallingBurstShimmer,
    g_RoomFallingBurstSprite,g_RoomFallingBurstPulse,g_RoomFallingBurstParticle;
FieldFxHandler g_RoomInitList[]={RoomEffect_FallingBurstInit,RoomEffect_InitFallingSprite,
    RoomEffect_InitImpactShimmer,RoomEffect_InitGroundPulse,RoomEffect_InitImpactParticles,
    (FieldFxHandler)(intptr_t)-1};
FieldFxHandler g_RoomDrawList[]={RoomEffect_FallingBurstNop,RoomEffect_DrawFallingSprite,
    RoomEffect_DrawImpactShimmer,RoomEffect_DrawGroundPulse,RoomEffect_DrawImpactParticles,
    (FieldFxHandler)(intptr_t)-1};
FieldFxHandler g_RoomUpdateList[]={RoomEffect_FallingBurstTick,RoomEffect_UpdateFallingSprite,
    RoomEffect_UpdateImpactShimmer,RoomEffect_UpdateGroundPulse,RoomEffect_UpdateImpactParticles,
    (FieldFxHandler)(intptr_t)-1};
#define FALLING_SIZE(bytes) (((bytes)+sizeof(void *)-1)&~(sizeof(void *)-1))
int g_RoomSpawnLayout[]={FALLING_SIZE(sizeof(RoomFallingBurstClock))|(FALLING_SIZE(sizeof(RoomFallingSpriteState))<<16),
    FALLING_SIZE(sizeof(RoomFallingBurstDecal))|(FALLING_SIZE(12)<<16),FALLING_SIZE(sizeof(RoomFallingBurstParticles))|0xffff0000u};
#undef FALLING_SIZE
int g_RoomScript[]={0x00010000,0x00100000,0x10050000,0x100b0028,0x100d0032,
    0x00200001,0x00020000,0x30fa0000,0x00420000,0x00110000,0x00010001,
    0x00020004,0x00320001,0x22fb0000,0x30f60000,0x00110001,0x00010001,
    0x30f30000,-1};
PmCommand g_RoomFallingBurstHandlers={RoomLib_ClassNop0,RoomLib_PlantScript,RoomLib_Spawn6,
    RoomLib_RegisterDrawList,RoomLib_RegisterPairedTables,RoomLib_CloseTarget,RoomLib_ClassNop0};
