#include "common.h"
#include <string.h>
#include "pe1/field_actor.h"
#include "pe1/field_sprite_draw.h"
#include "pe1/field_sprite_params.h"
#include "pe1/field_texture_tables.h"
#include "pe1/fieldfx_calls.h"
#include "pe1/fieldfx_task.h"
#include "pe1/geom_state.h"
#include "pe1/gte.h"
#include "pe1/gte_types.h"
#include "pe1/psyq_gpu.h"
#include "pe1/random.h"
#include "pe1/render_object.h"
#include "pe1/room_floor.h"
#include "pe1/room_glint_ribbon.h"
#include "pe1/room_orbit_spin_burst.h"
#include "pe1/room_orbit_trail.h"
#include "pe1/room_spark.h"
#include "pe1/scene_e19_2_ember.h"
#include "pe1/scene_e19_2_homing_burst.h"
#include "pe1/scene_e19_2_spin_ray.h"
#include "pe1/scene_e19_2_twin_glow.h"
#include "pe1/scene_e19_blast.h"
#include "pe1/scene_e19_selection.h"
#include "pe1/scene_grab_head.h"
#include "pe1/scene_grab_trigger.h"

#include "e19_externs.h"
#include "scene_e19_shared.h"
#include "scene_e19_state.h"

typedef struct SceneE19FrameGateLink {
    u8 reserved_00[0x16];
    u16 winLo;
} SceneE19FrameGateLink;

typedef struct SceneE19FrameGateEntity {
    u8 reserved_00[8];
    SceneE19FrameGateLink *link;
} SceneE19FrameGateEntity;

typedef struct SceneE19Effect {
    u16 x, y, z, pad;
    s16 state;
    s16 frame;
} SceneE19Effect;

/* Original homing particle state and callback, retained in this module. */
typedef struct SceneE19HomingParticle {
    GteShortVector position; /* position.pad is the particle index */
    GteShortVector angles;
    GteShortVector target;
    s16 speed;
    s16 turnPhase;
    s16 state;
    s16 timer;
} SceneE19HomingParticle;

void RoomLib_GrabOnApproach(SceneGrabHead *head) {
    SceneGrabTrigger *trigger = (SceneGrabTrigger *)head;
    FieldActor *actor = trigger->actor;
    SceneGrabControl *control = &trigger->control;
    FieldActorState *state = actor->state;

    if (state != 0 && state->control10.command_value <= 1000000) {
        *trigger->control.signal = 4;
        SceneE19_ResetGrabTarget((char *)trigger);
        return;
    }
    if (g_PlayerEntity->mode < 18) {
        int frame = actor->anim.parts.integer;

        if (frame < 4) {
            return;
        }
        if (frame < 12) {
            SceneGrabPoint point;
            SceneGrabPoint *target = &point;

            target->x = (s32)((u32)actor->render_object.matrices[11].translation[0] << 16);
            target->z = (s32)((u32)actor->render_object.matrices[11].translation[2] << 16);
            if (FieldFx_GetDistanceXZ((Vec3Fixed *)&g_PlayerEntity->pos_x, (Vec3Fixed *)&target->x) < 0x140) {
                FieldActor *player;

                control->callback = SceneE19_GrabSequence;
                Battle_BeginPlayerAction();
                player = g_PlayerEntity;
                player->render_object.animation_source = &actor->render_object;
                player->render_object.animation_state = 4;
                player->render_object.animation_id = 11;
                player->flags |= 0x10000;
                player->render_object.flags_9C |= 0x400;
                player->render_object.model_matrix = control->player_matrix;
                control->grabbed = 1;
                {
                    FieldActor *mover = g_PlayerEntity;

                    control->saved_x = mover->pos_x;
                    control->saved_z = mover->pos_z;
                }
            }
            return;
        }
    }
    *control->signal = 3;
    control->callback = SceneE19_RunAfterFrame37;
}

/* Grab sequence driver: pins the player to the selection matrix and steps
 * the partner's animation events. Both frame numbers are the integer halves
 * of the 16.16 positions taken with an unsigned shift, which gives retail's
 * lhu loads and the copies cse keeps for the later cases. */

void SceneE19_GrabSequence(SceneE19Selection *selection) {
    FieldActor *actor = selection->actor;
    SceneE19SelectionTail *tail = &selection->tail;
    FieldActorState *playerState;
    SceneE19ActorState *actorState;
    u16 currentFrame;
    u16 previousFrame;

    g_PlayerEntity->render_object.model_matrix = selection->matrix;
    currentFrame = (unsigned int)actor->anim.fixed >> 16;
    previousFrame = (unsigned int)actor->anim_prev >> 16;
    g_PlayerEntity->pos_x = selection->savedX;
    g_PlayerEntity->pos_z = selection->savedZ;
    if (g_PlayerEntity->state == 0 || g_PlayerEntity->state->amount <= 0) {
        SceneE19_ResetPlayerMotion(actor, (unsigned char *)tail);
        SceneE19_GrabRelease((PmSlotHeader *)selection);
        return;
    }

    switch (actor->mode) {
    case 16:
        if ((s16)currentFrame >= 11 && (s16)previousFrame < 11) {
            actor->anim.fixed = 0x280000;
        } else if ((s16)currentFrame >= actor->action - 1) {
            *tail->signal = 2;
        }
        return;
    case 13:
        actorState = (SceneE19ActorState *)actor->state;
        if (actorState != 0 && actorState->action_state != NULL && *actorState->action_state == 1) {
            *actorState->action_state = 2;
        }
        if ((s16)previousFrame < 18 && (s16)currentFrame >= 18) {
            Battle_ReturnToIdle();
            SceneE19_ResetPlayerMotion(actor, (unsigned char *)tail);
            return;
        }
        if ((s16)previousFrame < 3 && (s16)currentFrame >= 3) {
            playerState = g_PlayerEntity->state;
            if (playerState != 0 && playerState->amount > 0) {
                playerState->flags |= 0x4000;
                if (actor->state != 0) {
                    actor->state->core_flags |= 0x80000000;
                }
            }
            return;
        }
        if ((s16)currentFrame < actor->action - 1) {
            return;
        }
        actorState = (SceneE19ActorState *)actor->state;
        if (actorState != 0 && actorState->action_state != NULL) {
            *actorState->action_state = 4;
        }
        SceneE19_GrabRelease((PmSlotHeader *)selection);
        return;
    case 12:
        return;
    default:
        Battle_ReturnToIdle();
        SceneE19_ResetPlayerMotion(actor, (unsigned char *)tail);
        SceneE19_GrabRelease((PmSlotHeader *)selection);
        return;
    }
}


void SceneE19_ResetPlayerMotion(void *unused, unsigned char *state) {
    FieldActor *actor=g_PlayerEntity;
    (void)unused;
    actor->render_object.animation_source=NULL;
    actor->render_object.animation_state=0;
    actor->flags &= ~0x10000u;
    actor->render_object.flags_9C &= ~0x400u;
    Entity_FindFloor((struct BattleEntity *)actor);
    actor->motion_x=actor->motion_y=actor->motion_z=0;
    actor->accel_x=actor->accel_y=actor->accel_z=0;
    actor->base_x=actor->pos_x; actor->base_y=actor->pos_y; actor->base_z=actor->pos_z;
    g_GeomScroll.state.flags |= 0x80;
    ((SceneGrabControl *)state)->grabbed=0;
}

PE1_STATIC_ASSERT(PE1_OFFSETOF(SceneE19FrameGateLink, winLo) == 0x16,
                  scene_e19_frame_gate_window_offset);
PE1_STATIC_ASSERT(sizeof(SceneE19FrameGateLink) == 0x18,
                  scene_e19_frame_gate_link_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(SceneE19FrameGateEntity, link) == 8,
                  scene_e19_frame_gate_entity_link_offset);
PE1_STATIC_ASSERT(sizeof(SceneE19FrameGateEntity) == 8 + sizeof(void *),
                  scene_e19_frame_gate_entity_size);

void SceneE19_RunAfterFrame37(SceneE19FrameGateEntity *o)
{
    if (((SceneGrabTrigger *)o)->actor->anim.parts.integer >= 0x26) {
        SceneE19_GrabRelease((PmSlotHeader *)o);
    }
}

int SceneE19_GrabRelease(PmSlotHeader *raw) {
    SceneGrabTrigger *ent=(void *)raw;
    *ent->control.signal=0;
    ent->status=4;
    if(ent->control.grabbed) {
        if(g_PlayerEntity->state && g_PlayerEntity->state->amount>0) Battle_ReturnToIdle();
        SceneE19_ResetPlayerMotion(NULL,(unsigned char *)&ent->control);
    }
    return 0;
}

s32 SceneE19_ResetGrabTarget(char *raw) {
    SceneGrabTrigger *ent=(void *)raw;
    ent->status=4;
    if(ent->control.grabbed) {
        if(g_PlayerEntity->state && g_PlayerEntity->state->amount>0) Battle_ReturnToIdle();
        SceneE19_ResetPlayerMotion(NULL,(unsigned char *)&ent->control);
    }
    return 0;
}

void SceneE19_GrabNop6(void) {}

int SceneE19_BlastGlowCallback(int mode, void *memory, intptr_t argument) {
    SceneE19Effect *effect=memory;
    (void)argument;
    GteRotation position = g_SceneE19ZeroSpin;
    int angle, cosine, sine, brightness, palette, kind;
    u16 clut;

    switch (mode) {
    case 1: {
        if (effect->state != 0) return 0;
        effect->frame++;
        effect->y -= 128;
        if (effect->frame >= 8) return 1;
        break;
    }
    case 2: {
        if (effect->state != 0) return 0;
        position.z = 1024;
        angle = effect->frame << 7;
        cosine = rsin(angle);
        sine = rcos(angle);
        brightness = (g_SceneE19BlastGlow * sine) / 4096;
        kind = g_FieldFxSpriteParams.palette;
        palette = g_FieldFxPaletteTable[kind];
        if (kind == 4 && g_FieldFxAltPaletteFlag) palette += 4;
        clut = GetClut(144, palette);
        FieldFx_DrawShapeQuads((GteShortVector *)effect, &position, 4096, cosine, 0x88, clut, 1, brightness, 0);
        break;
    }
    }
    return 0;
}

int SceneE19_Blast(int mode, void *memory, intptr_t argument) {
    SceneE19Blast *blast=memory;
    (void)argument;
    GteShortVector center;
    GteShortVector ringPoint;
    GteRotation flareRotation = { 0x400, 0, 0, 1 };
    GteShortVector rotation;
    RenderColor glowColor = { 0x8C, 0x8C, 0x46, 0 };
    RenderColor flareColor = { 0x8C, 0x46, 0x8C, 0 };
    GteShortVector floorGlow0;
    GteShortVector floorRotation0;
    GteShortVector floorGlow1;
    GteShortVector floorRotation1;
    GteMatrix shellMatrix1;
    GteVector shellScale1;
    GteShortVector floorGlow2;
    GteShortVector floorRotation2;
    GteMatrix shellMatrix2;
    GteVector shellScale2;
    GteMatrix shellMatrix3a;
    GteVector shellScale3a;
    GteMatrix shellMatrix3b;
    GteVector shellScale3b;
    GteMatrix shellMatrix3c;
    GteVector shellScale3c;
    GteMatrix shellMatrix3d;
    GteVector shellScale3d;
    GteMatrix shellMatrix3e;
    GteVector shellScale3e;
    GteMatrix shellMatrix4a;
    GteVector shellScale4a;
    GteMatrix shellMatrix4b;
    GteVector shellScale4b;
    GteMatrix shellMatrix4c;
    GteVector shellScale4c;
    GteMatrix shellMatrix4d;
    GteVector shellScale4d;
    SceneE19BlastSpark *spark;
    FieldActorState *actor;
    u16 *pageSelector;
    RenderEffectParameters *params;
    u16 *palettes;
    GteShortVector *target;
    int spread;
    int phase;
    int modelPhase;
    int intensity;
    int radialScale;
    int verticalScale;
    int page;
    int palette;
    int i;
    int value;
    int height;
    s16 timer;

    switch (mode) {
    case 0:
        g_SceneE19BlastModel0 = Asset_FindTable08ByU32Key(g_LoadedSceneAssetBlock[0], 0xC54C0704);
        FieldFx_ModelScrollUv(g_SceneE19BlastModel0, 0, 0);
        g_SceneE19BlastModel1 = Asset_FindTable08ByU32Key(g_LoadedSceneAssetBlock[0], 0xC58C0704);
        FieldFx_ModelScrollUv(g_SceneE19BlastModel1, 0, 0);
        g_SceneE19BlastModel2 = Asset_FindTable08ByU32Key(g_LoadedSceneAssetBlock[0], 0xC5CC0704);
        FieldFx_ModelScrollUv(g_SceneE19BlastModel2, 0, 0);
        g_SceneE19BlastModel3 = Asset_FindTable08ByU32Key(g_LoadedSceneAssetBlock[0], 0xC60C0704);
        FieldFx_ModelScrollUv(g_SceneE19BlastModel3, 0, 0);
        g_SceneE19BlastModel4 = Asset_FindTable08ByU32Key(g_LoadedSceneAssetBlock[0], 0xC64C0704);
        FieldFx_ModelScrollUv(g_SceneE19BlastModel4, 0, 0);
        blast->state = 0;
        blast->timer = 0;
        FieldFx_GetActorPosition((char *)g_FieldFxOwner->actor, 1, &blast->position.x);
        blast->position.y = g_RoomFloorY->raw;
        FieldFx_GetActorPosition((char *)g_FieldFxOwner->actor, 0, &blast->target.x);
        FieldFx_GetTransformLookAngles((struct RoomFxTransformOwner *)g_FieldFxOwner->actor, 0, &blast->origin);
        return FieldFx_InitCallbackList((char *)g_FieldFxActiveTaskSlot->end, 0xC, 8, SceneE19_BlastGlowCallback);
    case 1:
        switch (blast->state) {
        case 0:
            timer = blast->timer + 1;
            blast->timer = timer;
            if (timer == 2 && g_LoadedSceneAssetBlock[0] != 0) {
                Akao_SendTableCommand(g_LoadedSceneAssetBlock[0], 0x5E3, FieldFx_GetOwnerProgress(), 0x80, 0x7F);
                if (g_LoadedSceneAssetBlock[0] != 0) {
                    Akao_SendTableCommand(g_LoadedSceneAssetBlock[0], 0x5E4, 0x80, 0x80, 0x7F);
                }
            }
            if (blast->timer < 0x10) {
                return 0;
            }
            blast->state = 1;
            blast->timer = 0;
            break;
        case 1:
            timer = blast->timer + 1;
            blast->timer = timer;
            if (timer < 0x10) {
                return 0;
            }
            blast->state = 2;
            blast->timer = 0;
            break;
        case 2:
            timer = blast->timer + 1;
            blast->timer = timer;
            if (timer < 0x10) {
                return 0;
            }
            blast->state = 3;
            blast->timer = 0;
            break;
        case 3:
            blast->timer++;
            if (FieldFx_IsPlayerWithinRadius(&blast->position, 0x960) != 0
                && (FieldFx_IsPlayerWithinRadius(&blast->position, 0x4B0) != 0
                    || FieldFx_IsPlayerWithinRadius(&blast->position, 0x6A4) == 0)
                && g_FieldFxTaskContext->flags != 0
                && (g_FieldFxOwner->actor->state->core_flags & 0x3F000000) == 0x01000000) {
                g_PlayerEntity->state->flags |= 0x4000;
                actor = g_FieldFxOwner->actor->state;
                actor->core_flags = (actor->core_flags & 0xC0FFFFFF) | 0x2D000000;
                actor = g_FieldFxOwner->actor->state;
                actor->core_flags |= 0x80000000;
            }
            if (blast->timer < 0x11) {
                Geo_ClipAlternatingArcPoint(2, 0x10, blast->timer);
            }
            spread = 0x200;
            spark = FieldFx_AcquireCallbackSlot((char *)g_FieldFxActiveTaskSlot->end);
            if (spark != 0) {
                spark->x = blast->position.x;
                spark->y = blast->position.y;
                spark->z = blast->position.z;
                spark->y -= rand() % 800;
                spark->x += rand() % spread - 0x100;
                value = rand() % spread;
                spark->state = 0;
                spark->timer = 0;
                spark->z += value - 0x100;
            }
            g_SceneE19BlastGlow = 0x80;
            if (blast->timer < 0x20) {
                return 0;
            }
            blast->state = 4;
            blast->timer = 0;
            break;
        case 4:
            blast->timer++;
            spread = 0x200;
            spark = FieldFx_AcquireCallbackSlot((char *)g_FieldFxActiveTaskSlot->end);
            if (spark != 0) {
                spark->x = blast->position.x;
                spark->y = blast->position.y;
                spark->z = blast->position.z;
                spark->y -= rand() % 800;
                spark->x += rand() % spread - 0x100;
                value = rand() % spread;
                spark->state = 0;
                spark->timer = 0;
                spark->z += value - 0x100;
            }
            g_SceneE19BlastGlow = rcos((blast->timer << 10) / 24) / 32;
            if (blast->timer < 0x18) {
                return 0;
            }
            blast->state = 4;
            blast->timer = 0;
            return 1;
        }
        break;
    case 2:
        center.x = blast->position.x;
        center.y = blast->position.y;
        center.z = blast->position.z;
        gte_ldrotmatrix(g_ViewMatrixSlot.value);
        gte_ldtransmatrix(g_ViewMatrixSlot.value);

        g_FieldFxSpriteParams.parameter0A = 0;
        g_FieldFxSpriteParams.depth = 4;
        switch (blast->state) {
        case 0:
            phase = blast->timer << 6;
            intensity = rsin(phase) / 32;
            g_FieldFxSpriteParams.parameter00 = 0x40;
            g_FieldFxSpriteParams.parameter02 = 4;
            g_FieldFxSpriteParams.extent_x = 0x40;
            g_FieldFxSpriteParams.extent_y = 0x40;
            g_FieldFxSpriteParams.tpage = g_FieldFxTpageTable[g_FieldFxTpageSlot11];
            g_FieldFxSpriteParams.palette = 3;
            g_FieldFxSpriteParams.parameter06 = 1;
            if (g_FieldFxCurrentAge & 1) {
                intensity = intensity * 2 / 3;
            }
            floorRotation0.pad = 1;
            floorRotation0.x = 0x400;
            floorRotation0.y = 0;
            floorRotation0.z = 0;
            floorGlow0.x = center.x;
            floorGlow0.z = center.z;
            floorGlow0.y = g_RoomFloorY->raw;
            FieldFx_DrawShapeQuads(&floorGlow0, (GteRotation *)&floorRotation0, 0x2000, 0x2000, 0,
                          GetClut(0, g_FieldFxPaletteEntry3 + 2), 1, intensity, 0);
            break;
        case 1:
            phase = blast->timer << 6;
            intensity = 0x80;
            g_FieldFxSpriteParams.parameter00 = 0x40;
            g_FieldFxSpriteParams.parameter02 = 4;
            g_FieldFxSpriteParams.extent_x = 0x40;
            g_FieldFxSpriteParams.extent_y = 0x40;
            g_FieldFxSpriteParams.tpage = g_FieldFxTpageTable[g_FieldFxTpageSlot11];
            g_FieldFxSpriteParams.palette = 3;
            g_FieldFxSpriteParams.parameter06 = 1;
            if (g_FieldFxCurrentAge & 1) {
                intensity = 0x55;
            }
            floorRotation1.pad = 1;
            floorRotation1.x = 0x400;
            floorRotation1.y = 0;
            floorRotation1.z = 0;
            floorGlow1.x = center.x;
            floorGlow1.z = center.z;
            floorGlow1.y = g_RoomFloorY->raw;
            FieldFx_DrawShapeQuads(&floorGlow1, (GteRotation *)&floorRotation1, 0x2000, 0x2000, 0,
                          GetClut(0, g_FieldFxPaletteEntry3 + 2), 1, intensity, 0);
            intensity = rsin(phase) / 32;
            if (g_FieldFxCurrentAge & 1) {
                intensity = intensity * 2 / 3;
            }
            radialScale = rsin(phase);
            if (g_FieldFxCurrentAge & 1) {
                radialScale = radialScale * 15 / 16;
            }
            rotation.x = -0x400;
            rotation.y = 0;
            rotation.z = g_FieldFxCurrentAge << 5;
            g_FieldFxSpriteParams.tpage = g_FieldFxTpageTable[g_FieldFxTpageSlots[3]];
            g_FieldFxSpriteParams.palette = 3;
            g_FieldFxSpriteParams.parameter06 = 0;
            page = (u16)(g_FieldFxTpageTable[g_FieldFxTpageSlots[3]] | GetTPage(0, 1, 0, 0));
            palette = g_FieldFxPaletteTable[g_FieldFxSpriteParams.palette];
            if (g_FieldFxSpriteParams.palette == 4 && g_FieldFxAltPaletteFlag != 0) {
                palette += 4;
            }
            FieldFx_ModelSetTexture(page, GetClut(0x20, palette));
            FieldFx_ModelSetSemiTrans(1);
            RotMatrixYXZ(&rotation, &shellMatrix1);
            shellScale1.x = radialScale / 2;
            shellScale1.y = radialScale / 2;
            shellMatrix1.t[0] = center.x;
            shellMatrix1.t[1] = center.y;
            shellMatrix1.t[2] = center.z;
            shellScale1.z = (blast->timer << 6) + 0x400;
            ScaleMatrix(&shellMatrix1, &shellScale1);
            FieldFx_ModelSaveColors(g_SceneE19BlastModel1);
            FieldFx_ModelScaleColors(g_SceneE19BlastModel1, 0x40);
            FieldFx_ModelDraw(g_SceneE19BlastModel1, &shellMatrix1);
            FieldFx_ModelRestoreColors(g_SceneE19BlastModel1);
            gte_ldrotmatrix(g_ViewMatrixSlot.value);
            gte_ldtransmatrix(g_ViewMatrixSlot.value);
            FieldFx_DrawStarFan(&center, 0x12C, 0x12C, 0xC, 0, 0x1000, 0x1000, &glowColor, 0, intensity, 1);
            intensity = rcos(phase) / 32;
            radialScale = rsin(phase);
            FieldFx_DrawRingBand(&center, 0x7D0, 0xB54, 0x18, &flareRotation, radialScale, radialScale, 0,
                          &glowColor, intensity, 1);
            break;
        case 2:
            phase = blast->timer << 6;
            intensity = 0x80;
            params = &g_FieldFxSpriteParams;
            params->parameter00 = 0x40;
            g_FieldFxSpriteParams.parameter02 = 4;
            g_FieldFxSpriteParams.extent_x = 0x40;
            g_FieldFxSpriteParams.extent_y = 0x40;
            (pageSelector = &g_FieldFxTpageSlots[11]);
            g_FieldFxSpriteParams.tpage = g_FieldFxTpageTable[pageSelector[0]];
            g_FieldFxSpriteParams.palette = 3;
            g_FieldFxSpriteParams.parameter06 = 1;
            if (g_FieldFxCurrentAge & 1) {
                intensity = 0x55;
            }
            floorRotation2.pad = 1;
            floorRotation2.x = 0x400;
            floorRotation2.y = 0;
            floorRotation2.z = 0;
            floorGlow2.x = center.x;
            floorGlow2.z = center.z;
            floorGlow2.y = g_RoomFloorY->raw;
            FieldFx_DrawShapeQuads(&floorGlow2, (GteRotation *)&floorRotation2, 0x2000, 0x2000, 0,
                          GetClut(0, g_FieldFxPaletteEntry3 + 2), 1, intensity, 0);
            radialScale = 0x1000;
            {
                s32 textureDepth;
                s32 blendMode;
                s32 textureX;
                u32 pageIndex;
                palettes = g_FieldFxPaletteTable;
                if (g_FieldFxCurrentAge & 1) {
                    radialScale = 0xF00;
                }
                textureDepth = 0;
                blendMode = 1;
                textureX = 0;
                rotation.x = -0x400;
                rotation.y = 0;
                pageIndex = pageSelector[-8];
                rotation.z = g_FieldFxCurrentAge << 5;
                params->tpage = g_FieldFxTpageTable[pageIndex];
                params->palette = 3;
                params->parameter06 = 0;
                page = (u16)(g_FieldFxTpageTable[pageSelector[-8]] | GetTPage(textureDepth, blendMode, textureX, 0));
            }
            {
                s32 paletteValue;
                paletteValue = palettes[params->palette];
                if (params->palette == 4 && g_FieldFxAltPaletteFlag != 0) {
                    paletteValue += 4;
                }
                FieldFx_ModelSetTexture(page, GetClut(0x20, paletteValue));
            }
            FieldFx_ModelSetSemiTrans(1);
            RotMatrixYXZ(&rotation, &shellMatrix2);
            shellScale2.x = radialScale / 2;
            shellScale2.y = radialScale / 2;
            shellMatrix2.t[0] = center.x;
            shellMatrix2.t[1] = center.y;
            shellMatrix2.t[2] = center.z;
            shellScale2.z = (blast->timer << 6) + 0x800;
            ScaleMatrix(&shellMatrix2, &shellScale2);
            FieldFx_ModelSaveColors(g_SceneE19BlastModel1);
            FieldFx_ModelScaleColors(g_SceneE19BlastModel1, 0x40);
            FieldFx_ModelDraw(g_SceneE19BlastModel1, &shellMatrix2);
            FieldFx_ModelRestoreColors(g_SceneE19BlastModel1);
            gte_ldrotmatrix(g_ViewMatrixSlot.value);
            gte_ldtransmatrix(g_ViewMatrixSlot.value);
            FieldFx_DrawStarFan(&center, 0x12C, 0x12C, 0xC, 0, 0x1000, 0x1000, &glowColor, 0, intensity, 1);
            intensity = 0x80;
            if (g_FieldFxCurrentAge & 1) {
                intensity = 0x64;
            }
            modelPhase = 2000;
            radialScale = rcos(phase) / 8;
            height = rsin(phase) * 2 / 3;
            rotation.x = 0x400;
            rotation.y = 0;
            rotation.z = 0;
            for (i = 0; i < 0x10; i++) {
                verticalScale = (i << 8) + g_FieldFxCurrentAge * 4;
                ringPoint.x = center.x;
                ringPoint.y = center.y;
                ringPoint.z = center.z;
                ringPoint.x += rcos(verticalScale) * modelPhase / 4096;
                ringPoint.z += rsin(verticalScale) * modelPhase / 4096;
                rotation.z = verticalScale + 0x400;
                FieldFx_DrawRotatedTriangle(&ringPoint, (GteRotation *)&rotation, height, radialScale,
                              &glowColor, 0, 0, (s16)intensity, 1);
            }
            intensity = rcos(phase) / 32;
            radialScale = rcos(phase);
            target = &blast->target;
            rotation.x = blast->origin.x;
            rotation.y = blast->origin.y;
            rotation.z = blast->origin.z;
            rotation.x -= 0x200;
            rotation.y += 0x400;
            FieldFx_DrawRingBand(target, 0x7D0, 0xA8C, 0x20, (GteRotation *)&rotation, radialScale, radialScale,
                          &flareColor, 0, intensity, 1);
            rotation.x += 0x400;
            FieldFx_DrawRingBand(target, 0x7D0, 0xA8C, 0x20, (GteRotation *)&rotation, radialScale, radialScale,
                          &flareColor, 0, intensity, 1);
            break;
        case 3:
            timer = blast->timer;
            phase = timer << 5;
            if (timer == 0) {
                FieldFx_DrawScreenTint((unsigned char *)&glowColor, 0x46, 2, 8);
            } else if (timer < 9) {
                FieldFx_DrawScreenTint((unsigned char *)&glowColor, 0x80 - timer * 0x10, 1, 8);
            }
            radialScale = rcos(phase) / 2 + 0x400;
            verticalScale = rsin(phase) / 6 + 0x555;
            intensity = 0x80;
            if (g_FieldFxCurrentAge & 1) {
                intensity = 0x78;
            }
            rotation.x = 0;
            rotation.y = g_FieldFxCurrentAge << 5;
            rotation.z = 0;
            g_FieldFxSpriteParams.tpage = g_FieldFxTpageTable[g_FieldFxTpageSlots[3]];
            g_FieldFxSpriteParams.palette = 3;
            g_FieldFxSpriteParams.parameter06 = 0;
            page = (u16)(g_FieldFxTpageTable[g_FieldFxTpageSlots[3]] | GetTPage(0, 1, 0, 0));
            palette = g_FieldFxPaletteTable[g_FieldFxSpriteParams.palette];
            if (g_FieldFxSpriteParams.palette == 4 && g_FieldFxAltPaletteFlag != 0) {
                palette += 4;
            }
            FieldFx_ModelSetTexture(page, GetClut(0x20, palette));
            FieldFx_ModelSetSemiTrans(1);
            RotMatrixYXZ(&rotation, &shellMatrix3a);
            shellScale3a.x = radialScale;
            shellScale3a.y = verticalScale;
            shellScale3a.z = radialScale;
            shellMatrix3a.t[0] = center.x;
            shellMatrix3a.t[1] = center.y;
            shellMatrix3a.t[2] = center.z;
            ScaleMatrix(&shellMatrix3a, &shellScale3a);
            FieldFx_ModelSaveColors(g_SceneE19BlastModel0);
            FieldFx_ModelScaleColors(g_SceneE19BlastModel0, (u16)intensity);
            FieldFx_ModelDraw(g_SceneE19BlastModel0, &shellMatrix3a);
            FieldFx_ModelRestoreColors(g_SceneE19BlastModel0);
            radialScale = rsin(phase) / 3 + 0x400;
            verticalScale = rcos(phase) / 2;
            intensity = rcos(phase) / 32;
            if (g_FieldFxCurrentAge & 1) {
                intensity = intensity * 15 / 16;
            }
            rotation.x = 0;
            rotation.y = -g_FieldFxCurrentAge * 0x30;
            rotation.z = 0;
            g_FieldFxSpriteParams.tpage = g_FieldFxTpageTable[g_FieldFxTpageSlots[3]];
            g_FieldFxSpriteParams.palette = 3;
            g_FieldFxSpriteParams.parameter06 = 0;
            page = (u16)(g_FieldFxTpageTable[g_FieldFxTpageSlots[3]] | GetTPage(0, 1, 0, 0));
            palette = g_FieldFxPaletteTable[g_FieldFxSpriteParams.palette];
            if (g_FieldFxSpriteParams.palette == 4 && g_FieldFxAltPaletteFlag != 0) {
                palette += 4;
            }
            FieldFx_ModelSetTexture(page, GetClut(0x20, palette));
            FieldFx_ModelSetSemiTrans(1);
            RotMatrixYXZ(&rotation, &shellMatrix3b);
            shellScale3b.x = radialScale;
            shellScale3b.y = verticalScale;
            shellScale3b.z = radialScale;
            shellMatrix3b.t[0] = center.x;
            shellMatrix3b.t[1] = center.y;
            shellMatrix3b.t[2] = center.z;
            ScaleMatrix(&shellMatrix3b, &shellScale3b);
            FieldFx_ModelSaveColors(g_SceneE19BlastModel2);
            FieldFx_ModelScaleColors(g_SceneE19BlastModel2, (u16)(intensity / 2));
            FieldFx_ModelDraw(g_SceneE19BlastModel2, &shellMatrix3b);
            FieldFx_ModelRestoreColors(g_SceneE19BlastModel2);
            modelPhase = (blast->timer << 10) / 56;
            radialScale = rsin(modelPhase) / 4 + 0xC00;
            verticalScale = rcos(modelPhase) / 4 + 0x400;
            intensity = rcos(modelPhase) / 32;
            if (g_FieldFxCurrentAge & 1) {
                intensity = intensity * 15 / 16;
            }
            rotation.x = 0;
            rotation.y = -g_FieldFxCurrentAge << 5;
            rotation.z = 0;
            g_FieldFxSpriteParams.tpage = g_FieldFxTpageTable[g_FieldFxTpageSlots[3]];
            g_FieldFxSpriteParams.palette = 3;
            g_FieldFxSpriteParams.parameter06 = 0;
            page = (u16)(g_FieldFxTpageTable[g_FieldFxTpageSlots[3]] | GetTPage(0, 1, 0, 0));
            palette = g_FieldFxPaletteTable[g_FieldFxSpriteParams.palette];
            if (g_FieldFxSpriteParams.palette == 4 && g_FieldFxAltPaletteFlag != 0) {
                palette += 4;
            }
            FieldFx_ModelSetTexture(page, GetClut(0x20, palette));
            FieldFx_ModelSetSemiTrans(1);
            height = center.y;
            center.y -= blast->timer * 0x18;
            RotMatrixYXZ(&rotation, &shellMatrix3c);
            shellScale3c.x = radialScale;
            shellScale3c.y = verticalScale;
            shellScale3c.z = radialScale;
            shellMatrix3c.t[0] = center.x;
            shellMatrix3c.t[1] = center.y;
            shellMatrix3c.t[2] = center.z;
            ScaleMatrix(&shellMatrix3c, &shellScale3c);
            FieldFx_ModelSaveColors(g_SceneE19BlastModel3);
            FieldFx_ModelScaleColors(g_SceneE19BlastModel3, (u16)(intensity / 2));
            FieldFx_ModelDraw(g_SceneE19BlastModel3, &shellMatrix3c);
            FieldFx_ModelRestoreColors(g_SceneE19BlastModel3);
            center.y = height;
            gte_ldrotmatrix(g_ViewMatrixSlot.value);
            gte_ldtransmatrix(g_ViewMatrixSlot.value);
            FieldFx_DrawStarFan(&center, 0x9C4, 0x9C4, 0xC, 0, 0x1000, 0x1000, &glowColor, 0, intensity, 1);
            radialScale = rsin(phase) / 4 + 0xC00;
            intensity = rcos(phase) / 32;
            FieldFx_DrawRingBand(&center, 0x76C, 0xA28, 0x18, &flareRotation, radialScale, radialScale,
                          &flareColor, 0, intensity, 1);
            center.y -= 0x400;
            radialScale = rsin(phase) * 3 / 2 + 0x1000;
            FieldFx_DrawRingBand(&center, 0x3E8, 0x5DC, 0x18, &flareRotation, radialScale, radialScale,
                          &glowColor, 0, intensity, 1);
            center.y += 0x400;
            radialScale = rsin(modelPhase) / 4 + 0x1200;
            verticalScale = rcos(modelPhase);
            intensity = rcos(modelPhase) / 32;
            if (g_FieldFxCurrentAge & 1) {
                intensity = intensity * 15 / 16;
            }
            rotation.x = 0;
            rotation.y = (g_FieldFxCurrentAge << 5) + 0x400;
            rotation.z = 0;
            g_FieldFxSpriteParams.tpage = g_FieldFxTpageTable[g_FieldFxTpageSlots[3]];
            g_FieldFxSpriteParams.palette = 3;
            g_FieldFxSpriteParams.parameter06 = 0;
            page = (u16)(g_FieldFxTpageTable[g_FieldFxTpageSlots[3]] | GetTPage(0, 1, 0, 0));
            palette = g_FieldFxPaletteTable[g_FieldFxSpriteParams.palette];
            if (g_FieldFxSpriteParams.palette == 4 && g_FieldFxAltPaletteFlag != 0) {
                palette += 4;
            }
            FieldFx_ModelSetTexture(page, GetClut(0x60, palette));
            FieldFx_ModelSetSemiTrans(1);
            RotMatrixYXZ(&rotation, &shellMatrix3d);
            shellScale3d.x = radialScale;
            shellScale3d.y = verticalScale;
            shellScale3d.z = radialScale;
            shellMatrix3d.t[0] = center.x;
            shellMatrix3d.t[1] = center.y;
            shellMatrix3d.t[2] = center.z;
            ScaleMatrix(&shellMatrix3d, &shellScale3d);
            FieldFx_ModelSaveColors(g_SceneE19BlastModel4);
            FieldFx_ModelScaleColors(g_SceneE19BlastModel4, (u16)intensity);
            FieldFx_ModelDraw(g_SceneE19BlastModel4, &shellMatrix3d);
            FieldFx_ModelRestoreColors(g_SceneE19BlastModel4);
            radialScale = rsin(modelPhase) / 6 + 0x1200;
            verticalScale = rcos(modelPhase) * 2;
            intensity = rcos(modelPhase) / 32;
            if (!(g_FieldFxCurrentAge & 1)) {
                intensity = intensity * 15 / 16;
            }
            rotation.x = 0;
            rotation.y = g_FieldFxCurrentAge << 5;
            rotation.z = 0;
            g_FieldFxSpriteParams.tpage = g_FieldFxTpageTable[g_FieldFxTpageSlots[3]];
            g_FieldFxSpriteParams.palette = 3;
            g_FieldFxSpriteParams.parameter06 = 0;
            page = (u16)(g_FieldFxTpageTable[g_FieldFxTpageSlots[3]] | GetTPage(0, 1, 0, 0));
            palette = g_FieldFxPaletteTable[g_FieldFxSpriteParams.palette];
            if (g_FieldFxSpriteParams.palette == 4 && g_FieldFxAltPaletteFlag != 0) {
                palette += 4;
            }
            FieldFx_ModelSetTexture(page, GetClut(0x60, palette));
            FieldFx_ModelSetSemiTrans(1);
            RotMatrixYXZ(&rotation, &shellMatrix3e);
            shellScale3e.x = radialScale;
            shellScale3e.y = verticalScale;
            shellScale3e.z = radialScale;
            shellMatrix3e.t[0] = center.x;
            shellMatrix3e.t[1] = center.y;
            shellMatrix3e.t[2] = center.z;
            ScaleMatrix(&shellMatrix3e, &shellScale3e);
            FieldFx_ModelSaveColors(g_SceneE19BlastModel4);
            FieldFx_ModelScaleColors(g_SceneE19BlastModel4, (u16)intensity);
            FieldFx_ModelDraw(g_SceneE19BlastModel4, &shellMatrix3e);
            FieldFx_ModelRestoreColors(g_SceneE19BlastModel4);
            break;
        case 4:
            phase = (blast->timer << 10) / 24;
            radialScale = (0x400 - phase) / 2 + 0x200;
            verticalScale = rsin(phase) / 6 + 0x6AA;
            intensity = rcos(phase) / 32;
            if (g_FieldFxCurrentAge & 1) {
                intensity = intensity * 15 / 16;
            }
            rotation.x = 0;
            rotation.y = g_FieldFxCurrentAge << 5;
            rotation.z = 0;
            g_FieldFxSpriteParams.tpage = g_FieldFxTpageTable[g_FieldFxTpageSlots[3]];
            g_FieldFxSpriteParams.palette = 3;
            g_FieldFxSpriteParams.parameter06 = 0;
            page = (u16)(g_FieldFxTpageTable[g_FieldFxTpageSlots[3]] | GetTPage(0, 1, 0, 0));
            palette = g_FieldFxPaletteTable[g_FieldFxSpriteParams.palette];
            if (g_FieldFxSpriteParams.palette == 4 && g_FieldFxAltPaletteFlag != 0) {
                palette += 4;
            }
            FieldFx_ModelSetTexture(page, GetClut(0x20, palette));
            FieldFx_ModelSetSemiTrans(1);
            RotMatrixYXZ(&rotation, &shellMatrix4a);
            shellScale4a.x = radialScale;
            shellScale4a.y = verticalScale;
            shellScale4a.z = radialScale;
            shellMatrix4a.t[0] = center.x;
            shellMatrix4a.t[1] = center.y;
            shellMatrix4a.t[2] = center.z;
            ScaleMatrix(&shellMatrix4a, &shellScale4a);
            FieldFx_ModelSaveColors(g_SceneE19BlastModel0);
            FieldFx_ModelScaleColors(g_SceneE19BlastModel0, (u16)intensity);
            FieldFx_ModelDraw(g_SceneE19BlastModel0, &shellMatrix4a);
            FieldFx_ModelRestoreColors(g_SceneE19BlastModel0);
            modelPhase = ((blast->timer + 0x20) << 10) / 56;
            radialScale = rsin(modelPhase) / 4 + 0xC00;
            verticalScale = rcos(modelPhase) / 4 + 0x400;
            intensity = rcos(modelPhase) / 32;
            if (g_FieldFxCurrentAge & 1) {
                intensity = intensity * 15 / 16;
            }
            rotation.x = 0;
            rotation.y = -g_FieldFxCurrentAge << 5;
            rotation.z = 0;
            g_FieldFxSpriteParams.tpage = g_FieldFxTpageTable[g_FieldFxTpageSlots[3]];
            g_FieldFxSpriteParams.palette = 3;
            g_FieldFxSpriteParams.parameter06 = 0;
            page = (u16)(g_FieldFxTpageTable[g_FieldFxTpageSlots[3]] | GetTPage(0, 1, 0, 0));
            palette = g_FieldFxPaletteTable[g_FieldFxSpriteParams.palette];
            if (g_FieldFxSpriteParams.palette == 4 && g_FieldFxAltPaletteFlag != 0) {
                palette += 4;
            }
            FieldFx_ModelSetTexture(page, GetClut(0x20, palette));
            FieldFx_ModelSetSemiTrans(1);
            height = center.y;
            center.y -= (blast->timer + 0x20) * 0x18;
            RotMatrixYXZ(&rotation, &shellMatrix4b);
            shellScale4b.x = radialScale;
            shellScale4b.y = verticalScale;
            shellScale4b.z = radialScale;
            shellMatrix4b.t[0] = center.x;
            shellMatrix4b.t[1] = center.y;
            shellMatrix4b.t[2] = center.z;
            ScaleMatrix(&shellMatrix4b, &shellScale4b);
            FieldFx_ModelSaveColors(g_SceneE19BlastModel3);
            FieldFx_ModelScaleColors(g_SceneE19BlastModel3, (u16)(intensity / 2));
            FieldFx_ModelDraw(g_SceneE19BlastModel3, &shellMatrix4b);
            FieldFx_ModelRestoreColors(g_SceneE19BlastModel3);
            center.y = height;
            radialScale = rsin(modelPhase) / 4 + 0x1200;
            verticalScale = rcos(modelPhase);
            intensity = rcos(modelPhase) / 32;
            if (g_FieldFxCurrentAge & 1) {
                intensity = intensity * 15 / 16;
            }
            rotation.x = 0;
            rotation.y = (g_FieldFxCurrentAge << 5) + 0x400;
            rotation.z = 0;
            g_FieldFxSpriteParams.tpage = g_FieldFxTpageTable[g_FieldFxTpageSlots[3]];
            g_FieldFxSpriteParams.palette = 3;
            g_FieldFxSpriteParams.parameter06 = 0;
            page = (u16)(g_FieldFxTpageTable[g_FieldFxTpageSlots[3]] | GetTPage(0, 1, 0, 0));
            palette = g_FieldFxPaletteTable[g_FieldFxSpriteParams.palette];
            if (g_FieldFxSpriteParams.palette == 4 && g_FieldFxAltPaletteFlag != 0) {
                palette += 4;
            }
            FieldFx_ModelSetTexture(page, GetClut(0x60, palette));
            FieldFx_ModelSetSemiTrans(1);
            RotMatrixYXZ(&rotation, &shellMatrix4c);
            shellScale4c.x = radialScale;
            shellScale4c.y = verticalScale;
            shellScale4c.z = radialScale;
            shellMatrix4c.t[0] = center.x;
            shellMatrix4c.t[1] = center.y;
            shellMatrix4c.t[2] = center.z;
            ScaleMatrix(&shellMatrix4c, &shellScale4c);
            FieldFx_ModelSaveColors(g_SceneE19BlastModel4);
            FieldFx_ModelScaleColors(g_SceneE19BlastModel4, (u16)intensity);
            FieldFx_ModelDraw(g_SceneE19BlastModel4, &shellMatrix4c);
            FieldFx_ModelRestoreColors(g_SceneE19BlastModel4);
            radialScale = rsin(modelPhase) / 6 + 0x1200;
            verticalScale = rcos(modelPhase) * 2;
            intensity = rcos(modelPhase) / 32;
            if (!(g_FieldFxCurrentAge & 1)) {
                intensity = intensity * 15 / 16;
            }
            rotation.x = 0;
            rotation.y = g_FieldFxCurrentAge << 5;
            rotation.z = 0;
            g_FieldFxSpriteParams.tpage = g_FieldFxTpageTable[g_FieldFxTpageSlots[3]];
            g_FieldFxSpriteParams.palette = 3;
            g_FieldFxSpriteParams.parameter06 = 0;
            page = (u16)(g_FieldFxTpageTable[g_FieldFxTpageSlots[3]] | GetTPage(0, 1, 0, 0));
            palette = g_FieldFxPaletteTable[g_FieldFxSpriteParams.palette];
            if (g_FieldFxSpriteParams.palette == 4 && g_FieldFxAltPaletteFlag != 0) {
                palette += 4;
            }
            FieldFx_ModelSetTexture(page, GetClut(0x60, palette));
            FieldFx_ModelSetSemiTrans(1);
            RotMatrixYXZ(&rotation, &shellMatrix4d);
            shellScale4d.x = radialScale;
            shellScale4d.y = verticalScale;
            shellScale4d.z = radialScale;
            shellMatrix4d.t[0] = center.x;
            shellMatrix4d.t[1] = center.y;
            shellMatrix4d.t[2] = center.z;
            ScaleMatrix(&shellMatrix4d, &shellScale4d);
            FieldFx_ModelSaveColors(g_SceneE19BlastModel4);
            FieldFx_ModelScaleColors(g_SceneE19BlastModel4, (u16)intensity);
            FieldFx_ModelDraw(g_SceneE19BlastModel4, &shellMatrix4d);
            FieldFx_ModelRestoreColors(g_SceneE19BlastModel4);
            break;
        }
        g_FieldFxSpriteParams.parameter00 = 0x10;
        g_FieldFxSpriteParams.parameter02 = 1;
        g_FieldFxSpriteParams.extent_x = 0x10;
        g_FieldFxSpriteParams.extent_y = 0x10;
        g_FieldFxSpriteParams.extent_x = 0x80;
        g_FieldFxSpriteParams.extent_y = 0x10;
        g_FieldFxSpriteParams.tpage = g_FieldFxTpageTable[g_FieldFxTpageSlots[3]];
        g_FieldFxSpriteParams.palette = 3;
        g_FieldFxSpriteParams.parameter06 = 0;
        g_FieldFxSpriteParams.parameter0A = 0;
        break;
    }
    return 0;
}

PE1_STATIC_ASSERT(sizeof(SceneE19HomingParticle) == 0x20,
                  scene_e19_homing_particle_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(SceneE19HomingParticle, state) == 0x1C,
                  scene_e19_homing_particle_state);
int FieldFx_GetDistanceNonZero(s16 *from, s16 *to);

/* Not declared in production headers; parameter order inferred from both
 * call sites and the ABI. The eighth argument is zero at both sites. */

int SceneE19_HomingParticleCallback(int mode, void *memory, intptr_t argument) {
    SceneE19HomingParticle *particle=memory;
    (void)argument;
    GteRotation templatePoint = g_SceneE19RollTilt;
    GteShortVector temporaryPosition;
    GteShortVector rotation;
    RenderColor color = g_SceneE19HomingParticleColor;
    GteShortVector floor_position;
    GteRotation floor_rotation;
    int divider;
    int bias, random;
    int remMagic;
    int test;
    int sum;
    int intensity, scale, palette, pulseSum;
    u16 clut;
    u16 floorY;
    u16 floorZ;
    int floorIndex;
    int floorTimer;
    int signedValue;

    switch (mode) {
    case 1:
        switch (particle->state) {
    case 0:
        particle->timer = 0;
        particle->state = 1;
        particle->angles.pad = 0;
        FieldFx_TransformMatrixPoint(
            (struct RoomFxTransformOwner *)g_FieldFxOwner->actor,
            0, (GteShortVector *)&templatePoint, &particle->position);
        FieldFx_GetTransformLookAngles((struct RoomFxTransformOwner *)g_FieldFxOwner->actor,
                     0, &rotation);
        rotation.x = 0;
        FieldFx_GetActorPosition((char *)g_PlayerEntity, 1, &particle->target.x);
        FieldFx_GetActorPosition((char *)g_PlayerEntity, 0, &temporaryPosition.x);

        test = FieldFx_GetDistanceNonZero(&particle->position.x, &temporaryPosition.x) < 1900;
        test = -test;
        bias = test & 500;
        random = rand();
        remMagic = 0x66660000;
        random &= 1023;
        sum = bias + 390;
        rotation.x += random - sum;
        rotation.y += -512 + (s16)(particle->position.pad % 5) * 256;
        particle->angles.x = rotation.x;
        particle->angles.y = rotation.y;
        particle->angles.z = rotation.z;
        particle->speed = 24;
        particle->turnPhase = 0;
        break;

    case 1:
        particle->timer++;
        FieldFx_CalculateLookAngles(&particle->position, &particle->target,
                                    &rotation);
        FieldFx_EaseAngles((u16 *)&rotation, (u16 *)&particle->angles,
                     (u16)(rsin(particle->turnPhase) / 5));

        if (particle->speed < 62)
            particle->speed += 4;
        if (particle->speed >= 40) {
            if (particle->turnPhase < 2048)
                particle->turnPhase += 102;
            if (particle->turnPhase > 2048)
                particle->turnPhase = 2048;
        }

        FieldFx_AnglesToVector(&particle->angles,
                     (s16)(particle->speed * particle->speed / 16),
                     &temporaryPosition);
        particle->position.x += temporaryPosition.x;
        particle->position.y += temporaryPosition.y;
        particle->position.z += temporaryPosition.z;

        if (FieldFx_IsPlayerWithinRadius(&particle->position.x, 190) &&
            particle->position.y > g_RoomFloorY->y - 514) {
            if (g_FieldFxTaskContext->flags &&
                (g_FieldFxOwner->actor->state->core_flags & 0x3F000000)
                    == 0x01000000) {
                ((FieldActor *)g_PlayerEntity)->state->flags |= 0x4000;
                g_FieldFxOwner->actor->state->core_flags =
                    (g_FieldFxOwner->actor->state->core_flags & 0xC0FFFFFF)
                    | 0x29000000;
                g_FieldFxOwner->actor->state->core_flags |= 0x80000000;
            }
            particle->state = 2;
            particle->timer = 0;
        }

        if (particle->position.y >= g_RoomFloorY->y) {
            particle->position.y = g_RoomFloorY->y;
            particle->state = 2;
            particle->timer = 0;
        }
        return 0;

    case 2:
        particle->timer++;
        if (FieldFx_IsPlayerWithinRadius(&particle->position.x, 200) &&
            g_FieldFxTaskContext->flags &&
            (g_FieldFxOwner->actor->state->core_flags & 0x3F000000)
                == 0x01000000) {
            ((FieldActor *)g_PlayerEntity)->state->flags |= 0x4000;
            g_FieldFxOwner->actor->state->core_flags =
                (g_FieldFxOwner->actor->state->core_flags & 0xC0FFFFFF)
                | 0x29000000;
            g_FieldFxOwner->actor->state->core_flags |= 0x80000000;
        }
        test = particle->timer < 24;
        if (test) return 0;
        return 1;

    case 3:
        test = (unsigned short)particle->timer + 1;
        particle->timer = test;

        test = (s16)test < 12;
test_timer:

        if (test) return 0;
        return 1;

        }
        return 0;
    case 2:
        switch (particle->state) {
    case 1:
        intensity = 128;
        if (particle->timer & 1)
            intensity = 92;
        rotation.x = 0;
        rotation.y = 0;
        signedValue = particle->timer;
        rotation.z = signedValue << 7;
        sum = 64;
        g_FieldFxSpriteParams.parameter00 = sum;
        g_FieldFxSpriteParams.extent_x = sum;
        g_FieldFxSpriteParams.extent_y = sum;
        rotation.pad = 0;
        g_FieldFxSpriteParams.parameter02 = 4;
        palette = g_FieldFxPaletteTable[g_FieldFxSpriteParams.palette];
        if (g_FieldFxSpriteParams.palette == 4 && g_FieldFxAltPaletteFlag)
            palette += 4;
        clut = GetClut(16, palette);
        FieldFx_DrawShapeQuads(&particle->position, (GteRotation *)&rotation, 6144, 6144,
                     68, clut, 1, intensity / 2, 0);
        g_FieldFxSpriteParams.parameter02 = 2;
        g_FieldFxSpriteParams.parameter00 = 32;
        g_FieldFxSpriteParams.extent_x = 32;
        g_FieldFxSpriteParams.extent_y = 32;
        test = rotation.z;
        rotation.z = -test;
        palette = g_FieldFxPaletteTable[g_FieldFxSpriteParams.palette];
        if (g_FieldFxSpriteParams.palette == 4 && g_FieldFxAltPaletteFlag)
            palette += 4;
        clut = GetClut(16, palette);
        FieldFx_DrawShapeQuads(&particle->position, (GteRotation *)&rotation, 8192, 8192,
                     152, clut, 1, intensity, 0);
        gte_ldrotmatrix(g_ViewMatrixSlot.value);
        gte_ldtransmatrix(g_ViewMatrixSlot.value);
        FieldFx_DrawStarFan(&particle->position, 600, 600, 5,
                     0, 4096, 4096, &color, 0, 70, 1);
        if (particle->timer < 24 && (particle->position.pad & 1)) {
            intensity = rcos((particle->timer << 10) / 24) / 64;
floorIndex = g_FieldFxSpriteParams.palette;
temporaryPosition.x = particle->position.x;
temporaryPosition.y = particle->position.y;
pulseSum = 64;
floorZ = particle->position.z;

temporaryPosition.z = floorZ;
g_FieldFxSpriteParams.parameter02 = 4;
g_FieldFxSpriteParams.parameter00 = pulseSum;
g_FieldFxSpriteParams.extent_x = pulseSum;
g_FieldFxSpriteParams.extent_y = pulseSum;
palette = g_FieldFxPaletteTable[floorIndex];
if (floorIndex == 4 && g_FieldFxAltPaletteFlag) palette += 4;
            clut = GetClut(160, palette);
            FieldFx_DrawPulledSprite(&temporaryPosition, 6144, 6144, 12,
                         clut, 1, intensity / 2, 0, 3072);
            g_FieldFxSpriteParams.parameter00 = 32;
            g_FieldFxSpriteParams.parameter02 = 2;
            g_FieldFxSpriteParams.extent_x = 32;
            g_FieldFxSpriteParams.extent_y = 32;
            palette = g_FieldFxPaletteTable[g_FieldFxSpriteParams.palette];
            if (g_FieldFxSpriteParams.palette == 4 && g_FieldFxAltPaletteFlag)
                palette += 4;
            clut = GetClut(176, palette);
            FieldFx_DrawPulledSprite(&temporaryPosition, 6144, 6144, 154,
                         clut, 1, intensity / 2, 0, 1536);
        }
        gte_ldrotmatrix(g_ViewMatrixSlot.value);
        gte_ldtransmatrix(g_ViewMatrixSlot.value);
        if (particle->timer < 13 && !(particle->position.pad & 1)) {
            FieldFx_DrawRotatedTriangle(&particle->position, (GteRotation *)&particle->angles,
                         1900, (12 - particle->timer) << 8,
                         &color, 0, 0, 62, 1);
        }
        break;
    case 2:
        divider = 0x2aaa0000;
        g_FieldFxSpriteParams.parameter00 = 64;
        g_FieldFxSpriteParams.parameter02 = 4;
        g_FieldFxSpriteParams.extent_x = 64;
        g_FieldFxSpriteParams.extent_y = 64;
        intensity = rcos((particle->timer << 10) / 24) / 32;
        scale = rcos((particle->timer << 10) / 24) * 2;
        if (particle->timer & 1)
            intensity = intensity * 2 / 3;
        palette = g_FieldFxPaletteTable[g_FieldFxSpriteParams.palette];
        if (g_FieldFxSpriteParams.palette == 4 && g_FieldFxAltPaletteFlag)
            palette += 4;
        clut = GetClut(16, palette);
        FieldFx_DrawShapeQuads(&particle->position, 0, scale, scale,
                     8, clut, 1, intensity, 0);
        floorIndex = g_FieldFxSpriteParams.palette;
        floor_rotation.x = 1024;
        floor_rotation.y = 0;
        floorTimer = particle->timer;
        floor_rotation.z = floorTimer << 5;
        floor_rotation.flags = 1;
        floor_position.x = particle->position.x;
        floorY = g_RoomFloorY->y;
        sum = (u16)particle->position.z;

        floor_position.y = floorY;
        floor_position.z = sum;
        palette = g_FieldFxPaletteTable[floorIndex];
        if (floorIndex == 4 && g_FieldFxAltPaletteFlag)
            palette += 4;
        clut = GetClut(16, palette);
        FieldFx_DrawShapeQuads(&floor_position, &floor_rotation,
                     8192 - scale, 8192 - scale,
                     64, clut, 1, intensity / 2, 0);
        g_FieldFxSpriteParams.parameter00 = 32;
        g_FieldFxSpriteParams.parameter02 = 2;
        g_FieldFxSpriteParams.extent_x = 32;
        g_FieldFxSpriteParams.extent_y = 32;
        break;

        }
        return 0;
    }
    return 0;
timer_return_zero:
    return 0;
timer_return_one:
    return 1;
}

/* Mode 0 anchors the burst on the room actor and wakes the actor's
 * object; mode 1 releases a homing particle on odd frames (twenty in all)
 * and reports 2 after eight frames; mode 2 draws the flash, the screen
 * tint and the glows that shrink over 32 frames. */
int SceneE19_HomingBurst(int mode, void *memory, intptr_t argument) {
    SceneHomingBurst *burst=memory;
    (void)argument;
    GteShortVector position;
    GteRotation rotation = g_SceneE19RollTilt;
    GteRotation tilt = g_SceneE19ZeroSpin;
    RenderColor flashColor = g_SceneE19HomingBurstFlashColor;
    RenderColor ringColor = g_SceneE19HomingBurstRingColor;
    SceneHomingBurstChild *child;
    int fade;
    int scale;
    int time;

    switch (mode) {
    case 0:
        FieldFx_TransformMatrixPoint((struct RoomFxTransformOwner *)g_FieldFxOwner->actor, 0, (GteShortVector *)&rotation, &burst->position);
        burst->timer = 0;
        burst->released = 0;
        if (g_FieldFxTaskContext->flags) {
            FieldActor *pool = g_FieldFxOwner->actor;
            if (pool) {
                FieldActorState *object = pool->state;
                if (object) {
                    if (*object->action_state == 1) *object->action_state = 2;
                }
            }
        }
        return FieldFx_InitCallbackList((char *)g_FieldFxActiveTaskSlot->end, 0x20, 0x20, SceneE19_HomingParticleCallback);
    case 1:
        burst->timer++;
        if (burst->released < 0x14 && (burst->timer & 1)) {
            child = FieldFx_AcquireCallbackSlot((char *)g_FieldFxActiveTaskSlot->end);
            if (child) {
                child->index = burst->released;
                child->state = 0;
                child->timer = 0;
            }
            burst->released++;
        }
        if (burst->timer < 8) break;
        return 2;
    case 2:
        position.x = burst->position.x;
        position.y = burst->position.y;
        position.z = burst->position.z;
        time = burst->timer;
        if (time < 0x19) {
            fade = 0x80 - (time << 7) / 24;
            scale = rsin((time << 10) / 24);
            {
                RenderMatrixSlot *matrixSlot = &g_ViewMatrixSlot;
                gte_ldrotmatrix(matrixSlot->value);
                gte_ldtransmatrix(matrixSlot->value);
            }
            FieldFx_DrawRingBand(&position, 0x6A4, 0x9C4, 0x16, 0, scale, scale, 0, &flashColor,
                          fade, 1);
            time = burst->timer;
        }
        if (time < 0xD) {
            FieldFx_DrawScreenTint((unsigned char *)&flashColor, 0x80 - (time << 7) / 12, 1, 8);
        }
        if (burst->timer < 0x21) {
            {
                int tpage;
                g_FieldFxSpriteParams.parameter00 = 0x20;
                g_FieldFxSpriteParams.parameter02 = 2;
                g_FieldFxSpriteParams.extent_x = 0x20;
                g_FieldFxSpriteParams.extent_y = 0x20;
                tpage = g_FieldFxTpageTable[TEXTURE_SLOT_RECORD(g_FieldFxTpageSlot3)];
                g_FieldFxSpriteParams.palette = 3;
                g_FieldFxSpriteParams.depth = 0x32;
                g_FieldFxSpriteParams.parameter06 = 0;
                g_FieldFxSpriteParams.parameter0A = 0;
                g_FieldFxSpriteParams.tpage = tpage;
            }
            {
                int ticks = burst->timer;
                fade = 0x80 - ticks * 4;
                tilt.z = ticks << 4;
            }
            {
                int kind;
                int palette;
                kind = g_FieldFxSpriteParams.palette;
                palette = g_FieldFxPaletteTable[kind];
                if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
                FieldFx_DrawShapeQuads(&position, &tilt, 0x3000, 0x3000, 0x98,
                              GetClut(0x10, palette), 1, fade, 0);
            }
            {
                int kind;
                int palette;
                kind = g_FieldFxSpriteParams.palette;
                palette = g_FieldFxPaletteTable[kind];
                if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
                FieldFx_DrawShapeQuads(&position, &tilt, 0x1800, 0x1800, 0x98,
                              GetClut(0x70, palette), 1, fade, 0);
            }
            g_FieldFxSpriteParams.parameter00 = 0x40;
            g_FieldFxSpriteParams.parameter02 = 4;
            g_FieldFxSpriteParams.extent_x = 0x40;
            g_FieldFxSpriteParams.extent_y = 0x40;
            {
                int kind;
                int palette;
                kind = g_FieldFxSpriteParams.palette;
                palette = g_FieldFxPaletteTable[kind];
                if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
                FieldFx_DrawShapeQuads(&position, 0, 0x2000, 0x2000, 8,
                              GetClut(0x90, palette), 1, fade / 2, 0);
            }
            {
                RenderMatrixSlot *matrixSlot = &g_ViewMatrixSlot;
                gte_ldrotmatrix(matrixSlot->value);
                gte_ldtransmatrix(matrixSlot->value);
            }
            scale = (burst->timer << 5) + 0x1000;
            FieldFx_DrawRingBand(&position, 0x2BC, 1000, 0x14, 0, scale, scale, 0, &ringColor,
                          fade / 2, 1);
        }
        g_FieldFxSpriteParams.parameter00 = 0x20;
        g_FieldFxSpriteParams.parameter02 = 2;
        g_FieldFxSpriteParams.extent_x = 0x20;
        g_FieldFxSpriteParams.extent_y = 0x20;
        {
            int tpage = g_FieldFxTpageTable[TEXTURE_SLOT_RECORD(g_FieldFxTpageSlot3)];
            g_FieldFxSpriteParams.palette = 3;
            g_FieldFxSpriteParams.parameter06 = 0;
            g_FieldFxSpriteParams.parameter0A = 0;
            g_FieldFxSpriteParams.depth = 8;
            g_FieldFxSpriteParams.tpage = tpage;
        }
        break;
    }
    return 0;
}

/* Mode 1 spins and ages the ray (states return 1 when they finish);
 * mode 2 draws it as a growing ray, a sweeping blade or a glow sprite. */
int SceneE19_SpinRayCallback(int mode, void *memory, intptr_t argument) {
    SceneSpinRay *ray=memory;
    (void)argument;
    RenderColor color;
    RenderColor rayColor;
    RenderColor bladeHead;
    RenderColor bladeTail;
    GteShortVector offset;
    int time;
    int width;
    int glow;
    int angle;

    rayColor = g_SceneE19SpinRayColor;
    bladeHead = g_SceneE19SpinRayBladeHeadColor;
    bladeTail = g_SceneE19SpinRayBladeTailColor;
    switch (mode) {
    case 1:
        switch (ray->state) {
        case 0:
            ray->timer++;
            ray->spin.angles.x += ray->reach;
            ray->spin.angles.y += ray->spinY;
            ray->spin.angles.z += 0x18;
            break;
        case 1:
            ray->timer++;
            ray->spin.angles.y += ray->spinY;
            ray->spin.angles.x += 8;
            ray->spin.angles.z += 0x18;
            if ((s16)ray->timer < 0x30) break;
            return 1;
        case 2:
            ray->timer++;
            ray->spin.angles.x += 4;
            ray->spin.angles.y += 4;
            ray->spin.angles.z += 0x30;
            if ((s16)ray->timer < 0x10) break;
            return 1;
        }
        break;
    case 2:
        {
            RenderMatrixSlot *matrixSlot = &g_ViewMatrixSlot;
            gte_ldrotmatrix(matrixSlot->value);
            gte_ldtransmatrix(matrixSlot->value);
        }
        switch (ray->state) {
        case 0:
            time = (s16)ray->timer;
            if (time >= 0x10) time = 0x10;
            width = rsin(time << 6) / 32;
            if (ray->timer & 1) glow = 0x5C;
            else glow = 0x80;
            FieldFx_SampleColorTrack(g_SceneE19SpinRayColorTrack, &color, time);
            FieldFx_DrawRotatedTriangle(&g_SceneE19SpinRayOrigin, &ray->spin.rotation, 0x9C4, width, &rayColor, 0, 0,
                          (s16)glow, 1);
            break;
        case 1: {
            int kind;
            int palette;
            angle = rcos(((s16)ray->timer << 10) / 48);
            glow = ((s16)ray->timer << 7) / 48;
            width = rsin(((s16)ray->timer << 10) / 48) / 4 + 0x1F4;
            FieldFx_AnglesToVector(&ray->spin.angles, (s16)(ray->reach * angle / 4096), &offset);
            offset.x += g_SceneE19SpinRayOrigin.x;
            offset.y += g_SceneE19SpinRayOrigin.y;
            offset.z += g_SceneE19SpinRayOrigin.z;
            kind = g_FieldFxSpriteParams.palette;
            palette = g_FieldFxPaletteTable[kind];
            if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
            FieldFx_DrawRotatedQuad(&offset, &ray->spin.rotation, width, 0x8C, 0x80, 0x80, 0x80, 0x10,
                          GetClut(0x70, palette), &bladeHead, &bladeTail,
                          (s16)glow, 1);
            break;
        }
        case 2: {
            int kind;
            int palette;
            width = rcos((s16)ray->timer << 6) * 2;
            FieldFx_SampleColorTrack(g_SceneE19SpinRayColorTrack, &color, (s16)ray->timer);
            {
                int tpage;
                g_FieldFxSpriteParams.parameter00 = 0x40;
                g_FieldFxSpriteParams.parameter02 = 4;
                g_FieldFxSpriteParams.extent_x = 0x40;
                g_FieldFxSpriteParams.extent_y = 0x40;
                tpage = g_FieldFxTpageTable[g_FieldFxTpageSlot3];
                g_FieldFxSpriteParams.palette = 3;
                g_FieldFxSpriteParams.parameter06 = 0;
                g_FieldFxSpriteParams.tpage = tpage;
            }
            ray->spin.rotation.flags = 1;
            kind = g_FieldFxSpriteParams.palette;
            palette = g_FieldFxPaletteTable[kind];
            if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
            FieldFx_DrawShapeQuads(&g_SceneE19SpinRayOrigin, &ray->spin.rotation, width, width, 0x40,
                          GetClut(0, palette), 1, 0x80, &color);
            break;
        }
        }
        break;
    }
    return 0;
}

/* Spin ray controller: for 48 frames it seeds spinning rays on odd frames,
 * casts one of its sixteen blades every fourth frame and fans out 26
 * blades on the first frame. While it runs it follows the room actor,
 * publishes that spot as the ray origin and draws a halo, two glows and
 * two rings there. */
int SceneE19_SpinRayBurst(int mode, void *memory, intptr_t argument) {
    SceneSpinRayBurst *burst=memory;
    (void)argument;
    GteRotation tilt = g_SceneE19RollTilt;
    GteShortVector position;
    RenderColor color;
    RenderColor ringColor = g_SceneE19SpinRayBurstRingColor;
    GteShortVector floor;
    GteRotation flat;
    SceneSpinRay *child;
    int i;
    int scale;
    int glow;

    switch (mode) {
    case 0:
        burst->timer = 0;
        burst->blades = 0x10;
        FieldFx_PlaySoundAtOwner(0x5E1, FieldFx_GetOwnerProgress());
        FieldFx_PlaySoundAtOwner(0x5E2, 0x80);
        return FieldFx_InitCallbackList(g_FieldFxActiveTaskSlot->end, 0x10, 0x32,
                             (FieldFxCallbackListCallback)SceneE19_SpinRayCallback);
    case 1:
        if (++burst->timer & 1) {
            child = FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
            if (child) {
                child->spin.angles.x = rand();
                child->spin.angles.y = rand();
                child->spin.angles.z = rand();
                child->timer = 0;
                child->state = 2;
            }
        }
        if (!(burst->timer & 3) && burst->blades > 0) {
            child = FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
            if (child) {
                FieldFx_RandomScatterOffsetXY(child);
                child->timer = 0;
                child->state = 0;
                child->reach = (rand() & 0x1F) - 0x10;
                child->spinY = (rand() & 0x1F) - 0x10;
            }
            burst->blades--;
        }
        if ((s16)burst->timer == 1) {
            for (i = 0; i < 26; i++) {
                child = FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
                if (child) {
                    FieldFx_RandomScatterOffsetXY(child);
                    child->timer = 0;
                    child->state = 1;
                    child->reach = (rand() & 0x1FF) + 600;
                    child->spinY = (rand() & 7) - 3;
                }
            }
        }
        if ((s16)burst->timer < 0x30) break;
        return 1;
    case 2:
        g_FieldFxSpriteParams.parameter00 = 0x40;
        g_FieldFxSpriteParams.parameter02 = 4;
        g_FieldFxSpriteParams.extent_x = 0x40;
        g_FieldFxSpriteParams.extent_y = 0x40;
        {
            int tpage = g_FieldFxTpageTable[g_FieldFxTpageSlot3];
            g_FieldFxSpriteParams.palette = 3;
            g_FieldFxSpriteParams.parameter06 = 0;
            g_FieldFxSpriteParams.parameter0A = 0;
            g_FieldFxSpriteParams.depth = 0x18;
            g_FieldFxSpriteParams.tpage = tpage;
        }
        FieldFx_TransformMatrixPoint((struct RoomFxTransformOwner *)g_FieldFxOwner->actor, 0, (GteShortVector *)&tilt, &position);
        g_SceneE19SpinRayOrigin.x = position.x;
        g_SceneE19SpinRayOrigin.y = position.y;
        g_SceneE19SpinRayOrigin.z = position.z;
        if (burst->timer & 1) glow = 0x60;
        else glow = 0x80;
        scale = rsin(((s16)burst->timer << 10) / 48) / 2 + 0x800;
        FieldFx_SampleColorTrack(g_SceneE19SpinRayBurstColorTrack, &color, (s16)burst->timer);
        FieldFx_DrawStarFan(&position, 800, 800, 12, 0, scale, scale, &color, 0, 100, 1);
        {
            int kind;
            int palette;
            kind = g_FieldFxSpriteParams.palette;
            palette = g_FieldFxPaletteTable[kind];
            if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
            FieldFx_DrawShapeQuads(&position, 0, scale, scale, 8, GetClut(0x90, palette), 1,
                          glow, 0);
        }
        flat.x = 0x400;
        flat.y = 0;
        flat.z = 0;
        flat.flags = 1;
        floor.x = position.x;
        floor.z = position.z;
        floor.y = g_RoomFloorY->raw;
        {
            int kind;
            int palette;
            kind = g_FieldFxSpriteParams.palette;
            palette = g_FieldFxPaletteTable[kind];
            if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
            FieldFx_DrawShapeQuads(&floor, &flat, scale, scale, 8, GetClut(0x10, palette), 3,
                          glow, 0);
        }
        glow = ((s16)burst->timer << 7) / 48;
        FieldFx_DrawRingBand(&position, 0x44C, 0x1770, 0x10, 0, 0x1000, 0x1000, 0, &ringColor,
                      glow, 1);
        FieldFx_DrawRingBand(&position, 1000, 0x4B0, 0x18, 0, 0x1000, 0x1000, &ringColor, 0,
                      glow, 1);
        g_FieldFxSpriteParams.parameter00 = 0x40;
        g_FieldFxSpriteParams.parameter02 = 4;
        g_FieldFxSpriteParams.extent_x = 0x40;
        g_FieldFxSpriteParams.extent_y = 0x40;
        {
            int tpage = g_FieldFxTpageTable[g_FieldFxTpageSlot3];
            g_FieldFxSpriteParams.palette = 3;
            g_FieldFxSpriteParams.parameter06 = 0;
            g_FieldFxSpriteParams.parameter0A = 0;
            g_FieldFxSpriteParams.depth = 0x18;
            g_FieldFxSpriteParams.tpage = tpage;
        }
        break;
    }
    return 0;
}

/* Ember flare particle: embers drift and bounce off the floor while their
 * sideways speed decays (state 0 slowly, state 1 quickly while falling),
 * and a wide flare pulses in place (state 2). */
int SceneE19_EmberFlareParticleCallback(int mode, void *memory, intptr_t argument) {
    RoomOrbitTrailParticle *p=memory;
    (void)argument;
    GteRotation spin = g_SceneE19ZeroSpin;
    GteRotation turn = g_SceneE19FlatTilt;
    RenderColor color = g_SceneE19EmberParticleColor;
    int fall;
    int bounce;
    int size;
    int glow;

    switch (mode) {
    case 1:
        switch (p->state) {
        case 0:
            p->timer++;
            p->x += p->heading.x;
            p->y += p->heading.y;
            p->z += p->heading.z;
            p->heading.x = p->heading.x * 31 / 32;
            p->heading.z = p->heading.z * 31 / 32;
            fall = (u16)p->heading.y;
            p->heading.y = fall;
            if (p->y >= g_RoomFloorY->y) {
                bounce = -(s16)fall;
                p->heading.y = bounce;
            }
            if ((s16)p->timer < 0x10) break;
            return 1;
        case 1:
            p->timer++;
            p->x += p->heading.x;
            p->y += p->heading.y;
            p->z += p->heading.z;
            p->heading.x = p->heading.x * 127 / 128;
            p->heading.z = p->heading.z * 127 / 128;
            fall = (u16)p->heading.y - 1;
            p->heading.y = fall;
            if (p->y >= g_RoomFloorY->y) {
                bounce = -(s16)fall;
                p->heading.y = bounce;
            }
            if ((s16)p->timer < 0x18) break;
            return 1;
        case 2:
            p->timer++;
            if ((s16)p->timer < 0x18) break;
            return 1;
        }
        break;
    case 2:
        switch (p->state) {
        case 0: {
            int kind;
            int palette;
            size = rsin((s16)p->timer << 7) * 3 / 2;
            glow = rsin((s16)p->timer << 7) / 160;
            spin.z = -((s16)p->timer * 32);
            g_FieldFxSpriteParams.parameter00 = 0x20;
            g_FieldFxSpriteParams.parameter02 = 2;
            g_FieldFxSpriteParams.extent_x = 0x20;
            g_FieldFxSpriteParams.extent_y = 0x20;
            FieldFx_SampleColorTrack(g_SceneE19EmberFlareColorTrack, &color, (s16)p->timer);
            kind = g_FieldFxSpriteParams.palette;
            palette = g_FieldFxPaletteTable[kind];
            if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
            FieldFx_DrawShapeQuads((GteShortVector *)p, &spin, size, size, 0x98,
                          GetClut(0x90, palette), 2, glow, 0);
            break;
        }
        case 1: {
            int kind;
            int palette;
            size = rcos(((s16)p->timer << 10) / 24);
            spin.z = (s16)p->timer * 32;
            g_FieldFxSpriteParams.parameter00 = 0x20;
            g_FieldFxSpriteParams.parameter02 = 2;
            g_FieldFxSpriteParams.extent_x = 0x20;
            g_FieldFxSpriteParams.extent_y = 0x20;
            FieldFx_SampleColorTrack(g_SceneE19EmberFlareColorTrack, &color, (s16)p->timer);
            kind = g_FieldFxSpriteParams.palette;
            palette = g_FieldFxPaletteTable[kind];
            if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
            FieldFx_DrawShapeQuads((GteShortVector *)p, &spin, size, size, 0x98,
                          GetClut(0x80, palette), 1, 0x80, &color);
            break;
        }
        case 2:
            glow = rcos(((s16)p->timer << 10) / 24) / 32;
            size = rsin(((s16)p->timer << 10) / 24);
            {
                RenderMatrixSlot *matrixSlot = &g_ViewMatrixSlot;
                gte_ldrotmatrix(matrixSlot->value);
                gte_ldtransmatrix(matrixSlot->value);
            }
            FieldFx_SampleColorTrack(g_SceneE19EmberFlareColorTrack, &color, (s16)p->timer * 2);
            FieldFx_DrawRingBand((GteShortVector *)p, 0x320, 0x578, 0x18, &turn, size, size,
                          0, &color, glow, 1);
            break;
        }
        break;
    }
    return 0;
}

/* Ember flare controller: rides the room actor and glows up for 16
 * frames, then for 32 frames sprays orbiting sparks, embers on odd frames
 * and flares every sixth; then it fades and, on its first fading frame,
 * hands the actor to the scene script. While it glows it draws two floor
 * glows, a halo and a ring. */
int SceneE19_EmberFlare(int mode, void *memory, intptr_t argument) {
    SceneE22EmberBurst *burst=memory;
    (void)argument;
    RenderColor color = g_SceneE19EmberFlareColor;
    GteShortVector position;
    GteRotation spin;
    GteShortVector floorPosition;
    GteRotation floorSpin;
    RoomOrbitTrailParticle *child;
    FieldFxObjectPrefix *channel;
    FieldActor *pool;
    FieldActorState *object;
    struct SceneAssetBlob **soundSlot;
    int volume;
    int glow;
    s16 z;

    switch (mode) {
    case 0:
        burst->state = 0;
        burst->timer = 0;
        burst->glow = 0;
        if (g_FieldFxTaskContext->flags) {
            pool = g_FieldFxOwner->actor;
            if (pool != 0 && pool->state != 0) {
                object = pool->state;
                if (*object->action_state == 1)
                    *object->action_state = 2;
            }
        }
        soundSlot = g_LoadedSceneAssetBlock;
        if (*soundSlot != 0) {
            volume = 0x7F;
            Akao_SendTableCommand(*soundSlot, 0x5E5, FieldFx_GetOwnerProgress(), 0x80, volume);
            /* Retail re-reads the sound owner for the test and again for
             * the argument. */
            if (*(void *volatile *)soundSlot != 0)
                Akao_SendTableCommand(*soundSlot, 0x5E6, 0x80, 0x80, volume);
        }
        FieldFx_GetActorPosition((char *)g_PlayerEntity, 1, (s16 *)burst);
        burst->y = g_RoomFloorY->y;
        return FieldFx_InitCallbackList(g_FieldFxActiveTaskSlot->end, 0x14, 0x28,
                             (FieldFxCallbackListCallback)SceneE19_EmberFlareParticleCallback);
    case 1:
        if (burst->state < 2)
            Battle_HaltOnPositiveX();
        switch (burst->state) {
        case 0:
            burst->timer++;
            burst->glow = rsin(burst->timer << 6) / 32;
            if (burst->timer < 0x10) break;
            burst->state = 1;
            burst->timer = 0;
            break;
        case 1:
            burst->timer++;
            child = FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
            if (child) {
                child->x = burst->x;
                child->y = burst->y;
                child->z = burst->z;
                child->heading.x = rand() % 50 - 25;
                child->heading.y = -(rand() % 32);
                child->heading.z = rand() % 50 - 25;
                child->state = 1;
                child->timer = 0;
            }
            if (burst->timer & 1) {
                child = FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
                if (child) {
                    child->x = burst->x;
                    child->y = burst->y;
                    child->z = burst->z;
                    child->y -= rand() & 0x1FF;
                    child->x += (rand() & 0x7F) - 0x40;
                    child->z += (rand() & 0x7F) - 0x40;
                    child->heading.x = rand() % 24 - 12;
                    child->heading.y = rand() % 24 - 12;
                    child->heading.z = rand() % 24 - 12;
                    child->state = 0;
                    child->timer = 0;
                }
            }
            if (burst->timer % 6 == 0) {
                child = FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
                if (child) {
                    child->x = burst->x;
                    child->y = burst->y;
                    child->z = burst->z;
                    child->state = 2;
                    child->timer = 0;
                }
            }
            if (burst->timer < 0x20) return 0;
            Battle_InitEntityColors();
            burst->state = 2;
            burst->timer = 0;
            break;
        case 2:
            burst->timer++;
            burst->glow = rcos((burst->timer << 10) / 24) / 32;
            if (burst->timer == 1 && g_FieldFxTaskContext->flags) {
                channel = g_FieldFxOwner;
                if ((channel->actor->state->core_flags & 0x3F000000) == 0x01000000) {
                    g_PlayerEntity->state->flags |= 0x4000;
                    channel->actor->state->core_flags =
                        (channel->actor->state->core_flags & 0xC0FFFFFF) | 0x19000000;
                    channel->actor->state->core_flags |= 0x80000000;
                }
            }
            if (burst->timer < 0x18) break;
            return 1;
        }
        break;
    case 2:
        {
            RenderMatrixSlot *matrixSlot = &g_ViewMatrixSlot;
            gte_ldrotmatrix(matrixSlot->value);
            gte_ldtransmatrix(matrixSlot->value);
        }
        g_FieldFxSpriteParams.parameter00 = 0x40;
        g_FieldFxSpriteParams.parameter02 = 4;
        g_FieldFxSpriteParams.extent_x = 0x40;
        g_FieldFxSpriteParams.extent_y = 0x40;
        {
            int tpage = g_FieldFxTpageTable[g_FieldFxTpageSlot11];
            g_FieldFxSpriteParams.palette = 3;
            g_FieldFxSpriteParams.parameter06 = 1;
            g_FieldFxSpriteParams.parameter0A = 0;
            g_FieldFxSpriteParams.tpage = tpage;
        }
        if (burst->glow != 0) {
            glow = burst->glow;
            if (burst->timer & 1)
                glow = glow * 15 / 16;
            if (g_FieldFxCurrentAge & 1)
                glow = glow * 2 / 3;
            spin.x = 0x400;
            spin.z = g_FieldFxCurrentAge << 3;
            g_FieldFxSpriteParams.depth = 0;
            spin.y = 0;
            spin.flags = 1;
            {
                int kind = g_FieldFxSpriteParams.palette;
                int palette;
                position.x = burst->x;
                z = burst->z;
                position.y = g_RoomFloorY->y;
                position.z = z;
                palette = g_FieldFxPaletteTable[kind];
                FieldFx_DrawShapeQuads(&position, &spin, 0x1000, 0x1000, 0,
                              GetClut(0, (kind == 4 && g_FieldFxAltPaletteFlag != 0) ? palette + 6
                                                                        : palette + 2),
                              1, glow, 0);
            }
            {
                int tpage = g_FieldFxTpageTable[TEXTURE_SLOT_RECORD(g_FieldFxTpageSlot3)];
                g_FieldFxSpriteParams.tpage = tpage;
                g_FieldFxSpriteParams.parameter06 = 0;
                g_FieldFxSpriteParams.palette = 3;
                floorSpin.x = 0x400;
                floorSpin.y = 0;
                floorSpin.z = g_FieldFxCurrentAge << 5;
                floorSpin.flags = 1;
            }
            floorPosition.x = burst->x;
            z = burst->z;
            floorPosition.y = g_RoomFloorY->y;
            floorPosition.z = z;
            {
                int kind = g_FieldFxSpriteParams.palette;
                int palette = g_FieldFxPaletteTable[kind];
                if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
                FieldFx_DrawShapeQuads(&floorPosition, &floorSpin, 0x1800, 0x1800, 0x40,
                              GetClut(0x10, palette), 1, glow, 0);
            }
            g_FieldFxSpriteParams.depth = 0x28;
            FieldFx_DrawStarFan((GteShortVector *)burst, 400, 400, 0x10, 0, 0x1000, 0x1000,
                          &color, 0, glow, 1);
            FieldFx_DrawRingBand((GteShortVector *)burst, 500, 600, 0x18, 0, 0x1000, 0x1000, 0,
                          &color, glow, 3);
        }
        {
            int tpage = g_FieldFxTpageTable[TEXTURE_SLOT_RECORD(g_FieldFxTpageSlot3)];
            g_FieldFxSpriteParams.palette = 3;
            g_FieldFxSpriteParams.parameter06 = 0;
            g_FieldFxSpriteParams.parameter0A = 0;
            g_FieldFxSpriteParams.depth = 4;
            g_FieldFxSpriteParams.tpage = tpage;
        }
        break;
    }
    return 0;
}

/* Orbit trail particle variant: the flash phase draws a spinning sprite
 * with its own parameter block and the closing ring is larger and
 * slower than RoomEffect_OrbitTrailParticle's. */
int SceneE19_OrbitTrailParticleCallback(int mode, void *memory, intptr_t argument) {
    RoomOrbitTrailParticle *p=memory;
    (void)argument;
    GteShortVector offset;
    GteShortVector blend;
    GteRotation rotation;
    RenderColor color;
    int fall;
    int bounce;
    int scale;
    int weight;
    int glow;

    color = g_SceneE19EmberParticleColor;
    switch (mode) {
    case 1:
        switch (p->state) {
        case 0:
            p->timer++;
            p->heading.x += 0x10;
            p->heading.y += 8;
            if ((s16)p->timer < 0x18) break;
            return 1;
        case 1:
            p->timer++;
            p->x += p->heading.x;
            p->y += p->heading.y;
            p->z += p->heading.z;
            p->heading.x = p->heading.x * 127 / 128;
            p->heading.z = p->heading.z * 127 / 128;
            fall = (u16)p->heading.y + 1;
            p->heading.y = fall;
            if (p->y >= g_RoomFloorY->y) {
                bounce = -(s16)fall;
                p->heading.y = bounce;
            }
            if ((s16)p->timer < 0x10) break;
            return 1;
        case 2:
            p->timer++;
            p->heading.z += 0x80;
            if ((s16)p->timer < 0xC) break;
            return 1;
        }
        break;
    case 2:
        switch (p->state) {
        case 0:
            scale = (rsin(((s16)p->timer << 10) / 24) + 0x1000) / 2;
            FieldFx_AnglesToVector(&p->heading, (s16)(p->radius * scale / 4096), &offset);
            offset.x += g_SceneE19OrbitTrailCenter.x;
            offset.y += g_SceneE19OrbitTrailCenter.y;
            offset.z += g_SceneE19OrbitTrailCenter.z;
            weight = rsin(((s16)p->timer << 10) / 24);
            LoadAverageShort12(&g_SceneE19OrbitTrailCenter, &offset, 0x1000 - weight,
                          weight, &blend);
            {
                RenderMatrixSlot *matrixSlot = &g_ViewMatrixSlot;
                gte_ldrotmatrix(matrixSlot->value);
                gte_ldtransmatrix(matrixSlot->value);
            }
            FieldFx_SampleColorTrack(g_SceneE19OrbitTrailColorTrack, &color, (s16)p->timer);
            FieldFx_DrawGlowLine(&blend, &offset, 0, (u8 *)&color, 0, 0x80, 1);
            break;
        case 1: {
            u16 clut;
            int kind;
            int palette;
            int size = rsin((s16)p->timer << 6) + 0x800;
            glow = rcos((s16)p->timer * 1204 / 16) / 128 + 0x28;
            g_FieldFxSpriteParams.parameter00 = 0x40;
            g_FieldFxSpriteParams.parameter02 = 4;
            g_FieldFxSpriteParams.extent_x = 0x40;
            g_FieldFxSpriteParams.extent_y = 0x40;
            rotation.x = 0;
            rotation.y = 0;
            rotation.z = (s16)p->timer * 32;
            rotation.flags = 0;
            kind = g_FieldFxSpriteParams.palette;
            palette = g_FieldFxPaletteTable[kind];
            if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
            clut = GetClut(0x60, palette);
            FieldFx_DrawShapeQuads((GteShortVector *)p, &rotation, size, size, 0x40, clut, 1,
                          glow / 2, 0);
            break;
        }
        case 2: {
            u16 clut;
            int kind;
            int palette;
            glow = rcos(((s16)p->timer << 10) / 12) / 32;
            scale = rsin(((s16)p->timer << 10) / 12);
            /* The scaled radius shares the palette kind's temporary, which
             * is what leaves the heading pointer the first saved register. */
            kind = p->radius * scale / 4096;
            FieldFx_AnglesToVector(&p->heading, (s16)kind, &offset);
            offset.x += g_SceneE19OrbitTrailCenter.x;
            offset.y += g_SceneE19OrbitTrailCenter.y;
            offset.z += g_SceneE19OrbitTrailCenter.z;
            {
                RenderMatrixSlot *matrixSlot = &g_ViewMatrixSlot;
                gte_ldrotmatrix(matrixSlot->value);
                gte_ldtransmatrix(matrixSlot->value);
            }
            *(u32 *)&color = 0x808080;
            {
                int tpage = g_FieldFxTpageTable[g_FieldFxTpageSlot3];
                g_FieldFxSpriteParams.palette = 3;
                g_FieldFxSpriteParams.parameter06 = 0;
                g_FieldFxSpriteParams.tpage = tpage;
            }
            kind = g_FieldFxSpriteParams.palette;
            palette = g_FieldFxPaletteTable[kind];
            if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
            clut = GetClut(0x10, palette);
            FieldFx_DrawRotatedQuad(&offset, (GteRotation *)&p->heading, 1000, 320, 0x80, 0x90,
                          0x1F, 0x1F, clut, &color, &color, (s16)glow, 1);
            break;
        }
        }
        break;
    }
    return 0;
}

/* Controller that bursts orbit spin particles from the actor during the
 * first four frames (falling sparks, sweeping trails and drifting rings),
 * flashes a glow, a halo, two tilted rings and three glow sprites while it
 * fades, and publishes the actor position as the orbit centre. */
int SceneE19_OrbitTrailBurst(int mode, void *memory, intptr_t argument) {
    RoomOrbitTrailBurst *burst=memory;
    (void)argument;
    RoomOrbitSpinBurstTilt tilt;
    RenderColor color = g_SceneE19OrbitTrailBurstColor;
    RenderColor ringColor = g_SceneE19OrbitTrailBurstRingColor;
    RoomOrbitTrailParticle *child;
    int i;

    switch (mode) {
    case 0:
        burst->counterB = 0;
        burst->counterA = 0;
        FieldFx_PlaySoundAtOwner(0x5DF, FieldFx_GetOwnerProgress());
        FieldFx_PlaySoundAtOwner(0x5E0, 0x80);
        FieldFx_GetActorPosition((char *)g_PlayerEntity, 0, (s16 *)burst);
        return FieldFx_InitCallbackList(g_FieldFxActiveTaskSlot->end, 0x14, 0x3C,
                             (FieldFxCallbackListCallback)SceneE19_OrbitTrailParticleCallback);
    case 1:
        if (g_FieldFxCurrentAge < 4) {
            for (i = 0; i < 4; i++) {
                child = (RoomOrbitTrailParticle *)FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
                if (child) {
                    child->x = burst->x;
                    child->y = burst->y;
                    child->z = burst->z;
                    child->heading.x = rand() % 32 - 0x10;
                    child->heading.y = rand() % 32 - 0x10;
                    child->heading.z = rand() % 32 - 0x10;
                    child->state = 1;
                    child->timer = 0;
                }
            }
            for (i = 0; i < 7; i++) {
                child = (RoomOrbitTrailParticle *)FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
                if (child) {
                    child->x = burst->x;
                    child->y = burst->y;
                    child->z = burst->z;
                    child->heading.x = rand();
                    child->heading.y = rand();
                    child->heading.z = rand();
                    child->radius = (rand() & 0x3FF) + 0x5DC;
                    child->state = 0;
                    child->timer = 0;
                }
            }
            for (i = 0; i < 5; i++) {
                child = (RoomOrbitTrailParticle *)FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
                if (child) {
                    child->x = burst->x;
                    child->y = burst->y;
                    child->z = burst->z;
                    child->heading.x = rand();
                    child->heading.y = rand();
                    child->heading.z = rand();
                    child->radius = (rand() & 0xFF) + 0x100;
                    child->state = 2;
                    child->timer = 0;
                }
            }
        }
        if (g_FieldFxCurrentAge < 0x1C) break;
        return 1;
    case 2:
        {
            RenderMatrixSlot *matrixSlot = &g_ViewMatrixSlot;
            gte_ldrotmatrix(matrixSlot->value);
            gte_ldtransmatrix(matrixSlot->value);
        }
        g_FieldFxSpriteParams.parameter00 = 0x20;
        g_FieldFxSpriteParams.parameter02 = 2;
        g_FieldFxSpriteParams.extent_x = 0x20;
        g_FieldFxSpriteParams.extent_y = 0x20;
        {
            int tpage = g_FieldFxTpageTable[g_FieldFxTpageSlot3];
            g_FieldFxSpriteParams.palette = 3;
            g_FieldFxSpriteParams.parameter06 = 0;
            g_FieldFxSpriteParams.parameter0A = 0;
            g_FieldFxSpriteParams.depth = 0x20;
            g_FieldFxSpriteParams.tpage = tpage;
        }
        if (g_FieldFxCurrentAge < 0x15) {
            int glow = rcos((g_FieldFxCurrentAge << 10) / 20) / 32;
            int scale;
            {
                u16 clut;
                int kind;
                int palette;
                kind = g_FieldFxSpriteParams.palette;
                palette = g_FieldFxPaletteTable[kind];
                if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
                clut = GetClut(0x90, palette);
                FieldFx_DrawShapeQuads((GteShortVector *)burst, 0, 0x2000, 0x2000, 0x98,
                              clut, 1, glow, 0);
            }
            FieldFx_DrawStarFan((GteShortVector *)burst, 0x320, 0x320, 12, 0, 0x1000,
                          0x1000, &color, 0, glow, 1);
            scale = rsin((g_FieldFxCurrentAge << 10) / 20) / 2 + 0x1000;
            FieldFx_DrawRingBand((GteShortVector *)burst, 0x226, 0x320, 0x14, 0, scale,
                          scale, 0, &color, glow / 2, 1);
            FieldFx_GetTransformLookAngles((struct RoomFxTransformOwner *)g_FieldFxOwner->actor, 0,
                          &tilt.vector);
            scale = rsin((g_FieldFxCurrentAge << 10) / 20);
            tilt.rotation.flags = 1;
            tilt.rotation.x += 0x200;
            tilt.rotation.y += 0x400;
            FieldFx_DrawRingBand((GteShortVector *)burst, 0x44C, 0x578, 0x14,
                          &tilt.rotation, scale, scale, 0, &ringColor, glow, 1);
            tilt.rotation.x -= 0x400;
            FieldFx_DrawRingBand((GteShortVector *)burst, 0x44C, 0x578, 0x14,
                          &tilt.rotation, scale, scale, 0, &ringColor, glow, 1);
            glow = rcos((g_FieldFxCurrentAge << 10) / 20) / 32;
            g_FieldFxSpriteParams.parameter00 = 0x40;
            g_FieldFxSpriteParams.parameter02 = 4;
            g_FieldFxSpriteParams.extent_x = 0x40;
            g_FieldFxSpriteParams.extent_y = 0x40;
            {
                u16 clut;
                int kind;
                int palette;
                kind = g_FieldFxSpriteParams.palette;
                palette = g_FieldFxPaletteTable[kind];
                if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
                clut = GetClut(0xA0, palette);
                FieldFx_DrawPulledSprite((GteShortVector *)burst, 0x1000, 0x1000, 12, clut,
                              1, glow, 0, 0xC00);
            }
            g_FieldFxSpriteParams.parameter00 = 0x20;
            g_FieldFxSpriteParams.parameter02 = 2;
            g_FieldFxSpriteParams.extent_x = 0x20;
            g_FieldFxSpriteParams.extent_y = 0x20;
            {
                u16 clut;
                int kind;
                int palette;
                kind = g_FieldFxSpriteParams.palette;
                palette = g_FieldFxPaletteTable[kind];
                if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
                clut = GetClut(0xB0, palette);
                FieldFx_DrawPulledSprite((GteShortVector *)burst, 0x1000, 0x1000, 0x9A, clut,
                              1, glow, 0, 0xA00);
            }
            {
                u16 clut;
                int kind;
                int palette;
                kind = g_FieldFxSpriteParams.palette;
                palette = g_FieldFxPaletteTable[kind];
                if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
                clut = GetClut(0xC0, palette);
                FieldFx_DrawPulledSprite((GteShortVector *)burst, 0x1000, 0x1000, 0x9C, clut,
                              1, glow, 0, 0x600);
            }
        }
        g_SceneE19OrbitTrailCenter.x = burst->x;
        g_SceneE19OrbitTrailCenter.y = burst->y;
        g_SceneE19OrbitTrailCenter.z = burst->z;
        g_FieldFxSpriteParams.parameter00 = 0x20;
        g_FieldFxSpriteParams.parameter02 = 2;
        g_FieldFxSpriteParams.extent_x = 0x20;
        g_FieldFxSpriteParams.extent_y = 0x20;
        {
            int tpage = g_FieldFxTpageTable[g_FieldFxTpageSlot3];
            g_FieldFxSpriteParams.palette = 3;
            g_FieldFxSpriteParams.parameter06 = 0;
            g_FieldFxSpriteParams.parameter0A = 0;
            g_FieldFxSpriteParams.depth = 8;
            g_FieldFxSpriteParams.tpage = tpage;
        }
        break;
    }
    return 0;
}

/* Spark that drifts with damped velocity and bounces off the floor
 * height; state 0 draws a spinning glint that pulses with a sine of its
 * age, state 1 spins while it drifts and draws a tinted fading glint. */
int SceneE19_GlintRibbonSparkCallback(int mode, void *memory, intptr_t argument) {
    RoomDampedSpark *spark=memory;
    (void)argument;
    GteRotation rotation;
    RenderColor color = g_SceneE19GlintRibbonSparkColor;
    int fall;
    int bounce;
    int angle;
    int size;
    int fade;
    int frame;

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
            fall = (u16)spark->vy + 1;
            spark->vy = fall;
            if (spark->y >= g_RoomFloorY->y) {
                bounce = -(s16)fall;
                spark->vy = bounce;
            }
            if ((s16)spark->timer < 24) break;
            return 1;
        case 1:
            spark->timer++;
            spark->angle += 32;
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
            angle = ((s16)spark->timer << 10) / 24;
            size = rsin(angle) + 0x1800;
            fade = rcos(angle) / 128;
            rotation.x = 0;
            rotation.y = 0;
            rotation.z = spark->angle + (s16)spark->timer * 32;
            rotation.flags = 0;
            frame = (s16)spark->timer / 8;
            g_FieldFxSpriteParams.parameter00 = 16;
            g_FieldFxSpriteParams.parameter02 = 1;
            g_FieldFxSpriteParams.extent_x = 16;
            g_FieldFxSpriteParams.extent_y = 16;
            kind = g_FieldFxSpriteParams.palette;
            palette = g_FieldFxPaletteTable[kind];
            if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
            clut = GetClut(0x80, palette);
            FieldFx_DrawShapeQuads((GteShortVector *)spark, &rotation, size, size,
                          (s16)g_FieldFxSpriteParams.parameter02 * frame + 0xFD, clut, 1,
                          fade, 0);
            break;
        }
        case 1: {
            u16 clut;
            int kind;
            int palette;
            angle = (s16)spark->timer << 6;
            fade = rcos(angle) / 64 + 16;
            rotation.z = spark->angle;
            size = rsin(angle) / 2 + 0x800;
            g_FieldFxSpriteParams.parameter00 = 16;
            g_FieldFxSpriteParams.parameter02 = 1;
            g_FieldFxSpriteParams.extent_x = 16;
            g_FieldFxSpriteParams.extent_y = 16;
            kind = g_FieldFxSpriteParams.palette;
            palette = g_FieldFxPaletteTable[kind];
            if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
            clut = GetClut(0x80, palette);
            FieldFx_DrawShapeQuads((GteShortVector *)spark, &rotation, size, size,
                          (s16)g_FieldFxSpriteParams.parameter02 * ((s16)spark->timer / 2) + 0xB8,
                          clut, 1, fade, &color);
            break;
        }
        }
        break;
    }
    return 0;
}

/* Strings two light ribbons across three actor joints.  For twelve frames
 * the last joint and its floor shadow each shed a glint spark per frame,
 * after which the ribbons fade out. */
int SceneE19_GlintRibbon(int mode, void *memory, intptr_t argument) {
    RoomGlintRibbon *ribbon=memory;
    (void)argument;
    GteShortVector first;
    GteShortVector middle;
    GteShortVector last;
    GteShortVector offset0 = g_SceneE19JointOrigin;
    GteShortVector offset1 = g_SceneE19JointAnchor;
    RenderColor color0 = g_SceneE19GlintRibbonHeadColor;
    RenderColor color1 = g_SceneE19GlintRibbonMidColor;
    RenderColor color2 = g_SceneE19GlintRibbonTailColor;
    RoomDampedSpark *spark;

    switch (mode) {
    case 0:
        ribbon->alpha = 0x80;
        ribbon->reserved00 = 0;
        ribbon->timer = 0;
        FieldFx_TransformMatrixPoint((struct RoomFxTransformOwner *)g_FieldFxOwner->actor, g_SceneE19GlintRibbonJoints[0],
                      &offset0, &first);
        FieldFx_TransformMatrixPoint((struct RoomFxTransformOwner *)g_FieldFxOwner->actor, g_SceneE19GlintRibbonJoints[1],
                      &offset0, &middle);
        FieldFx_TransformMatrixPoint((struct RoomFxTransformOwner *)g_FieldFxOwner->actor, g_SceneE19GlintRibbonJoints[2],
                      &offset1, &last);
        FieldFx_DrawHistoryTrail(&first, &middle, 0x3F2, (unsigned char *)&color0, (unsigned char *)&color1, 0x80,
                      (struct FieldTrailPair *)ribbon->trailA, 1);
        FieldFx_DrawHistoryTrail(&middle, &last, 0x3F2, (unsigned char *)&color1, (unsigned char *)&color2, 0x80,
                      (struct FieldTrailPair *)ribbon->trailB, 1);
        return FieldFx_InitCallbackList(g_FieldFxActiveTaskSlot->end, 0x14, 0x24,
                             (FieldFxCallbackListCallback)SceneE19_GlintRibbonSparkCallback);
    case 1:
        ribbon->timer++;
        FieldFx_TransformMatrixPoint((struct RoomFxTransformOwner *)g_FieldFxOwner->actor, g_SceneE19GlintRibbonJoints[2],
                      &offset0, &first);
        middle.x = first.x;
        middle.y = first.y;
        middle.z = first.z;
        first.y = g_RoomFloorY->y;
        if (ribbon->timer < 13) {
            spark = FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
            if (spark != 0) {
                spark->x = first.x;
                spark->y = first.y;
                spark->z = first.z;
                spark->vx = rand() % 32 - 0x10;
                spark->vy = -(rand() % 32);
                spark->vz = rand() % 32 - 0x10;
                spark->state = 0;
                spark->timer = 0;
            }
            spark = FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
            if (spark != 0) {
                spark->x = middle.x;
                spark->y = middle.y;
                spark->z = middle.z;
                spark->vx = rand() % 32 - 0x10;
                spark->vy = rand() % 32 - 0x10;
                spark->vz = rand() % 32 - 0x10;
                spark->state = 1;
                spark->timer = 0;
            }
        } else if (ribbon->alpha > 0) {
            ribbon->alpha -= 0x10;
        }
        if (ribbon->timer < 12) break;
        return 2;
    case 2:
        if (ribbon->alpha >= 0) {
            FieldFx_TransformMatrixPoint((struct RoomFxTransformOwner *)g_FieldFxOwner->actor, g_SceneE19GlintRibbonJoints[0],
                          &offset0, &first);
            FieldFx_TransformMatrixPoint((struct RoomFxTransformOwner *)g_FieldFxOwner->actor, g_SceneE19GlintRibbonJoints[1],
                          &offset0, &middle);
            FieldFx_TransformMatrixPoint((struct RoomFxTransformOwner *)g_FieldFxOwner->actor, g_SceneE19GlintRibbonJoints[2],
                          &offset1, &last);
            FieldFx_DrawHistoryTrail(&first, &middle, 10, (unsigned char *)&color0, (unsigned char *)&color1, ribbon->alpha,
                          (struct FieldTrailPair *)ribbon->trailA, 1);
            FieldFx_DrawHistoryTrail(&middle, &last, 10, (unsigned char *)&color1, (unsigned char *)&color2, ribbon->alpha,
                          (struct FieldTrailPair *)ribbon->trailB, 1);
        }
        g_FieldFxSpriteParams.parameter00 = 0x10;
        g_FieldFxSpriteParams.parameter02 = 1;
        g_FieldFxSpriteParams.extent_x = 0x10;
        g_FieldFxSpriteParams.extent_y = 0x10;
        {
            int tpage = g_FieldFxTpageTable[g_FieldFxTpageSlot2];
            g_FieldFxSpriteParams.palette = 2;
            g_FieldFxSpriteParams.parameter06 = 0;
            g_FieldFxSpriteParams.parameter0A = 0;
            g_FieldFxSpriteParams.depth = 0x10;
            g_FieldFxSpriteParams.tpage = tpage;
        }
        break;
    }
    return 0;
}

/* Spark that drifts with damped velocity and bounces off the floor
 * height for 24 frames; state 0 draws a spinning glint whose size and
 * fade follow a sine and a cosine of its age, state 1 also spins its
 * seed angle and draws a fixed frame glint that shrinks as it fades. */
int SceneE19_TwinGlowSparkCallback(int mode, void *memory, intptr_t argument) {
    RoomDampedSpark *spark=memory;
    (void)argument;
    GteRotation rotation;
    int fall;
    int bounce;
    int angle;
    int size;
    int fade;
    int frame;

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
            fall = (u16)spark->vy + 1;
            spark->vy = fall;
            if (spark->y >= g_RoomFloorY->y) {
                bounce = -(s16)fall;
                spark->vy = bounce;
            }
            if ((s16)spark->timer < 24) break;
            return 1;
        case 1:
            spark->timer++;
            spark->angle += 20;
            spark->x += spark->vx;
            spark->y += spark->vy;
            spark->z += spark->vz;
            spark->vx = spark->vx * 31 / 32;
            spark->vz = spark->vz * 31 / 32;
            fall = (u16)spark->vy + 1;
            spark->vy = fall;
            if (spark->y >= g_RoomFloorY->y) {
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
            angle = ((s16)spark->timer << 10) / 24;
            size = rsin(angle) + 0x2000;
            fade = rcos(angle) / 80;
            rotation.x = 0;
            rotation.y = 0;
            rotation.z = spark->angle + (s16)spark->timer * 32;
            rotation.flags = 0;
            frame = (s16)spark->timer / 8;
            g_FieldFxSpriteParams.parameter00 = 16;
            g_FieldFxSpriteParams.parameter02 = 1;
            g_FieldFxSpriteParams.extent_x = 16;
            g_FieldFxSpriteParams.extent_y = 16;
            kind = g_FieldFxSpriteParams.palette;
            palette = g_FieldFxPaletteTable[kind];
            if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
            clut = GetClut(0x80, palette);
            FieldFx_DrawShapeQuads((GteShortVector *)spark, &rotation, size, size,
                          (s16)g_FieldFxSpriteParams.parameter02 * frame + 0xFD, clut, 1,
                          fade, 0);
            break;
        }
        case 1: {
            u16 clut;
            int kind;
            int palette;
            angle = ((s16)spark->timer << 10) / 24;
            fade = rcos(angle) / 46;
            rotation.z = spark->angle;
            size = rcos(angle) / 4;
            g_FieldFxSpriteParams.parameter00 = 32;
            g_FieldFxSpriteParams.parameter02 = 2;
            g_FieldFxSpriteParams.extent_x = 32;
            g_FieldFxSpriteParams.extent_y = 32;
            kind = g_FieldFxSpriteParams.palette;
            palette = g_FieldFxPaletteTable[kind];
            if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
            clut = GetClut(0x10, palette);
            FieldFx_DrawShapeQuads((GteShortVector *)spark, &rotation, size, size, 0x98,
                          clut, 1, fade, 0);
            break;
        }
        }
        break;
    }
    return 0;
}

/* Mode 0 picks the two joints; mode 1 sprays a ring of sixteen sparks and
 * sixteen falling sparks at frame 15 and reports 2 after frame 33; mode 2
 * draws pulsing glows on both joints, then fading glows, a ring and a
 * floor glow. */
int SceneE19_TwinGlow(int mode, void *memory, intptr_t argument) {
    SceneTwinGlow *glow=memory;
    (void)argument;
    GteShortVector jointA;
    GteShortVector jointB;
    GteRotation ring = g_SceneE19FlatTilt;
    GteShortVector offset = g_SceneE19TwinGlowJointOrigin;
    RenderColor color = g_SceneE19TwinGlowColor;
    GteShortVector floor;
    GteRotation flat;
    RoomOrbitTrailParticle *child;
    int i;
    int scale;
    int fade;
    int time;

    switch (mode) {
    case 0:
        glow->jointA = 6;
        glow->jointB = 0x12;
        glow->state = 0;
        glow->timer = 0;
        return FieldFx_InitCallbackList(g_FieldFxActiveTaskSlot->end, 0x14, 0x20, SceneE19_TwinGlowSparkCallback);
    case 1:
        glow->timer++;
        if (glow->timer == 0xF) {
            for (i = 0; i < 16; i++) {
                time = i << 8;
                child = FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
                if (child) {
                    child->x = glow->position.x;
                    child->y = glow->position.y;
                    child->z = glow->position.z;
                    child->x += (rand() & 0xFF) - 0x80;
                    child->z += (rand() & 0xFF) - 0x80;
                    child->heading.x = rcos(time) * 60 / 4096;
                    child->heading.z = rsin(time) * 60 / 4096;
                    child->heading.y = -(rand() & 0xF) - 8;
                    child->state = 0;
                    child->timer = 0;
                }
                child = FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
                if (child) {
                    child->x = glow->position.x;
                    child->y = glow->position.y;
                    child->z = glow->position.z;
                    child->heading.x = rand() % 49 - 0x18;
                    child->heading.y = rand() % 128 - 0x40;
                    child->heading.z = rand() % 49 - 0x18;
                    child->state = 1;
                    child->timer = 0;
                }
            }
        }
        if (glow->timer < 0x22) break;
        return 2;
    case 2:
        {
            RenderMatrixSlot *matrixSlot = &g_ViewMatrixSlot;
            gte_ldrotmatrix(matrixSlot->value);
            gte_ldtransmatrix(matrixSlot->value);
        }
        FieldFx_TransformMatrixPoint((struct RoomFxTransformOwner *)g_FieldFxOwner->actor, glow->jointA, &offset, &jointA);
        FieldFx_TransformMatrixPoint((struct RoomFxTransformOwner *)g_FieldFxOwner->actor, glow->jointB, &offset, &jointB);
        g_FieldFxSpriteParams.parameter00 = 0x40;
        g_FieldFxSpriteParams.parameter02 = 4;
        g_FieldFxSpriteParams.extent_x = 0x40;
        g_FieldFxSpriteParams.extent_y = 0x40;
        {
            int tpage = g_FieldFxTpageTable[TEXTURE_SLOT_RECORD(g_FieldFxTpageSlot3)];
            g_FieldFxSpriteParams.palette = 3;
            g_FieldFxSpriteParams.parameter06 = 0;
            g_FieldFxSpriteParams.parameter0A = 0;
            g_FieldFxSpriteParams.depth = 0x20;
            g_FieldFxSpriteParams.tpage = tpage;
        }
        if (glow->timer < 0xE) {
            scale = rsin(glow->timer << 5) / 2 + 0x800;
            if (glow->timer < 4) fade = glow->timer << 5;
            else fade = ((glow->timer & 1) << 6) + 0x80;
            {
                int kind;
                int palette;
                g_FieldFxSpriteParams.parameter00 = 0x20;
                g_FieldFxSpriteParams.parameter02 = 2;
                g_FieldFxSpriteParams.extent_x = 0x20;
                g_FieldFxSpriteParams.extent_y = 0x20;
                kind = g_FieldFxSpriteParams.palette;
                palette = g_FieldFxPaletteTable[kind];
                if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
                FieldFx_DrawShapeQuads(&jointA, 0, scale * 2 / 3, scale * 2 / 3, 0x98,
                              GetClut(0x10, palette), 1, fade, 0);
            }
            {
                int kind;
                int palette;
                g_FieldFxSpriteParams.parameter00 = 0x40;
                g_FieldFxSpriteParams.parameter02 = 4;
                g_FieldFxSpriteParams.extent_x = 0x40;
                g_FieldFxSpriteParams.extent_y = 0x40;
                kind = g_FieldFxSpriteParams.palette;
                palette = g_FieldFxPaletteTable[kind];
                if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
                FieldFx_DrawShapeQuads(&jointA, 0, scale, scale, 8, GetClut(0x10, palette), 3,
                              fade, 0);
            }
            {
                int kind;
                int palette;
                g_FieldFxSpriteParams.parameter00 = 0x20;
                g_FieldFxSpriteParams.parameter02 = 2;
                g_FieldFxSpriteParams.extent_x = 0x20;
                g_FieldFxSpriteParams.extent_y = 0x20;
                kind = g_FieldFxSpriteParams.palette;
                palette = g_FieldFxPaletteTable[kind];
                if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
                FieldFx_DrawShapeQuads(&jointB, 0, scale * 2 / 3, scale * 2 / 3, 0x98,
                              GetClut(0x10, palette), 1, fade, 0);
            }
            {
                int kind;
                int palette;
                g_FieldFxSpriteParams.parameter00 = 0x40;
                g_FieldFxSpriteParams.parameter02 = 4;
                g_FieldFxSpriteParams.extent_x = 0x40;
                g_FieldFxSpriteParams.extent_y = 0x40;
                kind = g_FieldFxSpriteParams.palette;
                palette = g_FieldFxPaletteTable[kind];
                if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
                FieldFx_DrawShapeQuads(&jointB, 0, scale, scale, 8, GetClut(0x10, palette), 3,
                              fade, 0);
            }
        } else if (glow->timer < 0x23) {
            if (glow->timer == 0xE) {
                glow->position.x = jointA.x;
                glow->position.y = jointA.y;
                glow->position.z = jointA.z;
                glow->position.y = g_RoomFloorY->raw;
            }
            time = glow->timer - 0xE;
            scale = rsin((time << 10) / 20) + 0x1000;
            fade = 0x80 - (time << 7) / 20;
            {
                int kind;
                int palette;
                g_FieldFxSpriteParams.parameter00 = 0x20;
                g_FieldFxSpriteParams.parameter02 = 2;
                g_FieldFxSpriteParams.extent_x = 0x20;
                g_FieldFxSpriteParams.extent_y = 0x20;
                kind = g_FieldFxSpriteParams.palette;
                palette = g_FieldFxPaletteTable[kind];
                if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
                FieldFx_DrawShapeQuads(&jointA, 0, scale * 2 / 3, scale * 2 / 3, 0x98,
                              GetClut(0x10, palette), 1, fade, 0);
            }
            {
                int kind;
                int palette;
                g_FieldFxSpriteParams.parameter00 = 0x40;
                g_FieldFxSpriteParams.parameter02 = 4;
                g_FieldFxSpriteParams.extent_x = 0x40;
                g_FieldFxSpriteParams.extent_y = 0x40;
                kind = g_FieldFxSpriteParams.palette;
                palette = g_FieldFxPaletteTable[kind];
                if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
                FieldFx_DrawShapeQuads(&jointA, 0, scale, scale, 8, GetClut(0x10, palette), 3,
                              fade, 0);
            }
            {
                int kind;
                int palette;
                g_FieldFxSpriteParams.parameter00 = 0x20;
                g_FieldFxSpriteParams.parameter02 = 2;
                g_FieldFxSpriteParams.extent_x = 0x20;
                g_FieldFxSpriteParams.extent_y = 0x20;
                kind = g_FieldFxSpriteParams.palette;
                palette = g_FieldFxPaletteTable[kind];
                if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
                FieldFx_DrawShapeQuads(&jointB, 0, scale * 2 / 3, scale * 2 / 3, 0x98,
                              GetClut(0x10, palette), 1, fade, 0);
            }
            {
                int kind;
                int palette;
                g_FieldFxSpriteParams.parameter00 = 0x40;
                g_FieldFxSpriteParams.parameter02 = 4;
                g_FieldFxSpriteParams.extent_x = 0x40;
                g_FieldFxSpriteParams.extent_y = 0x40;
                kind = g_FieldFxSpriteParams.palette;
                palette = g_FieldFxPaletteTable[kind];
                if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
                FieldFx_DrawShapeQuads(&jointB, 0, scale, scale, 8, GetClut(0x10, palette), 3,
                              fade, 0);
            }
            FieldFx_DrawRingBand(&glow->position, 800, 400, 0x14, &ring, scale, scale, &color, 0,
                          fade / 2, 1);
            scale = rsin(time * 1248 / 20 + 800) + 0x800;
            flat.x = 0x400;
            flat.y = 0;
            flat.z = 0;
            flat.flags = 1;
            floor.x = glow->position.x;
            floor.z = glow->position.z;
            floor.y = g_RoomFloorY->raw;
            {
                int kind;
                int palette;
                kind = g_FieldFxSpriteParams.palette;
                palette = g_FieldFxPaletteTable[kind];
                if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
                FieldFx_DrawShapeQuads(&floor, &flat, scale, scale, 8, GetClut(0x10, palette), 1,
                              fade * 2 / 3, 0);
            }
        }
        g_FieldFxSpriteParams.parameter00 = 0x20;
        g_FieldFxSpriteParams.parameter02 = 2;
        g_FieldFxSpriteParams.extent_x = 0x20;
        g_FieldFxSpriteParams.extent_y = 0x20;
        g_FieldFxSpriteParams.parameter0A = 0;
        break;
    }
    return 0;
}

/* Spark that drifts with damped velocity, jitters and spins for 8 frames
 * while drawing an animated sprite scaled by its size field. */
int SceneE19_JitterSparkCallback(int mode, void *memory, intptr_t argument) {
    RoomJitterSpark *spark=memory;
    (void)argument;
    GteRotation rotation = g_SceneE19ZeroSpin;
    int fall;
    int bounce;
    int size;

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
            int kind;
            int palette;
            rcos((s16)spark->timer << 5);
            rotation.z = spark->angle;
            size = spark->size * 4;
            kind = g_FieldFxSpriteParams.palette;
            palette = g_FieldFxPaletteTable[kind];
            if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
            FieldFx_DrawShapeQuads((GteShortVector *)spark, &rotation, size * 3 / 2, size * 3 / 2,
                          (s16)g_FieldFxSpriteParams.parameter02 * (s16)spark->timer + 184,
                          GetClut(208, palette), 1, 88, 0);
            break;
        }
        }
        break;
    }
    return 0;
}

/* Spawner that, while the primary scene object is past frame 10, scatters
 * up to three jitter sized sparks a frame up the object's anchor. */
int SceneE19_JitterSparkSpawner(int mode, void *memory, intptr_t argument) {
    (void)memory; (void)argument;
    GteShortVector position;
    RoomJitterSpark *child;
    int i;

    switch (mode) {
    case 0:
        return FieldFx_InitCallbackList(g_FieldFxActiveTaskSlot->end, 20, 10,
                             (FieldFxCallbackListCallback)SceneE19_JitterSparkCallback);
    case 1:
        if ((void *)g_FieldFxOwner != (void *)g_PmSlotTable) return 1;
        if (g_FieldFxOwner->actor->state->control10.command_value < 11) return 1;
        FieldFx_GetActorPosition((char *)g_FieldFxOwner->actor, 0, (s16 *)&position);
        for (i = 0; i < 3; i++) {
            if (rand() & 1) {
                child = (RoomJitterSpark *)FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
                if (child) {
                    position.y += 110;
                    child->x = position.x;
                    child->y = position.y;
                    child->z = position.z;
                    child->x += rand() % 400 - 200;
                    child->y += rand() % 800 - 400;
                    child->z += rand() % 400 - 200;
                    child->size = (rand() & 0x1FF) + 682;
                    child->angle = rand();
                    child->vx = rand() % 24 - 12;
                    child->vy = -(rand() % 24);
                    child->vz = rand() % 24 - 12;
                    child->state = 0;
                    child->timer = 0;
                }
            }
        }
        break;
    case 2: {
        int tpage;
        g_FieldFxSpriteParams.parameter00 = 16;
        g_FieldFxSpriteParams.parameter02 = 1;
        g_FieldFxSpriteParams.extent_x = 16;
        g_FieldFxSpriteParams.extent_y = 16;
        tpage = g_FieldFxTpageTable[g_FieldFxTpageSlot3];
        g_FieldFxSpriteParams.palette = 3;
        g_FieldFxSpriteParams.parameter06 = 0;
        g_FieldFxSpriteParams.parameter0A = 0;
        g_FieldFxSpriteParams.depth = 32;
        g_FieldFxSpriteParams.tpage = tpage;
        break;
    }
    }
    return 0;
}

intptr_t SceneE19_BlastConfigure(int mode, intptr_t value, intptr_t extra, intptr_t unused) {
    return (intptr_t)&g_SceneE19BlastGlow;
}

intptr_t SceneE19_SetHomingBurstParams(int unused, intptr_t first, intptr_t second, intptr_t extra) {
    s16 *result = g_SceneE19HomingBurstParams;

    *result = first;
    g_SceneE19HomingBurstParams[1] = second;
    return (intptr_t)result;
}

intptr_t SceneE19_EmberFlareConfigure(int mode, intptr_t value, intptr_t extra, intptr_t unused) {
    return (intptr_t)&g_SceneE19BlastGlow;
}

intptr_t SceneE19_OrbitTrailConfigure(int mode, intptr_t value, intptr_t extra, intptr_t unused) {
    return (intptr_t)&g_SceneE19BlastGlow;
}

intptr_t SceneE19_GlintRibbonConfigure(int mode, intptr_t value, intptr_t extra, intptr_t unused) {
    return (intptr_t)&g_SceneE19BlastGlow;
}

intptr_t SceneE19_TwinGlowConfigure(int mode, intptr_t value, intptr_t extra, intptr_t unused) {
    return (intptr_t)&g_SceneE19BlastGlow;
}

intptr_t SceneE19_JitterSparkConfigure(int mode, intptr_t value, intptr_t extra, intptr_t unused) {
    return (intptr_t)&g_SceneE19BlastGlow;
}


/* Authored USA M429/E19 constants and complete native descriptors. */
s32 g_SceneE19BlastGlow;
void *g_SceneE19BlastModel0;
void *g_SceneE19BlastModel1;
void *g_SceneE19BlastModel2;
void *g_SceneE19BlastModel3;
void *g_SceneE19BlastModel4;
RenderColor g_SceneE19EmberFlareColor = {100,80,200,0};
u8 g_SceneE19EmberFlareColorTrack[96] = {0,0,0,0,0,0,0,0,200,200,200,8,0,0,0,0,120,80,200,8,0,0,0,0,96,0,128,8,0,0,0,0,0,0,0,0,0,0,0,0,236,120,25,128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,16,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,178,25,128,100,180,25,128};
RenderColor g_SceneE19EmberParticleColor = {128,128,128,0};
GteRotation g_SceneE19FlatTilt = {1024,0,0,1};
RenderColor g_SceneE19GlintRibbonHeadColor = {0,0,0,0};
s16 g_SceneE19GlintRibbonJoints[3] = {41,42,43};
RenderColor g_SceneE19GlintRibbonMidColor = {0,0,140,0};
RenderColor g_SceneE19GlintRibbonSparkColor = {10,120,230,0};
RenderColor g_SceneE19GlintRibbonTailColor = {10,160,200,0};
RenderColor g_SceneE19HomingBurstFlashColor = {40,100,140,0};
s16 g_SceneE19HomingBurstParams[2] = {0,0};
RenderColor g_SceneE19HomingBurstRingColor = {180,140,80,0};
RenderColor g_SceneE19HomingParticleColor = {120,100,240,0};
GteShortVector g_SceneE19JointAnchor = {0,0,-40,0};
GteShortVector g_SceneE19JointOrigin = {0,0,0,0};
RenderColor g_SceneE19OrbitTrailBurstColor = {200,190,120,0};
RenderColor g_SceneE19OrbitTrailBurstRingColor = {92,0,128,0};
GteShortVector g_SceneE19OrbitTrailCenter = {0,0,0,0};
u8 g_SceneE19OrbitTrailColorTrack[96] = {0,0,0,0,0,0,0,0,200,200,200,8,0,0,0,0,200,80,128,8,0,0,0,0,128,0,128,8,0,0,0,0,0,0,0,0,0,0,0,0,20,137,25,128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,12,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,178,25,128,228,180,25,128};
GteRotation g_SceneE19RollTilt = {0,0,-500,0};
RenderColor g_SceneE19SpinRayBladeHeadColor = {140,70,0,0};
RenderColor g_SceneE19SpinRayBladeTailColor = {140,140,10,0};
u8 g_SceneE19SpinRayBurstColorTrack[96] = {0,0,0,0,0,0,0,0,0,0,0,16,0,0,0,0,128,0,80,16,0,0,0,0,140,40,200,16,0,0,0,0,190,100,200,0,0,0,0,0,196,96,25,128,204,108,25,128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,12,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,232,177,25,128,184,179,25,128};
RenderColor g_SceneE19SpinRayBurstRingColor = {110,0,70,0};
RenderColor g_SceneE19SpinRayColor = {128,10,92,0};
u8 g_SceneE19SpinRayColorTrack[32] = {0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,128,0,80,12,0,0,0,0,200,170,200,0,0,0,0,0};
GteShortVector g_SceneE19SpinRayOrigin = {0,0,0,0};
RenderColor g_SceneE19TwinGlowColor = {80,70,70,0};
GteShortVector g_SceneE19TwinGlowJointOrigin = {0,0,-32,0};
GteRotation g_SceneE19ZeroSpin = {0,0,0,0};
_Alignas(GteMatrix) char g_SceneE19GrabIdentityMatrix[32] = {0,16,0,0,0,0,0,0,0,16,0,0,0,0,0,0,0,16,0,0,-80,0,0,0,64,1,0,0,-48,-1,-1,-1};
_Alignas(GteShortVector) char g_SceneE19GrabAngles[8] = {0,15,0,12,0,0,0,0};
s32 g_SceneE19GrabPlayerTranslation[3] = {176,320,-48};
static u16 g_SceneE19JitterSparkProgramScript[] = {1,6,94,2,0,0,0,0,0,0};
FieldFxTaskProgram g_SceneE19JitterSparkProgram = {{{SceneE19_JitterSparkSpawner,NULL,NULL,NULL,NULL,NULL,NULL,NULL},{12,0,0,0,0,0,0,0}},SceneE19_JitterSparkConfigure,g_SceneE19JitterSparkProgramScript};
static u16 g_SceneE19GlintRibbonProgramScript[] = {1,6,92,2,0,0,0,0,0,0};
FieldFxTaskProgram g_SceneE19GlintRibbonProgram = {{{SceneE19_GlintRibbon,NULL,NULL,NULL,NULL,NULL,NULL,NULL},{360,0,0,0,0,0,0,0}},SceneE19_GlintRibbonConfigure,g_SceneE19GlintRibbonProgramScript};
static u16 g_SceneE19TwinGlowProgramScript[] = {1,6,93,2,0,0,0,0,0,0,0,0,0,0,120,2740,0,0,10250,2790,0,0,0,0,0,0};
FieldFxTaskProgram g_SceneE19TwinGlowProgram = {{{SceneE19_TwinGlow,NULL,NULL,NULL,NULL,NULL,NULL,NULL},{16,0,0,0,0,0,0,0}},SceneE19_TwinGlowConfigure,g_SceneE19TwinGlowProgramScript};
static u16 g_SceneE19HomingProgramScript[] = {5,0,0,1,6,61,4,11,0,2,1,0,4,48,0,2,0,0,0,0,0,0,0,0,0,0,0,1024,0,0,128,3152,0,0,43720,200,0,0,0,0,0,0,0,4096,0,0,128,4176,0,0,10380,4296,0,0,25790,200,0,0};
FieldFxTaskProgram g_SceneE19HomingProgram = {{{SceneE19_HomingBurst,SceneE19_SpinRayBurst,NULL,NULL,NULL,NULL,NULL,NULL},{12,4,0,0,0,0,0,0}},SceneE19_SetHomingBurstParams,g_SceneE19HomingProgramScript};
static u16 g_SceneE19BlastProgramScript[] = {5,0,0,1,6,60,2,0,0,0,0,0,0,0,0,0,51400,3272,0,0,51340,4196,0,0,20480,3272,0,0,0,0,0,0};
FieldFxTaskProgram g_SceneE19BlastProgram = {{{SceneE19_Blast,NULL,NULL,NULL,NULL,NULL,NULL,NULL},{28,0,0,0,0,0,0,0}},SceneE19_BlastConfigure,g_SceneE19BlastProgramScript};
static u16 g_SceneE19OrbitProgramScript[] = {1,6,63,4,8,0,2,0,0,0,0,0,0,0,0,0,51400,2248,0,0,20680,2176,0,0,128,2176,0,0,0,0,0,0};
FieldFxTaskProgram g_SceneE19OrbitProgram = {{{SceneE19_OrbitTrailBurst,NULL,NULL,NULL,NULL,NULL,NULL,NULL},{12,0,0,0,0,0,0,0}},SceneE19_OrbitTrailConfigure,g_SceneE19OrbitProgramScript};
static u16 g_SceneE19EmberProgramScript[] = {5,0,0,1,6,62,4,16,0,2,0,0,0,0,0,0,0,0,0,0,51400,2248,0,0,20600,2248,0,0,96,2176,0,0,0,0,0,0};
FieldFxTaskProgram g_SceneE19EmberProgram = {{{SceneE19_EmberFlare,NULL,NULL,NULL,NULL,NULL,NULL,NULL},{16,0,0,0,0,0,0,0}},SceneE19_EmberFlareConfigure,g_SceneE19EmberProgramScript};
void SceneE19_GrabNop0(void);
void SceneE19_GrabInit(PmSlotHeader *);
int SceneE19_GrabConfigure(PmSlotHeader *,int,int,intptr_t,intptr_t,intptr_t);
int SceneE19_GrabNop3(PmSlotHeader *);
int RoomLib_UpdateCallback(PmSlotHeader *);
PmCommand g_SceneE19GrabClass = {SceneE19_GrabNop0,SceneE19_GrabInit,SceneE19_GrabConfigure,SceneE19_GrabNop3,RoomLib_UpdateCallback,SceneE19_GrabRelease,SceneE19_GrabNop6};
void SceneE19_ResetModule(void) { g_SceneE19BlastGlow=0; memset(g_SceneE19HomingBurstParams,0,sizeof(g_SceneE19HomingBurstParams)); g_SceneE19SpinRayOrigin=(GteShortVector){0}; g_SceneE19OrbitTrailCenter=(GteShortVector){0}; g_SceneE19BlastModel0=g_SceneE19BlastModel1=g_SceneE19BlastModel2=g_SceneE19BlastModel3=g_SceneE19BlastModel4=NULL; }
