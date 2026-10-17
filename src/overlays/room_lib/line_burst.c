/*
 * The line burst: a controller anchored on the actor that spits bounce
 * particles, sweeps a line from its anchor to a sprite point, rings it with
 * particles, holds a shrinking glow that tags the player and fades out.
 *
 * Fourteen room overlays link the same two functions in this order, with the
 * particle colour, the anchor template and the sprite rotation as the only
 * read-only data; this unit is that object, compiled into each of them.
 */

#include "pe1/gte.h"
#include "pe1/geom_state.h"
#include "pe1/room_line_burst.h"
#include "pe1/fieldfx_task.h"
#include "pe1/battle.h"
#include "pe1/room_module.h"
#include "room_lib.h"

static const RoomLineBurstColor s_LineBurstParticleColor = { 0x80, 0x80, 0x80, 0 };
static const RoomLineBurstWords8 s_LineBurstAnchorTemplate = { { 0, 0xFF88 } };
static const RoomLineBurstWords8 s_LineBurstSpriteRotation = { { 0x400, 0x10000 } };

/* The burst's bounce particle. */
#define ROOMLIB_UPDATE_BOUNCE_RENDER_NAME RoomEffect_LineBurstParticle
#define ROOMLIB_BOUNCE_RENDER_BLOB s_LineBurstParticleColor
#include "bounce_render.inc"

/* Four-phase burst anchored on the actor: a pair of bounce particles and a
 * line swept from the anchor to the sprite point, a ring of eight particles
 * once the line arrives, a shrinking glow that tags the player while it
 * holds, then a fade.  Mode 2 projects the line through RTPT into a LINE_F2
 * packet and draws the glow sprite. */
int RoomEffect_LineBurstController(int mode, void *data, intptr_t argument) {
    RoomLineBurstState *state=data;
    RoomLineBurstParams *params=(void *)argument;
    RoomLineBurstWords8 template = s_LineBurstAnchorTemplate;
    RoomLineBurstWords8 rotation = s_LineBurstSpriteRotation;
    s16 target[4];
    int otz;
    int weight;
    RoomLineBurstParticle *child;
    int angle;
    int i;

    switch (mode) {
    case 0:
        state->state = 0;
        state->timer = 0;
        state->scale = 0;
        state->intensity = 0;
        state->countdown = 0;
        state->px = params->x;
        state->py = params->y;
        state->pz = params->z;
        state->py = g_RoomFloorY->y;
        FieldFx_PlaySoundAtOwner(0x57B, FieldFx_GetOwnerProgress());
        if (g_FieldFxTaskContext->flags) {
            FieldActor *owner=g_FieldFxOwner->actor;
            if (owner && owner->state && owner->state->action_state) {
                u8 *flag=owner->state->action_state;
                if (*flag == 1) *flag=2;
            }
        }
        return FieldFx_InitCallbackList(g_FieldFxActiveTaskSlot->end, 0x14, 0x18,
                             RoomEffect_LineBurstParticle);
    case 1:
        FieldFx_TransformMatrixPoint((RoomFxTransformOwner *)g_FieldFxOwner->actor, 0x13, (GteShortVector *)&template, (GteShortVector *)state);
        FieldFx_GetTransformLookAngles((struct RoomFxTransformOwner *)g_FieldFxOwner->actor, 0,
                      (GteShortVector *)target);
        angle = -target[1] + 0x400;
        switch (state->state) {
        case 0:
            state->timer++;
            if (state->timer == 1) {
                for (i = 0; i < 2; i++) {
                    child = FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
                    if (child) {
                        child->x = state->x;
                        child->y = state->y;
                        child->z = state->z;
                        child->vx = rcos(angle) / 256;
                        child->vz = rsin(angle) / 256;
                        child->vy = 0;
                        child->state = i;
                        child->timer = 0;
                    }
                }
            }
            weight = 0x1000 - rcos((state->timer * 1024) / 6);
            LoadAverageShort12(state, &state->px, 0x1000 - weight, weight, &state->ex);
            state->color = 0x80 - (state->timer * 32) / 6;
            if (state->timer < 6) break;
            state->state = 1;
            state->timer = 0;
            for (i = 0; i < 8; i++) {
                child = FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
                if (child) {
                    int speed;
                    child->x = state->px;
                    child->y = state->py;
                    child->z = state->pz;
                    speed = (rand() & 7) + 0x14;
                    child->vx = rcos(angle) * speed / 4096;
                    child->vz = rsin(angle) * speed / 4096;
                    child->vy = 0;
                    child->state = 0;
                    child->timer = 0;
                }
                angle += 0x200;
            }
            if (Geo_PointInPoly(state->px * 65536, state->pz * 65536, g_FloorPolygonVertices,
                              g_FloorPolygonVertexCount)) {
                break;
            }
            return 1;
        case 1:
            state->color = rcos((++state->timer * 1024) / 6) / 128 + 0x40;
            state->ex = state->px;
            state->ey = state->py;
            state->ez = state->pz;
            state->scale = rsin((state->timer * 1024) / 6) + 0x800;
            state->intensity = 0x80;
            if (state->timer < 6) break;
            state->state = 2;
            state->timer = 0;
            state->color = 0;
            for (i = 0; i < 12; i++) {
                child = FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
                if (child) {
                    weight = rand() & 0xFFF;
                    LoadAverageShort12(state, &state->px, 0x1000 - weight, weight, child);
                    child->vy = 0;
                    child->ay = (rand() & 3) + 1;
                    child->state = 1;
                    child->timer = 0;
                }
            }
            break;
        case 2:
            state->timer++;
            if (state->scale > 0x1000) state->scale -= 0x400;
            state->intensity = rsin((state->timer * 16384) / params->duration) / 128 + 0x80;
            if (state->countdown != 0) state->countdown--;
            if (FieldFx_IsPlayerWithinRadius(&state->px, 0x104) && state->countdown == 0) {
                if (g_FieldFxTaskContext->flags) {
                    FieldActorState *owner=g_FieldFxOwner->actor->state;
                    if ((owner->core_flags & 0x3F000000) == 0x01000000) {
                        ((Combatant *)g_PlayerEntity->state)->stateFlags |= 0x4000;
                        owner->core_flags=(owner->core_flags & 0xC0FFFFFF) | 0x99000000;
                    }
                }
                state->countdown = params->countdown;
            }
            if (state->timer < params->duration) break;
            state->state = 3;
            state->timer = 0;
            break;
        case 3:
            state->scale = rcos(++state->timer * 32);
            state->intensity = rcos(state->timer * 32) / 32;
            if (state->timer >= 0x20) return 1;
            break;
        }
        break;
    case 2: {
        RenderMatrixSlot *matrixSlot = &g_ViewMatrixSlot;
        gte_ldrotmatrix(matrixSlot->value);
        gte_ldtransmatrix(matrixSlot->value);
        if (state->color != 0) {
            RoomLineBurstLinePacket *line =
                (RoomLineBurstLinePacket *)(D_800B0E38.packets[g_ActiveDrawSlot] + g_DrawPacketOffset);
            g_DrawPacketOffset += sizeof(*line);
            gte_ldv3(state, &state->ex, state);
            gte_rtpt_padded();
            line->tag.length = 3;
            line->code = 0x40;
            line->r = state->color;
            line->g = state->color;
            line->b = state->color;
            gte_stszotz(&otz);
            if ((u32)(otz - 1) < 0xFFF) {
                gte_stsxy3(&line->x0, &line->x1, &weight);
                FieldFx_LinkBlendedPacket(&D_800B0E38.ordering[g_ActiveDrawSlot][otz], line, 1);
            }
        }
        g_FieldFxSpriteParams.parameter00 = 0x20;
        g_FieldFxSpriteParams.parameter02 = 2;
        g_FieldFxSpriteParams.extent_x = 0x20;
        g_FieldFxSpriteParams.extent_y = 0x20;
        {
            int tpage = g_FieldFxTpageTable[g_FieldFxTpageSlot4];
            g_FieldFxSpriteParams.palette = 4;
            g_FieldFxSpriteParams.parameter06 = 0;
            g_FieldFxSpriteParams.parameter0A = 5;
            g_FieldFxSpriteParams.depth = 0;
            g_FieldFxSpriteParams.tpage = tpage;
        }
        if (state->intensity != 0) {
            int kind = g_FieldFxSpriteParams.palette;
            int palette = g_FieldFxPaletteTable[kind];
            u16 clut;
            if (kind == 4 && g_FieldFxAltPaletteFlag != 0) palette += 4;
            clut = GetClut(0, palette);
            FieldFx_DrawShapeQuads((GteShortVector *)&state->px, (GteRotation *)&rotation,
                          state->scale, state->scale, 0, clut, 1,
                          state->intensity / 2, 0);
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
            g_FieldFxSpriteParams.depth = 8;
            g_FieldFxSpriteParams.tpage = tpage;
        }
        break;
    }
    }
    return 0;
}

/* Original M122 program, including its aligned script tail. Other room
 * variants are bound only after their complete module is audited. */
u16 g_RoomLineBurstScript[16]={5,0,0,1,6,51,4,29,0,2};
FieldFxTaskProgram g_RoomLineBurstProgram={{{RoomEffect_LineBurstController},{sizeof(RoomLineBurstState)}},RoomLib_SlotSet,g_RoomLineBurstScript};
