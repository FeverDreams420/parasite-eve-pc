#include "common.h"
#include "pe1/geom_state.h"
#include "pe1/fieldfx_task.h"
#include "pe1/room_fx.h"
#include "pe1/battle.h"
#include "pe1/room_module.h"
#include "pe1/akao_send.h"
#include "pe1/field_owner_actor.h"
#include "pe1/field_sprite_draw.h"
#include "pe1/field_sprite_params.h"
#include "pe1/field_texture_tables.h"
#include "pe1/field_transform_point.h"
#include "pe1/fieldfx_callback_list.h"
#include "pe1/fieldfx_calls.h"
#include "pe1/fieldfx_task_pool.h"
#include "pe1/gte.h"
#include "pe1/gte_types.h"
#include "pe1/psyq_gpu.h"
#include "pe1/render_matrix_slot.h"
#include "pe1/room_floor.h"
#include "pe1/room_m123.h"
#include "pe1/room_m123_beacon.h"
#include "pe1/room_m123_glow_burst.h"
#include "pe1/room_m123_joint_triangle.h"
#include "pe1/room_sound_slot.h"

#include "m123_externs.h"
#include "room_m123_effects.h"

typedef struct RoomM123Burst {
    u16 frame;
    u16 phase;
    u16 x, y, z;
    u16 pad;
} RoomM123Burst;

int RoomM123_JointTriangleCallback(int mode, void *data, intptr_t argument) {
    RoomM123JointTriangle *triangle=data;
    GteShortVector corners[4];
    GteShortVector offset = g_RoomM123JointOffset;
    GteShortVector tip = g_RoomM123JointOrigin;
    RenderColor color = g_RoomM123TriangleColor;
    int weight;
    int width;
    int position;
    RenderMatrixSlot *matrixSlot;

    switch (mode) {
    case 1:
        switch (triangle->state) {
        case 0:
            triangle->frame++;
            if (triangle->frame >= 24) return 1;
            break;
        case 1:
            triangle->frame++;
            triangle->position = (s16)(triangle->position + 0x200) % 0x3000;
            if (triangle->frame >= 24) return 1;
            break;
        default:
            return 0;
        }
        break;
    case 2:
        gte_ldrotmatrix(g_ViewMatrixSlot.value);
        gte_ldtransmatrix(g_ViewMatrixSlot.value);
        FieldFx_TransformMatrixPoint((struct RoomFxTransformOwner *)g_FieldFxOwner->actor, 0x28, &offset, &corners[0]);
        FieldFx_TransformMatrixPoint((struct RoomFxTransformOwner *)g_FieldFxOwner->actor, 0x22, &offset, &corners[1]);
        FieldFx_TransformMatrixPoint((struct RoomFxTransformOwner *)g_FieldFxOwner->actor, 0x1D, &tip, &corners[2]);
        switch (triangle->state) {
        case 0:
            FieldFx_TransformMatrixPoint((struct RoomFxTransformOwner *)g_FieldFxOwner->actor, 0x17, &tip, &corners[3]);
            weight = 0x5000 - rsin(((s32)((u32)triangle->frame * 1024u)) / 24) * 4;
            LoadAverageShort12(&corners[3], &corners[0], 0x1000 - weight, weight, &corners[0]);
            LoadAverageShort12(&corners[3], &corners[1], 0x1000 - weight, weight, &corners[1]);
            LoadAverageShort12(&corners[3], &corners[2], 0x1000 - weight, weight, &corners[2]);
            width = ((s32)((u32)triangle->frame * 128u)) / 24;
            gte_ldrotmatrix(g_ViewMatrixSlot.value);
            gte_ldtransmatrix(g_ViewMatrixSlot.value);
            FieldFx_DrawGlowLine(&corners[0], &corners[1], (u8 *)&color, (u8 *)&color, width, width, 1);
            FieldFx_DrawGlowLine(&corners[1], &corners[2], (u8 *)&color, (u8 *)&color, width, width, 1);
            FieldFx_DrawGlowLine(&corners[2], &corners[0], (u8 *)&color, (u8 *)&color, width, width, 1);
            return 0;
        case 1:
            position = triangle->position;
            if (position < 0x1000) {
                weight = position;
                LoadAverageShort12(&corners[0], &corners[1], 0x1000 - weight, weight, &corners[0]);
            } else if (position < 0x2000) {
                weight = position - 0x1000;
                LoadAverageShort12(&corners[1], &corners[2], 0x1000 - weight, weight, &corners[0]);
            } else {
                weight = position - 0x2000;
                LoadAverageShort12(&corners[2], &corners[0], 0x1000 - weight, weight, &corners[0]);
            }
            matrixSlot = &g_ViewMatrixSlot;
            gte_ldrotmatrix(matrixSlot->value);
            gte_ldtransmatrix(matrixSlot->value);
            width = rsin((s32)((u32)triangle->frame * 256u)) / 32;
            FieldFx_DrawHaloedPoint(&corners[0], &color, width, 1);
            break;
        default:
            return 0;
        }
        break;
    }
    return 0;
}

int RoomM123_JointTriangleEmitter(int mode, void *data, intptr_t argument) {
    int frame, index;
    RoomM123JointTriangleEmitterView *position;

    switch (mode) {
    case 0:
        return FieldFx_InitCallbackList(g_FieldFxActiveTaskSlot->end, 8, 16,
                             RoomM123_JointTriangleCallback);
    case 1:
        frame = g_FieldFxCurrentAge;
        if (frame == (frame / 6) * 6) {
            if (frame >= 40) return 2;
            for (index = 0; index < 3; index++) {
                position = FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
                if (position != 0) {
                    position->x = 1;
                    position->y = 0;
                    position->z = index << 12;
                }
            }
            if (g_FieldFxCurrentAge < 25) {
                position = FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
                if (position != 0) {
                    position->x = 0;
                    position->y = 0;
                    position->z = 0;
                }
            }
        }
        if (g_FieldFxCurrentAge >= 40) return 2;
        break;
    case 2: {
        int paletteIndex = g_FieldFxTpageSlot3;
        int palette;
        g_FieldFxSpriteParams.parameter00 = 16;
        g_FieldFxSpriteParams.parameter02 = 1;
        g_FieldFxSpriteParams.extent_x = 16;
        g_FieldFxSpriteParams.extent_y = 16;
        palette = g_FieldFxTpageTable[paletteIndex];
        g_FieldFxSpriteParams.palette = 3;
        g_FieldFxSpriteParams.parameter06 = 0;
        g_FieldFxSpriteParams.parameter0A = 0;
        g_FieldFxSpriteParams.depth = 12;
        g_FieldFxSpriteParams.tpage = palette;
        break;
    }
    }
    return 0;
}

int RoomM123_GlowBurstParticleCallback(int mode, void *data, intptr_t argument) {
    RoomM123PulsingParticle *particle=data;
    RenderColor black={0};
    GteShortVector position;
    GteShortVector output;
    GteShortVector color;
    int scale;
    int palette;
    int kind;
    int blend;
    u16 clut;
    RenderMatrixSlot *matrixSlot;

    switch (mode) {
    case 1:
        particle->frame++;
        particle->offset += 16;
        if ((s16)particle->frame >= 48) return 1;
        break;
    case 2:
        scale = particle->scale *
                rcos(((s32)((u32)(s16)particle->frame * 1024u)) / 48) / 4096;
        FieldFx_OffsetPointByAngles(&g_RoomM123GlowBurstCenter, &position, scale / 2, &g_RoomM123GlowBurstAngles.vector,
                      scale, (s16)particle->offset);
        if ((s16)particle->frame < 16) {
            FieldFx_SampleColorTrack(g_RoomM123GlowColorTrack, &color, 0x30 - (scale * 48) / 1024);
            kind = g_FieldFxSpriteParams.palette;
            palette = g_FieldFxPaletteTable[kind];
            if (kind == 4 && g_FieldFxAltPaletteFlag) palette += 4;
            clut = GetClut(0x20, palette);
            FieldFx_DrawShapeQuads(&position, 0, 0x1000, 0x1000,
                          (s16)g_FieldFxSpriteParams.parameter02 * 2 + 0xD8, clut, 3, g_RoomScriptRecord,
                          (RenderColor *)&color);
            matrixSlot = &g_ViewMatrixSlot;
            gte_ldrotmatrix(matrixSlot->value);
            gte_ldtransmatrix(matrixSlot->value);
            FieldFx_DrawHaloedPoint(&position, &color, g_RoomScriptRecord, 1);
            /* The blended tail starts at frame 16. */
            if ((s16)particle->frame < 16) break;
        }
        matrixSlot = &g_ViewMatrixSlot;
        gte_ldrotmatrix(matrixSlot->value);
        gte_ldtransmatrix(matrixSlot->value);
        FieldFx_SampleColorTrack(g_RoomM123GlowColorTrack, &color, 0x30 - (scale * 48) / 1024);
        blend = rcos((s32)((u32)((s16)particle->frame - 16) * 32u));
        LoadAverageShort12(&position, &g_RoomM123GlowBurstCenter, blend, 0x1000 - blend, &output);
        /* Retail reads address zero for the unlit end. With its zero end
         * scale and the post-CD zero-page blue byte, that end is black. */
        FieldFx_DrawGlowLine(&output, &position, (u8 *)&color, (u8 *)&black, g_RoomScriptRecord, 0, 1);
        break;
    }
    return 0;
}

/* Glow burst: pulsing particles, joint glow layers and swept flare. */

int RoomM123_GlowBurst(int mode, void *data, intptr_t argument) {
    RoomM123GlowBurst *burst=data;
    GteShortVector template = g_RoomM123JointAnchor;
    GteShortVector position;
    GteRotation rotation;
    RenderColor color;
    RoomM123BurstParticle *child;
    int i;
    int intensity;
    int step;
    int fade;

    switch (mode) {
    case 0:
        burst->timer = 0;
        return FieldFx_InitCallbackList(g_FieldFxActiveTaskSlot->end, 8, 0x10,
                             RoomM123_GlowBurstParticleCallback);
    case 1:
        if (burst->timer < 0x80) burst->timer += 4;
        if (g_FieldFxCurrentAge == 1) {
            for (i = 0; i < 16; i++) {
                child = FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
                if (child) {
                    child->frame = 0;
                    child->scale = (rand() & 0x1FF) + 0x200;
                    child->offset = rand();
                }
            }
            FieldFx_PlaySoundAtOwner(0x580, FieldFx_GetOwnerProgress());
            FieldFx_PlaySoundAtOwner(0x5BD, 0x80);
        }
        if (g_FieldFxCurrentAge >= 33) return 1;
        break;
    case 2:
        g_FieldFxSpriteParams.parameter00 = 0x40;
        g_FieldFxSpriteParams.parameter02 = 4;
        g_FieldFxSpriteParams.extent_x = 0x40;
        g_FieldFxSpriteParams.extent_y = 0x40;
        g_FieldFxSpriteParams.tpage = g_FieldFxTpageTable[g_FieldFxTpageSlots[11]];
        g_FieldFxSpriteParams.palette = 3;
        g_FieldFxSpriteParams.parameter06 = 1;
        g_FieldFxSpriteParams.parameter0A = 0;
        g_FieldFxSpriteParams.depth = 0x32;
        FieldFx_TransformMatrixPoint((struct RoomFxTransformOwner *)g_FieldFxOwner->actor, 0x17, &template, &position);
        rotation.x = 0;
        rotation.y = 0;
        rotation.z = (s32)(0u - (u32)g_FieldFxCurrentAge * 64u);
        rotation.flags = 0;
        intensity = burst->timer;
        if (g_FieldFxCurrentAge != 0) intensity = intensity * 2 / 3;
        rcos(((s32)((u32)g_FieldFxCurrentAge * 1024u)) / 40);
        {
            int kind;
            int palette;
            g_FieldFxSpriteParams.parameter00 = 0x40;
            g_FieldFxSpriteParams.parameter02 = 4;
            g_FieldFxSpriteParams.extent_x = 0x40;
            g_FieldFxSpriteParams.extent_y = 0x40;
            kind = g_FieldFxSpriteParams.palette;
            palette = g_FieldFxPaletteTable[kind];
            FieldFx_DrawPulledSprite(&position, 0x1000, 0x1000, 0x50,
                          GetClut(0, (kind == 4 && g_FieldFxAltPaletteFlag) ? palette + 6 : palette + 2),
                          1, intensity, 0, 0);
        }
        g_FieldFxSpriteParams.parameter00 = 0x20;
        g_FieldFxSpriteParams.parameter02 = 2;
        g_FieldFxSpriteParams.extent_x = 0x20;
        g_FieldFxSpriteParams.extent_y = 0x20;
        g_FieldFxSpriteParams.tpage = g_FieldFxTpageTable[g_FieldFxTpageSlots[3]];
        g_FieldFxSpriteParams.palette = 3;
        g_FieldFxSpriteParams.parameter06 = 0;
        {
            int kind = g_FieldFxSpriteParams.palette;
            int palette = g_FieldFxPaletteTable[kind];
            if (kind == 4 && g_FieldFxAltPaletteFlag) palette += 4;
            FieldFx_DrawPulledSprite(&position, 0x1000, 0x1000, 0x4D, GetClut(0x50, palette),
                          1, intensity, 0, 0xA00);
        }
        g_FieldFxSpriteParams.parameter00 = 0x10;
        g_FieldFxSpriteParams.parameter02 = 1;
        g_FieldFxSpriteParams.extent_x = 0x10;
        g_FieldFxSpriteParams.extent_y = 0x10;
        {
            int kind = g_FieldFxSpriteParams.palette;
            int palette = g_FieldFxPaletteTable[kind];
            if (kind == 4 && g_FieldFxAltPaletteFlag) palette += 4;
            FieldFx_DrawPulledSprite(&position, 0x1000, 0x1000, 0x48, GetClut(0x30, palette),
                          1, intensity, 0, 0xC00);
        }
        g_FieldFxSpriteParams.parameter00 = 0x40;
        g_FieldFxSpriteParams.parameter02 = 4;
        g_FieldFxSpriteParams.extent_x = 0x40;
        g_FieldFxSpriteParams.extent_y = 0x40;
        {
            int kind = g_FieldFxSpriteParams.palette;
            int palette = g_FieldFxPaletteTable[kind];
            if (kind == 4 && g_FieldFxAltPaletteFlag) palette += 4;
            FieldFx_DrawPulledSprite(&position, 0x1000, 0x1000, 0x49, GetClut(0x40, palette),
                          1, intensity, 0, 0xE00);
        }
        FieldFx_SampleColorTrack(g_RoomM123GlowColorTrack, &color, g_FieldFxCurrentAge / 2);
        FieldFx_DrawStarFan(&position, 700, 700, 0x10, 0, 0x1000, 0x1000, &color, 0, 0x40, 1);
        FieldFx_SampleColorTrack(g_RoomM123GlowBurstColorTrack, &color, g_FieldFxCurrentAge * 48 / 40);
        FieldFx_DrawStarFan(&position, 0xA0, 0xA0, 8, 0, 0x1000, 0x1000, &color, 0, 0x80, 1);
        g_RoomScriptRecord = burst->timer;
        g_RoomM123GlowBurstCenter.x = position.x;
        g_RoomM123GlowBurstCenter.y = position.y;
        g_RoomM123GlowBurstCenter.z = position.z;
        FieldFx_GetTransformLookAngles((struct RoomFxTransformOwner *)g_FieldFxOwner->actor, 0, &g_RoomM123GlowBurstAngles.vector);
        step = g_FieldFxCurrentAge - 16;
        if ((unsigned int)step < 17) {
            fade = rcos((s32)((u32)step * 64u));
            FieldFx_SampleColorTrack(g_RoomM123GlowBurstColorTrack, &color, step * 3);
            g_RoomM123GlowBurstAngles.rotation.flags = 1;
            FieldFx_DrawRingBand(&position, 700, 1000, 0x18, &g_RoomM123GlowBurstAngles.rotation, fade, fade, 0, &color,
                          0x80, 1);
        }
        g_FieldFxSpriteParams.parameter00 = 0x10;
        g_FieldFxSpriteParams.parameter02 = 1;
        g_FieldFxSpriteParams.extent_x = 0x10;
        g_FieldFxSpriteParams.extent_y = 0x10;
        g_FieldFxSpriteParams.tpage = g_FieldFxTpageTable[g_FieldFxTpageSlot2];
        g_FieldFxSpriteParams.palette = 2;
        g_FieldFxSpriteParams.parameter06 = 0;
        g_FieldFxSpriteParams.depth = 0xC;
        break;
    }
    return 0;
}

/* Joint beacon: rides joint 0x17, swells in, pulses and fades out, tagging
 * the player once on contact; draws a glow sprite pair with two star fans,
 * a fading three-sprite ring on the ground, a turning model while the effect
 * starts and three scaled passes of a second model. */
int RoomM123_JointBeacon(int mode, void *data, intptr_t argument) {
    RoomM123Beacon *state=data;
    GteVector scale;
    GteShortVector position;
    GteShortVector rotation;
    RoomM123BeaconOffset offset = g_RoomM123BeaconAnchor;
    GteMatrix matrix;
    GteMatrix spin;
    RoomM123BeaconColor color;
    int size;
    int page;

    switch (mode) {
    case 0:
        state->fade = 200;
        state->phase = 0;
        state->timer = 0;
        state->glow = 0;
        state->width = 0;
        state->depth = 0x1000;
        if (g_FieldFxTaskContext->flags) {
            FieldActor *pool = g_FieldFxOwner->actor;
            if (pool && pool->state) {
                u8 *flag = pool->state->action_state;
                if (*flag == 1) *flag = 2;
            }
        }
        state->armed = 1;
        g_RoomM123BeaconModel0 = Asset_FindTable08ByU32Key(g_LoadedSceneAssetBlock[0], 0xC5483704);
        g_RoomM123BeaconModel1 = Asset_FindTable08ByU32Key(g_LoadedSceneAssetBlock[0], 0xC5883704);
        FieldFx_ModelScrollUv((union FieldTexturedModel *)g_RoomM123BeaconModel0, 0, 0);
        FieldFx_ModelScrollUv((union FieldTexturedModel *)g_RoomM123BeaconModel1, 0, 0xF0);
        return 0;
    case 1:
        FieldFx_TransformMatrixPoint((struct RoomFxTransformOwner *)g_FieldFxOwner->actor, 0x17, (GteShortVector *)&offset, &state->position);
        FieldFx_GetTransformLookAngles((struct RoomFxTransformOwner *)(struct RoomFxTransformOwner *)g_FieldFxOwner->actor, 0,
                      &state->rotation);
        state->alpha = 0x80;
        switch (state->phase) {
        case 0:
            state->timer++;
            state->height = rcos((s32)((u32)state->timer * 128u)) / 2 + 0x1000;
            if (state->width < 0x800) state->width += 0x100;
            state->depth -= 0x80;
            state->glow = rcos((s32)((u32)state->timer * 128u)) + 0x1000;
            state->shade = state->timer;
            if (state->timer >= 8) {
                state->phase = 1;
                state->timer = 0;
            }
            break;
        case 1:
            state->timer++;
            state->height = 0x1000;
            state->glow = ((g_FieldFxCurrentAge & 1) << 9) + 0x1000;
            if (state->shade < 0x20) state->shade++;
            if (state->timer >= 0x20) {
                state->phase = 2;
                state->timer = 0;
            }
            break;
        case 2:
            state->height = 0x1000;
            state->width = 0x800;
            state->glow = 0x1000;
            state->timer++;
            state->shade++;
            state->alpha = 0x80 - state->timer * 8;
            if (state->timer >= 0x10) return 1;
            break;
        }
        FieldFx_AnglesToVector(&state->rotation, 0x7D0, &position);
        state->target.x = state->position.x;
        state->target.y = state->position.y;
        state->target.z = state->position.z;
        state->target.x += position.x;
        state->target.y += position.y;
        state->target.z += position.z;
        if (FieldFx_IsPlayerInSegmentBand(state, &state->target, 0x140) == 0) return 0;
        if (state->armed == 0) return 0;
        if (g_FieldFxTaskContext->flags && g_FieldFxOwner->actor && g_FieldFxOwner->actor->state) {
            FieldActorState *core = g_FieldFxOwner->actor->state;
            if ((core->core_flags & 0x3F000000) == 0x01000000) {
                ((Combatant *)g_PlayerEntity->state)->stateFlags |= 0x4000;
                core->core_flags =
                    (core->core_flags & 0xC0FFFFFF) | 0x11000000;
                core->core_flags |= 0x80000000;
            }
        }
        state->armed = 0;
        break;
    case 2:
        g_FieldFxSpriteParams.depth = 0x32;
        g_FieldFxSpriteParams.parameter00 = 0x40;
        g_FieldFxSpriteParams.parameter02 = 4;
        g_FieldFxSpriteParams.extent_x = 0x40;
        g_FieldFxSpriteParams.extent_y = 0x40;
        g_FieldFxSpriteParams.tpage = g_FieldFxTpageTable[g_FieldFxTpageSlot11];
        g_FieldFxSpriteParams.palette = 3;
        g_FieldFxSpriteParams.parameter06 = 1;
        g_FieldFxSpriteParams.parameter0A = 0;
        if (state->glow != 0) {
            rotation.x = 0;
            rotation.y = 0;
            rotation.pad = 0;
            rotation.z = (s32)((u32)g_FieldFxCurrentAge * 64u);
            size = state->glow;
            FieldFx_SampleColorTrack(g_RoomM123BeaconColorTrack, &color, state->shade);
            {
                int kind = g_FieldFxSpriteParams.palette;
                int palette = g_FieldFxPaletteTable[kind];
                FieldFx_DrawShapeQuads(&state->position, (GteRotation *)&rotation, size * 3 / 2, size * 3 / 2, 0x50,
                              GetClut(0, (kind == 4 && g_FieldFxAltPaletteFlag) ? palette + 6
                                                                  : palette + 2),
                              1, 0xA0, (struct RenderColor *)&color);
            }
            FieldFx_DrawStarFan(&state->position, 0x2BC, 0x2BC, 0xC, 0, size, size, (struct RenderColor *)&color, 0, 0x50, 1);
            FieldFx_DrawStarFan(&state->position, 0xF0, 0xF0, 8, 0, size, size, (struct RenderColor *)&color, 0, 0x80, 1);
            if (state->fade > 0) {
                state->fade -= 8;
                if (state->fade > 0) {
                    position.x = state->position.x;
                    position.y = state->position.y;
                    position.z = state->position.z;
                    g_FieldFxSpriteParams.parameter00 = 0x20;
                    g_FieldFxSpriteParams.parameter02 = 2;
                    g_FieldFxSpriteParams.extent_x = 0x20;
                    g_FieldFxSpriteParams.extent_y = 0x20;
                    g_FieldFxSpriteParams.tpage = g_FieldFxTpageTable[g_FieldFxTpageSlots[3]];
                    g_FieldFxSpriteParams.palette = 3;
                    g_FieldFxSpriteParams.parameter06 = 0;
                    {
                        int kind = g_FieldFxSpriteParams.palette;
                        int palette = g_FieldFxPaletteTable[kind];
                        if (kind == 4 && g_FieldFxAltPaletteFlag) palette += 4;
                        FieldFx_DrawPulledSprite(&position, 0x1000, 0x1000, 0x4D, GetClut(0x50, palette), 1,
                                      state->fade, 0, 0xA00);
                    }
                    g_FieldFxSpriteParams.parameter00 = 0x10;
                    g_FieldFxSpriteParams.parameter02 = 1;
                    g_FieldFxSpriteParams.extent_x = 0x10;
                    g_FieldFxSpriteParams.extent_y = 0x10;
                    {
                        int kind = g_FieldFxSpriteParams.palette;
                        int palette = g_FieldFxPaletteTable[kind];
                        if (kind == 4 && g_FieldFxAltPaletteFlag) palette += 4;
                        FieldFx_DrawPulledSprite(&position, 0x1000, 0x1000, 0x48, GetClut(0x30, palette), 1,
                                      state->fade, 0, 0xC00);
                    }
                    g_FieldFxSpriteParams.parameter00 = 0x40;
                    g_FieldFxSpriteParams.parameter02 = 4;
                    g_FieldFxSpriteParams.extent_x = 0x40;
                    g_FieldFxSpriteParams.extent_y = 0x40;
                    {
                        int kind = g_FieldFxSpriteParams.palette;
                        int palette = g_FieldFxPaletteTable[kind];
                        if (kind == 4 && g_FieldFxAltPaletteFlag) palette += 4;
                        FieldFx_DrawPulledSprite(&position, 0x1000, 0x1000, 0x49, GetClut(0x40, palette), 1,
                                      state->fade, 0, 0xE00);
                    }
                }
            }
        }
        {
            u16 *index = &g_FieldFxTpageSlot3;
            u16 *tpages = g_FieldFxTpageTable;
            g_FieldFxSpriteParams.tpage = g_FieldFxTpageTable[*index];
            g_FieldFxSpriteParams.palette = 3;
            g_FieldFxSpriteParams.parameter06 = 0;
            if (g_FieldFxCurrentAge < 0x11) {
                RenderMatrixSlot *slot;
                rotation.x = 0x800;
                rotation.y = 0;
                rotation.z = (s32)((u32)g_FieldFxCurrentAge * 16u);
                RotMatrixYXZ(&rotation, &spin);
                memcpy(&matrix,g_FieldFxOwner->actor->render_object.matrices,sizeof(matrix));
                MulMatrix0(&matrix, &spin, &matrix);
                scale.x = scale.y = rsin((s32)((u32)g_FieldFxCurrentAge * 64u)) / 4 + 0x400;
                scale.z = 0x1000;
                ScaleMatrix(&matrix, &scale);
                matrix.t[0] = state->position.x;
                matrix.t[1] = state->position.y;
                matrix.t[2] = state->position.z;
                slot = &g_ViewMatrixSlot;
                gte_ldrotmatrix(slot->value);
                gte_ldtransmatrix(slot->value);
                g_FieldFxSpriteParams.tpage = tpages[*index];
                g_FieldFxSpriteParams.palette = 3;
                g_FieldFxSpriteParams.parameter06 = 0;
                page = (u16)(tpages[*index] | GetTPage(0, 1, 0, 0));
                {
                    int kind = g_FieldFxSpriteParams.palette;
                    int palette = g_FieldFxPaletteTable[kind];
                    if (kind == 4 && g_FieldFxAltPaletteFlag) palette += 4;
                    FieldFx_ModelSetTexture(page, GetClut(0x60, palette));
                }
                FieldFx_ModelSetSemiTrans(1);
                FieldFx_ModelSaveColors((char *)g_RoomM123BeaconModel1);
                FieldFx_ModelSetColors((char *)g_RoomM123BeaconModel1, color.r, color.g, color.b);
                size = rcos((s32)((u32)g_FieldFxCurrentAge * 64u)) / 64;
                FieldFx_ModelScaleColors((char *)g_RoomM123BeaconModel1, size);
                FieldFx_ModelDraw((union FieldTexturedModel *)g_RoomM123BeaconModel1, &matrix);
                FieldFx_ModelRestoreColors((char *)g_RoomM123BeaconModel1);
            }
        }
        rotation.x = 0x800;
        rotation.y = 0;
        rotation.z = (s32)((u32)g_FieldFxCurrentAge * 128u);
        RotMatrixYXZ(&rotation, &spin);
        memcpy(&matrix,g_FieldFxOwner->actor->render_object.matrices,sizeof(matrix));
        MulMatrix0(&matrix, &spin, &matrix);
        scale.x = scale.y = state->height * 2 / 5;
        scale.z = 0x1800;
        ScaleMatrix(&matrix, &scale);
        matrix.t[0] = state->position.x;
        matrix.t[1] = state->position.y;
        matrix.t[2] = state->position.z;
        {
            RenderMatrixSlot *slot = &g_ViewMatrixSlot;
            gte_ldrotmatrix(slot->value);
            gte_ldtransmatrix(slot->value);
        }
        {
            u16 *index = &g_FieldFxTpageSlot3;
            g_FieldFxSpriteParams.tpage = g_FieldFxTpageTable[*index];
            g_FieldFxSpriteParams.palette = 3;
            g_FieldFxSpriteParams.parameter06 = 0;
            page = (u16)(g_FieldFxTpageTable[*index] | GetTPage(0, 3, 0, 0));
        }
        {
            int kind = g_FieldFxSpriteParams.palette;
            int palette = g_FieldFxPaletteTable[kind];
            if (kind == 4 && g_FieldFxAltPaletteFlag) palette += 4;
            FieldFx_ModelSetTexture(page, GetClut(0x60, palette));
        }
        FieldFx_ModelSetSemiTrans(1);
        FieldFx_ModelSaveColors((char *)g_RoomM123BeaconModel0);
        FieldFx_ModelSetColors((char *)g_RoomM123BeaconModel0, color.r, color.g, color.b);
        FieldFx_ModelDraw((union FieldTexturedModel *)g_RoomM123BeaconModel0, &matrix);
        FieldFx_ModelRestoreColors((char *)g_RoomM123BeaconModel0);
        FieldFx_ModelSaveColors((char *)g_RoomM123BeaconModel0);
        scale.z = 0x1000;
        scale.x = scale.y = state->depth;
        ScaleMatrix(&matrix, &scale);
        {
            u16 *index = &g_FieldFxTpageSlot3;
            g_FieldFxSpriteParams.tpage = g_FieldFxTpageTable[*index];
            g_FieldFxSpriteParams.palette = 3;
            g_FieldFxSpriteParams.parameter06 = 0;
            page = (u16)(g_FieldFxTpageTable[*index] | GetTPage(0, 3, 0, 0));
        }
        {
            int kind = g_FieldFxSpriteParams.palette;
            int palette = g_FieldFxPaletteTable[kind];
            if (kind == 4 && g_FieldFxAltPaletteFlag) palette += 4;
            FieldFx_ModelSetTexture(page, GetClut(0x60, palette));
        }
        FieldFx_ModelSetSemiTrans(1);
        FieldFx_ModelSetColors((char *)g_RoomM123BeaconModel0, color.r, color.g, color.b);
        FieldFx_ModelDraw((union FieldTexturedModel *)g_RoomM123BeaconModel0, &matrix);
        FieldFx_ModelRestoreColors((char *)g_RoomM123BeaconModel0);
        FieldFx_ModelSaveColors((char *)g_RoomM123BeaconModel0);
        scale.z = 0x1000;
        scale.x = scale.y = state->width;
        ScaleMatrix(&matrix, &scale);
        {
            u16 *index = &g_FieldFxTpageSlot3;
            g_FieldFxSpriteParams.tpage = g_FieldFxTpageTable[*index];
            g_FieldFxSpriteParams.palette = 3;
            g_FieldFxSpriteParams.parameter06 = 0;
            page = (u16)(g_FieldFxTpageTable[*index] | GetTPage(0, 1, 0, 0));
        }
        {
            int kind = g_FieldFxSpriteParams.palette;
            int palette = g_FieldFxPaletteTable[kind];
            if (kind == 4 && g_FieldFxAltPaletteFlag) palette += 4;
            FieldFx_ModelSetTexture(page, GetClut(0x60, palette));
        }
        FieldFx_ModelSetSemiTrans(1);
        FieldFx_ModelSetColors((char *)g_RoomM123BeaconModel0, color.r, color.g, color.b);
        FieldFx_ModelDraw((union FieldTexturedModel *)g_RoomM123BeaconModel0, &matrix);
        FieldFx_ModelRestoreColors((char *)g_RoomM123BeaconModel0);
        break;
    }
    return 0;
}

int RoomM123_JointSwirlParticleCallback(int mode, void *data, intptr_t argument) {
    RoomM123Particle *particle=data;
    GteShortVector position;
    int textureFrame, scale, kind, palette;
    u16 clut;

    switch (mode) {
    case 1: {
        int frame = particle->frame;
        int speed = g_RoomM123SwirlSpeed;
        int shift = speed * 24;

        frame++;
        particle->frame = frame;
        particle->offset += shift / 4096;
        if ((s16)particle->frame >= 24) return 1;
    } break;
    case 2: {
        int sampleFrame;

        scale = g_RoomM123SwirlSpeed * particle->scale;
        sampleFrame = (s16)particle->frame;
        FieldFx_OffsetPointByAngles((GteShortVector *)g_RoomM123SwirlOrigin, &position, (s32)((u32)sampleFrame * 64u), (GteShortVector *)g_RoomM123SwirlAngles,
                      scale / 4096, (s16)particle->offset);
        kind = g_FieldFxSpriteParams.palette;
        palette = g_FieldFxPaletteTable[kind];
        if (kind == 4 && g_FieldFxAltPaletteFlag) palette += 4;
        clut = GetClut(0x20, palette);
        textureFrame = (s16)g_FieldFxSpriteParams.parameter02 * (particle->frame & 7) + 0x40;
        FieldFx_DrawShapeQuads(&position, 0, 0x555, 0x555, textureFrame, clut, 1,
                      g_RoomM123SwirlBrightness, 0);
    } break;
    }
    return 0;
}

int RoomM123_JointSwirl(int mode, void *data, intptr_t argument) {
    GteShortVector seed = g_RoomM123JointAnchor;
    GteShortVector output;
    RoomM123Particle *particle;
    int i;
    int time;
    int scale;
    u16 outX, outY, outZ;
    struct RoomFxTransformOwner *context;
    void *texture;
    u16 palette;

    switch (mode) {
    case 0:
        return FieldFx_InitCallbackList(g_FieldFxActiveTaskSlot->end, 8, 24,
                             RoomM123_JointSwirlParticleCallback);
    case 1:
        if (g_FieldFxCurrentAge == 1) {
            for (i = 0; i < 24; i++) {
                particle = FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
                if (particle != 0) {
                    particle->frame = (i << 6) / 24;
                    particle->scale = (rand() & 63) + 64;
                    particle->offset = rand();
                }
            }
        } else {
            particle = FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
            if (particle != 0) {
                particle->frame = 0;
                particle->scale = (rand() & 63) + 64;
                particle->offset = rand();
            }
        }
        time = g_FieldFxCurrentAge;
        g_RoomM123SwirlSpeed = 0x1000;
        if (time >= 50)
            g_RoomM123SwirlSpeed = 0x1000 - ((s32)(((u32)time - 50u) * 4096u)) / 30;
        if (time < 80) break;
        return 1;
    case 2:
        FieldFx_TransformMatrixPoint((struct RoomFxTransformOwner *)g_FieldFxOwner->actor, 23, &seed, &output);
        scale = rsin(((s32)((u32)g_FieldFxCurrentAge * 2048u)) / 80);
        if (scale < 0)
            scale += 31;
        outX = output.x;
        outY = output.y;
        outZ = output.z;
        context = (struct RoomFxTransformOwner *)g_FieldFxOwner->actor;
        texture = g_RoomM123SwirlAngles;
        g_RoomM123SwirlOrigin[0] = outX;
        g_RoomM123SwirlOrigin[1] = outY;
        g_RoomM123SwirlOrigin[2] = outZ;
        g_RoomM123SwirlBrightness = scale >> 5;
        FieldFx_GetTransformLookAngles(context, 0, texture);
        g_FieldFxSpriteParams.parameter00 = 16;
        g_FieldFxSpriteParams.parameter02 = 1;
        g_FieldFxSpriteParams.extent_x = 16;
        g_FieldFxSpriteParams.extent_y = 16;
        palette = g_FieldFxTpageTable[g_FieldFxTpageSlot3];
        g_FieldFxSpriteParams.palette = 3;
        g_FieldFxSpriteParams.parameter06 = 0;
        g_FieldFxSpriteParams.parameter0A = 0;
        g_FieldFxSpriteParams.depth = 12;
        g_FieldFxSpriteParams.tpage = palette;
        break;
    }
    return 0;
}

int RoomM123_PulseRingCallback(int mode, void *data, intptr_t argument)
{
    RoomPulseParticle *particle=data;
    void **reference=(void *)argument;
    int color[2];
    GteShortVector position = g_RoomM123PulseRingTilt;
    GteShortVector *positionPtr;
    GteMatrix **matrixSlot;
    int wave;

    positionPtr = &position;
    switch (mode) {
    case 1:
        if (particle->state != 0)
            break;
        particle->frame++;
        if (particle->frame >= 24)
            return 1;
        break;
    case 2:
        if (particle->state != 0)
            break;
        matrixSlot = &g_ViewMatrixSlot.value;
        gte_ldrotmatrix(*matrixSlot);
        gte_ldtransmatrix(*matrixSlot);
        FieldFx_SampleColorTrack(*reference, color, particle->frame);
        wave = rsin(((s32)((u32)g_FieldFxCurrentAge * 1024u)) / 24);
        FieldFx_DrawRingBand((GteShortVector *)particle, 1000, 1500, 24, (GteRotation *)positionPtr, wave, wave, 0, (struct RenderColor *)color, 128, 1);
        break;
    }
    return 0;
}

int RoomM123_PulseRing(int mode, void *data, intptr_t argument) {
    RoomPulseParticle *particle=data;
    RoomPulseParticle *next;
    FieldActor *pool;
    FieldActorState *object;
    RoomSoundSlot *soundSlot;
    int time;
    int volume;
    u16 palette;

    switch (mode) {
    case 0:
        FieldFx_GetActorPosition((char *)g_FieldFxOwner->actor, 1, (s16 *)&particle->z);
        particle->x = 0;
        particle->y = 0;
        if (g_FieldFxTaskContext->flags) {
            pool = g_FieldFxOwner->actor;
            if (pool != 0 && pool->state != 0) {
                object = pool->state;
                if (*object->action_state == 1)
                    *object->action_state = 2;
            }
        }
        soundSlot = (RoomSoundSlot *)g_LoadedSceneAssetBlock;
        if (soundSlot->channel != 0) {
            volume = 0x7F;
            time = FieldFx_GetOwnerProgress();
            Akao_SendTableCommand(soundSlot->channel, 0x582, time, 0x80, volume);
            if (soundSlot->channel != 0)
                Akao_SendTableCommand(soundSlot->channel, 0x5B1, 0x80, 0x80, volume);
        }
        return FieldFx_InitCallbackList(g_FieldFxActiveTaskSlot->end, 12, 8,
                             RoomM123_PulseRingCallback);
    case 1:
        if (g_FieldFxCurrentAge < 32 && g_FieldFxCurrentAge % 6 == 0) {
            next = FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
            if (next != 0) {
                next->x = particle->z;
                next->y = particle->pad;
                next->z = particle->state;
                next->y = g_RoomFloorY->raw;
                next->state = 0;
                next->frame = 0;
            }
        }
        if (g_FieldFxCurrentAge == 32 && g_FieldFxTaskContext->flags &&
            g_FieldFxOwner->actor && g_FieldFxOwner->actor->state) {
            object = g_FieldFxOwner->actor->state;
            if ((object->core_flags & 0x3F000000) == 0x01000000) {
                ((Combatant *)g_PlayerEntity->state)->stateFlags |= 0x4000;
                object->core_flags = (object->core_flags & 0xC0FFFFFF) | 0x21000000;
                object->core_flags |= 0x80000000;
            }
        }
        if (g_FieldFxCurrentAge < 8) break;
        return 2;
    case 2:
        g_FieldFxSpriteParams.parameter00 = 32;
        g_FieldFxSpriteParams.parameter02 = 2;
        g_FieldFxSpriteParams.extent_x = 32;
        g_FieldFxSpriteParams.extent_y = 32;
        palette = g_FieldFxTpageTable[g_FieldFxTpageSlot3];
        g_FieldFxSpriteParams.palette = 3;
        g_FieldFxSpriteParams.parameter06 = 0;
        g_FieldFxSpriteParams.parameter0A = 0;
        g_FieldFxSpriteParams.depth = 0;
        g_FieldFxSpriteParams.tpage = palette;
        break;
    }
    return 0;
}

int RoomM123_WaveCallback(int mode, void *data, intptr_t argument) {
    RoomM123Wave *wave=data;
    int color[2];
    GteShortVector position = g_RoomM123WaveSpinBase;
    int result, scale, kind, palette;
    u16 clut;
    switch (mode) {
    case 1:
        if (wave->state != 0) break;
        wave->frame++;
        wave->y -= 3;
        if ((s16)wave->frame >= 24) return 1;
        break;
    case 2:
        if (wave->state != 0) break;
        FieldFx_SampleColorTrack(g_RoomM123WaveColorTrack, color, (s16)wave->frame);
        result = rcos(((s32)((u32)g_FieldFxCurrentAge * 1024u)) / 24);
        position.z = (s32)((u32)g_FieldFxCurrentAge * 32u);
        scale = result / 2 + 2048;
        kind = g_FieldFxSpriteParams.palette;
        palette = g_FieldFxPaletteTable[kind];
        if (kind == 4 && g_FieldFxAltPaletteFlag) palette += 4;
        clut = GetClut(48, palette);
        FieldFx_DrawShapeQuads((GteShortVector *)wave, (GteRotation *)&position, scale, scale,
                      (s16)g_FieldFxSpriteParams.parameter02 + 220, clut, 1, 128, (struct RenderColor *)color);
        break;
    }
    return 0;
}

int RoomM123_WaveBurst(int mode, void *data, intptr_t argument) {
    RoomM123Burst *burst=data;
    int *choice=(void *)argument;
    GteShortVector position = g_RoomM123JointOffset;
    RoomM123Wave *child;
    int value;

    switch (mode) {
    case 0:
        value = *choice ? 34 : 40;
        FieldFx_TransformMatrixPoint((struct RoomFxTransformOwner *)g_FieldFxOwner->actor, value, &position, (GteShortVector *)&burst->x);
        burst->frame = 0;
        burst->phase = 0;
        FieldFx_PlaySoundAtOwner(0x581, FieldFx_GetOwnerProgress());
        return FieldFx_InitCallbackList(g_FieldFxActiveTaskSlot->end, 12, 24,
                             RoomM123_WaveCallback);
    case 1:
        if (g_FieldFxCurrentAge < 32) {
            child = FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
            if (child != 0) {
                child->x = burst->x;
                child->y = burst->y;
                child->z = burst->z;
                {
                    int sample = rand();
                    int previous = child->x;
                    child->x = (previous - 128) + (sample & 255);
                }
                {
                    int sample = rand();
                    int previous = child->y;
                    child->y = (previous - 128) + (sample & 255);
                }
                {
                    int sample = rand();
                    int previous = child->z;
                    child->z = (previous - 128) + (sample & 255);
                }
                child->state = 0;
                child->frame = 0;
            }
        }
        if (g_FieldFxCurrentAge >= 8) return 2;
        break;
    case 2: {
        int paletteIndex = g_FieldFxTpageSlot2;
        int palette;
        g_FieldFxSpriteParams.parameter00 = 16;
        g_FieldFxSpriteParams.parameter02 = 1;
        g_FieldFxSpriteParams.extent_x = 16;
        g_FieldFxSpriteParams.extent_y = 16;
        palette = g_FieldFxTpageTable[paletteIndex];
        g_FieldFxSpriteParams.palette = 2;
        g_FieldFxSpriteParams.parameter06 = 0;
        g_FieldFxSpriteParams.parameter0A = 0;
        g_FieldFxSpriteParams.depth = 0;
        g_FieldFxSpriteParams.tpage = palette;
        break;
    }
    }
    return 0;
}

/* Original M123 task tables, scripts, constants and work data. */
GteShortVector g_RoomM123JointOffset={0,0,-110,0};
GteShortVector g_RoomM123JointOrigin={0,0,0,0};
GteShortVector g_RoomM123JointAnchor={0,0,-60,0};
GteShortVector g_RoomM123PulseRingTilt={1024,0,0,1};
GteShortVector g_RoomM123WaveSpinBase={0,0,0,0};
RenderColor g_RoomM123TriangleColor={160,70,0,0};
RoomM123BeaconOffset g_RoomM123BeaconAnchor={{0,65466}};
u8 g_RoomM123GlowColorTrack[40]={0,0,0,0,0,0,0,0,128,0,0,16,0,0,0,0,200,128,0,16,0,0,0,0,240,240,0,16,0,0,0,0,240,240,128,0,0,0,0,0};
u8 g_RoomM123GlowBurstColorTrack[40]={0,0,0,0,0,0,0,0,0,0,0,16,0,0,0,0,200,128,0,16,0,0,0,0,240,240,0,16,0,0,0,0,240,240,128,0,0,0,0,0};
u8 g_RoomM123BeaconColorTrack[48]={0,0,0,0,0,0,0,0,255,250,200,8,0,0,0,0,240,240,0,8,0,0,0,0,0,200,250,16,0,0,0,0,0,128,200,16,0,0,0,0,0,0,0,0,0,0,0,0};
char g_RoomM123PulseColorTrack0[32]={0,0,0,0,0,0,0,0,-66,110,30,12,0,0,0,0,70,0,-128,12,0,0,0,0,0,0,0,0,0,0,0,0};
char g_RoomM123PulseColorTrack1[32]={0,0,0,0,0,0,0,0,70,120,-116,12,0,0,0,0,20,0,110,12,0,0,0,0,0,0,0,0,0,0,0,0};
char g_RoomM123WaveColorTrack[40]={0,0,0,0,0,0,0,0,0,0,-128,4,0,0,0,0,0,80,-66,8,0,0,0,0,-128,80,0,12,0,0,0,0,0,0,0,0,0,0,0,0};
u16 g_RoomM123JointScript[24]={5,0,0,1,6,43,2,0,0,2,1,0,4,34,0,2,2,0,2,3,0,0,0,0};
u16 g_RoomM123PulseRingScript[16]={5,0,0,1,6,58,4,16,0,2,0,0,0,0,0,0};
u16 g_RoomM123WaveScript[12]={1,6,59,4,16,0,2,0,0,0,0,0};
RoomM123JointAngles g_RoomM123GlowBurstAngles;
GteShortVector g_RoomM123GlowBurstCenter;
u8 *g_RoomM123BeaconModel0,*g_RoomM123BeaconModel1;
s16 g_RoomM123SwirlBrightness,g_RoomM123SwirlSpeed;
_Alignas(GteShortVector) u8 g_RoomM123SwirlAngles[8];
u16 g_RoomM123SwirlOrigin[3];
void *g_RoomM123SelectedColorTrack;
intptr_t RoomM123_SelectSlotPointer(int, intptr_t, intptr_t, intptr_t);
FieldFxTaskProgram g_RoomM123JointEffects={
    {{RoomM123_JointTriangleEmitter,RoomM123_GlowBurst,RoomM123_JointBeacon,RoomM123_JointSwirl},{4,4,52,4}},
    RoomLib_SetTaskVar0,g_RoomM123JointScript};
FieldFxTaskProgram g_RoomM123PulseRingEffects={
    {{RoomM123_PulseRing},{12}},RoomM123_SelectSlotPointer,g_RoomM123PulseRingScript};
FieldFxTaskProgram g_RoomM123WaveBurstEffects={
    {{RoomM123_WaveBurst},{12}},RoomLib_SetArg1,g_RoomM123WaveScript};
