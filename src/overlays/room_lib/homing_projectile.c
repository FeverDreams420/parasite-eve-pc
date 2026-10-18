/*
 * The thrown homing projectile: its spark callback, the flight itself, the
 * throw controller that drives the actor's throw animation and seeds the
 * model, and the entry the scene script sets the launch parameters with.
 *
 * room_m141, room_m146, room_m153, room_m154, room_m328, scene_e02, scene_e04
 * and scene_e05 link these four functions in this order as the last code of
 * the overlay; this unit is that object, compiled into each of them. The six
 * rooms and scene_e02 also carry the same 0x28 bytes of seeds in their data
 * (homing_projectile_seeds.c); the seeds of scene_e04 and scene_e05
 * lie past the end of their extracted image and are named in their symbol
 * files. The launch parameters live in each room's own data
 * (g_RoomHomingLaunch).
 */

#include "pe1/field_texture_tables.h"
#include "pe1/gte.h"
#include "pe1/psyq_gpu.h"
#include "pe1/room_homing_model.h"
#include "pe1/field_actor.h"
#include <string.h>

/* Spark left where the projectile bounces: lives eight frames, drawn as a
 * pulsing sprite in the current palette. */
int RoomEffect_HomingProjectileSpark(int mode,void *data,intptr_t argument) {
    GteShortVector *spark=data;
    int size;
    int kind;
    int palette;
    int frame;
    int pulse;
    u16 clut;

    if (mode == 1) {
        if (g_FieldFxCurrentAge >= 9) {
            return 1;
        }
    } else if (mode == 2) {
        frame = g_FieldFxCurrentAge - 1;
        kind = g_FieldFxSpriteParams.palette;
        size = rsin((((u32)frame << 9) & 0x3E00) >> 2) + 0x200;
        palette = g_FieldFxPaletteTable[kind];
        /* The pulse reads the cosine half through its own copy of the
         * frame, as retail does. */
        pulse = frame;
        if (kind == 4 && g_FieldFxAltPaletteFlag) {
            palette += 4;
        }
        clut = GetClut(0x80, palette);
        FieldFx_DrawShapeQuads(spark, &g_RoomHomingSparkRotation, (s16)size, (s16)size,
                      (s16)g_FieldFxSpriteParams.parameter02 * 2 + 0xFD, clut, 1,
                      rcos((((u32)pulse << 9) & 0x3E00) >> 2) >> 5,
                      &g_RoomHomingSparkColor);
    }
    return 0;
}

/* Thrown projectile: launched from the actor's hand matrix, it flies along
 * its heading, reflects off the walkable polygon's edges (one retry per
 * frame) or turns away from the player once outside, drops onto the floor
 * with a damped bounce, then reports its resting position through the
 * scene script.  Drawn as a glow sprite plus a floor ring while in flight. */
int RoomEffect_HomingProjectile(int mode,void *data,intptr_t argument) {
    RoomHomingModel *model;
    /* Retail keeps this address in script variables1/2. This program never
     * uses variable opcodes; widen across1..4 and keep variables5/6 intact. */
    memcpy(&model,&g_FieldFxTaskContext->variables[1],sizeof(model));
    Vec3Fixed playerPosition={g_PlayerEntity->pos_x,g_PlayerEntity->pos_y,g_PlayerEntity->pos_z};
    GteVector position;
    s16 vector[4];
    s16 rotated[4];
    GteMatrix matrix;
    s32 area;
    FieldActor *actor;
    int i;

    switch (mode) {
    case 0:
        model->state = 0;
        model->angle = g_FieldFxOwner->actor->rot_y;
        return FieldFx_InitCallbackList(g_FieldFxActiveTaskSlot->end, 8, 4,
                             RoomEffect_HomingProjectileSpark);
    case 1:
        actor = g_FieldFxOwner->actor;
        switch (model->state) {
        case 0: {
            GteMatrix transform;
            int frame = actor->anim.parts.integer;
            if (actor->mode == 8) {
                if (frame < 8) {
                    model->matrixIndex = 0x1A;
                } else {
                    model->matrixIndex = 9;
                }
            } else if (actor->mode == 9) {
                model->matrixIndex = 9;
                if (frame >= 2) {
                    model->state = 1;
                    model->soundHandle = Asset_Find08w(
                        0x585, actor->state->progress,
                        (s16)actor->render_object.matrices[9].translation[0],
                        (s16)actor->render_object.matrices[9].translation[1],
                        (s16)actor->render_object.matrices[9].translation[2]);
                }
            }
            if (model->matrixIndex == 9) {
                vector[0] = -0x10;
                vector[1] = 0;
                vector[2] = -0x40;
            } else {
                vector[0] = 0;
                vector[1] = 0;
                vector[2] = 0;
            }
            memcpy(&transform,&actor->render_object.matrices[model->matrixIndex],sizeof(transform));
            gte_ldrotmatrix(&transform);
            gte_ldtransmatrix(&transform);
            gte_ldv0(vector);
            gte_rtv0tr_mac();
            gte_stsv(vector);
            for (i = 0; i < 3; i++) {
                model->position[i].word = (s32)((u32)(s32)vector[i] << 16);
            }
            return 0;
        }
        case 1: {
            s16 retries = 1;
            int point;
            int previous;
            int current;
            int sine;
            vector[0] = 0;
            vector[1] = 0;
            vector[2] = (s32)(0u-(u32)model->speed) >> 12;
            matrix.t[0] = 0;
            matrix.t[1] = 0;
            matrix.t[2] = 0;
            do {
                i = rcos(model->angle);
                sine = rsin(model->angle);
                matrix.m[0][0] = i;
                matrix.m[0][2] = sine;
                matrix.m[2][0] = -sine;
                matrix.m[2][2] = i;
                matrix.t[2] = 0;
                matrix.t[1] = 0;
                matrix.t[0] = 0;
                matrix.m[2][1] = 0;
                matrix.m[1][2] = 0;
                matrix.m[1][0] = 0;
                matrix.m[0][1] = 0;
                matrix.m[1][1] = 0x1000;
                gte_ldrotmatrix(&matrix);
                gte_ldtransmatrix(&matrix);
                gte_ldv0(vector);
                gte_rtv0tr_mac();
                gte_stsv(rotated);
                position.x = (s32)((u32)model->position[0].word + ((u32)(s32)rotated[0] << 12));
                position.z = (s32)((u32)model->position[2].word + ((u32)(s32)rotated[2] << 12));
                point = ((u32)position.x >> 16) | (position.z & 0xFFFF0000);
                previous = (u16)g_FloorPolygonVertices[g_FloorPolygonVertexCount-1].x |
                           ((u32)(u16)g_FloorPolygonVertices[g_FloorPolygonVertexCount-1].z<<16);
                i = 0;
                while (i < g_FloorPolygonVertexCount) {
                    current = (u16)g_FloorPolygonVertices[i].x | ((u32)(u16)g_FloorPolygonVertices[i].z<<16);
                    gte_ldsxy0(point);
                    gte_ldsxy2(previous);
                    gte_ldsxy1(current);
                    gte_nclip();
                    gte_stmac0(&area);
                    if (area < 0) break;
                    i++;
                    previous = current;
                }
                if (i >= g_FloorPolygonVertexCount) {
                    model->escaped = 1;
                    break;
                }
                if (model->escaped == 0) {
                    model->angle = (FieldFx_VecToAngle((Vec3Fixed *)&model->position[0].word,
                                                        &playerPosition) + 0x800) & 0xFFF;
                    break;
                }
                i = ratan2((s16)current - (s16)previous,
                              (current >> 16) - (previous >> 16)) & 0xFFF;
                if (i >= 0x800) i -= 0x800;
                retries--;
                model->angle = (i + (i - (u16)model->angle)) & 0xFFF;
            } while (retries != -1);
            model->position[0].word = position.x;
            model->position[2].word = position.z;
            if (retries <= 0) {
                model->speed = (s32)((u32)model->speed*(u32)(s32)model->damping) / 16;
            }
            if (model->gravity != 0) {
                model->position[1].word=(s32)((u32)model->position[1].word-(u32)model->lift);
                if ((model->position[1].word >> 16) >= model->floor - 0x20) {
                    model->position[1].word = (s32)(((u32)model->floor-0x20u)<<16);
                    model->lift = (s32)((0u-(u32)model->lift)*(u32)(s32)model->bounce) / 16;
                    model->speed = (s32)((u32)model->speed*(u32)(s32)model->damping) / 16;
                    if (model->lift <= 0x2FFFFF) {
                        model->gravity = 0;
                    } else {
                        GteShortVector *spark = FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
                        if (spark != 0) {
                            spark->x = model->position[0].part.integer;
                            spark->y = g_RoomFloorY->raw;
                            spark->z = model->position[2].part.integer;
                        }
                    }
                } else {
                    model->lift=(s32)((u32)model->lift-(u32)model->gravity);
                }
            } else {
                model->speed=(s32)((u32)model->speed-(u32)model->drag);
                if (model->speed <= 0) model->finished = 1;
            }
            if (model->floor - 0x200 < model->position[1].part.integer) {
                area = FieldFx_GetDistanceXZ((Vec3Fixed *)&position.x, &playerPosition);
                if (area < 0x80) model->finished = 1;
            }
            if (model->finished == 0) return 0;
            if (model->soundHandle != -1) {
                Akao_Cmd_21(model->soundHandle, 0);
            }
            model->position[0].word = model->position[0].part.integer;
            model->position[1].word = model->position[1].part.integer + 0x20;
            model->position[2].word = model->position[2].part.integer;
            {
                int handle = Scene_LoadRoomAssets(0x25, actor);
                u32 n = 0;
                do {
                    Pm_SendCmd(handle, 0, n + 1,
                                  ((s32 *)model)[g_RoomHomingReportFields[n]],
                                  0, 0);
                    n++;
                } while (n < 5);
            }
            return 1;
        }
        }
        return 1;
    case 2: {
        GteShortVector sprite;
        int axis;
        int frame;
        int scale;
        int intensity;
        u16 clut;
        int kind;
        int palette;
        u16 *tpages;
        for (axis = 0; axis < 3; axis++) {
            ((s16 *)&sprite)[axis] = model->position[axis].part.integer;
        }
        frame = g_FieldFxCurrentAge;
        if (frame < 8) {
            scale = rsin((((u32)frame << 10) & 0x3C00) >> 2) + 0x1000;
            intensity = 0x80;
        } else {
            intensity = rcos((((u32)frame << 12) & 0x3000) >> 2) / 64 + 0x60;
            scale = 0x1000;
        }
        g_FieldFxSpriteParams.parameter00 = 0x20;
        {
            int index = TEXTURE_SLOT_RECORD(g_FieldFxTpageSlot4);
            int tpage;
            g_FieldFxSpriteParams.parameter02 = 2;
            g_FieldFxSpriteParams.extent_x = 0x20;
            g_FieldFxSpriteParams.extent_y = 0x20;
            g_FieldFxSpriteParams.extent_x = 0x20;
            g_FieldFxSpriteParams.extent_y = 0x20;
            tpage = g_FieldFxTpageTable[index];
            g_FieldFxSpriteParams.palette = 4;
            g_FieldFxSpriteParams.parameter06 = 0;
            g_FieldFxSpriteParams.parameter0A = 0;
            g_FieldFxSpriteParams.depth = 0;
            g_FieldFxSpriteParams.tpage = tpage;
        }
        kind = g_FieldFxSpriteParams.palette;
        palette = g_FieldFxPaletteTable[kind];
        if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
        tpages = g_FieldFxTpageTable;
        clut = GetClut(0x30, palette);
        FieldFx_DrawShapeQuads(&sprite, 0, scale, scale, 0x42, clut, 1, intensity, 0);
        if (model->state != 0) {
            sprite.y = model->floor;
            FieldFx_DrawStarFan(&sprite, 0x50, 0x50, 8,
                          &g_RoomHomingRingRotation, 0x1000, 0x1000,
                          &g_RoomHomingRingInner,
                          &g_RoomHomingRingOuter, intensity, 1);
        }
        g_FieldFxSpriteParams.parameter00 = 0x10;
        {
            int index = TEXTURE_SLOT_RECORD(g_FieldFxTpageSlot2);
            int tpage;
            g_FieldFxSpriteParams.parameter02 = 1;
            g_FieldFxSpriteParams.extent_x = 0x10;
            g_FieldFxSpriteParams.extent_y = 0x10;
            tpage = tpages[index];
            g_FieldFxSpriteParams.palette = 2;
            g_FieldFxSpriteParams.parameter06 = 0;
            g_FieldFxSpriteParams.tpage = tpage;
        }
        break;
    }
    }
    return 0;
}

/* Throw controller: seeds the model from the launch parameters, then
 * follows the actor's throw animation until it either releases the
 * projectile (the cue reaches the last frame) or is interrupted. */
int RoomEffect_HomingProjectileThrow(int mode,void *data,intptr_t argument) {
    RoomHomingModel *model=data;
    FieldActor *actor=g_FieldFxOwner->actor;
    u8 *cue;
    int motion;
    u16 frame;
    u16 lastFrame;

    if (mode == 0) {
        int floor;
        memcpy(&g_FieldFxTaskContext->variables[1],&model,sizeof(model));
        actor->state->core_flags |= 0x40000000;
        floor = g_RoomFloorY->y;
        model->matrixIndex = 0x1A;
        model->finished = 0;
        model->escaped = 0;
        model->released = 0;
        model->armed = 0;
        model->soundHandle = -1;
        model->floor = floor;
        model->speed = g_RoomHomingLaunch.speed;
        model->drag = g_RoomHomingLaunch.drag;
        model->lift = g_RoomHomingLaunch.lift;
        model->gravity = g_RoomHomingLaunch.gravity;
        model->scriptArg0 = g_RoomHomingLaunch.scriptArg0;
        model->scriptArg1 = g_RoomHomingLaunch.scriptArg1;
        model->bounce = g_RoomHomingLaunch.bounce;
        model->damping = g_RoomHomingLaunch.damping;
    } else if (mode == 1) {
        if (model->released == 0) {
            if (actor->state == 0 || Battle_GetEnemyContextField((struct FieldActor *)actor, 0x2C) <= 0) {
                model->released = 1;
            } else if ((actor->state->core_flags & 0x1800) != 0) {
                model->released = 1;
            } else if ((int)((actor->state->core_flags >> 1) & 7) > 0) {
                model->released = 1;
            }
        }
        if (model->state == 0 && model->armed != 0 && (!actor->state || !actor->state->action_state || *actor->state->action_state == 0)) {
            model->released = 1;
        }
        if (model->released != 0) {
            if (model->state != 0) {
                return model->finished != 0;
            }
            model->state = -1;
            return 1;
        }

        cue = actor->state->action_state;
        if (cue && *cue == 1) {
            *cue = 2;
            model->armed = 1;
        }
        if (model->finished != 0) {
            model->finished++;
        }
        if (actor->mode != 9) {
            return 0;
        }
        /* Two reads of the frame, as retail: the last-frame test keeps
         * its own copy. */
        frame = actor->anim.parts.integer;
        lastFrame = actor->anim.parts.integer;
        motion = actor->anim_step;
        if (motion >= 0) {
            if ((s16)frame >= 0x1C && model->finished < 0x18) {
                actor->anim_step=(s32)(0u-(u32)motion);
                return 0;
            }
            if ((s16)lastFrame < actor->action - 1) {
                return 0;
            }
            cue = actor->state->action_state;
            if (cue && *cue == 2) {
                *cue = 4;
            }
            return 1;
        }
        if ((s16)frame < 0x17) {
            actor->anim_step=(s32)(0u-(u32)motion);
        }
    }
    return 0;
}

/* Scene script entry: mode 0 sets speed, drag and damping, mode 1 lift,
 * gravity and bounce, mode 2 the two values reported back. */
intptr_t RoomEffect_HomingProjectileConfigure(int mode,intptr_t first,intptr_t second,intptr_t third) {
    s32 a=(s32)first,b=(s32)second,c=(s32)third;
    switch (mode) {
    case 0:
        g_RoomHomingLaunch.speed = a;
        g_RoomHomingLaunch.drag = b;
        g_RoomHomingLaunch.damping = c;
        if (c == 0) {
            g_RoomHomingLaunch.damping = 8;
        }
        break;
    case 1:
        g_RoomHomingLaunch.lift = a;
        g_RoomHomingLaunch.gravity = b;
        g_RoomHomingLaunch.bounce = c;
        if (c == 0) {
            g_RoomHomingLaunch.bounce = 10;
        }
        break;
    case 2:
        g_RoomHomingLaunch.scriptArg0 = a;
        g_RoomHomingLaunch.scriptArg1 = b;
        break;
    }
    return 0;
}
