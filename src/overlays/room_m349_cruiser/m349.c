#include "pe1/room_m349_effects.h"

#include "room_m349.h"

/* Spark that jitters its velocity (states 0 and 1) or its position
 * (state 2) by a small random amount each frame and bounces off the floor
 * height; draws as a spinning flare, the last state with a glow ring. */
int RoomM349_FlareSparkCallback(int mode, RoomM349FlareSpark *spark) {
    GteRotation rotation = D_8018EFF4;
    RenderColor color = D_8018EFFC;
    int angle;
    int size;
    int fade;
    int frame;
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
            spark->vx = spark->vx * 63 / 64;
            spark->vz = spark->vz * 63 / 64;
            fall = (u16)spark->vy - 1;
            spark->vy = fall;
            if (spark->y >= g_RoomFloorY->y) {
                bounce = -(s16)fall;
                spark->vy = bounce;
            }
            spark->vx += (rand() & 0xF) - 7;
            spark->vy += (rand() & 0xF) - 7;
            spark->vz += (rand() & 0xF) - 7;
            if ((s16)spark->timer < 48) break;
            return 1;
        case 1:
            spark->timer++;
            spark->x += spark->vx;
            spark->y += spark->vy;
            spark->z += spark->vz;
            spark->vx = spark->vx * 63 / 64;
            spark->vz = spark->vz * 63 / 64;
            fall = (u16)spark->vy - 2;
            spark->vy = fall;
            if (spark->y >= g_RoomFloorY->y) {
                bounce = -(s16)fall;
                spark->vy = bounce;
            }
            spark->vx += (rand() & 7) - 3;
            spark->vy += (rand() & 7) - 3;
            spark->vz += (rand() & 7) - 3;
            if ((s16)spark->timer < 32) break;
            return 1;
        case 2:
            spark->timer++;
            spark->x += spark->vx;
            spark->y += spark->vy;
            spark->z += spark->vz;
            spark->vx = spark->vx * 63 / 64;
            spark->vz = spark->vz * 63 / 64;
            fall = (u16)spark->vy;
            spark->vy = fall;
            if (spark->y >= g_RoomFloorY->y) {
                bounce = -(s16)fall;
                spark->vy = bounce;
            }
            spark->x += (rand() & 7) - 3;
            spark->y += (rand() & 7) - 3;
            spark->z += (rand() & 7) - 3;
            spark->angle += 8;
            if ((s16)spark->timer < 8) break;
            return 1;
        }
        break;
    case 2:
        switch (spark->state) {
        case 0: {
            u16 clut;
            int kind;
            int palette;
            angle = ((s16)spark->timer << 10) / 48;
            fade = rsin(angle * 2) / 32;
            size = rcos(angle) / 2 + 0x2000;
            rotation.z = spark->angle - (s16)spark->timer * 8;
            frame = ((s16)spark->timer / 2) & 7;
            if (frame >= 4) frame += 12;
            g_FieldFxSpriteParams.depth = 0x226;
            kind = g_FieldFxSpriteParams.palette;
            palette = g_FieldFxPaletteTable[kind];
            if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
            clut = GetClut(0, palette);
            FieldFx_DrawShapeQuads((GteShortVector *)spark, &rotation, size, size,
                          (s16)g_FieldFxSpriteParams.parameter02 * frame, clut, 2,
                          fade / 3, 0);
            break;
        }
        case 1: {
            u16 clut;
            int kind;
            int palette;
            angle = (s16)spark->timer << 5;
            fade = rsin(angle * 2) / 32;
            size = rcos(angle) / 4 + 0x2000;
            rotation.z = spark->angle + (s16)spark->timer * 12;
            frame = ((s16)spark->timer / 2) & 7;
            if (frame >= 4) frame += 12;
            g_FieldFxSpriteParams.depth = 0x80;
            kind = g_FieldFxSpriteParams.palette;
            palette = g_FieldFxPaletteTable[kind];
            if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
            clut = GetClut(0, palette);
            FieldFx_DrawShapeQuads((GteShortVector *)spark, &rotation, size, size,
                          (s16)g_FieldFxSpriteParams.parameter02 * frame, clut, 3, fade, 0);
            break;
        }
        case 2: {
            u16 clut;
            int kind;
            int palette;
            fade = rcos((s16)spark->timer << 5) / 40;
            rotation.z = spark->angle;
            size = spark->size;
            g_FieldFxSpriteParams.depth = 0x80;
            kind = g_FieldFxSpriteParams.palette;
            palette = g_FieldFxPaletteTable[kind];
            if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
            clut = GetClut(0x10, palette);
            FieldFx_DrawShapeQuads((GteShortVector *)spark, &rotation, size, size * 2,
                          (s16)g_FieldFxSpriteParams.parameter02 * ((s16)spark->timer / 2) + 0x80,
                          clut, 1, 0x50, 0);
            FieldFx_DrawStarFan((GteShortVector *)spark, 500, 500, 8, 0, size, size,
                          &color, 0, fade, 3);
            break;
        }
        }
        break;
    }
    return 0;
}

/* Spawns flare sparks on a ring around the target every `period` frames
 * (and a second kind half a period later) until `lastFrame`, scatters
 * glints around a model joint on random even frames, stops the looping
 * sound 16 frames after the ring ends and draws two joint glows. */
int RoomM349_FlareRing(int mode, RoomM349FlareTarget *target,
                  RoomM349FlareParams *params) {
    GteShortVector joint;
    GteShortVector anchor;
    GteRotation rotation = D_8018F000;
    GteRotation rotation2 = D_8018F008;
    RenderColor color = D_8018EFFC;
    RoomM349FlareSpark *child;
    int radius;
    int period;
    int reach;
    int size;
    /* Ring angle while spawning, glow fade while drawing: one variable,
     * as retail keeps both in the same saved register. */
    int phase;

    switch (mode) {
    case 0:
        target->position.x = params->x;
        target->position.y = params->y;
        target->position.z = params->z;
        target->position.y = g_RoomFloorY->y;
        return FieldFx_InitCallbackList(ANIM_POOL_AS(RoomSparkChannel)->pool, 20, 70,
                             (FieldFxCallbackListCallback)RoomM349_FlareSparkCallback);
    case 1:
        FieldFx_GetActorPosition(ANIM_OWNER_AS(RoomSparkChannel)->pool, 0, (s16 *)&joint);
        radius = params->radius;
        period = params->period;
        if (g_FieldFxCurrentAge <= params->lastFrame) {
            if (g_FieldFxCurrentAge % period == 0) {
                child = FieldFx_AcquireCallbackSlot(ANIM_POOL_AS(RoomSparkChannel)->pool);
                if (child) {
                    child->x = target->position.x;
                    child->y = target->position.y;
                    child->z = target->position.z;
                    phase = rand();
                    reach = radius * (rand() & 0xFF) / 256;
                    child->x += rcos(phase) * reach / 4096;
                    child->y -= rand() & 0x1F;
                    child->z += rsin(phase) * reach / 4096;
                    child->angle = rand();
                    child->vx = rand() % 32 - 16;
                    child->vy = -(rand() % 32);
                    child->vz = rand() % 32 - 16;
                    child->state = 0;
                    child->timer = 0;
                }
            }
            if (g_FieldFxCurrentAge % period == period / 2) {
                child = FieldFx_AcquireCallbackSlot(ANIM_POOL_AS(RoomSparkChannel)->pool);
                if (child) {
                    child->x = target->position.x;
                    child->y = target->position.y;
                    child->z = target->position.z;
                    phase = rand();
                    reach = radius * (rand() & 0xFF) / 256;
                    child->x += rcos(phase) * reach / 4096;
                    child->y -= rand() & 0x1F;
                    child->z += rsin(phase) * reach / 4096;
                    child->angle = rand();
                    child->vx = rand() % 24 - 12;
                    child->vy = -(rand() % 24);
                    child->vz = rand() % 24 - 12;
                    child->state = 1;
                    child->timer = 0;
                }
            }
        }
        if (!(g_FieldFxCurrentAge & 1) && !(rand() & 1)) {
            child = FieldFx_AcquireCallbackSlot(ANIM_POOL_AS(RoomSparkChannel)->pool);
            if (child) {
                child->x = joint.x;
                child->y = joint.y;
                child->z = joint.z;
                child->x += (rand() & 0x1FF) - 0x100;
                child->y += rand() % 800 - 400;
                child->z += (rand() & 0x1FF) - 0x100;
                child->size = (rand() & 0x1FF) + 0x2AA;
                child->angle = rand();
                child->vx = rand() % 24 - 12;
                child->vy = -(rand() % 24);
                child->vz = rand() % 24 - 12;
                child->state = 2;
                child->timer = 0;
            }
        }
        if (g_FieldFxCurrentAge == params->lastFrame + 16 && target->sound != -1) {
            Akao_Cmd_21(target->sound, 0);
        }
        if (g_FieldFxCurrentAge < 16) break;
        return 2;
    case 2:
        g_FieldFxSpriteParams.parameter00 = 16;
        g_FieldFxSpriteParams.parameter02 = 1;
        g_FieldFxSpriteParams.extent_x = 16;
        g_FieldFxSpriteParams.extent_y = 16;
        {
            int tpage = g_FieldFxTpageTable[g_FieldFxTpageSlot2];
            g_FieldFxSpriteParams.palette = 2;
            g_FieldFxSpriteParams.parameter06 = 0;
            g_FieldFxSpriteParams.parameter0A = 0;
            g_FieldFxSpriteParams.depth = 4;
            g_FieldFxSpriteParams.tpage = tpage;
        }
        size = 0x2AA;
        phase = (rand() & 0x1F) + 0x20;
        FieldFx_TransformMatrixPoint(ANIM_OWNER_AS(RoomSparkChannel)->pool, 13, &rotation, &anchor);
        {
            u16 clut;
            int kind;
            int palette;
            kind = g_FieldFxSpriteParams.palette;
            palette = g_FieldFxPaletteTable[kind];
            if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
            clut = GetClut(0x20, palette);
            FieldFx_DrawShapeQuads(&anchor, 0, size, size,
                          (s16)g_FieldFxSpriteParams.parameter02 + 0xD8, clut, 1, phase,
                          &color);
        }
        FieldFx_TransformMatrixPoint(ANIM_OWNER_AS(RoomSparkChannel)->pool, 13, &rotation2, &anchor);
        {
            u16 clut;
            int kind;
            int palette;
            kind = g_FieldFxSpriteParams.palette;
            palette = g_FieldFxPaletteTable[kind];
            if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
            clut = GetClut(0x20, palette);
            FieldFx_DrawShapeQuads(&anchor, 0, size, size,
                          (s16)g_FieldFxSpriteParams.parameter02 + 0xD8, clut, 1, phase,
                          &color);
        }
        g_FieldFxSpriteParams.parameter00 = 0x40;
        g_FieldFxSpriteParams.parameter02 = 4;
        g_FieldFxSpriteParams.extent_x = 0x40;
        g_FieldFxSpriteParams.extent_y = 0x40;
        {
            int tpage = g_FieldFxTpageTable[g_FieldFxTpageSlot3];
            g_FieldFxSpriteParams.palette = 3;
            g_FieldFxSpriteParams.parameter06 = 0;
            g_FieldFxSpriteParams.parameter0A = 0;
            g_FieldFxSpriteParams.tpage = tpage;
        }
        break;
    }
    return 0;
}

short *RoomM349_SetFlareParams(int arg0, int arg1, int arg2, int arg3) {
    if (arg0 == 1) {
        g_RoomM349FlareParams = arg1;
        D_80190196 = arg2;
        D_80190198 = arg3;
    }
    if (arg0 == 2) {
        D_8019019C = arg1;
        D_801901A4 = arg2;
    } else {
        if (arg1 < 2) {
            arg1 = 2;
        }
        D_801901A0 = arg1;
    }
    return &g_RoomM349FlareParams;
}
