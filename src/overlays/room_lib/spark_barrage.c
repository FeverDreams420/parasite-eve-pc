/*
 * Spark barrage: five spark attacks, each a particle callback and its
 * controller. Phased sparks drop from above the player, land on the floor
 * and flash the scene; bouncing sparks spray from the actor's hand; wave
 * sparks ride the actor's joints and collapse into a burst; comet sparks
 * are fired along random directions in bursts of ten; lifted sparks rise
 * from the actor and bounce on the floor.
 *
 * room_m188 and room_m390 link these ten functions in this order, with the
 * same 0x34 bytes of read-only seeds; this unit is that object, compiled
 * into each of them. The colour ramps and the comet's trail record live in
 * each room's own data.
 */

#include "pe1/gte.h"
#include "pe1/geom_state.h"
#include "pe1/fieldfx_task.h"
#include "pe1/field_actor.h"
#include "pe1/battle.h"
#include "pe1/room_module.h"
#include "room_lib.h"
#include <string.h>
#include "pe1/room_spark_barrage.h"

static const GteRotation s_FloorRotation = { 0x400, 0, 0, 1 };
static const GteRotation s_ZeroRotation = { 0, 0, 0, 0 };
static const GteShortVector s_HandOffset = { 0, 0, -24, 0 };
static const RenderColor s_BounceShade = { 0xC8, 0x00, 0xC8, 0 };
static const GteShortVector s_WaveOffsetA = { 0x6E, 0, 0, 0 };
static const GteShortVector s_WaveOffsetB = { -0x6E, 0, 0, 0 };
static const GteShortVector s_WaveOffsetC = { 0, 0, -40, 0 };

/* The original hit operation updates actor flags, not a GPU packet. */
#define ROOMEFFECT_SPARK_BARRAGE_FLASH() \
    { \
        FieldActorState *owner = g_FieldFxOwner->actor->state; \
        if ((owner->core_flags & 0x3F000000) == 0x01000000) { \
            ((Combatant *)g_PlayerEntity->state)->stateFlags |= 0x4000; \
            owner->core_flags = (owner->core_flags & 0xC0FFFFFF) | 0x99000000; \
        } \
    }

/* Spark that drops from above the actor, lands on the floor height, then
 * sheds flashing and drifting children while the scene flashes. */
int RoomEffect_PhasedSparkParticle(int mode, void *data, intptr_t argument) {
    RoomPhasedSpark *spark = data;
    GteRotation rotation = s_FloorRotation;
    GteRotation spin = s_ZeroRotation;
    GteShortVector target;
    GteShortVector floorPos;
    GteRotation floorSpin;
    RoomPhasedSpark *child;
    /* Shared across the draw cases: a single-set temporary would be
     * scheduled to the end of its block, retail keeps the quotient early. */
    int scale;
    int fade;

    switch (mode) {
    case 1:
        switch (spark->state) {
        case 0:
            switch (spark->phase) {
            case 0:
                spark->timer++;
                spark->y -= 0x6E;
                if ((s16)spark->timer >= 16) {
                    spark->phase = 1;
                    spark->timer = 0;
                    FieldFx_GetActorPosition((char *)g_PlayerEntity, 0, (s16 *)&target);
                    spark->x = target.x + rand() % 1200 - 600;
                    spark->z = target.z + rand() % 1200 - 600;
                }
                break;
            case 1:
                spark->timer++;
                spark->y += 0x6E;
                if (spark->y >= (s16)g_RoomFloorY->raw) {
                    spark->state = 1;
                    spark->timer = 0;
                    spark->y = (s16)g_RoomFloorY->raw;
                }
                break;
            }
            if (spark->phase < 2 && (g_FieldFxCurrentAge & 1)) {
                if (rand() & 1) {
                    child = FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
                    if (child) {
                        child->x = spark->x;
                        child->y = spark->y;
                        child->z = spark->z;
                        child->state = 2;
                        child->timer = 0;
                        child->phase = 0;
                    }
                } else {
                    child = FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
                    if (child) {
                        child->x = spark->x;
                        child->y = spark->y;
                        child->z = spark->z;
                        child->vx = (rand() & 0x1F) - 16;
                        child->vz = (rand() & 0x1F) - 16;
                        child->vy = (rand() & 3) - 2;
                        child->state = 3;
                        child->timer = 0;
                    }
                }
            }
            if (FieldFx_IsPlayerWithinRadius(spark, 0x6E) == 0) break;
            if (spark->y < (s16)g_RoomFloorY->raw - 0x202) break;
            if (g_FieldFxTaskContext->flags == 0) break;
            ROOMEFFECT_SPARK_BARRAGE_FLASH();
            break;
        case 1:
            spark->timer++;
            if (g_FieldFxCurrentAge & 1) {
                child = FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
                if (child) {
                    child->x = spark->x;
                    child->y = spark->y;
                    child->z = spark->z;
                    child->x = (rand() & 0xFF) - 0x80;
                    child->z = (rand() & 0xFF) - 0x80;
                    child->vx = (rand() & 0x1F) - 16;
                    child->vz = (rand() & 0x1F) - 16;
                    child->vy = (rand() & 3) - 2;
                    child->state = 3;
                    child->timer = 0;
                }
            }
            if (FieldFx_IsPlayerWithinRadius(spark, 0xC8) && (s16)spark->timer < 9 &&
                g_FieldFxTaskContext->flags) {
                ROOMEFFECT_SPARK_BARRAGE_FLASH();
            }
            if ((s16)spark->timer < 16) break;
            return 1;
        case 2:
            spark->timer++;
            spark->y += spark->phase;
            spark->phase += 2;
            if ((s16)spark->timer < 8) break;
            return 1;
        case 3:
            spark->timer++;
            spark->x += spark->vx;
            spark->y += spark->vy;
            spark->z += spark->vz;
            if ((s16)spark->timer < 16) break;
            return 1;
        }
        break;
    case 2:
        switch (spark->state) {
        case 0: {
            u16 clut;
            int kind;
            int palette;
            scale = (rcos(g_FieldFxCurrentAge << 10) / 8 + 0x1000) * 3 / 4;
            g_FieldFxSpriteParams.parameter00 = 0x20;
            g_FieldFxSpriteParams.parameter02 = 2;
            g_FieldFxSpriteParams.extent_x = 0x20;
            g_FieldFxSpriteParams.extent_y = 0x20;
            spin.z = g_FieldFxCurrentAge << 8;
            kind = g_FieldFxSpriteParams.palette;
            palette = g_FieldFxPaletteTable[kind];
            if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
            clut = GetClut(0x20, palette);
            FieldFx_DrawShapeQuads((GteShortVector *)spark, &spin, scale, scale, 0xA0, clut, 0,
                          0xBE, 0);
            }
            {
            u16 clut;
            int kind;
            int palette;
            g_FieldFxSpriteParams.parameter00 = 0x10;
            g_FieldFxSpriteParams.extent_x = 0x10;
            g_FieldFxSpriteParams.extent_y = 0x10;
            g_FieldFxSpriteParams.parameter02 = 1;
            floorSpin.x = 0x400;
            floorSpin.y = 0;
            floorSpin.z = 0;
            floorSpin.flags = 1;
            floorPos.x = spark->x;
            floorPos.z = spark->z;
            floorPos.y = (s16)g_RoomFloorY->raw;
            kind = g_FieldFxSpriteParams.palette;
            palette = g_FieldFxPaletteTable[kind];
            if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
            clut = GetClut(0x10, palette);
            FieldFx_DrawShapeQuads(&floorPos, &floorSpin, scale * 2, scale * 2,
                          (s16)g_FieldFxSpriteParams.parameter02 * 3 + 0xA7, clut, 2, 0x64, 0);
            break;
        }
        case 1: {
            u16 clut;
            int kind;
            int palette;
            scale = rcos((s16)spark->timer * 64) + 0x1000;
            fade = (s16)spark->timer / 2;
            if (fade >= 7) fade = 6;
            g_FieldFxSpriteParams.parameter00 = 0x20;
            g_FieldFxSpriteParams.parameter02 = 2;
            g_FieldFxSpriteParams.extent_x = 0x20;
            g_FieldFxSpriteParams.extent_y = 0x20;
            kind = g_FieldFxSpriteParams.palette;
            palette = g_FieldFxPaletteTable[kind];
            if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
            clut = GetClut(0, palette);
            FieldFx_DrawShapeQuads((GteShortVector *)spark, &rotation, scale * 3 / 2,
                          scale * 3 / 2, (s16)g_FieldFxSpriteParams.parameter02 * fade, clut,
                          0, 0xBE, 0);
            break;
        }
        case 2: {
            u16 clut;
            int kind;
            int palette;
            scale = rcos((s16)spark->timer * 128) / 2 + 0x800;
            fade = rcos((s16)spark->timer * 128) / 32;
            g_FieldFxSpriteParams.parameter00 = 0x10;
            g_FieldFxSpriteParams.parameter02 = 1;
            g_FieldFxSpriteParams.extent_x = 0x10;
            g_FieldFxSpriteParams.extent_y = 0x10;
            kind = g_FieldFxSpriteParams.palette;
            palette = g_FieldFxPaletteTable[kind];
            if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
            clut = GetClut(0x30, palette);
            FieldFx_DrawShapeQuads((GteShortVector *)spark, 0, scale, scale, 0xA2, clut, 1,
                          fade, 0);
            break;
        }
        case 3: {
            u16 clut;
            int kind;
            int palette;
            scale = rsin(((s16)spark->timer * 1024) / 24) * 2 + 0x1000;
            fade = rcos(((s16)spark->timer * 1204) / 24) / 32;
            g_FieldFxSpriteParams.parameter00 = 0x20;
            g_FieldFxSpriteParams.parameter02 = 2;
            g_FieldFxSpriteParams.extent_x = 0x20;
            g_FieldFxSpriteParams.extent_y = 0x20;
            kind = g_FieldFxSpriteParams.palette;
            palette = g_FieldFxPaletteTable[kind];
            if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
            clut = GetClut(0x10, palette);
            FieldFx_DrawShapeQuads((GteShortVector *)spark, 0, 0x1000, scale,
                          (s16)g_FieldFxSpriteParams.parameter02 * ((s16)spark->timer / 2) + 0x20,
                          clut, 1, fade, 0);
            break;
        }
        }
        break;
    }
    return 0;
}


/* Phased spark controller: every sixth frame drops one spark at joint 6
 * until the six in the caller's counter are spent. */
int RoomEffect_PhasedSparkController(int mode, void *data, intptr_t argument) {
    s16 *counter = data;
    GteShortVector position = s_HandOffset;
    GteShortVector output;
    RoomPhasedSpark *particle;

    switch (mode) {
    case 0: {
        FieldFxTaskContext *event = g_FieldFxTaskContext;
        *counter = 6;
        if (event->flags != 0) {
            FieldActor *owner = g_FieldFxOwner->actor;
            if (owner && owner->state && owner->state->action_state) {
                u8 *status = owner->state->action_state;
                if (*status == 1) *status = 2;
            }
        }
        return FieldFx_InitCallbackList(g_FieldFxActiveTaskSlot->end, 24, 32,
                             RoomEffect_PhasedSparkParticle);
    }
    case 1:
        FieldFx_TransformMatrixPoint((struct RoomFxTransformOwner *)g_FieldFxOwner->actor, 6, &position, &output);
        if (g_FieldFxCurrentAge % 6 == 0 && *counter > 0) {
            particle = FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
            if (particle != 0) {
                particle->x = output.x;
                particle->y = output.y;
                particle->z = output.z;
                particle->state = 0;
                particle->phase = 0;
                particle->timer = 0;
            }
            --*counter;
        }
        if (g_FieldFxCurrentAge < 2) break;
        return 2;
    case 2: {
        u16 tpage = g_FieldFxTpageTable[g_FieldFxTpageSlot3];
        g_FieldFxSpriteParams.palette = 3;
        g_FieldFxSpriteParams.parameter06 = 0;
        g_FieldFxSpriteParams.parameter0A = 0;
        g_FieldFxSpriteParams.depth = 8;
        g_FieldFxSpriteParams.tpage = tpage;
        break;
    }
    }
return 0;
}


/* Spark that rises against gravity, falls once it passes the floor height,
 * then settles; drawn as a spinning sprite, a shaded blob and a flattened
 * sprite in its three states. */
int RoomEffect_BouncingSparkParticle(int mode, void *data, intptr_t argument) {
    RoomDampedSpark *spark = data;
    GteRotation rotation = s_ZeroRotation;
    RenderColor color;
    RenderColor shade = s_BounceShade;
    RenderMatrixSlot *matrixSlot;
    int scale;

    switch (mode) {
    case 1:
        switch (spark->state) {
        case 0:
            spark->timer++;
            spark->x += spark->vx;
            spark->y += spark->vy;
            spark->z += spark->vz;
            spark->vx = spark->vx * 31 / 32;
            spark->vz = spark->vz * 31 / 32;
            spark->vy += 2;
            if ((s16)spark->timer < 24) break;
            return 1;
        case 1:
            spark->timer++;
            spark->x += spark->vx;
            spark->y += spark->vy;
            spark->z += spark->vz;
            spark->vx = spark->vx * 31 / 32;
            spark->vz = spark->vz * 31 / 32;
            spark->vy += 3;
            if (spark->y >= (s16)g_RoomFloorY->raw) {
                spark->state = 2;
            }
            if ((s16)spark->timer < 16) break;
            return 1;
        case 2:
            spark->timer++;
            spark->x += spark->vx;
            spark->y += spark->vy;
            spark->z += spark->vz;
            spark->vx = spark->vx * 31 / 32;
            spark->vz = spark->vz * 31 / 32;
            spark->vy += -1;
            if ((s16)spark->timer < 16) break;
            return 1;
        }
        break;
    case 2:
        switch (spark->state) {
        case 0: {
            u16 clut;
            int kind;
            int palette;
            scale = rcos((g_FieldFxCurrentAge << 10) / 24) + 0x1000;
            rotation.z = g_FieldFxCurrentAge * 50;
            FieldFx_SampleColorTrack(g_RoomBouncingSparkColors, &color,
                          (s16)spark->timer);
            kind = g_FieldFxSpriteParams.palette;
            palette = g_FieldFxPaletteTable[kind];
            if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
            clut = GetClut(0x10, palette);
            FieldFx_DrawShapeQuads((GteShortVector *)spark, &rotation, scale, scale,
                          (s16)g_FieldFxSpriteParams.parameter02 * (s16)((s16)spark->timer / 3) + 0x20,
                          clut, 1, 0x80, &color);
            break;
        }
        case 1:
            matrixSlot = &g_ViewMatrixSlot;
            gte_ldrotmatrix(matrixSlot->value);
            gte_ldtransmatrix(matrixSlot->value);
            FieldFx_DrawPointTile((GteShortVector *)spark, (u8 *)&shade, 0x80, 0xFF);
            break;
        case 2: {
            u16 clut;
            int kind;
            int palette;
            int intensity;
            scale = rsin(((s16)spark->timer * 1024) / 24) + 0x1000;
            intensity = rcos(((s16)spark->timer * 1204) / 24) / 32;
            kind = g_FieldFxSpriteParams.palette;
            palette = g_FieldFxPaletteTable[kind];
            if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
            clut = GetClut(0x10, palette);
            FieldFx_DrawShapeQuads((GteShortVector *)spark, 0, 0x1000, scale,
                          (s16)g_FieldFxSpriteParams.parameter02 * ((s16)spark->timer / 2) + 0x20,
                          clut, 1, intensity, 0);
            break;
        }
        }
        break;
    }
    return 0;
}


/* Bouncing spark controller: plays the sound on frame 8 and sprays one
 * spark per frame from joint 7 for 74 frames, one in three of them faster,
 * rotated by the joint's matrix. */
int RoomEffect_BouncingSparkController(int mode, void *unused, intptr_t argument) {
    GteShortVector position = s_HandOffset;
    GteShortVector output;
    GteShortVector motion;
    GteShortVector velocity;
    GteMatrixWords matrix;
    RoomDampedSpark *particle;
    int kind;

    switch (mode) {
    case 0:
        if (g_FieldFxTaskContext->flags != 0) {
            FieldActor *owner = g_FieldFxOwner->actor;
            if (owner && owner->state && owner->state->action_state) {
                u8 *status = owner->state->action_state;
                if (*status == 1) *status = 2;
            }
        }
        return FieldFx_InitCallbackList(g_FieldFxActiveTaskSlot->end, 20, 32,
                             RoomEffect_BouncingSparkParticle);
    case 1:
        FieldFx_TransformMatrixPoint((struct RoomFxTransformOwner *)g_FieldFxOwner->actor, 7, &position, &output);
        if (g_FieldFxCurrentAge == 8)
            FieldFx_PlaySoundAtOwner(0x593, FieldFx_GetOwnerProgress());
        if (g_FieldFxCurrentAge < 74) {
            particle = FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
            if (particle != 0) {
                particle->x = output.x;
                particle->y = output.y;
                particle->z = output.z;
                motion.x = (rand() & 31) - 16;
                motion.y = (rand() & 31) - 16;
                motion.z = -32 - (rand() & 31);
                kind = rand() % 3;
                if (kind == mode) {
                    motion.x = motion.x * 3 / 2;
                    motion.y = motion.y * 3 / 2;
                    motion.z = motion.z * 3 / 2;
                }
                memcpy(&matrix, &g_FieldFxOwner->actor->render_object.matrices[7], sizeof(matrix));
                FieldFx_RotateVector(&matrix, &motion, &velocity);
                particle->vx = velocity.x;
                particle->vy = velocity.y;
                particle->vz = velocity.z;
                particle->state = kind;
                particle->timer = 0;
            }
        }
        if (g_FieldFxCurrentAge < 2) break;
        return 2;
    case 2: {
        u16 tpage;
        g_FieldFxSpriteParams.parameter00 = 32;
        g_FieldFxSpriteParams.parameter02 = 2;
        g_FieldFxSpriteParams.extent_x = 32;
        g_FieldFxSpriteParams.extent_y = 32;
        tpage = g_FieldFxTpageTable[g_FieldFxTpageSlot3];
        g_FieldFxSpriteParams.palette = 3;
        g_FieldFxSpriteParams.parameter06 = 0;
        g_FieldFxSpriteParams.parameter0A = 0;
        g_FieldFxSpriteParams.depth = 8;
        g_FieldFxSpriteParams.tpage = tpage;
        break;
    }
    }
return 0;
}


/* Small flash drawn at the projected position during the first frames. */
#define ROOMEFFECT_WAVE_SPARK_FLASH()                                        \
    if ((s16)spark->timer < 4) {                                             \
        u16 clut;                                                            \
        int kind;                                                            \
        int palette;                                                         \
        g_FieldFxSpriteParams.parameter00 = 0x10;                                       \
        g_FieldFxSpriteParams.parameter02 = 1;                                          \
        g_FieldFxSpriteParams.extent_x = 0x10;                                          \
        g_FieldFxSpriteParams.extent_y = 0x10;                                          \
        kind = g_FieldFxSpriteParams.palette;                                           \
        palette = g_FieldFxPaletteTable[kind];                                          \
        if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;                      \
        clut = GetClut(0x40, palette);                                 \
        FieldFx_DrawShapeQuads(&position, 0, 0x2000, 0x2000,                          \
                      (s16)g_FieldFxSpriteParams.parameter02 * (spark->timer & 3) + 0xA3, \
                      clut, 1, 0x80, 0);                                     \
    }

/* Larger sine-scaled sprite at the projected position. */
#define ROOMEFFECT_WAVE_SPARK_BLOOM(recompute)                               \
    {                                                                        \
        u16 clut;                                                            \
        int kind;                                                            \
        int palette;                                                         \
        g_FieldFxSpriteParams.parameter00 = 0x20;                                       \
        g_FieldFxSpriteParams.parameter02 = 2;                                          \
        g_FieldFxSpriteParams.extent_x = 0x20;                                          \
        g_FieldFxSpriteParams.extent_y = 0x20;                                          \
        if (recompute) {                                                     \
            fade = rcos(((s16)spark->timer * 1024) / 24) / 32;       \
            scale = rsin(((s16)spark->timer * 1024) / 24) + 0x800;   \
        }                                                                    \
        kind = g_FieldFxSpriteParams.palette;                                           \
        palette = g_FieldFxPaletteTable[kind];                                          \
        if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;                      \
        clut = GetClut(0x60, palette);                                 \
        FieldFx_DrawShapeQuads(&position, 0, scale, scale, 0xE, clut, 1, fade, 0);    \
    }

/* Spark that rides the effect channel's wave slots: it projects itself
 * along the current and next slot, flashes briefly, then collapses into a
 * floor sprite and a coloured burst between the two projected points. */
int RoomEffect_WaveSparkParticle(int mode, void *data, intptr_t argument) {
    RoomPhasedSpark *spark = data;
    GteShortVector position;
    GteShortVector second;
    GteShortVector rotationA = s_WaveOffsetA;
    GteShortVector rotationB = s_WaveOffsetB;
    GteShortVector rotationC = s_WaveOffsetC;
    RenderColor color;
    GteShortVector floorPos;
    GteRotation floorSpin;
    RenderMatrixSlot *matrixSlot;
    int scale;
    int fade;
    int phase;

    switch (mode) {
    case 1:
        switch (spark->state) {
        case 0:
            spark->timer++;
            if ((s16)spark->timer < 24) break;
            return 1;
        case 1:
            spark->timer++;
            if ((s16)spark->timer < 8) break;
            return 1;
        }
        break;
    case 2:
        switch (spark->state) {
        case 0:
            phase = spark->phase;
            if (phase < 7) {
                FieldFx_TransformMatrixPoint((struct RoomFxTransformOwner *)g_FieldFxOwner->actor, phase + 1, &rotationA, &position);
                ROOMEFFECT_WAVE_SPARK_FLASH();
                ROOMEFFECT_WAVE_SPARK_BLOOM(1);
                FieldFx_TransformMatrixPoint((struct RoomFxTransformOwner *)g_FieldFxOwner->actor, spark->phase + 1, &rotationB, &position);
            } else {
                FieldFx_TransformMatrixPoint((struct RoomFxTransformOwner *)g_FieldFxOwner->actor, phase + 9, &rotationC, &position);
                ROOMEFFECT_WAVE_SPARK_FLASH();
                ROOMEFFECT_WAVE_SPARK_BLOOM(1);
                FieldFx_TransformMatrixPoint((struct RoomFxTransformOwner *)g_FieldFxOwner->actor, spark->phase + 13, &rotationC, &position);
            }
            ROOMEFFECT_WAVE_SPARK_FLASH();
            ROOMEFFECT_WAVE_SPARK_BLOOM(0);
            break;
        case 1:
            FieldFx_TransformMatrixPoint((struct RoomFxTransformOwner *)g_FieldFxOwner->actor, 11, &rotationC, &position);
            FieldFx_TransformMatrixPoint((struct RoomFxTransformOwner *)g_FieldFxOwner->actor, 15, &rotationC, &second);
            {
                u16 clut;
                int kind;
                int palette;
                g_FieldFxSpriteParams.parameter00 = 0x10;
                g_FieldFxSpriteParams.parameter02 = 1;
                g_FieldFxSpriteParams.extent_x = 0x10;
                g_FieldFxSpriteParams.extent_y = 0x10;
                position.x = (position.x + second.x) / 2;
                position.y = (position.y + second.y) / 2;
                position.z = (position.z + second.z) / 2;
                kind = g_FieldFxSpriteParams.palette;
                palette = g_FieldFxPaletteTable[kind];
                if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
                clut = GetClut(0x50, palette);
                FieldFx_DrawShapeQuads(&position, 0, 0x2000, 0x2000,
                              (s16)g_FieldFxSpriteParams.parameter02 * (s16)spark->timer + 0xA7,
                              clut, 1, 0x80, 0);
            }
            {
                u16 clut;
                int kind;
                int palette;
                floorSpin.x = 0x400;
                floorSpin.y = 0;
                floorSpin.z = 0;
                floorSpin.flags = 1;
                floorPos.x = position.x;
                floorPos.z = position.z;
                floorPos.y = (s16)g_RoomFloorY->raw;
                kind = g_FieldFxSpriteParams.palette;
                palette = g_FieldFxPaletteTable[kind];
                if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
                clut = GetClut(0x50, palette);
                FieldFx_DrawShapeQuads(&floorPos, &floorSpin, 0x1000, 0x1000,
                              (s16)g_FieldFxSpriteParams.parameter02 * (s16)spark->timer + 0xA7,
                              clut, 3, 0x80, 0);
            }
            scale = rcos((s16)spark->timer * 128);
            FieldFx_SampleColorTrack(g_RoomWaveSparkColors, &color, (s16)spark->timer);
            matrixSlot = &g_ViewMatrixSlot;
            gte_ldrotmatrix(matrixSlot->value);
            gte_ldtransmatrix(matrixSlot->value);
            FieldFx_DrawRingBand(&position, 0x15E, 0x2BC, 0x18, 0, scale, scale, 0, &color,
                          0x80, 1);
            break;
        }
        break;
    }
    return 0;
}


/* Wave spark controller: plays the sound, then spawns one spark per frame
 * for eleven frames, each on the next joint; the last one collapses. */
int RoomEffect_WaveSparkController(int mode, void *data, intptr_t argument) {
    s16 *counter = data;
    /* These initial positions are never consumed: draw derives every point
     * from the actor joints. Avoid copying indeterminate PS1 stack bytes. */
    u16 seed[3] = {0, 0, 0};
    RoomPhasedSpark *particle;

    switch (mode) {
    case 0:
        *counter = 0;
        FieldFx_PlaySoundAtOwner(0x594, FieldFx_GetOwnerProgress());
        if (g_FieldFxTaskContext->flags != 0) {
            FieldActor *owner = g_FieldFxOwner->actor;
            if (owner && owner->state && owner->state->action_state) {
                u8 *status = owner->state->action_state;
                if (*status == 1) *status = 2;
            }
        }
        return FieldFx_InitCallbackList(g_FieldFxActiveTaskSlot->end, 24, 14,
                             RoomEffect_WaveSparkParticle);
    case 1:
        if (*counter < 11) {
            particle = FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
            if (particle != 0) {
                particle->x = seed[0];
                particle->y = seed[1];
                particle->z = seed[2];
                particle->timer = 0;
                particle->phase = *counter;
                if (*counter >= 10) particle->state = 1;
                else particle->state = 0;
            }
            ++*counter;
        }
        if (g_FieldFxCurrentAge < 10) break;
        return 2;
    case 2: {
        u16 tpage = g_FieldFxTpageTable[g_FieldFxTpageSlot3];
        g_FieldFxSpriteParams.palette = 3;
        g_FieldFxSpriteParams.parameter06 = 0;
        g_FieldFxSpriteParams.parameter0A = 0;
        g_FieldFxSpriteParams.depth = 8;
        g_FieldFxSpriteParams.tpage = tpage;
        break;
    }
    }
return 0;
}


/* Spark that is born between two wave slots, steers along two random
 * rotations, bounces on the floor and draws as a sprite with a coloured
 * trail. */
int RoomEffect_CometSparkParticle(int mode, void *data, intptr_t argument) {
    RoomCometSpark *spark = data;
    GteRotation rotationA = s_FloorRotation;
    GteShortVector rotationB = s_HandOffset;
    GteShortVector first;
    GteShortVector second;
    RenderColor color;
    GteShortVector floorPos;
    GteRotation floorSpin;
    GteRotation *spin;
    GteShortVector *slot;
    int scale;
    int fall;
    int bounce;

    spin = &rotationA;
    slot = &rotationB;
    switch (mode) {
    case 1:
        switch (spark->state) {
        case 0:
            spark->timer++;
            FieldFx_TransformMatrixPoint((struct RoomFxTransformOwner *)g_FieldFxOwner->actor, 11, slot, &first);
            FieldFx_TransformMatrixPoint((struct RoomFxTransformOwner *)g_FieldFxOwner->actor, 15, slot, &second);
            spark->x = (first.x + second.x) / 2;
            spark->y = (first.y + second.y) / 2;
            spark->z = (first.z + second.z) / 2;
            LoadAverageShort12(spark, &spark->vx, 0xAAB, 0x555, &spark->steer);
            LoadAverageShort12(spark, &spark->vx, 0x556, 0xAAA, &spark->drift);
            spark->drift.x += (rand() & 0x1FF) - 0x100;
            spark->drift.y += (rand() & 0x1FF) - 0x100;
            spark->drift.z += (rand() & 0x1FF) - 0x100;
            spark->steer.x += (rand() & 0xFF) - 0x80;
            spark->steer.y += (rand() & 0xFF) - 0x80;
            spark->steer.z += (rand() & 0xFF) - 0x80;
            if ((s16)spark->timer < 8) break;
            return 1;
        case 1:
            spark->timer++;
            if (FieldFx_IsPlayerWithinRadius(spark, 0x190) && g_FieldFxTaskContext->flags) {
                FieldActorState *owner = g_FieldFxOwner->actor->state;
                if ((owner->core_flags & 0x3F000000) == 0x01000000) {
                    ((Combatant *)g_PlayerEntity->state)->stateFlags |= 0x4000;
                    owner->core_flags = (owner->core_flags & 0xC0FFFFFF) | 0xA1000000;
                }
            }
            if ((s16)spark->timer < 16) break;
            return 1;
        case 2:
            spark->timer++;
            spark->x += spark->vx;
            spark->y += spark->vy;
            spark->z += spark->vz;
            spark->vx = spark->vx * 31 / 32;
            spark->vz = spark->vz * 31 / 32;
            fall = (u16)spark->vy + 2;
            spark->vy = fall;
            if (spark->y >= (s16)g_RoomFloorY->raw) {
                bounce = -(s16)fall;
                spark->vy = bounce;
            }
            if ((s16)spark->timer < 16) break;
            return 1;
        }
        break;
    case 2:
        switch (spark->state) {
        case 0: {
            {
                u16 clut;
                /* The sprite setup is a do/while (0) block, as from a macro.
                 * cse1 stops at its loop end, so the shared constant 1 only
                 * reaches the draw below in cse2, after mode and the state
                 * value have kept their own registers; cse2 also addresses
                 * the block through a base (la s0). */
                do {
                    g_FieldFxSpriteParams.parameter00 = 0x20;
                    g_FieldFxSpriteParams.parameter02 = 2;
                    g_FieldFxSpriteParams.extent_x = 0x20;
                    g_FieldFxSpriteParams.extent_y = 0x20;
                    g_FieldFxSpriteParams.tpage = g_FieldFxTpageTable[g_FieldFxTpageSlots[11]];
                    g_FieldFxSpriteParams.palette = 3;
                    g_FieldFxSpriteParams.parameter06 = 1;
                    g_FieldFxSpriteParams.parameter0A = 5;
                    clut = GetClut(0, g_FieldFxPaletteTable[g_FieldFxSpriteParams.palette] + 2);
                } while (0);
                FieldFx_DrawShapeQuads((GteShortVector *)spark, 0, 0x800, 0x800,
                              (s16)g_FieldFxSpriteParams.parameter02 * (s16)spark->timer + 0x40, clut, 1,
                              0x80, 0);
            }
            {
                u16 clut;
                int kind;
                int palette;
                floorSpin.x = 0x400;
                floorSpin.y = 0;
                floorSpin.z = 0;
                floorSpin.flags = 1;
                floorPos.x = spark->x;
                floorPos.z = spark->z;
                floorPos.y = (s16)g_RoomFloorY->raw;
                kind = g_FieldFxSpriteParams.palette;
                palette = g_FieldFxPaletteTable[kind];
                clut = GetClut(0, (kind == 4 && g_FieldFxAltPaletteFlag != 0) ? palette + 6
                                                                       : palette + 2);
                FieldFx_DrawShapeQuads(&floorPos, &floorSpin, 0x800, 0x800,
                              (s16)g_FieldFxSpriteParams.parameter02 * (s16)spark->timer + 0x40, clut, 3,
                              0x80, 0);
            }
            {
                u16 clut;
                int kind;
                int palette;
                g_RoomCometSparkTrail.steer = spark->steer;
                g_RoomCometSparkTrail.drift = spark->drift;
                memcpy(&g_RoomCometSparkTrail.position, spark, sizeof(GteRotation));
                memcpy(&g_RoomCometSparkTrail.velocity, &spark->vx, sizeof(GteRotation));
                gte_ldrotmatrix(g_ViewMatrixSlot.value);
                gte_ldtransmatrix(g_ViewMatrixSlot.value);
                FieldFx_SampleColorTrack(g_RoomCometSparkColors, &color,
                              (s16)spark->timer * 2);
                kind = g_FieldFxSpriteParams.palette;
                palette = g_FieldFxPaletteTable[kind];
                clut = GetClut(0, (kind == 4 && g_FieldFxAltPaletteFlag != 0) ? palette + 6
                                                                       : palette + 2);
                FieldFx_DrawTexturedRibbon((GteShortVector *)&g_RoomCometSparkTrail, 3, 0x12C, 0, 0x81, 0x7F,
                              0x1E, g_FieldFxTpageTable[g_FieldFxTpageSlots[11]], clut, 0x80, &color, &color,
                              1);
            }
            break;
        }
        case 1:
            g_FieldFxSpriteParams.parameter00 = 0x20;
            g_FieldFxSpriteParams.parameter02 = 2;
            g_FieldFxSpriteParams.extent_x = 0x20;
            g_FieldFxSpriteParams.extent_y = 0x20;
            g_FieldFxSpriteParams.tpage = g_FieldFxTpageTable[g_FieldFxTpageSlots[11]];
            g_FieldFxSpriteParams.palette = 3;
            g_FieldFxSpriteParams.parameter06 = 1;
            g_FieldFxSpriteParams.parameter0A = 5;
            scale = rsin((s16)spark->timer * 64);
            gte_ldrotmatrix(g_ViewMatrixSlot.value);
            gte_ldtransmatrix(g_ViewMatrixSlot.value);
            FieldFx_SampleColorTrack(g_RoomCometSparkColors, &color, (s16)spark->timer);
            FieldFx_DrawRingBand((GteShortVector *)spark, 0x1F4, 0x320, 0x18, spin, scale,
                          scale, 0, &color, 0x80, 1);
            if ((s16)spark->timer < 8) {
                u16 clut;
                int kind;
                int palette;
                kind = g_FieldFxSpriteParams.palette;
                palette = g_FieldFxPaletteTable[kind];
                clut = GetClut(0, (kind == 4 && g_FieldFxAltPaletteFlag != 0) ? palette + 6
                                                                       : palette + 2);
                FieldFx_DrawShapeQuads((GteShortVector *)spark, &rotationA, 0x2000, 0x2000,
                              (s16)g_FieldFxSpriteParams.parameter02 * (s16)spark->timer + 0x40, clut, 1,
                              0x80, 0);
            }
            break;
        case 2: {
            u16 clut;
            int half;
            do {
                g_FieldFxSpriteParams.parameter00 = 0x10;
                g_FieldFxSpriteParams.parameter02 = 1;
                g_FieldFxSpriteParams.extent_x = 0x10;
                g_FieldFxSpriteParams.extent_y = 0x10;
                g_FieldFxSpriteParams.tpage = g_FieldFxTpageTable[g_FieldFxTpageSlots[3]];
                g_FieldFxSpriteParams.palette = 3;
                g_FieldFxSpriteParams.parameter06 = 0;
                g_FieldFxSpriteParams.parameter0A = 0;
                clut = GetClut(0x50, g_FieldFxPaletteTable[g_FieldFxSpriteParams.palette]);
            } while (0);
            half = (s16)spark->timer / 2;
            FieldFx_DrawShapeQuads((GteShortVector *)spark, 0, 0x800, 0x800,
                          (s16)g_FieldFxSpriteParams.parameter02 * half + 0xA7, clut, 1, 0x80, 0);
            break;
        }
        }
        break;
    }
    return 0;
}


/* Controller for the comet sparks: anchors on the actor, marks the primary
 * channel's first node, then every fourth frame fires a burst of ten sparks
 * along a random direction until the countdown runs out. */
int RoomEffect_CometSparkController(int mode, void *data, intptr_t argument) {
    RoomCometSparkAnchor *anchor = data;
    GteShortVector direction;
    GteShortVector origin;
    RoomCometSpark *child;
    int i;

    switch (mode) {
    case 0:
        anchor->count = 6;
        FieldFx_GetActorPosition((char *)g_FieldFxOwner->actor, 0, (s16 *)anchor);
        if (g_FieldFxTaskContext->flags) {
            FieldActor *owner = g_FieldFxOwner->actor;
            if (owner && owner->state && owner->state->action_state) {
                u8 *status = owner->state->action_state;
                if (*status == 1) *status = 2;
            }
        }
        return FieldFx_InitCallbackList(g_FieldFxActiveTaskSlot->end, 0x24, 22,
                             RoomEffect_CometSparkParticle);
    case 1:
        if ((g_FieldFxCurrentAge & 3) == 0) {
            if (anchor->count <= 0) return 2;
            FieldFx_GetTransformLookAngles((struct RoomFxTransformOwner *)(char *)g_FieldFxOwner->actor, 0, &direction);
            direction.z = 0;
            direction.x = 0;
            direction.y += (rand() & 0x1FF) - 0x100;
            FieldFx_AnglesToVector(&direction, (s16)(rand() % 1400 + 1500), &origin);
            origin.x += anchor->x;
            origin.y += anchor->y;
            origin.z += anchor->z;
            origin.y = (s16)g_RoomFloorY->raw;
            child = FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
            if (child) {
                child->vx = origin.x;
                child->vy = origin.y;
                child->vz = origin.z;
                child->state = 0;
                child->timer = 0;
            }
            child = FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
            if (child) {
                child->x = origin.x;
                child->y = origin.y;
                child->z = origin.z;
                child->state = 1;
                child->timer = 0;
            }
            for (i = 0; i < 8; i++) {
                child = FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
                if (child) {
                    child->x = origin.x;
                    child->y = origin.y;
                    child->z = origin.z;
                    child->vx = rand() % 128 - 64;
                    child->vy = -(rand() % 128);
                    child->vz = rand() % 128 - 64;
                    child->state = 2;
                    child->timer = 0;
                }
            }
            anchor->count--;
        }
        if (anchor->count > 0) break;
        return 2;
    case 2:
        g_FieldFxSpriteParams.depth = 8;
        break;
    }
    return 0;
}


/* Spark that is pushed upward each frame, bounces off the floor height and
 * draws as a fading sprite or a spinning coloured one. */
int RoomEffect_LiftedSparkParticle(int mode, void *data, intptr_t argument) {
    RoomDampedSpark *spark = data;
    GteRotation rotation = s_ZeroRotation;
    RenderColor color;
    int scale;
    int fall;
    int bounce;

    switch (mode) {
    case 1:
        switch (spark->state) {
        case 0:
            spark->timer++;
            spark->x += spark->vx;
            spark->y += spark->vy;
            spark->z += spark->vz;
            spark->vx = spark->vx * 127 / 128;
            spark->vz = spark->vz * 127 / 128;
            fall = (u16)spark->vy + 5;
            spark->vy = fall;
            if (spark->y >= (s16)g_RoomFloorY->raw) {
                bounce = -(s16)fall;
                spark->vy = bounce;
            }
            if ((s16)spark->timer < 16) break;
            return 1;
        case 1:
            spark->timer++;
            spark->x += spark->vx;
            spark->y += spark->vy;
            spark->z += spark->vz;
            spark->vx = spark->vx * 255 / 256;
            spark->vz = spark->vz * 255 / 256;
            fall = (u16)spark->vy + 2;
            spark->vy = fall;
            if (spark->y >= (s16)g_RoomFloorY->raw) {
                bounce = -(s16)fall;
                spark->vy = bounce;
            }
            if ((s16)spark->timer < 24) break;
            return 1;
        }
        break;
    case 2:
        switch (spark->state) {
        case 0: {
            u16 clut;
            int kind;
            int palette;
            int intensity;
            intensity = rcos((s16)spark->timer * 64) / 32;
            scale = rcos((s16)spark->timer * 64) / 2;
            g_FieldFxSpriteParams.parameter00 = 0x20;
            g_FieldFxSpriteParams.parameter02 = 2;
            g_FieldFxSpriteParams.extent_x = 0x20;
            g_FieldFxSpriteParams.extent_y = 0x20;
            kind = g_FieldFxSpriteParams.palette;
            palette = g_FieldFxPaletteTable[kind];
            if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
            clut = GetClut(0x20, palette);
            FieldFx_DrawShapeQuads((GteShortVector *)spark, 0, scale, scale, 0xA0, clut, 1,
                          intensity, 0);
            break;
        }
        case 1: {
            int clut;
            int kind;
            int palette;
            scale = rsin(((s16)spark->timer * 1024) / 24) * 2 + 0x2000;
            rcos(((s16)spark->timer * 1204) / 24);
            g_FieldFxSpriteParams.parameter00 = 0x20;
            g_FieldFxSpriteParams.parameter02 = 2;
            g_FieldFxSpriteParams.extent_x = 0x20;
            g_FieldFxSpriteParams.extent_y = 0x20;
            rotation.z = g_FieldFxCurrentAge << 5;
            FieldFx_SampleColorTrack(g_RoomLiftedSparkColors, &color,
                          (s16)spark->timer);
            kind = g_FieldFxSpriteParams.palette;
            palette = g_FieldFxPaletteTable[kind];
            if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
            clut = GetClut(0x10, palette);
            FieldFx_DrawShapeQuads((GteShortVector *)spark, &rotation, scale, scale,
                          (s16)g_FieldFxSpriteParams.parameter02 * ((s16)spark->timer / 4) + 0x20,
                          clut, 1, 0xA0, &color);
            break;
        }
        }
        break;
    }
    return 0;
}


/* Controller that sprays lifted sparks from the actor on odd frames of the
 * first minute, bursts sixteen more on frame 48, plays the sound on frame
 * two, and configures the sprite palette. */
int RoomEffect_LiftedSparkController(int mode, void *unused, intptr_t argument) {
    GteShortVector target;
    RoomDampedSpark *child;
    char *pool;
    int handle;
    int i;

    switch (mode) {
    case 0:
        pool = g_FieldFxActiveTaskSlot->end;
        return FieldFx_InitCallbackList(pool, 20, 32,
                             RoomEffect_LiftedSparkParticle);
    case 1:
        pool = (char *)g_FieldFxOwner->actor;
        FieldFx_GetActorPosition(pool, 0, (s16 *)&target);
        if ((g_FieldFxCurrentAge & 1) && g_FieldFxCurrentAge < 60) {
            pool = g_FieldFxActiveTaskSlot->end;
            child = FieldFx_AcquireCallbackSlot(pool);
            if (child) {
                child->x = target.x;
                child->y = target.y;
                child->z = target.z;
                child->vx = rand() % 80 - 40;
                child->vy = -(rand() % 60);
                child->vz = rand() % 80 - 40;
                child->state = 1;
                child->timer = 0;
            }
        }
        if (g_FieldFxCurrentAge == 48) {
            for (i = 0; i < 16; i++) {
                pool = g_FieldFxActiveTaskSlot->end;
                child = FieldFx_AcquireCallbackSlot(pool);
                if (child) {
                    child->x = target.x;
                    child->y = target.y;
                    child->z = target.z;
                    child->vx = rand() % 192 - 96;
                    child->vy = -(rand() % 170);
                    child->vz = rand() % 192 - 96;
                    child->state = 0;
                    child->timer = 0;
                }
            }
        }
        if (g_FieldFxCurrentAge == 2) {
            handle = FieldFx_GetOwnerProgress();
            FieldFx_PlaySoundAtOwner(0x5B4, handle);
        }
        if (g_FieldFxCurrentAge < 2) break;
        return 2;
    case 2: {
        int index = g_FieldFxTpageSlot3;
        int tpage = g_FieldFxTpageTable[index];
        g_FieldFxSpriteParams.palette = 3;
        g_FieldFxSpriteParams.parameter06 = 0;
        g_FieldFxSpriteParams.parameter0A = 0;
        g_FieldFxSpriteParams.depth = 8;
        g_FieldFxSpriteParams.tpage = tpage;
        break;
    }
    }
    return 0;
}

/* Original shared M188/M390 data and all three task descriptors. */
u8 g_RoomBouncingSparkColors[32] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xbe, 0x00, 0xbe, 0x10, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xbe, 0x08, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
u8 g_RoomWaveSparkColors[32] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x64, 0x04, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x80, 0xff, 0x04, 0x00, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00,
};
u8 g_RoomCometSparkColors[40] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x80, 0x80, 0x80, 0x02, 0x00, 0x00, 0x00, 0x00,
    0xc8, 0xc8, 0xc8, 0x07, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xff, 0x07, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
u8 g_RoomLiftedSparkColors[32] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xbe, 0x00, 0xbe, 0x10, 0x00, 0x00, 0x00, 0x00,
    0xbe, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
RoomCometSparkTrail g_RoomCometSparkTrail;
static u16 s_SparkBarrageScript124[] = { 5, 0, 0, 1, 6, 39, 2, 1, 0, 4, 29, 0, 2, 0, 0, 0, 0, 0 };
FieldFxTaskProgram g_RoomSparkBarrageProgram124 = {
    {{RoomEffect_PhasedSparkController, RoomEffect_BouncingSparkController}, {4, 4}},
    RoomLib_SetArgA, s_SparkBarrageScript124
};
static u16 s_SparkBarrageScript125[] = { 5, 0, 0, 1, 6, 40, 2, 0, 0, 4, 25, 0, 2, 1, 0, 0, 0, 0 };
FieldFxTaskProgram g_RoomSparkBarrageProgram125 = {
    {{RoomEffect_WaveSparkController, RoomEffect_CometSparkController}, {4, 12}},
    RoomLib_SetArgB, s_SparkBarrageScript125
};
static u16 s_SparkBarrageScript127[] = { 1, 6, 42, 4, 20, 0, 2, 0, 0, 0, 0, 0 };
FieldFxTaskProgram g_RoomSparkBarrageProgram127 = {
    {{RoomEffect_LiftedSparkController}, {4}},
    NULL, s_SparkBarrageScript127
};
