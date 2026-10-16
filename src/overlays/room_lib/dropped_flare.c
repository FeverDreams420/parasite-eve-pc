/*
 * Dropped flares: the particle, the emitter that drops them and the setter
 * of the room's two flare arguments. The three functions follow each other
 * in this order in thirteen hospital rooms and in scenes e09 to e13, and
 * the unit's rodata (the particle's two seed initialisers, its two jump
 * tables, the emitter's rectangle) is one contiguous block in all of them.
 */

#include "common.h"
#include "pe1/field_texture_tables.h"
#include "pe1/gte.h"
#include "pe1/geom_state.h"
#include "pe1/battle.h"
#include "pe1/fieldfx_task.h"
#include "pe1/room_flare.h"

/* Mark player contact and advance the owning attack through the original
 * combat-state flags. */
#define ROOMEFFECT_DROPPED_FLARE_FLASH() do { \
    FieldActor *owner=g_FieldFxOwner->actor; \
    if ((owner->state->core_flags & 0x3F000000)==0x01000000) { \
        ((Combatant *)g_PlayerEntity->state)->stateFlags|=0x4000; \
        owner->state->core_flags=(owner->state->core_flags & 0xC0FFFFFF)|0x91000000; \
    } \
} while(0)

/* Particle that falls under gravity, flashes the scene when it hits the
 * walkable area, scatters children every other frame, and settles on the
 * floor; the lift argument scales the sprite and the scatter spread. */
int RoomEffect_DroppedFlareParticle(int mode, void *data, intptr_t argument) {
    RoomDroppedFlare *flare=data;
    RoomDroppedFlareSpawn *spawn=(void *)argument;
    GteRotation rotation = {0x400, 0, 0, 1};
    RoomFlareColor color = {0xC8, 0x08, 0x00, 0x00};
    GteShortVector floorPos;
    GteRotation floorSpin;
    RoomDroppedFlare *child;
    RenderMatrixSlot *matrixSlot;
    int lift = spawn->lift;
    int spread;
    int scale;
    int fade;

    switch (mode) {
    case 1:
        switch (flare->state) {
        case 0:
            flare->x += flare->velocity.x;
            flare->y += flare->velocity.y;
            flare->z += flare->velocity.z;
            flare->velocity.y += 2;
            if (FieldFx_IsPlayerWithinRadius(flare, lift * 50 / 4096)) {
                if (g_FieldFxTaskContext->flags) {
                    ROOMEFFECT_DROPPED_FLARE_FLASH();
                }
                flare->state = 1;
                flare->timer = 0;
            }
            if (g_FieldFxCurrentAge & 1) {
                spread = lift * 48 / 4096;
                child = FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
                if (child) {
                    child->x = flare->x;
                    child->y = flare->y;
                    child->z = flare->z;
                    child->x += rand() % spread - spread / 2;
                    child->y += rand() % spread - spread / 2;
                    child->z += rand() % spread - spread / 2;
                    child->state = 4;
                    child->swing = 0;
                    child->timer = 0;
                }
            }
            if (flare->y < g_RoomFloorY->y) break;
            flare->state = 2;
            flare->timer = 0;
            break;
        case 1:
            flare->timer++;
            flare->y += rsin((s16)flare->timer * 512) / 512;
            if ((s16)flare->timer < 8) break;
            return 1;
        case 2:
            flare->timer++;
            if ((s16)flare->timer < 16) break;
            return 1;
        case 3:
            flare->timer++;
            flare->y -= 2;
            if ((s16)flare->timer < 24) break;
            return 1;
        case 4:
            flare->timer++;
            flare->y += flare->swing;
            flare->swing += 2;
            if (flare->y >= g_RoomFloorY->y) {
                flare->swing = 0;
                flare->y = g_RoomFloorY->y;
            }
            if ((s16)flare->timer < 24) break;
            return 1;
        }
        break;
    case 2:
    {
        int unit = 0x20;
        int index = g_FieldFxTpageSlot4;
        int tpage;
        g_FieldFxSpriteParams.parameter00 = unit;
        g_FieldFxSpriteParams.parameter02 = 2;
        g_FieldFxSpriteParams.extent_x = unit;
        g_FieldFxSpriteParams.extent_y = unit;
        tpage = g_FieldFxTpageTable[index];
        g_FieldFxSpriteParams.palette = 4;
        g_FieldFxSpriteParams.parameter06 = 0;
        g_FieldFxSpriteParams.tpage = tpage;
        switch (flare->state) {
        case 0:
            {
            u16 clut;
            int kind;
            int palette;
            scale = (rcos((s32)((u32)g_FieldFxCurrentAge * 256u)) / 8 + 0x800) / 2;
            scale = lift * scale / 4096;
            kind = g_FieldFxSpriteParams.palette;
            palette = g_FieldFxPaletteTable[kind];
            if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
            clut = GetClut(0x10, palette);
            FieldFx_DrawShapeQuads((GteShortVector *)flare, 0, scale, scale, 0x20, clut, 0xFF, 0x80,
                          (struct RenderColor *)&color);
            }
            {
            u16 clut;
            int kind;
            int palette;
            floorSpin.x = 0x400;
            floorSpin.y = 0;
            floorSpin.z = 0;
            floorSpin.flags = 1;
            floorPos.x = flare->x;
            floorPos.z = flare->z;
            floorPos.y = g_RoomFloorY->y;
            kind = g_FieldFxSpriteParams.palette;
            palette = g_FieldFxPaletteTable[kind];
            if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
            clut = GetClut(0x10, palette);
            FieldFx_DrawShapeQuads(&floorPos, &floorSpin, scale, scale, 0x20, clut, 2,
                          0x46, 0);
            }
            return 0;
        case 1:
            {
            u16 clut;
            int kind;
            int palette;
            kind = g_FieldFxSpriteParams.palette;
            palette = g_FieldFxPaletteTable[kind];
            if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
            clut = GetClut(0, palette);
            FieldFx_DrawShapeQuads((GteShortVector *)flare, 0, lift, lift,
                          (s16)g_FieldFxSpriteParams.parameter02 * (s16)flare->timer, clut, 1,
                          0x80, (struct RenderColor *)&color);
            }
            return 0;
        case 2:
            {
            u16 clut;
            int kind;
            int palette;
            kind = g_FieldFxSpriteParams.palette;
            palette = g_FieldFxPaletteTable[kind];
            if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
            clut = GetClut(0, palette);
            FieldFx_DrawShapeQuads((GteShortVector *)flare, &rotation, lift, lift,
                          (s16)g_FieldFxSpriteParams.parameter02 * ((s16)flare->timer / 2),
                          clut, 0, 0x80, (struct RenderColor *)&color);
            }
            break;
        case 3:
            {
            u16 clut;
            int kind;
            int palette;
            int unit2 = 0x10;
            int index2 = TEXTURE_SLOT_RECORD(g_FieldFxTpageSlot2);
            int tpage2;
            g_FieldFxSpriteParams.parameter00 = unit2;
            g_FieldFxSpriteParams.parameter02 = 1;
            g_FieldFxSpriteParams.extent_x = unit2;
            g_FieldFxSpriteParams.extent_y = unit2;
            tpage2 = g_FieldFxTpageTable[index2];
            g_FieldFxSpriteParams.palette = 2;
            g_FieldFxSpriteParams.parameter06 = 0;
            g_FieldFxSpriteParams.tpage = tpage2;
            fade = rcos(((s16)flare->timer * 1024) / 24) / 32;
            scale = ((s16)flare->timer * 2048) / 24 + 0x1000;
            scale = lift * scale / 4096;
            kind = g_FieldFxSpriteParams.palette;
            palette = g_FieldFxPaletteTable[kind];
            if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
            clut = GetClut(0x80, palette);
            FieldFx_DrawShapeQuads((GteShortVector *)flare, &rotation, scale, scale,
                          (s16)g_FieldFxSpriteParams.parameter02 * 2 + 0xFD, clut, 1, fade, 0);
            }
            return 0;
        case 4:
            matrixSlot = &g_ViewMatrixSlot;
            gte_ldrotmatrix(matrixSlot->value);
            gte_ldtransmatrix(matrixSlot->value);
            fade = rcos(((s16)flare->timer * 1024) / 24) / 32;
            FieldFx_DrawHaloedPoint(flare, &color, fade, 1);
            break;
        }
        break;
    }
    }
    return 0;
}

/* Drops the flares: mode 0 aims the emitter (at the scene anchor, or at the
 * player) and spawns the particle, mode 1 sheds a falling flare and a
 * rising spark on frame 1, mode 2 resets the sprite parameters. */
int RoomEffect_DroppedFlareEmitter(int mode, void *data, intptr_t argument) {
    RoomDroppedFlareEmitter *emitter=data;
    s16 *speed=(void *)argument;
    RoomFlareRect rect = {{0x00, 0x00, 0x00, 0x00, 0xF0, 0xFF, 0x00, 0x00}};

    switch (mode) {
    case 0:
        FieldFx_TransformMatrixPoint((RoomFxTransformOwner *)g_FieldFxOwner->actor, 0x13, (GteShortVector *)&rect, (GteShortVector *)emitter);
        switch (g_FieldFxTaskContext->variables[0]) {
        case 0:
            FieldFx_GetTransformLookAngles((struct RoomFxTransformOwner *)g_FieldFxOwner->actor, 0x13,
                          &emitter->angles);
            break;
        case 1:
            {
                s16 target[4];
                FieldFx_GetActorPosition((char *)D_8009D254, 0, target);
                FieldFx_CalculateLookAngles((GteShortVector *)emitter, (GteShortVector *)target, &emitter->angles);
                emitter->angles.x = 0x180;
            }
            break;
        }
        if (g_FieldFxTaskContext->flags != 0) {
            FieldActor *owner=g_FieldFxOwner->actor;
            if (owner && owner->state) {
                u8 *state = owner->state->action_state;
                if (*state == 1) {
                    *state = 2;
                }
            }
        }
        return FieldFx_InitCallbackList(g_FieldFxActiveTaskSlot->end, 0x14, 0x18,
                             RoomEffect_DroppedFlareParticle);
    case 1:
        if (g_FieldFxCurrentAge == mode) {
            RoomDroppedFlare *flare = FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
            if (flare != 0) {
                flare->x = emitter->x;
                flare->y = emitter->y;
                flare->z = emitter->z;
                FieldFx_AnglesToVector(&emitter->angles, *speed, &flare->velocity);
                flare->state = 0;
                flare->timer = 0;
            }
            flare = FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
            if (flare != 0) {
                flare->x = emitter->x;
                flare->y = emitter->y;
                flare->z = emitter->z;
                flare->state = 3;
                flare->timer = 0;
            }
            FieldFx_PlaySoundAtOwner(0x586, FieldFx_GetOwnerProgress());
        }
        if (g_FieldFxCurrentAge >= 2) {
            return 2;
        }
        break;
    case 2:
        g_FieldFxSpriteParams.parameter0A = 0;
        g_FieldFxSpriteParams.depth = 8;
        break;
    }
    return 0;
}

/* Stores the room's two flare arguments and returns the first. */
intptr_t RoomEffect_DroppedFlareSetArgs(int unused, intptr_t first, intptr_t second, intptr_t third) {
    (void)unused;(void)third;
    int *slot = g_RoomDroppedFlareArgs;
    *slot = (s32)(u32)first;
    g_RoomDroppedFlareArgs[1] = (s32)(u32)second;
    return (intptr_t)slot;
}

/* Whole original task122 module data; the two script parameters share one
 * contiguous native array just as the resident overlay words do. */
int g_RoomDroppedFlareArgs[2];
u16 g_RoomDroppedFlareScript[18]={5,0,0,1,6,37,4,66,0,1,0,1,2,0,0,0,0,0};
FieldFxTaskProgram g_RoomDroppedFlareProgram={{{RoomEffect_DroppedFlareEmitter},{16}},RoomEffect_DroppedFlareSetArgs,g_RoomDroppedFlareScript};
