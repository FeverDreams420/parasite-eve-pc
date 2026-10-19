/*
 * The homing flash: a controller that anchors a flash to the actor, then
 * releases sparks that home on the player and flag the battle actor when
 * they arrive.
 *
 * room_m162, m163, m398, m400, m403, scene_e24 and scene_e25 link the same
 * two functions in this order, right after the room library, with the
 * spark's hit colour and the two rotation seeds as their only read-only
 * data; this unit is that object, compiled into each of them. The colour
 * ramp the controller blends through is room data (g_RoomFlashColorRamp),
 * named in each room's symbol file.
 */

#include "pe1/gte.h"
#include "pe1/geom_state.h"
#include "pe1/battle.h"
#include "pe1/room_module.h"
#include "pe1/room_homing_flash.h"

static const RoomSoundBurstColor s_HomingSparkHitColor = { 0xFF, 0x80, 0, 0 };
static const GteRotation s_HomingSparkRotation = { 0x400, 0, 0, 1 };
static const GteRotation s_FlashSpriteRotation = { 0, 0, -100, 0 };

/* Spark that homes on the player, flags the battle actor when it arrives,
 * then burns out; sub-states 1 and 2 play the hit and fade animations. */
int RoomEffect_HomingSpark(int mode, void *data, intptr_t argument) {
    RoomHomingSpark *spark = data;
    RoomHomingSparkParams *params = (void *)argument;
    RoomSoundBurstColor color = s_HomingSparkHitColor;
    GteRotation rotation = s_HomingSparkRotation;
    GteShortVector angles;
    GteShortVector target;
    RoomHomingSpark *child;
    char *pool;
    u16 clut;
    int fade;

    switch (mode) {
    case 1:
        switch (spark->state) {
        case 0:
            FieldFx_GetActorPosition((char *)D_8009D254, 0, (s16 *)&target);
            FieldFx_CalculateLookAngles(spark, &target, &angles);
            angles.x = 0;
            FieldFx_EaseAngles((unsigned short *)&angles, &spark->hx, params->speed);
            FieldFx_AnglesToVector((GteShortVector *)&spark->hx, params->distance, &target);
            spark->x += target.x;
            spark->y += target.y;
            spark->z += target.z;
            if (FieldFx_IsPlayerWithinRadius(spark, 50)) {
                if (g_FieldFxTaskContext->flags) {
                    FieldActor *owner = g_FieldFxOwner->actor;
                    if (owner && owner->state &&
                        (owner->state->core_flags & 0x3F000000) == 0x01000000) {
                        if (g_PlayerEntity && g_PlayerEntity->state)
                            ((Combatant *)g_PlayerEntity->state)->stateFlags |= 0x4000;
                        owner->state->core_flags = (owner->state->core_flags & 0xC0FFFFFF) | 0x11000000;
                        owner->state->core_flags |= 0x80000000;
                    }
                }
                spark->state = 1;
                spark->timer = 0;
                if (spark->soundHandle != -1) {
                    Akao_Cmd_21(spark->soundHandle, 0);
                }
            }
            if (g_FieldFxCurrentAge & 1) {
                pool = g_FieldFxActiveTaskSlot->end;
                child = FieldFx_AcquireCallbackSlot(pool);
                if (child) {
                    child->x = spark->x;
                    child->y = spark->y;
                    child->z = spark->z;
                    child->state = 2;
                    child->timer = 0;
                }
            }
            if (g_FieldFxCurrentAge < 36) return 0;
            if (spark->soundHandle != -1) {
                Akao_Cmd_21(spark->soundHandle, 0);
            }
            return 1;
        case 1:
            spark->timer++;
            spark->y -= 8;
            if (spark->timer < 8) break;
            return 1;
        case 2:
            spark->timer++;
            if (spark->timer < 16) break;
            return 1;
        }
        break;
    case 2:
        switch (spark->state) {
        case 0: {
            int kind;
            int palette;
            fade = (g_FieldFxCurrentAge & 1) ? 0xBE : 0x80;
            kind = g_FieldFxSpriteParams.palette;
            palette = g_FieldFxPaletteTable[kind];
            if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
            clut = GetClut(0x10, palette);
            FieldFx_DrawShapeQuads((GteShortVector *)spark, &rotation, 0x1000, 0x1000, 0x20, clut, 1,
                          fade / 2, 0);
            break;
        }
        case 1: {
            int kind;
            int palette;
            kind = g_FieldFxSpriteParams.palette;
            palette = g_FieldFxPaletteTable[kind];
            if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
            clut = GetClut(0, palette);
            FieldFx_DrawShapeQuads((GteShortVector *)spark, &rotation, 0x1000, 0x1000,
                          (s16)g_FieldFxSpriteParams.parameter02 * spark->timer, clut, 1,
                          0x80, (struct RenderColor *)&color);
            break;
        }
        case 2: {
            int kind;
            int palette;
            fade = 0x29 - spark->timer * 41 / 16;
            kind = g_FieldFxSpriteParams.palette;
            palette = g_FieldFxPaletteTable[kind];
            if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
            clut = GetClut(0x10, palette);
            FieldFx_DrawShapeQuads((GteShortVector *)spark, &rotation, 0x1000, 0x1000, 0x20, clut, 1,
                          fade, 0);
            break;
        }
        }
        break;
    }
    return 0;
}

/* Controller that anchors a flash to the actor, plays a sound, waits sixteen
 * frames, releases two particles, then draws a pulsing sprite that blends
 * into the actor position. */
int RoomEffect_FlashSpriteController(int mode, void *data, intptr_t argument) {
    RoomFlashSpriteState *state = data;
    RoomFlashSpriteSpawn *spawn = (void *)argument;
    GteRotation rotation = s_FlashSpriteRotation;
    GteShortVector target;
    GteShortVector output;
    RoomSoundBurstColor color;
    RoomFlashSpriteChild *child;
    RenderMatrixSlot *matrixSlot;
    char *pool;
    int handle;
    int kind;
    int palette;
    u16 clut;
    int scale;
    int blend;
    int opacity;
    int frame;
    int unit;

    switch (mode) {
    case 0:
        pool = (char *)g_FieldFxOwner->actor;
        FieldFx_GetTransformLookAngles((struct RoomFxTransformOwner *)pool, 0,
                      (GteShortVector *)&state->ax);
        state->soundHandle = -1;
        switch (g_FieldFxTaskContext->variables[0]) {
        case 0:
            state->attachment = 0x20;
            state->ay -= spawn->lift;
            handle = FieldFx_GetOwnerProgress();
            handle = FieldFx_PlaySoundAtOwner(0x58D, handle);
            state->soundHandle = handle;
            break;
        case 1:
            state->attachment = 0x16;
            break;
        case 2:
            state->attachment = 0x1B;
            state->ay += spawn->lift;
            break;
        }
        pool = (char *)g_FieldFxOwner->actor;
        FieldFx_TransformMatrixPoint((struct RoomFxTransformOwner *)pool, state->attachment, (GteShortVector *)&rotation, (GteShortVector *)state);
        state->y = g_RoomFloorY->y;
        FieldFx_AnglesToVector((GteShortVector *)&state->ax, 0x12C, &target);
        state->x += target.x;
        state->y += target.y;
        state->z += target.z;
        if (g_FieldFxTaskContext->flags) {
            FieldActor *owner = g_FieldFxOwner->actor;
            if (owner && owner->state && owner->state->action_state) {
                u8 *flag = owner->state->action_state;
                if (*flag == 1) *flag = 2;
            }
        }
        state->state = 0;
        state->frame = 0;
        pool = g_FieldFxActiveTaskSlot->end;
        return FieldFx_InitCallbackList(pool, 24, 16,
                             RoomEffect_HomingSpark);
    case 1:
        switch (state->state) {
        case 0:
            state->frame++;
            if ((s16)state->frame < 16) break;
            state->state = 1;
            state->frame = 0;
            break;
        case 1:
            state->frame++;
            if ((s16)state->frame < 4) break;
            state->state = 2;
            state->frame = 0;
            pool = g_FieldFxActiveTaskSlot->end;
            child = FieldFx_AcquireCallbackSlot(pool);
            if (child) {
                child->x = state->x;
                child->y = state->y;
                child->z = state->z;
                child->wx = state->ax;
                child->wy = state->ay;
                child->wz = state->az;
                child->wx = 0;
                child->state = 0;
                child->timer = 0;
                child->soundHandle = state->soundHandle;
            }
            pool = g_FieldFxActiveTaskSlot->end;
            child = FieldFx_AcquireCallbackSlot(pool);
            if (child) {
                child->x = state->x;
                child->y = state->y;
                child->z = state->z;
                child->state = 1;
                child->timer = 0;
            }
            break;
        case 2:
            state->frame++;
            if ((s16)state->frame < 4) break;
            return 2;
        }
        break;
    case 2:
        pool = (char *)g_FieldFxOwner->actor;
        FieldFx_TransformMatrixPoint((struct RoomFxTransformOwner *)pool, state->attachment, (GteShortVector *)&rotation, &target);
        {
            int index;
            int tpage;
            unit = 0x20;
            index = g_FieldFxTpageSlot4;
            g_FieldFxSpriteParams.parameter00 = unit;
            g_FieldFxSpriteParams.parameter02 = 2;
            g_FieldFxSpriteParams.extent_x = unit;
            g_FieldFxSpriteParams.extent_y = unit;
            tpage = g_FieldFxTpageTable[index];
            g_FieldFxSpriteParams.palette = 4;
            g_FieldFxSpriteParams.parameter06 = 0;
            g_FieldFxSpriteParams.parameter0A = 0;
            g_FieldFxSpriteParams.depth = 0xC;
            g_FieldFxSpriteParams.tpage = tpage;
        }
        switch (state->state) {
        case 0:
            rsin((s16)state->frame * 64);
            opacity = (state->frame & 1) ? 0x40 : 0x80;
            scale = rsin((s16)state->frame * 64);
            kind = g_FieldFxSpriteParams.palette;
            palette = g_FieldFxPaletteTable[kind];
            if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
            clut = GetClut(0x10, palette);
            FieldFx_DrawShapeQuads((GteShortVector *)&target, 0, scale, scale, 0x20, clut, 1, opacity, 0);
            break;
        case 1:
            blend = (s16)state->frame * 1024;
            LoadAverageShort12(&target, state, 0x1000 - blend, blend, &output);
            FieldFx_SampleColorTrack(g_RoomFlashColorRamp, &color,
                          (s16)state->frame);
            matrixSlot = &g_ViewMatrixSlot;
            gte_ldrotmatrix(matrixSlot->value);
            gte_ldtransmatrix(matrixSlot->value);
            FieldFx_DrawGlowLine(&target, &output, (u8 *)&color, (u8 *)&color, 0, 0x80, 1);
            break;
        case 2:
            frame = (s16)state->frame;
            if (frame >= 5) break;
            FieldFx_SampleColorTrack(g_RoomFlashColorRamp, &color, frame + 4);
            matrixSlot = &g_ViewMatrixSlot;
            gte_ldrotmatrix(matrixSlot->value);
            gte_ldtransmatrix(matrixSlot->value);
            FieldFx_DrawGlowLine(&target, (GteShortVector *)state, (u8 *)&color, (u8 *)&color, 0, 0x80, 1);
            break;
        }
        break;
    }
    return 0;
}

/* USA M162/M163/M398/M400/M403 share this complete module, including
 * descriptor 80193254, script 801931E4 and colour track 80193234. */
u8 g_RoomFlashColorRamp[32] = {
    0,0,0,0,0,0,0,0, 128,0,0,4,0,0,0,0,
    255,128,0,4,0,0,0,0, 0,0,0,0,0,0,0,0
};
static u16 s_HomingFlashScript[40] = {
    5,0,0, 1,6,38, 4,8,0, 1,0,0, 2,0,0, 4,3,0,
    1,0,1, 2,0,0, 4,3,0, 1,0,2, 2,0,0, 4,3,0, 0,0,0,0
};
FieldFxTaskProgram g_RoomHomingFlashProgram = {
    .table = {.callbacks = {RoomEffect_FlashSpriteController},
              .sizes = {sizeof(RoomFlashSpriteState)}},
    .initialize = RoomLib_SetArgs3,
    .script = s_HomingFlashScript,
};
