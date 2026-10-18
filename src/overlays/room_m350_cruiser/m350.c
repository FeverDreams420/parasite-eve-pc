#include "common.h"
#include "pe1/asset_find.h"
#include "pe1/field_actor.h"
#include "pe1/field_model_draw.h"
#include "pe1/field_sprite_draw.h"
#include "pe1/field_sprite_params.h"
#include "pe1/field_texture_tables.h"
#include "pe1/field_vector_math.h"
#include "pe1/fieldfx_callback_list.h"
#include "pe1/fieldfx_task_pool.h"
#include "pe1/gte.h"
#include "pe1/gte_sine_table.h"
#include "pe1/gte_types.h"
#include "pe1/player_entity.h"
#include "pe1/psyq_gpu.h"
#include "pe1/psyq_libc.h"
#include "pe1/render_object.h"
#include "pe1/room_floor.h"
#include "pe1/room_m350_effects.h"
#include "pe1/room_m350_sweep_trap.h"
#include "pe1/scene_archive.h"
#include "pe1/scene_assets.h"

#include "m350_externs.h"
#include "room_m350_data.h"
#include "room_m350_shared.h"

typedef GteShortVector Particle;

typedef struct { Vector rotation; Vector *position; int color; } PulsingPalettePairParticle;

typedef struct { short delay, count; } State;

typedef struct { Vector *position; short scale, reserved; } RampedPalettePairParticle;

typedef struct { short target, current; } RampedPalettePairState;

typedef RoomM350EffectInstance Instance;

typedef RoomM350EffectActor Actor;

typedef union RoomM350Angles {
    GteShortVector angles;
    GteRotation rotation;
} RoomM350Angles;

typedef union RoomM350Step {
    GteShortVector vector;
    s32 words[2];
} RoomM350Step;

typedef RoomM350SoundOwner RoomM350ControllerOwner;

typedef RoomM350TransformMatrix Transform;

typedef FieldActor SweepTrapFlowInstance;

typedef RoomM350Actor RoomM350ControllerChannel;

typedef struct { short x,y,z,frame; } FallingTexturePairParticle;

typedef struct { Vector *position; } TripleScalePairParticle;

typedef FieldActorState Owner;

typedef struct { int color; short x, y, z, size; } ModelShellLayersParticle;

typedef RoomM350TableShadeEntry TableEntry;

typedef FieldActor Animation;
typedef FieldFxObjectPrefix AnimationBurstActor;

typedef struct { short remaining, delay; } AnimationBurstState;

typedef struct { Vector *position; short offset,reserved; } PointerQueueParticle;

typedef RoomM350PointerQueue Queue;

typedef struct { Vector position; short delay,reserved; } PositionQueueParticle;

typedef struct { short position[3]; short size; } TableTransformParticle;

typedef GteVector Scale;

typedef struct { void *asset; int yaw; short size,brightness,unknown,turn; unsigned char initialized; } TurningAreaControllerState;

typedef struct { short x, y, z, velocityY, gravity, reserved; } DualBallisticParticle;

int RoomM350_HandSparkEmitter(int event, void *_payload, intptr_t argument) {
    Vector offset;
    Matrix matrix;
    Vector vector;
    switch (event) {
    case 0:

        return FieldFx_InitCallbackList(g_FieldFxActiveTaskSlot->end,8,64,RoomM350_HandSparkCallback);
        case 1:
    {
        int timer=g_FieldFxCurrentAge;
        int count;
        int i;
        Vector *out;
        if(timer>=33) return 2;
        count=(rcos((((((unsigned int)timer<<7)+2048)&4095)*2) / 2)+4096)/2048+1;
        if(count*2<=0) return 0;
        i=0;
        out=&offset;
        do {
            int random;
            int highByte;
            int divisor;
            Vector *base;
            Particle *particle=FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
            if(!particle) return 0;
            vector.x=(unsigned int)Inv_ScrambleGrid()<<4;
            vector.y=(unsigned int)Inv_ScrambleGrid()<<4;
            vector.z=0;
            RotMatrixYXZ(&vector,&matrix);
offset_start:
            vector.x=0; vector.y=0;
            highByte=Inv_ScrambleGrid();
            random=((unsigned int)highByte<<8)|(unsigned int)Inv_ScrambleGrid();
            divisor=192;
            vector.z=random%divisor+64;
            ApplyMatrixSV(&matrix,&vector,out);
            base=&g_RoomM350HandPoints[i&1];
            particle->x=(unsigned short)offset.x+(unsigned short)base->x;
            particle->y=(unsigned short)offset.y+(unsigned short)base->y;
            particle->z=(unsigned short)offset.z+(unsigned short)base->z;
            i++;
        } while(i<count*2);
    }
        break;
    case 2:

    {
        int index=g_FieldFxTpageSlot2;
        int palette;
        g_FieldFxSpriteParams.parameter00=16; g_FieldFxSpriteParams.parameter02=1;
        g_FieldFxSpriteParams.extent_x=16; g_FieldFxSpriteParams.extent_y=16;
        g_FieldFxSpriteParams.extent_x=16; g_FieldFxSpriteParams.extent_y=16;
        palette=g_FieldFxTpageTable[index];
        g_FieldFxSpriteParams.palette=2; g_FieldFxSpriteParams.parameter06=0;
        g_FieldFxSpriteParams.parameter0A=0; g_FieldFxSpriteParams.depth=0; g_FieldFxSpriteParams.tpage=palette;
    }
    }
    return 0;
}

/* The spawner owns the particles rendered by this callback. */

int RoomM350_HandPulseCallback(int event, void *_payload, intptr_t argument) {
    PulsingPalettePairParticle *effect = _payload;
    if (event == 1) {
        if (g_FieldFxCurrentAge >= 16) return 1;
    } else if (event == 2) {
        int scale = rcos((((unsigned int)(g_FieldFxCurrentAge - 1) << 8) & 0x1F00) / 2) * 4 + 2048;
        int kind = g_FieldFxSpriteParams.palette;
        int palette = g_FieldFxPaletteTable[kind];
        int handle;
        if (kind == 4 && g_FieldFxAltPaletteFlag) palette += 4;
        handle = GetClut(48, palette);
        FieldFx_DrawShapeQuads(effect->position, (struct GteRotation *)effect, scale, scale, 110,
            (unsigned short)handle, 1,
            (s16)rsin(((unsigned int)(g_FieldFxCurrentAge - 1) << 7) & 0xF80) >> 5,
            (struct RenderColor *)&effect->color);
    }
    return 0;
}

int RoomM350_HandPulseEmitter(int event, void *_payload, intptr_t argument) {
    State *state = _payload;
    switch (event) {
    case 0:

        state->delay = 0;
        state->count = 0;
        return FieldFx_InitCallbackList(g_FieldFxActiveTaskSlot->end,sizeof(PulsingPalettePairParticle),18,RoomM350_HandPulseCallback);
        case 1:
    {
        int i;
        if (g_FieldFxCurrentAge >= 17) return 2;
        for (i = 0; i < 2; i++) {
            PulsingPalettePairParticle *particle = FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
            if (!particle) break;
            particle->rotation.x = (unsigned int)Inv_ScrambleGrid() << 4;
            particle->rotation.y = (unsigned int)Inv_ScrambleGrid() << 4;
            particle->rotation.pad = 1;
            particle->rotation.z = 0;
            particle->position = &g_RoomM350HandPoints[i];
            particle->color = g_RoomM350HandPulseColors[(state->count + i) & 7];
        }
        {
            int count = (unsigned short)state->count;
            state->count = count + 1;
            state->delay = 4 - count;
            if (state->delay <= 0) state->delay = 1;
        }
    }
        break;
    case 2:

    {
        int index = g_FieldFxTpageSlot3;
        int palette;
        g_FieldFxSpriteParams.parameter00 = 32; g_FieldFxSpriteParams.parameter02 = 2;
        g_FieldFxSpriteParams.extent_x = 32; g_FieldFxSpriteParams.extent_y = 32;
        g_FieldFxSpriteParams.extent_x = 32; g_FieldFxSpriteParams.extent_y = 32;
        palette = g_FieldFxTpageTable[index];
        g_FieldFxSpriteParams.palette = 3; g_FieldFxSpriteParams.parameter06 = 0;
        g_FieldFxSpriteParams.parameter0A = 0; g_FieldFxSpriteParams.depth = 0; g_FieldFxSpriteParams.tpage = palette;
    }
    }
    return 0;
}

/* The spawner owns the scaled particles rendered by this callback. */

int RoomM350_HandGlowCallback(int event, void *_payload, intptr_t argument) {
    RampedPalettePairParticle *object = _payload;
    if (event == 1) {
        if (g_FieldFxCurrentAge >= 16) {
            return 1;
        }
    } else if (event == 2) {
        int size = rsin(((unsigned int)(g_FieldFxCurrentAge - 1) << 6) & 0xFC0) * object->scale / 4096;
        int kind = g_FieldFxSpriteParams.palette;
        int palette = g_FieldFxPaletteTable[kind];
        int handle;

        if (kind == 4 && g_FieldFxAltPaletteFlag) {
            palette += 4;
        }
        handle = GetClut(32, palette);
        FieldFx_DrawShapeQuads(object->position, 0, size, size, (s16)g_FieldFxSpriteParams.parameter02 + 216,
            (unsigned short)handle, 1,
            (s16)rsin(((unsigned int)(g_FieldFxCurrentAge - 1) << 7) & 0xF80) >> 5,
            &g_RoomM350GlowSpriteColor);
    }
    return 0;
}

int RoomM350_HandGlowEmitter(int event, void *_payload, intptr_t argument) {
    RampedPalettePairState *state = _payload;
    switch (event) {
    case 0:

        state->target = 16384;
        state->current = 0;
        return FieldFx_InitCallbackList(g_FieldFxActiveTaskSlot->end,sizeof(RampedPalettePairParticle),6,RoomM350_HandGlowCallback);
        case 1:
    if (!state->target) {
        if (state->current <= 0) return 2;
    } else if (g_FieldFxOwner->actor->anim.parts.integer >= 41) state->target = 0;
    if (g_FieldFxCurrentAge & 7) return 0;
    if (state->current < state->target) state->current += 4096;
    else if (state->target < state->current) state->current -= 4096;
    if (state->current) {
        int i;
        for (i = 0; i < 2; i++) {
            RampedPalettePairParticle *particle = FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
            if (!particle) return 0;
            particle->position = &g_RoomM350HandPoints[i];
            particle->scale = state->current;
        }
    }
        break;
    case 2:

    {
        int unit = 16;
        int palette;
                palette = g_FieldFxTpageTable[g_FieldFxTpageSlot2];
        g_FieldFxSpriteParams.palette = 2; g_FieldFxSpriteParams.parameter06 = 0;
        g_FieldFxSpriteParams.parameter00 = unit; g_FieldFxSpriteParams.parameter02 = 1;
        g_FieldFxSpriteParams.extent_x = unit; g_FieldFxSpriteParams.extent_y = unit;
        g_FieldFxSpriteParams.extent_x = unit; g_FieldFxSpriteParams.extent_y = unit;
        g_FieldFxSpriteParams.parameter0A = 0; g_FieldFxSpriteParams.depth = 0; g_FieldFxSpriteParams.tpage = palette;
    }
    }
    return 0;
}

/* Sweeping beam trap: sprites swept down a column from the actor's head,
 * a flare and two fan blades towards the anchor, then a projected quad test
 * that tags the player once. */
int RoomM350_HandSweepTrapCallback(int mode, void *_payload, intptr_t argument) {
    RoomM350SweepTrap *trap = _payload;
    GteShortVector position;
    RoomM350Step step;
    RoomM350Angles rotation;
    RoomM350Angles spin;
    GteMatrix matrix;
    GteShortVector quad[4];
    GteMatrix frame;
    int outside;
    int phase;
    int intensity;
    int dist;
    int i;
    int k;
    int c;
    int s;
    int dx;
    int dz;

    if (mode == 1) {
        if (g_FieldFxCurrentAge >= 0x10) return 1;
    } else if (mode == 2) {
        phase = g_FieldFxCurrentAge - 1;
        intensity = phase < 9;
        i = 8;
        if (intensity) i = phase;
        rotation.angles.x = -rsin((i * 128) & 0xFFF) * 0x260 / 4096 - 0x500;
        rotation.angles.y = g_FieldFxOwner->actor->rot_y;
        rotation.angles.z = 0x400;
        RotMatrixYXZ(&rotation.angles, &matrix);
        matrix.t[0] = 0;
        matrix.t[1] = 0;
        matrix.t[2] = 0;
        step.words[0] = 0;
        step.vector.z = 0x70;
        gte_ldrotmatrix(&matrix);
        gte_ldtransmatrix(&matrix);
        gte_ldv0(&step.vector);
        gte_rt();
        gte_stsv(&step.vector);
        rotation.angles.pad = 1;
        rotation.angles.x += 0x400;
        position = *trap->anchor;
        if (intensity) intensity = 0x80;
        else intensity = rcos(((phase - 8) * 128) & 0xFFF) >> 5;
        g_FieldFxSpriteParams.tpage = g_FieldFxTpageTable[g_FieldFxTpageSlot3];
        g_FieldFxSpriteParams.palette = 3;
        g_FieldFxSpriteParams.parameter06 = 0;
        for (i = phase; position.y < g_RoomFloorY->y; i++) {
            int kind = g_FieldFxSpriteParams.palette;
            int palette = g_FieldFxPaletteTable[kind];
            if (kind == 4 && g_FieldFxAltPaletteFlag) palette += 4;
            FieldFx_DrawShapeQuads(&position, &rotation.rotation, 0x800, 0x1800,
                          (s16)g_FieldFxSpriteParams.parameter02 * (i & 7) + 0x40,
                          GetClut(0, palette), 1, intensity, &g_RoomM350HandSweepTrapPulseColors[g_FieldFxCurrentAge & 1]);
            position.x += step.vector.x;
            position.y += step.vector.y;
            position.z += step.vector.z;
        }
        spin.angles.x = 0x400;
        spin.angles.y = 0;
        spin.angles.pad = 1;
        spin.angles.z = g_FieldFxCurrentAge << 7;
        g_FieldFxSpriteParams.tpage = g_FieldFxTpageTable[g_FieldFxTpageSlot11];
        g_FieldFxSpriteParams.palette = 3;
        g_FieldFxSpriteParams.parameter06 = 1;
        {
            int kind = g_FieldFxSpriteParams.palette;
            int palette = g_FieldFxPaletteTable[kind];
            FieldFx_DrawShapeQuads(&position, &spin.rotation, 0x2000, 0x2000,
                          (s16)g_FieldFxSpriteParams.parameter02 * 17 + 0x84,
                          GetClut(0, (kind == 4 && g_FieldFxAltPaletteFlag) ? palette + 8 : palette + 4),
                          1, intensity, &g_RoomM350HandSweepTrapPulseColors[g_FieldFxCurrentAge & 1]);
        }
        dx = position.x - trap->anchor->x;
        dz = position.z - trap->anchor->z;
        dist = Math_IntSqrt(dx * dx + dz * dz);
        rotation.angles.z = 0;
        rotation.angles.x = 0;
        rotation.angles.y -= 0x14;
        for (i = 0; i < 2; i++) {
            FieldFx_DrawRotatedTriangle(&position, &rotation.rotation, dist, 0x200, &g_RoomM350HandSweepTrapTriangleColor, &g_RoomM350BlackColor,
                          &g_RoomM350BlackColor, (s16)intensity, 1);
            rotation.angles.y += 0x28;
        }
        spin.angles.x = 0;
        spin.angles.y = 0;
        spin.angles.pad = 0;
        spin.angles.z = g_FieldFxCurrentAge << 7;
        {
            int scale = rsin((phase * 64) & 0xFFF) + 0x1000;
            FieldFx_DrawRingBand(trap->anchor, 0x100, 0x1C0, 0x10, &spin.rotation, scale, scale,
                          &g_RoomM350BlackColor, &g_RoomM350HandSweepTrapRingColor,
                      rcos(((g_FieldFxCurrentAge - 1) * 64) & 0xFFF) >> 5, 1);
        }
        if (trap->done) return 0;
        if (g_PlayerEntity->mode < 4) return 0;
        g_RoomM350HandSweepTrapCorners[2].z = g_RoomM350HandSweepTrapCorners[3].z = -dist;
        c = rcos(g_FieldFxOwner->actor->rot_y);
        s = rsin(g_FieldFxOwner->actor->rot_y);
        frame.m[0][2] = s;
        frame.m[2][0] = -s;
        frame.m[1][1] = 0x1000;
        frame.m[0][0] = c;
        frame.m[2][2] = c;
        frame.t[2] = 0;
        frame.t[1] = 0;
        frame.t[0] = 0;
        frame.m[2][1] = 0;
        frame.m[1][2] = 0;
        frame.m[1][0] = 0;
        frame.m[0][1] = 0;
        frame.t[0] = position.x;
        frame.t[1] = position.y;
        frame.t[2] = position.z;
        gte_ldrotmatrix(&frame);
        gte_ldtransmatrix(&frame);
        for (k = 0; k < 4; k++) {
            gte_ldv0(&g_RoomM350HandSweepTrapCorners[k]);
            gte_rt();
            gte_stsv(&quad[k]);
        }
        {
            int player = ((u32)g_PlayerEntity->render_object.model_matrix.translation[2] << 16) | (u16)g_PlayerEntity->render_object.model_matrix.translation[0];
            int prev = ((u32)quad[3].z << 16) | (u16)quad[3].x;
            k = 0;
            while (k < 4) {
                int cur = ((u32)quad[k].z << 16) | (u16)quad[k].x;
                gte_ldsxy0(player);
                gte_ldsxy2(prev);
                gte_ldsxy1(cur);
                gte_nclip();
                gte_stmac0(&outside);
                if (outside < 0) break;
                k++;
                prev = cur;
            }
        }
        if (k < 4) return 0;
        g_PlayerEntity->state->flags |= 0x4000;
        {
            RoomM350TrapNode *node = g_FieldFxOwner->actor->state;
            GteShortVector unused;
            if (node) node->core_flags |= 0x80000000;
        }
        trap->done = 1;
        g_RoomM350HandSweepHit = 1;
    }
    return 0;
}
int RoomM350_HandSweepTrapEmitter(int event, void *_payload, intptr_t argument) {
    switch (event) {
    case 0:

        return FieldFx_InitCallbackList(g_FieldFxActiveTaskSlot->end,sizeof(RoomM350SweepTrap),2,RoomM350_HandSweepTrapCallback);
        case 1:
    {
        SweepTrapFlowInstance *instance = g_FieldFxOwner->actor;
        int frame;
        int previous;
        int i;
        if (instance->mode != 11) return 0;
        frame = instance->anim.parts.integer;
        if (frame >= 50) return 2;
        previous = (u16)(instance->anim_prev >> 16);
        if (frame < 8) return 0;
        if (previous < 8) {
            for (i=0;i<2;i++) {
                RoomM350SweepTrap *particle = FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
                if (!particle) break;
                particle->anchor = &g_RoomM350HandPoints[i];
                particle->done = 0;
            }
            {
                SweepTrapFlowInstance *current = g_FieldFxOwner->actor;
                Asset_Find08w(0x5C6,current->state->progress,
                    (short)current->render_object.matrices->translation[0],
                    (short)current->render_object.matrices->translation[1],
                    (short)current->render_object.matrices->translation[2]);
            }
        }
    }
        break;
    case 2:

    g_FieldFxSpriteParams.parameter00=32;
    g_FieldFxSpriteParams.parameter02=2;
    g_FieldFxSpriteParams.extent_x=32;
    g_FieldFxSpriteParams.extent_y=32;
    g_FieldFxSpriteParams.extent_x=32;
    g_FieldFxSpriteParams.extent_y=32;
    g_FieldFxSpriteParams.parameter0A=0;
    g_FieldFxSpriteParams.depth=0;
    }
    return 0;
}

PE1_STATIC_ASSERT(sizeof(RoomM350SweepTrap) == 2*sizeof(void *), room_m350_sweep_trap_size);

/* The spawner owns the falling particles updated and rendered by this callback. */

int RoomM350_HandSweepHitCallback(int event, void *_payload, intptr_t argument) {
    short *position = _payload;
    if (event == 1) {
        if (g_FieldFxCurrentAge >= 16) return 1;
        position[1] -= position[3]++;
    } else if (event == 2) {
        int kind = g_FieldFxSpriteParams.palette;
        int texture = g_FieldFxPaletteTable[kind];
        int handle;
        handle = GetClut(0, (kind == 4 && g_FieldFxAltPaletteFlag != 0) ? texture + 6 : texture + 2);
        FieldFx_DrawShapeQuads((GteShortVector *)position, 0, 8192, 8192,
            (s16)g_FieldFxSpriteParams.parameter02 * (((g_FieldFxCurrentAge - 1) >> 1) & 7),
            (unsigned short)handle, 1,
            rcos(((unsigned int)(g_FieldFxCurrentAge - 1) << 6) & 0xFC0) >> 5,
            &g_RoomM350HandSweepHitColor);
    }
    return 0;
}

int RoomM350_HandSweepHitEmitter(int event, void *_payload, intptr_t argument) {
    switch (event) {
    case 0:

        return FieldFx_InitCallbackList(g_FieldFxActiveTaskSlot->end,8,4,RoomM350_HandSweepHitCallback);
        case 1:
    if(g_RoomM350HandSweepDone) return 2;
    if(g_RoomM350HandSweepHit) {
        FallingTexturePairParticle *particle=FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
        RoomM350TransformOwner *player;
        if(!particle) return 0;
        {
            player=(RoomM350TransformOwner *)g_PlayerEntity;
            particle->x=player->render_object.matrices->translation[0];
            particle->y=player->render_object.matrices->translation[1];
            particle->z=player->render_object.matrices->translation[2];
            particle->frame=0;
        }
        g_RoomM350HandSweepHit=0;
    }
        break;
    case 2:

    g_FieldFxSpriteParams.parameter00 = 32;
    g_FieldFxSpriteParams.parameter02 = 2;
    g_FieldFxSpriteParams.extent_x = 32;
    g_FieldFxSpriteParams.extent_y = 32;
    g_FieldFxSpriteParams.extent_x = 32;
    g_FieldFxSpriteParams.extent_y = 32;
    g_FieldFxSpriteParams.tpage = g_FieldFxTpageTable[g_FieldFxTpageSlots[11]];
    g_FieldFxSpriteParams.palette = 3;
    g_FieldFxSpriteParams.parameter06 = 1;
    g_FieldFxSpriteParams.parameter0A = 0;
    g_FieldFxSpriteParams.depth = 0;
    }
    return 0;
}

/* The controller owns the position pointers rendered by this callback. */

int RoomM350_HandFlashCallback(int event, void *_payload, intptr_t argument) {
    void **object = _payload;
    if (event == 1) {
        if (g_FieldFxCurrentAge >= 16) {
            return 1;
        }
    } else if (event == 2) {
        int sample = rsin(((unsigned int)(g_FieldFxCurrentAge - 1) << 6) & 0xFC0);
        int kind = g_FieldFxSpriteParams.palette;
        int size = sample * 3;
        int palette = g_FieldFxPaletteTable[kind];
        int handle;
        handle = GetClut(0, (kind == 4 && g_FieldFxAltPaletteFlag != 0) ? palette + 8 : palette + 4);
        FieldFx_DrawShapeQuads((GteShortVector *)*object, 0, size, size, (s16)g_FieldFxSpriteParams.parameter02 * 6 + 128,
            (unsigned short)handle, 1,
            (s16)rsin(((unsigned int)(g_FieldFxCurrentAge - 1) << 7) & 0xF80) >> 5, 0);
    }
    return 0;
}

int RoomM350_HandFlashEmitter(int event, void *_payload, intptr_t argument) {
    switch (event) {
    case 0:

        return FieldFx_InitCallbackList(g_FieldFxActiveTaskSlot->end,sizeof(void *),2,RoomM350_HandFlashCallback);
        case 1:
    {
        Instance *instance = g_FieldFxOwner->actor;
        int frame, previous, i;
        if (instance->mode != 11) return 0;
        frame = instance->anim.parts.integer;
        previous = (u16)(instance->anim_prev >> 16);
        if (frame >= 5 && previous < 5) {
            for (i=0;i<2;i++) {
                TripleScalePairParticle *particle = FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
                if (!particle) break;
                particle->position = &g_RoomM350HandPoints[i];
            }
            return 0;
        }
        if (frame >= 41) return 2;
    }
        break;
    case 2:

    g_FieldFxSpriteParams.parameter00 = 32;
    g_FieldFxSpriteParams.parameter02 = 2;
    g_FieldFxSpriteParams.extent_x = 32;
    g_FieldFxSpriteParams.extent_y = 32;
    g_FieldFxSpriteParams.extent_x = 32;
    g_FieldFxSpriteParams.extent_y = 32;
    g_FieldFxSpriteParams.tpage = g_FieldFxTpageTable[g_FieldFxTpageSlots[11]];
    g_FieldFxSpriteParams.palette = 3;
    g_FieldFxSpriteParams.parameter06 = 1;
    g_FieldFxSpriteParams.parameter0A = 0;
    g_FieldFxSpriteParams.depth = 0;
    }
    return 0;
}

int RoomM350_HandSweepController(int event, void *_payload, intptr_t argument) {
    FieldActor *instance=(FieldActor *)g_FieldFxOwner->actor;
    Vector offset;
    if(event==0) {
        Owner *owner=instance->state;
        unsigned int flags=owner->core_flags;
        owner->core_flags=flags|0x40000000;
        g_RoomM350HandSweepRepeats=3; g_RoomM350HandSweepDone=0; g_RoomM350HandSweepHit=0;
        return 0;
    }
    if(event==1) {
        short i;
        Vector *input;
        Vector *base;
        int frame;
        short *repeat;
        if(instance->state && instance->state->action_state[0]==1)
            instance->state->action_state[0]=2;
        i=0;
        input=&offset;
        base=g_RoomM350HandPoints;
        offset.x=-112; offset.y=0; offset.z=-80;
        for(;i<2;i++) {
            RenderMatrix *matrix=g_RoomM350HandActor->render_object.matrices;
            Vector *out;
            if(i) matrix+=15; else matrix+=11;
            {
                int x;
                int y;
                int z;
                const GteMatrixWords *words=(const GteMatrixWords *)matrix;
                /* Field-specific barriers preserve retail load scheduling around
                 * the individual GTE transfers; they are not hardware hazards. */
                x=words->r11_r12;
                y=words->r13_r21;
                gte_ctc2_0(x);
                gte_ctc2_1(y);
                x=words->r22_r23;
                y=words->r31_r32;
                z=words->r33_pad;
                gte_ctc2_2(x);
                gte_ctc2_3(y);
                gte_ctc2_4(z);
                x=words->tx;
                y=words->ty;
                gte_ctc2_5(x);
                z=words->tz;
                gte_ctc2_6(y);
                gte_ctc2_7(z);
            }
            gte_ldv0(input); gte_rtv0tr_mac();
            out=base+i;
            {
                int x;
                int y;
                int z;
                gte_mfc2_9(x); gte_mfc2_10(y); gte_mfc2_11(z);
                out->x=x; out->y=y; out->z=z;
            }
            offset.x=0u-(unsigned short)offset.x;
            g_RoomM350HandPoints[i].pad=i;
        }
        frame=instance->anim.parts.integer;
        if(instance->mode!=11) return 0;
        repeat=&g_RoomM350HandSweepRepeats;
        if(*repeat>0) {
            int speed,bias,angle;
            int signedFrame=(short)frame;
            if(signedFrame>=40) {
                instance->anim.fixed=0x30000;
                *repeat=*(volatile unsigned short *)repeat-1;
            }
            if(signedFrame<28) return 0;
            speed=rsin((((signedFrame-28)*170)&0xFFE))/32;
            bias=-128;
            if(*(unsigned short *)repeat&1) bias=128;
            angle=FieldFx_VecToAngle((Vec3Fixed *)g_PlayerEntity->render_object.model_matrix.translation,(Vec3Fixed *)instance->render_object.model_matrix.translation);
            instance->rot_y=FieldFx_TurnToward(instance->rot_y,(short)(bias+angle),speed);
        } else if((short)frame>=instance->action-1) {
            Owner *owner=instance->state;
            if(owner) owner->action_state[0]=4;
            g_RoomM350HandSweepDone=1;
            return 1;
        }
    }
    return 0;
}

int RoomM350_OrbHandSparkCallback(int event, void *_payload, intptr_t argument) {
    ModelShellLayersParticle *particle = _payload;
    char frameGap[8];
    if (event == 1) {
        if (g_FieldFxCurrentAge >= 8) return 1;
    } else if (event == 2) {
        int size = rsin((((unsigned int)g_FieldFxCurrentAge - 1) << 7) & 0xF80) * particle->size / 4096;
        int kind = g_FieldFxSpriteParams.palette;
        int palette = g_FieldFxPaletteTable[kind];
        unsigned short clut;
        if (kind == 4 && g_FieldFxAltPaletteFlag) palette += 4;
        clut = GetClut(48, palette);
        FieldFx_DrawShapeQuads((GteShortVector *)&particle->x, 0, size, size, (s16)g_FieldFxSpriteParams.parameter02 * 5 + 216, clut, 1,
            rcos((((unsigned int)g_FieldFxCurrentAge - 1) << 7) & 0xF80) >> 5, (struct RenderColor *)particle);
        FieldFx_DrawStarFan((GteShortVector *)&particle->x, 96, 96, 8, 0, size, size, (struct RenderColor *)g_RoomM350OrbHandSparkColor, &g_RoomM350BlackColor,
            (s16)rsin((((unsigned int)g_FieldFxCurrentAge - 1) << 8) & 0xF00) >> 5, 1);
    }
    return 0;
}

int RoomM350_OrbHandSparkEmitter(int event, void *_payload, intptr_t argument) {
    Vector offset;
    Matrix matrix;
    Vector vector;
    switch (event) {
    case 0:

        return FieldFx_InitCallbackList(g_FieldFxActiveTaskSlot->end,12,64,RoomM350_OrbHandSparkCallback);
        case 1:
    {
        int i = 0;
        Vector *out;
        if (g_FieldFxCurrentAge >= 41) return 2;
        out = &offset;
        for (; i < 2; i++) {
            int random;
            int highByte;
            int r, g, b, divisor;
            int j, *position;
            ModelShellLayersParticle *particle = FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
            if (!particle) return 0;
            vector.x = (unsigned int)Inv_ScrambleGrid() << 4;
            vector.y = (unsigned int)Inv_ScrambleGrid() << 4;
            vector.z = 0;
            RotMatrixYXZ(&vector, &matrix);
offset_start:
            vector.x = 0; vector.y = 0;
            highByte = Inv_ScrambleGrid();
            random = ((unsigned int)highByte << 8) | (unsigned int)Inv_ScrambleGrid();
            divisor = 192;
            vector.z = random % divisor + 128;
            ApplyMatrixSV(&matrix, &vector, out);
            position = g_RoomM350OrbActor->render_object.matrices[i ? 11 : 15].translation;
            for (j = 0; j < 3; j++) (&particle->x)[j] = ((short *)out)[j] + (unsigned int)position[j];
            particle->size = (unsigned int)(int)((unsigned int)(unsigned short)rsin(((int)((unsigned int)g_FieldFxCurrentAge << 11) / 40) & 4095) | ((unsigned int)(unsigned short)rcos(((int)((unsigned int)g_FieldFxCurrentAge << 11) / 40) & 4095) << 16)) + 4096;
            r = Inv_ScrambleGrid(); g = Inv_ScrambleGrid(); b = Inv_ScrambleGrid();
            particle->color = (r >> 1) | ((unsigned int)(g >> 1) << 8) | ((unsigned int)(b >> 1) << 16);
        }
    }
        break;
    case 2:

    {
        int index = g_FieldFxTpageSlot2;
        int palette;
        g_FieldFxSpriteParams.parameter00 = 16; g_FieldFxSpriteParams.parameter02 = 1;
        g_FieldFxSpriteParams.extent_x = 16; g_FieldFxSpriteParams.extent_y = 16;
        g_FieldFxSpriteParams.extent_x = 16; g_FieldFxSpriteParams.extent_y = 16;
        palette = g_FieldFxTpageTable[index];
        g_FieldFxSpriteParams.palette = 2; g_FieldFxSpriteParams.parameter06 = 0;
        g_FieldFxSpriteParams.parameter0A = 0; g_FieldFxSpriteParams.depth = 0; g_FieldFxSpriteParams.tpage = palette;
    }
    }
    return 0;
}

int RoomM350_OrbBurstCallback(int event, void *_payload, intptr_t argument) {
    void **object = _payload;
    short vector[4];
    if (event == 1) {
        if (g_FieldFxCurrentAge >= 8) {
            return 1;
        }
    } else if (event == 2) {
        int counter = g_FieldFxCurrentAge;
        int sample = ((unsigned int)(counter - 1) << 7) & 0xF80;
        int size = rsin(sample);

        vector[0] = 0;
        vector[1] = 0;
        vector[2] = counter * 128;
        vector[3] = 0;
        FieldFx_DrawStarFan(*object, 320, 320, 16, (struct GteRotation *)vector, size, size,
            (struct RenderColor *)g_RoomM350OrbBurstRingColorA, &g_RoomM350BlackColor, rcos(sample) >> 5, 1);
        {
            int nextCounter = g_FieldFxCurrentAge;
            size = nextCounter * 512 + 2048;
            FieldFx_DrawRingBand(*object, 256, 512, 16, (struct GteRotation *)vector, size, size,
                &g_RoomM350BlackColor, (struct RenderColor *)g_RoomM350OrbBurstRingColorB,
                rsin(((unsigned int)(nextCounter - 1) << 8) & 0xF00) >> 5, 1);
        }
    }
    return 0;
}

int RoomM350_OrbBurstEmitter(int event, void *_payload, intptr_t argument) {
    AnimationBurstState *state = _payload;
    switch (event) {
    case 0:
        state->remaining = 0;
        return FieldFx_InitCallbackList(g_FieldFxActiveTaskSlot->end,sizeof(void *),4,RoomM350_OrbBurstCallback);
    case 1:
        {
            Animation *animation = g_FieldFxOwner->actor;
            if (animation->mode == 13) {
                int frame = animation->anim.parts.integer;
                if (frame >= 26) {
                    return 2;
                }
                {
                    int previous = (u16)(animation->anim_prev >> 16);
                    if (frame >= 8 && previous < 8) {
                        state->remaining = 4;
                        state->delay = 0;
                    }
                }
            }
            if (state->remaining != 0 && state->delay-- <= 0) {
                short **effect = FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
                if (effect == 0) {
                    return 0;
                }
                *effect = &g_RoomM350OrbSpawnPoint.x;
                state->delay = 1;
                state->remaining--;
                Asset_Find08w(1479, g_FieldFxOwner->actor->state->progress,
                    (*effect)[0], (*effect)[1], (*effect)[2]);
            }
        }
        break;
    case 2:
        break;
    }
    return 0;
}

/* Moving effect with collision, queued trail positions and ground rendering. */
int RoomM350_OrbCallback(int event, void *_payload, intptr_t argument) {
    RoomM350Particle *p = _payload;
    GteMatrix matrix;
    GteShortVector offset,ground;
    if(event==1) {
        int timer=g_FieldFxCurrentAge;
        int brightness;
        if(timer>=48) {
            int result=1;
            g_RoomM350OrbState.activeCount=(unsigned short)g_RoomM350OrbState.activeCount-1;
            return result;
        }
        if(p->tail.value.active.frame>=48) return 0;
        if(!p->tail.value.active.hit) {
            if(p->motion.value.rotation.x>0) p->motion.value.rotation.x-=64;
            else {
                int speed=rsin((timer<<4)&0xFF0)/4;
                int angle=FieldFx_VecToAngle((Vec3Fixed *)g_PlayerEntity->render_object.model_matrix.translation,(Vec3Fixed *)g_FieldFxOwner->actor->render_object.model_matrix.translation);
                int yaw=FieldFx_TurnToward(p->motion.value.rotation.y,(short)angle,speed);
                int sum,product;
                p->motion.value.rotation.y=yaw;
                product=rsin((g_FieldFxCurrentAge+2048)&4095)*160;
                sum=yaw+80;
                p->motion.value.rotation.y=sum-product/4096;
            }
            RotMatrixYXZ(&p->motion.value.rotation,&matrix);
            matrix.t[0]=p->position.x;
            matrix.t[1]=p->position.y;
            matrix.t[2]=p->position.z;
            offset.x=0; offset.y=0; offset.z=0u-(unsigned short)p->tail.value.active.speed;
            gte_ldrotmatrix(&matrix); gte_ldtransmatrix(&matrix);
            gte_ldv0(&offset); gte_rtv0tr_mac();
            gte_stsv(&p->position);
        }
        if(p->tail.value.active.frame<40) brightness=((s16)rsin((p->tail.value.active.frame<<9)&0xE00)>>6)+128;
        else {
            unsigned int value=rcos(((p->tail.value.active.frame-40)<<7)&0xF80);
            brightness=value>>5;
        }
        p->tail.value.active.brightness=brightness;
        if(!p->tail.value.active.hit && g_PlayerEntity->mode>=4) {
            int dx=g_PlayerEntity->render_object.model_matrix.translation[0]-p->position.x;
            int dz=g_PlayerEntity->render_object.model_matrix.translation[2]-p->position.z;
            if(Math_IntSqrt((unsigned int)dx*dx+(unsigned int)dz*dz)<128) {
                short n;
                p->tail.value.active.hit=1;
                g_PlayerEntity->state->flags|=0x4000;
                if(g_FieldFxOwner->actor->state) g_FieldFxOwner->actor->state->core_flags|=0x80000000;
                p->tail.value.active.speed=0;
                if(p->tail.value.active.frame<40) p->tail.value.active.frame=40;
                n=g_RoomM350OrbState.pointerCount;
                if(n<4) {
                    int next=n;
                    g_RoomM350OrbState.pointerCount=next+1;
                    {
                        RoomM350PointerQueue *queue=(RoomM350PointerQueue *)((char *)&g_RoomM350OrbState.pointerCount-PE1_OFFSETOF(RoomM350PointerQueue,count));
                        queue->positions[n]=&p->position;
                    }
                }
            }
        }
        if(p->tail.value.active.frame>=36 && !(p->tail.value.active.frame&3)) {
            short n=g_RoomM350OrbState.pointerCount;
            if(n<4) {
                int next=n;
                g_RoomM350OrbState.pointerCount=next+1;
                {
                    RoomM350PointerQueue *queue=(RoomM350PointerQueue *)((char *)&g_RoomM350OrbState.pointerCount-PE1_OFFSETOF(RoomM350PointerQueue,count));
                    queue->positions[n]=&p->position;
                }
            }
        }
        {
            unsigned short n=g_RoomM350OrbState.positionCount;
            int next=n+1;
            int shifted=(unsigned int)n<<16;
            g_RoomM350OrbState.positionCount=next;
            {
                RoomM350PositionQueue *queue=(RoomM350PositionQueue *)((char *)&g_RoomM350OrbState.positionCount-PE1_OFFSETOF(RoomM350PositionQueue,count));
                queue->positions[shifted>>16]=p->position;
            }
        }
        p->tail.value.active.frame=(unsigned short)p->tail.value.active.frame+1;
    } else if(event==2) {
        int clut,palette;
        if(p->tail.value.active.frame>=48) return 0;
        palette=g_FieldFxPaletteTable[g_FieldFxSpriteParams.palette];
        if(g_FieldFxSpriteParams.palette==4 && g_FieldFxAltPaletteFlag) palette+=4;
        clut=GetClut(32,palette);
        FieldFx_DrawShapeQuads(&p->position,0,4096,4096,108,(unsigned short)clut,1,p->tail.value.active.brightness,0);
        ground.x=p->position.x; ground.y=g_RoomFloorY->raw; ground.z=p->position.z;
        FieldFx_DrawStarFan(&ground,192,192,8,&g_RoomM350StarFanTilt,4096,4096,&g_RoomM350OrbGroundFanColor,&g_RoomM350BlackColor,p->tail.value.active.brightness,1);
    }
    return 0;
}
int RoomM350_OrbEmitter(int event, void *_payload, intptr_t argument) {
    switch (event) {
    case 0:

        return FieldFx_InitCallbackList(g_FieldFxActiveTaskSlot->end,28,3,RoomM350_OrbCallback);
        case 1:
    {
        RoomM350EffectInstance *instance;
        RoomM350Particle *particle;
        int frame, previous;
        if (g_RoomM350OrbState.completionFlags[0]) return 2;
        instance=g_FieldFxOwner->actor;
        if (instance->mode != 13) return 0;
        frame=instance->anim.parts.integer;
        previous=(u16)(instance->anim_prev >> 16);
        if (frame<15 || previous>=15) return 0;
        particle=FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
        particle->position=g_RoomM350OrbSpawnPoint;
        particle->position.pad=16;
        particle->motion.value.spawn.speed=512;
        {
            int yaw=g_FieldFxOwner->actor->rot_y;
            particle->motion.value.spawn.phase=0;
            particle->motion.value.spawn.yaw=yaw;
        }
        {
            int count=g_RoomM350OrbState.activeCount;
            particle->tail.value.spawn.duration=16;
            particle->tail.value.spawn.frame=0;
            particle->tail.value.spawn.state=0;
            particle->tail.value.spawn.size=100-count*20;
        }
        g_RoomM350OrbState.activeCount++;
    }
        break;
    case 2:

    g_FieldFxSpriteParams.parameter00 = 32;
    g_FieldFxSpriteParams.parameter02 = 2;
    g_FieldFxSpriteParams.extent_x = 32;
    g_FieldFxSpriteParams.extent_y = 32;
    g_FieldFxSpriteParams.extent_x = 32;
    g_FieldFxSpriteParams.extent_y = 32;
    g_FieldFxSpriteParams.tpage = g_FieldFxTpageTable[g_FieldFxTpageSlots[3]];
    g_FieldFxSpriteParams.palette = 3;
    g_FieldFxSpriteParams.parameter06 = 0;
    g_FieldFxSpriteParams.parameter0A = 0;
    g_FieldFxSpriteParams.depth = 64;
    }
    return 0;
}

int RoomM350_OrbImpactCallback(int event, void *_payload, intptr_t argument) {
    PointerQueueParticle *object = _payload;
    Vector position;
    if (event == 1) {
        if (g_FieldFxCurrentAge >= 16) return 1;
        object->offset -= 16;
    } else if (event == 2) {
        int frame,kind,palette,handle;
        position = *object->position;
        position.y += object->offset;
        kind = g_FieldFxSpriteParams.palette;
        frame = g_FieldFxCurrentAge - 1;
        palette = g_FieldFxPaletteTable[kind];
        handle = GetClut(0, (kind == 4 && g_FieldFxAltPaletteFlag != 0) ? palette + 6 : palette + 2);
        FieldFx_DrawShapeQuads((GteShortVector *)&position, 0, 8192, 8192, (s16)g_FieldFxSpriteParams.parameter02 * (frame >> 1),
            (unsigned short)handle, 1,
            (s16)rsin(((unsigned int)frame << 7) & 0xF80) >> 5,
            &g_RoomM350OrbImpactColor);
    }
    return 0;
}

int RoomM350_OrbImpactEmitter(int event, void *_payload, intptr_t argument) {
    int firstIndex;
    switch (event) {
    case 0:

        return FieldFx_InitCallbackList(g_FieldFxActiveTaskSlot->end,sizeof(PointerQueueParticle),16,RoomM350_OrbImpactCallback);
        case 1:
    if (g_RoomM350OrbState.completionFlags[0] && g_RoomM350OrbState.activeCount==0) return 2;
    {
        short *count=&g_RoomM350OrbState.pointerCount;
        if (g_RoomM350OrbState.pointerCount) {
            int i;
            firstIndex=0;
            i=firstIndex;
            if (g_RoomM350OrbState.pointerCount>firstIndex) {
                Queue *queue=(Queue *)((char *)count-(unsigned long)&((Queue *)0)->count);
                Vector **position=queue->positions;
                while (i<queue->count) {
                    {
                        PointerQueueParticle *particle=FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
                        if (!particle) break;
                        particle->position=*position;
                        particle->offset=0;
                    }
                    i++;
                    position++;
                }
            }
            g_RoomM350OrbState.pointerCount=0;
        }
    }
        break;
    case 2:

    g_FieldFxSpriteParams.parameter00 = 32;
    g_FieldFxSpriteParams.parameter02 = 2;
    g_FieldFxSpriteParams.extent_x = 32;
    g_FieldFxSpriteParams.extent_y = 32;
    g_FieldFxSpriteParams.extent_x = 32;
    g_FieldFxSpriteParams.extent_y = 32;
    g_FieldFxSpriteParams.tpage = g_FieldFxTpageTable[g_FieldFxTpageSlots[11]];
    g_FieldFxSpriteParams.palette = 3;
    g_FieldFxSpriteParams.parameter06 = 1;
    g_FieldFxSpriteParams.parameter0A = 0;
    g_FieldFxSpriteParams.depth = 0;
    }
    return 0;
}

int RoomM350_OrbTrailCallback(int event, void *_payload, intptr_t argument) {
    PositionQueueParticle *object = _payload;
    short vector[4];
    if (event == 1) {
        if (g_FieldFxCurrentAge >= 16) return 1;
    } else if (event == 2) {
        int frame = g_FieldFxCurrentAge - 2;
        if (frame >= 0) {
            int size,kind,palette,handle;
            vector[0] = 1024; vector[1] = object->position.pad;
            vector[2] = 1024; vector[3] = 1;
            size = rcos((((unsigned int)frame << 7) & 0x1F80) / 2);
            kind = g_FieldFxSpriteParams.palette; palette = g_FieldFxPaletteTable[kind];
            if (kind == 4 && g_FieldFxAltPaletteFlag) palette += 4;
            handle = GetClut(0, palette);
            FieldFx_DrawShapeQuads((GteShortVector *)object, (struct GteRotation *)vector, size, size * 2, (s16)g_FieldFxSpriteParams.parameter02 * object->delay + 64,
                (unsigned short)handle, 1,
                rcos(((unsigned int)frame << 6) & 0xFC0) >> 5,
                &g_RoomM350GlowSpriteColor);
        }
    }
    return 0;
}

int RoomM350_OrbTrailEmitter(int event, void *_payload, intptr_t argument) {
    int firstIndex;
    switch (event) {
    case 0:

        return FieldFx_InitCallbackList(g_FieldFxActiveTaskSlot->end,12,50,RoomM350_OrbTrailCallback);
        case 1:
    if (g_RoomM350OrbState.completionFlags[0] && g_RoomM350OrbState.activeCount==0) return 2;
    {
        short *count=&g_RoomM350OrbState.positionCount;
        if (g_RoomM350OrbState.positionCount) {
            int i;
            firstIndex=0;
            i=firstIndex;
            if (g_RoomM350OrbState.positionCount>firstIndex) {
                RoomM350PositionQueue *queue=(RoomM350PositionQueue *)((char *)count-(unsigned long)&((RoomM350PositionQueue *)0)->count);
                Vector *position=queue->positions;
                while (i<queue->count) {
                    {
                        PositionQueueParticle *particle=FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
                        if (!particle) break;
                        particle->position=*position;
                        particle->position.pad=g_FieldFxOwner->actor->rot_y;
                        particle->delay=g_FieldFxCurrentAge & 7;
                    }
                    i++;
                    position++;
                }
            }
            g_RoomM350OrbState.positionCount=0;
        }
    }
        break;
    case 2:

    {
        int index=g_FieldFxTpageSlot3, palette;
        g_FieldFxSpriteParams.parameter00=32; g_FieldFxSpriteParams.parameter02=2;
        g_FieldFxSpriteParams.extent_x=32; g_FieldFxSpriteParams.extent_y=32; g_FieldFxSpriteParams.extent_x=32; g_FieldFxSpriteParams.extent_y=32;
        palette=g_FieldFxTpageTable[index];
        g_FieldFxSpriteParams.palette=3; g_FieldFxSpriteParams.parameter06=0; g_FieldFxSpriteParams.parameter0A=0; g_FieldFxSpriteParams.depth=0; g_FieldFxSpriteParams.tpage=palette;
    }
    }
    return 0;
}

int RoomM350_OrbTableFlashCallback(int event, void *_payload, intptr_t argument) {
    short *position = _payload;
    if (event == 1) {
        if (g_FieldFxCurrentAge >= 8) return 1;
    } else if (event == 2) {
        int kind = g_FieldFxSpriteParams.palette;
        int texture = g_FieldFxPaletteTable[kind];
        int handle;
        handle = GetClut(0, (kind == 4 && g_FieldFxAltPaletteFlag != 0) ? texture + 8 : texture + 4);
        FieldFx_DrawShapeQuads((GteShortVector *)position, 0, position[3], position[3],
            (s16)g_FieldFxSpriteParams.parameter02 * g_RoomM350OrbTableFlashSizes[(g_FieldFxCurrentAge - 1) / 2] + 128,
            (unsigned short)handle, 1,
            (s16)rsin(((unsigned int)(g_FieldFxCurrentAge - 1) << 7) & 0xF80) >> 5,
            &g_RoomM350GlowSpriteColor);
    }
    return 0;
}

int RoomM350_OrbTableFlashEmitter(int event, void *_payload, intptr_t argument) {
    switch (event) {
    case 0:

        return FieldFx_InitCallbackList(g_FieldFxActiveTaskSlot->end,8,10,RoomM350_OrbTableFlashCallback);
        case 1:
    if(g_RoomM350OrbState.completionFlags[0]) return 2;
    {
        int frame=g_FieldFxOwner->actor->anim.parts.integer;
        unsigned int relative=frame-7;
        int i;
        if(relative>=6) return 0;
        for(i=0;i<2;i++) {
            Entry *entry=&g_RoomM350OrbTableFlashEntries[frame][i];
            TableTransformParticle *particle;
            Instance *instance;
            int j;
            if(!entry->index) return 0;
            particle=FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
            if(!particle) return 0;
            if(entry->alternate) instance=(Instance *)g_RoomM350OrbActor;
            else instance=g_FieldFxOwner->actor;
            {
                int *source=instance->render_object.matrices[entry->index].translation;
                for(j=0;j<3;j++) particle->position[j]=source[j];
            }
            particle->size=(int)((unsigned int)(unsigned short)rsin((relative<<7)&0xF80) | ((unsigned int)(unsigned short)rcos((relative<<7)&0xF80) << 16))+2048;
        }
    }
        break;
    case 2:

    g_FieldFxSpriteParams.parameter00 = 32;
    g_FieldFxSpriteParams.parameter02 = 2;
    g_FieldFxSpriteParams.extent_x = 32;
    g_FieldFxSpriteParams.extent_y = 32;
    g_FieldFxSpriteParams.extent_x = 32;
    g_FieldFxSpriteParams.extent_y = 32;
    g_FieldFxSpriteParams.tpage = g_FieldFxTpageTable[g_FieldFxTpageSlots[11]];
    g_FieldFxSpriteParams.palette = 3;
    g_FieldFxSpriteParams.parameter06 = 1;
    g_FieldFxSpriteParams.parameter0A = 0;
    g_FieldFxSpriteParams.depth = 0;
    }
    return 0;
}

int RoomM350_OrbController(int event, void *_payload, intptr_t argument) {
    FieldActor *instance=(FieldActor *)g_FieldFxOwner->actor;
    Vector offset;
    if(event==0) {
        int one=1;
        {
            Owner *owner=instance->state;
            unsigned int flags=owner->core_flags;
            flags |= 0x40000000;
            owner->core_flags=flags;
        }
        one=2;
        g_RoomM350OrbState.repeats=one;
        g_RoomM350OrbState.completionFlags[0]=0; g_RoomM350OrbState.completionFlags[1]=0; g_RoomM350OrbState.completionFlags[2]=0;
        g_RoomM350OrbState.pointerCount=0; g_RoomM350OrbState.positionCount=0; g_RoomM350OrbState.activeCount=0;
        return 0;
    }
    if(event==1) {
        RenderMatrix *matrix;
        int frame;
        int repetitions;
        if(instance->state && instance->state->action_state[0]==1)
            instance->state->action_state[0]=2;
        matrix=&g_RoomM350OrbActor->render_object.matrices[17];
        offset.x=0; offset.y=-144; offset.z=128;
        gte_ldrotmatrix(matrix);
        gte_ldtransmatrix(matrix);
        gte_ldv0(&offset);
        gte_rtv0tr_mac();
        gte_stsv(&g_RoomM350OrbSpawnPoint);
        if(instance->mode!=13) return 0;
        repetitions=g_RoomM350OrbState.repeats;
        frame=instance->anim.parts.integer;
        if(repetitions>0) {
            s16 speed;
            int angle;
            frame=(short)frame;
            if(frame>=25) {
                instance->anim.fixed=0x60000;
                g_RoomM350OrbState.repeats=(unsigned short)g_RoomM350OrbState.repeats-1;
            }
            if(frame<19) return 0;
            speed=rsin(((frame-19)*341)&4095);
            speed/=32;
            angle=FieldFx_VecToAngle((Vec3Fixed *)g_PlayerEntity->render_object.model_matrix.translation,(Vec3Fixed *)instance->render_object.model_matrix.translation);
            instance->rot_y=FieldFx_TurnToward(instance->rot_y,(short)angle,speed);
        } else {
            if((short)frame>=instance->action-1) {
                if(instance->state) instance->state->action_state[0]=4;
                g_RoomM350OrbState.completionFlags[0]=1;
                return 1;
            }
        }
    }
    return 0;
}

int RoomM350_AreaSparkCallback(int event, void *_payload, intptr_t argument) {
    RoomM350RadialSpriteParticle *object = _payload;
    short position[4];
    if (event == 1) {
        if (g_FieldFxCurrentAge >= 8) {
            return 1;
        }
    } else if (event == 2) {
        int entry = (int)((unsigned int)(unsigned short)rsin(((unsigned int)g_FieldFxCurrentAge << 7) & 0xF80) | ((unsigned int)(unsigned short)rcos(((unsigned int)g_FieldFxCurrentAge << 7) & 0xF80) << 16));
        int scale = entry >> 16;
        int shade = (short)entry >> 5;
        int *origin = g_FieldFxOwner->actor->render_object.matrices[33].translation;
        int i;
        int kind, palette, handle;

        for (i = 0; i < 3; i++) {
            position[i] = object->position[i] * scale / 4096 + origin[i];
        }
        kind = g_FieldFxSpriteParams.palette;
        palette = g_FieldFxPaletteTable[kind];
        if (kind == 4 && g_FieldFxAltPaletteFlag) {
            palette += 4;
        }
        handle = GetClut(32, palette);
        FieldFx_DrawShapeQuads((GteShortVector *)position, 0, object->size, object->size, (s16)g_FieldFxSpriteParams.parameter02 + 216,
            (unsigned short)handle, 1, shade, (struct RenderColor *)g_RoomM350AreaSparkColor);
    }
    return 0;
}

/* Event state: rotation, emission count, brightness, fade phase and size. */

int RoomM350_AreaSparkEmitter(int event, void *_payload, intptr_t argument) {
    RoomM350RadialSpriteState *inputState = _payload;
    RoomM350RadialSpriteState *state=inputState;
    union { GteMatrix matrix; GteShortVector position; } local;
    GteShortVector vector;
    switch (event) {
    case 0:

        {
            RoomM350EffectInstance *instance=g_FieldFxOwner->actor;
            int *position=instance->render_object.matrices[33].translation;
            RoomM350EffectEmitter *emitter;
            Asset_Find08w(0x5C9,(int)instance->state->progress,(short)position[0],(short)position[1],(short)position[2]);
            *(int *)&state->rotation=0;
            emitter=ANIM_POOL_AS(RoomM350EffectEmitter);
            ((int *)&state->rotation)[1]=0;
            state->count=0; state->fade=0;
            return FieldFx_InitCallbackList(emitter->end,8,18,
                                 RoomM350_AreaSparkCallback);
        }
        case 1:
    {
        int phase,sample;
        int i;
        if(g_RoomM350AreaDone) {
            unsigned short old=*(volatile unsigned short *)&state->fade;
            phase=(short)old*64+1024;
            state->fade=old+1;
            if(phase>=2048) return 2;
        } else {
            int timer=g_FieldFxCurrentAge;
            phase=timer<32 ? (unsigned int)timer<<5:1024;
        }
        sample=(s16)rsin(phase&4095);
        state->brightness=sample>>5;
        state->size=(unsigned int)sample<<2;
        if(state->count>=16) return 0;
        i=0;
        while(i<2) {
            RoomM350RadialSpriteParticle *particle=FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
            int random;
            int highByte;
            int divisor;
            if(!particle) break;
            i++;
            vector.x=(unsigned int)Inv_ScrambleGrid()<<4;
            vector.y=(unsigned int)Inv_ScrambleGrid()<<4; vector.z=0;
            RotMatrixYXZ(&vector,&local.matrix);
radial:
            vector.x=0; vector.y=0;
            highByte=Inv_ScrambleGrid();
            random=((unsigned int)highByte<<8)|Inv_ScrambleGrid();
            divisor=44;
            vector.z=random%divisor+320;
            ApplyMatrixSV(&local.matrix,&vector,(GteShortVector *)particle);
            particle->size=(unsigned int)state->count*128+2048;
        }
        state->count=(unsigned short)state->count+1;
    }
        break;
    case 2:

    {
        int index=g_FieldFxTpageSlot2;
        int i;
        int palette;
        RoomM350EffectActor *actor;
        int specialKind;
        unsigned short *palettes;
        volatile int *position;
        short *out;
        g_FieldFxSpriteParams.parameter00=16; g_FieldFxSpriteParams.parameter02=1;
        g_FieldFxSpriteParams.extent_x=16; g_FieldFxSpriteParams.extent_y=16; g_FieldFxSpriteParams.extent_x=16; g_FieldFxSpriteParams.extent_y=16;
        palette=g_FieldFxTpageTable[index];

        actor=ANIM_OWNER_AS(RoomM350EffectActor);
        g_FieldFxSpriteParams.palette=2; g_FieldFxSpriteParams.parameter06=0; g_FieldFxSpriteParams.parameter0A=0; g_FieldFxSpriteParams.depth=0; g_FieldFxSpriteParams.tpage=palette;

        i=0;
        position=actor->actor->render_object.matrices[33].translation;
        out=&local.position.x;
        for(;i<3;i++) *out++=*position++;
        i=0;
        palettes=g_FieldFxPaletteTable;
        state->rotation.z=(unsigned int)g_FieldFxCurrentAge<<8;
        specialKind=4;
        while(i<2) {
            int kind=g_FieldFxSpriteParams.palette;
            int clut,pal=palettes[kind];
            if(kind==specialKind && g_FieldFxAltPaletteFlag) pal+=4;
            clut=GetClut(32,pal);
            FieldFx_DrawShapeQuads(&local.position,&state->rotation,state->size,state->size,(s16)g_FieldFxSpriteParams.parameter02+216,(unsigned short)clut,1,state->brightness,(struct RenderColor *)g_RoomM350AreaSparkEmitterColor);
            state->rotation.z=0u-(unsigned short)state->rotation.z;
            ++i;
        }
    }
    }
    return 0;
}

int RoomM350_AreaZoneEmitter(int event, void *_payload, intptr_t argument) {
    TurningAreaControllerState *state = _payload;
    union { struct { Vector vertices[4]; int area; } collision; Matrix matrix; } local;
    Matrix base;
    Vector position,rotation;
    Scale scale;
    FieldActor *instance=(FieldActor *)g_FieldFxOwner->actor;
    switch (event) {
case 0:
    {
        state->asset=Asset_FindTable08ByU32Key(g_LoadedSceneAssetBlock[0],0xC54E0704);
        FieldFx_ModelScrollUv(state->asset,0,0);
        state->size=0;
        {
            int yaw=(g_RoomM350AreaConfigYaw[0]);
            state->initialized=0;
            state->yaw=yaw*256;
        }
        return 0;
    }
case 1:
    {
        int timer=g_FieldFxCurrentAge,size;
        int high;
        int quotient;
        int intermediate;
        if(timer<16) size=((4096-rsin(((unsigned int)timer<<6)&0xFC0))>>1)+2048;
        else if(timer>=91) {
            int sample=rcos(((unsigned int)(timer-90)<<6)&0xFC0);
            size=sample>>1;
        }
        else size=2048;
        /* Expose the high product; the final sign correction still implements /3. */
        intermediate=size/3+(size>>31);
        high=intermediate;
        quotient=high-(size>>31);
        state->size=quotient;
        if(state->size<=0) return 1;
        state->brightness=size<=2048 ? size>>4:(4096-size)>>4;
        if(!state->initialized) {
            if(g_RoomM350AreaTrackPlayer) {
                int distance=FieldFx_GetDistanceXZ((Vec3Fixed *)&g_PlayerEntity->pos_x,(Vec3Fixed *)&instance->pos_x);
                int angle=FieldFx_VecToAngle((Vec3Fixed *)&g_PlayerEntity->pos_x,(Vec3Fixed *)&instance->pos_x);
                int yaw=state->yaw;
                int rawDelta=((unsigned int)angle-(yaw>>8))&4095;
                int delta=rawDelta;
                int speed,factor;
                if(rawDelta<512) { state->yaw=(unsigned int)yaw-0x20000; delta=rawDelta+512; }
                if((short)delta>3584) { delta-=512; state->yaw=(unsigned int)state->yaw+0x20000; }
                if((short)delta>2048) delta=4096-delta;
                speed=(short)distance-512;
                if(speed<0) speed=0;
                factor=1536; if(speed>1024) factor=1024;
                speed=rcos((((speed/4)&4095)*2) / 2)*factor/4096;
                if(speed<0) speed=0;
                speed+=(short)delta;
                if(((short)angle-(state->yaw>>8))&2048) speed=-speed;
                state->turn=((int)((unsigned int)speed<<8))/128;
            } else state->turn=0;
            state->initialized=1;
        }
        {
            int cosine,sine,i;
            Matrix *matrix;
            int *out;
            state->yaw=(unsigned int)state->yaw+state->turn;
            cosine=rcos(state->yaw>>8);
            sine=rsin(state->yaw>>8);
            i=0; matrix=&g_RoomM350AreaMatrix; out=matrix->t;
            matrix->m[0][0]=cosine;
            g_RoomM350AreaMatrix.m[0][2]=sine; g_RoomM350AreaMatrix.m[2][0]=0u-sine; g_RoomM350AreaMatrix.m[2][2]=cosine;
            g_RoomM350AreaMatrix.t[2]=0; g_RoomM350AreaMatrix.t[1]=0; g_RoomM350AreaMatrixTxView=0;
            g_RoomM350AreaMatrix.m[2][1]=0; g_RoomM350AreaMatrix.m[1][2]=0; g_RoomM350AreaMatrix.m[1][0]=0; g_RoomM350AreaMatrix.m[0][1]=0; g_RoomM350AreaMatrix.m[1][1]=4096;
            for(;i<3;i++) *out++=instance->render_object.model_matrix.translation[i];
        }
        if(state->brightness<64) return 0;
        {
            Matrix *matrix=&g_RoomM350AreaMatrix;
            Vector *in;
            Vector *out;
            int i;
            gte_ldrotmatrix(matrix);
            gte_ldtransmatrix(matrix);
            i=0; out=local.collision.vertices; in=g_RoomM350AreaCorners;
            for(;i<4;i++) {
                gte_ldv0(in);
                gte_rtv0tr_mac();
                gte_stsv(out);
                out++; in++;
            }
        }
        {
            int i,previous,point;
            Vector *out;
            int *area;
            i=0; area=&local.collision.area; out=local.collision.vertices;
            previous=((unsigned int)local.collision.vertices[3].z<<16)|(unsigned short)local.collision.vertices[3].x;
            point=((unsigned int)g_PlayerEntity->render_object.model_matrix.translation[2]<<16)|(unsigned short)g_PlayerEntity->render_object.model_matrix.translation[0];
            for(;i<4;i++) {
                int current=((unsigned int)out->z<<16)|(unsigned short)out->x;
                gte_ldsxy0(point); gte_ldsxy2(previous); gte_ldsxy1(current);
                gte_nclip(); gte_stmac0(area);
                if(local.collision.area<0) break;
                out++; previous=current;
            }
            if(i>=4) {
                unsigned char *inside=&g_RoomM350AreaPlayerInside;
                if(!*inside) {
                    Owner *owner;
                    g_PlayerEntity->state->flags|=0x4000;
                    owner=instance->state;
                    if(owner) owner->core_flags|=0x80000000;
                    *inside=1; g_RoomM350AreaEntered=1;
                }
            } else g_RoomM350AreaPlayerInside=0;
        }
    }
    break;
case 2:
    {
        int page,palette;
        u16 rawPage;
        int kind;
        int i;
        int size;
        int *in,*out;
        Matrix *transforms;
        i=0;
        base=g_RoomM350AreaMatrix;
        transforms=(Matrix *)instance->render_object.matrices;
        out=base.t; in=transforms[33].t;
        for(;i<3;i++) *out++=*in++;
        rawPage=GetTPage(0,1,0,0);
        page=(unsigned short)(g_FieldFxTpageTable[g_FieldFxTpageSlot11]|rawPage);
        kind=g_FieldFxSpriteParams.palette;
        palette=g_FieldFxPaletteTable[kind];
        palette=GetClut(0,(kind==4 && g_FieldFxAltPaletteFlag!=0) ? palette+7 : palette+3);
        FieldFx_ModelSetTexture(page,(unsigned short)palette);
        FieldFx_ModelSetSemiTrans(1);
        FieldFx_ModelSaveColors(state->asset);
        FieldFx_ModelScaleColors(state->asset,(unsigned short)state->brightness);
        i=0;
        for (;i<3;) {
            int next;
            int product;
            int high;
            int intermediate;
            void *clearTarget;
            int clearValue;
            int clearLength;
            local.matrix=base;
            next=i+1;
            product=state->size*next;
            intermediate=product/3+(product>>31);
            clearTarget=&scale;
            clearValue=0;
            clearLength=16;
            i=next;
            product>>=31;
            (high = intermediate);
            size=high-product;
            memset(clearTarget,clearValue,clearLength);
            scale.x=size; scale.y=size; scale.z=2048;
            ScaleMatrix((GteMatrix *)&local.matrix, (const GteVector *)&scale);
            FieldFx_ModelDraw(state->asset,(GteMatrix *)&local.matrix);
        }
        FieldFx_ModelRestoreColors(state->asset);
        i=0;
        position.x=instance->render_object.model_matrix.translation[0]; position.y=instance->render_object.model_matrix.translation[1]; position.z=instance->render_object.model_matrix.translation[2];
        {
            int yaw;
            rotation.x=0;
            yaw=state->yaw;
            rotation.z=0;
            rotation.y=(yaw>>8)+2024;
        }
        {
            int triple=state->size*3;
            size=triple*2;
            for (;i<3;i++) {
                FieldFx_DrawRotatedTriangle(&position,(GteRotation *)&rotation,size,512,(RenderColor *)g_RoomM350AreaZoneTriangleColor,&g_RoomM350BlackColor,&g_RoomM350BlackColor,state->brightness,1);
                rotation.y=(unsigned short)rotation.y+24;
            }
        }
    }
    }
    return 0;
}

int RoomM350_AreaDropletCallback(int event, void *_payload, intptr_t argument) {
    short *position = _payload;
    if (event == 1) {
        if (g_FieldFxCurrentAge >= 16) return 1;
        position[1] += position[3];
        position[3] -= position[4];
    } else if (event == 2) {
        int kind = g_FieldFxSpriteParams.palette;
        int palette = g_FieldFxPaletteTable[kind];
        int handle;
        if (kind == 4 && g_FieldFxAltPaletteFlag) palette += 4;
        handle = GetClut(16, palette);
        FieldFx_DrawShapeQuads((GteShortVector *)position, 0, 12288, 12288,
            (s16)g_FieldFxSpriteParams.parameter02 * (((g_FieldFxCurrentAge - 1) >> 1) & 7) + 200,
            (unsigned short)handle, 1,
            (s16)rsin(((unsigned int)(g_FieldFxCurrentAge - 1) << 7) & 0xF80) >> 5,
            (struct RenderColor *)g_RoomM350AreaDropletColor);
    }
    return 0;
}

int RoomM350_AreaDropletEmitter(int event, void *_payload, intptr_t argument) {
    switch (event) {
    case 0:

        return FieldFx_InitCallbackList(g_FieldFxActiveTaskSlot->end,12,16,RoomM350_AreaDropletCallback);
        case 1:
    if (g_FieldFxCurrentAge >= 107) return 2;
    if (g_FieldFxCurrentAge & 1) {
        RenderMatrix *matrices = ((Instance *)g_FieldFxOwner->actor)->render_object.matrices;
        DualBallisticParticle *particle = FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
        if (particle) {
            int random = Inv_ScrambleGrid();
            particle->x = (unsigned int)matrices[33].translation[0] - (((unsigned int)random << 1) - 256);
            particle->y = matrices[33].translation[1];
            random = Inv_ScrambleGrid();
            particle->z = (unsigned int)matrices[33].translation[2] - (((unsigned int)random << 1) - 256);
            particle->velocityY = 0;
            particle->gravity = ((g_FieldFxCurrentAge >> 1) & 1) + 2;
        }
    }
    if (g_RoomM350AreaPlayerInside) {
        RenderMatrix *matrices = ((Instance *)g_PlayerEntity)->render_object.matrices;
        DualBallisticParticle *particle = FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
        if (!particle) return 0;
        {
            int random = Inv_ScrambleGrid();
            particle->x = (unsigned int)matrices[0].translation[0] - (((unsigned int)random << 1) - 256);
            particle->y = matrices[0].translation[1];
            random = Inv_ScrambleGrid();
            particle->z = (unsigned int)matrices[0].translation[2] - (((unsigned int)random << 1) - 256);
            particle->velocityY = 0;
            particle->gravity = ((g_FieldFxCurrentAge >> 1) & 1) + 2;
        }
    }
        break;
    case 2:

    {
        int index = g_FieldFxTpageSlot2;
        int palette;
        g_FieldFxSpriteParams.parameter00 = 16; g_FieldFxSpriteParams.parameter02 = 1;
        g_FieldFxSpriteParams.extent_x = 16; g_FieldFxSpriteParams.extent_y = 16;
        g_FieldFxSpriteParams.extent_x = 16; g_FieldFxSpriteParams.extent_y = 16;
        palette = g_FieldFxTpageTable[index];
        g_FieldFxSpriteParams.palette = 2; g_FieldFxSpriteParams.parameter06 = 0;
        g_FieldFxSpriteParams.parameter0A = 0; g_FieldFxSpriteParams.depth = 0; g_FieldFxSpriteParams.tpage = palette;
    }
    }
    return 0;
}

/* Original M350 immutable data and native storage for its adjacent runtime state. */
GteRotation g_RoomM350StarFanTilt = {1024,0,0,1};
RenderColor g_RoomM350BlackColor = {0,0,0,0};
int g_RoomM350OrbHandSparkColor[1] = {4210752};
RenderColor g_RoomM350GlowSpriteColor = {128,128,192,0};
RenderColor g_RoomM350HandSparkColor = {80,80,128,0};
int g_RoomM350HandPulseColors[8] = {128,32896,16512,32768,8404992,8421376,8388608,8388736};
RenderColor g_RoomM350HandSweepTrapPulseColors[2] = {{192,192,240,0},{128,128,240,0}};
RenderColor g_RoomM350HandSweepTrapTriangleColor = {48,48,64,0};
RenderColor g_RoomM350HandSweepTrapRingColor = {24,24,32,0};
GteShortVector g_RoomM350HandSweepTrapCorners[4] = {{-128,0,0,0},{128,0,0,0},{128,0,-4096,0},{-128,0,-4096,0}};
RenderColor g_RoomM350HandSweepHitColor = {0,128,240,0};
/* The original frame-indexed table spans the adjacent immutable colour and
 * size bytes; frames 7..12 use the final six rows. */
Entry g_RoomM350OrbTableFlashEntries[13][2] = {{{0,0},{0,0}},{{0,0},{0,0}},{{192,192},{240,0}},{{16,16},{32,0}},{{80,80},{80,0}},{{16,64},{240,0}},{{2,3},{6,7}},{{15,1},{11,1}},{{14,1},{10,1}},{{13,1},{9,1}},{{29,0},{0,0}},{{30,0},{0,0}},{{31,0},{0,0}}};
int g_RoomM350OrbBurstRingColorA[1] = {15777984};
int g_RoomM350OrbBurstRingColorB[1] = {2101264};
RenderColor g_RoomM350OrbGroundFanColor = {80,80,80,0};
RenderColor g_RoomM350OrbImpactColor = {16,64,240,0};
unsigned char g_RoomM350OrbTableFlashSizes[28] = {2,3,6,7,15,1,11,1,14,1,10,1,13,1,9,1,29,0,0,0,30,0,0,0,31,0,0,0};
int g_RoomM350AreaSparkColor[1] = {16752880};
char g_RoomM350AreaSparkEmitterColor[4] = {192,128,192,0};
GteShortVector g_RoomM350AreaCorners[4] = {{-128,0,0,0},{128,0,0,0},{160,0,-4096,0},{-160,0,-4096,0}};
char g_RoomM350AreaZoneTriangleColor[4] = {32,16,32,0};
int g_RoomM350AreaDropletColor[1] = {8405120};
int g_RoomM350AreaPlayerPulseColor[1] = {12632192};
int g_RoomM350DropFlareColors[2] = {8405056,0};
RenderColor g_RoomM350DropColors[3] = {{128,128,240,0},{40,40,80,0},{0,0,0,0}};
int g_RoomM350DropSplashColor[1] = {4202528};
RenderColor g_RoomM350DropHitColor = {16,64,240,0};
RenderColor g_RoomM350DropTrailColor = {64,64,128,0};
int g_RoomM350DropBurstColor[1] = {15761536};
int g_RoomM350RushFlareColors[4] = {15790320,15749360,15790320,15749200};
RenderColor g_RoomM350BodyRingColor1 = {128,64,64,0};
RenderColor g_RoomM350BodyRingColor0 = {0,0,0,0};
FieldActor *g_RoomM350HandActor;
GteShortVector g_RoomM350HandPoints[4];
s16 g_RoomM350HandSweepRepeats;
u8 g_RoomM350HandSweepDone,g_RoomM350HandSweepHit;
RoomM350OrbStorage g_RoomM350OrbData;
RoomM350AreaStorage g_RoomM350AreaData;
RoomM350TrailQueue g_RoomEffectTrailPositions;
RoomM350RushStorage g_RoomM350RushData;

/* Original six M350 task programs; runtime pointer payloads use native sizes. */
u16 g_RoomM350HandScript[30] = {2,7,0,4,1,0,2,0,0,4,4,0,2,1,0,2,2,0,2,3,0,2,4,0,2,5,0,0,0,0};
FieldFxTaskProgram g_RoomM350HandEffects = {{{RoomM350_HandSparkEmitter,RoomM350_HandPulseEmitter,RoomM350_HandGlowEmitter,RoomM350_HandSweepTrapEmitter,RoomM350_HandSweepHitEmitter,RoomM350_HandFlashEmitter,NULL,RoomM350_HandSweepController},{0,4,4,0,0,0,0,0}},RoomM350_HandConfigure,g_RoomM350HandScript};
/* The last operand and STOP overlap the original table's zero prefix. */
u16 g_RoomM350OrbScript[30] = {2,7,0,4,1,0,2,0,0,2,1,0,4,23,0,2,2,0,2,3,0,2,4,0,2,5,0,0,0,0};
FieldFxTaskProgram g_RoomM350OrbEffects = {{{RoomM350_OrbHandSparkEmitter,RoomM350_OrbBurstEmitter,RoomM350_OrbEmitter,RoomM350_OrbImpactEmitter,RoomM350_OrbTrailEmitter,RoomM350_OrbTableFlashEmitter,NULL,RoomM350_OrbController},{0,4,0,0,0,0,0,0}},RoomM350_OrbConfigure,g_RoomM350OrbScript};
u16 g_RoomM350AreaScript[22] = {2,7,0,2,0,0,4,30,0,2,1,0,2,2,0,2,3,0,0,0,0,0};
FieldFxTaskProgram g_RoomM350AreaEffects = {{{RoomM350_AreaSparkEmitter,RoomM350_AreaZoneEmitter,RoomM350_AreaDropletEmitter,RoomM350_AreaPlayerPulseEmitter,NULL,NULL,NULL,RoomM350_AreaController},{16,sizeof(TurningAreaControllerState),0,0,0,0,0,0}},RoomM350_AreaConfigure,g_RoomM350AreaScript};
u16 g_RoomM350DropScript[30] = {2,7,0,4,1,0,2,5,0,4,10,0,2,0,0,2,1,0,2,2,0,2,3,0,2,4,0,0,0,0};
FieldFxTaskProgram g_RoomM350DropEffects = {{{RoomM350_DropFlareEmitter,RoomM350_DropEmitter,RoomM350_DropSplashEmitter,RoomM350_DropHitEmitter,RoomM350_DropTrailEmitter,RoomM350_DropBurstEmitter,NULL,RoomM350_DropController},{0,0,0,0,0,0,0,4}},RoomM350_DropConfigure,g_RoomM350DropScript};
u16 g_RoomM350RushScript[16] = {2,7,0,2,2,0,4,24,0,2,0,0,0,0,0,0};
FieldFxTaskProgram g_RoomM350RushEffects = {{{RoomM350_RushFlareEmitter,NULL,RoomM350_RushCloudEmitter,NULL,NULL,NULL,NULL,RoomM350_RushController},{0,0,0,0,0,0,0,12}},RoomM350_RushConfigure,g_RoomM350RushScript};
u16 g_RoomM350BodyScript[18] = {5,0,0,4,8,0,2,0,0,4,60,0,2,1,0,0,0,0};
FieldFxTaskProgram g_RoomM350BodyEffects = {{{RoomM350_BodyDebrisEmitter,RoomM350_BodyRingEmitter,NULL,NULL,NULL,NULL,NULL,NULL},{4,4,0,0,0,0,0,0}},NULL,g_RoomM350BodyScript};
