/*
 * The drop field effect set: its module class and the effects it runs.
 *
 * The class is the seven methods the class table lists in front of the
 * set. The registration method clears the target's 0x40000000 flag before
 * it registers the draw list, and both it and the start method act only
 * while the field engine runs the object (status 3). room_m348 links the
 * class as its only module; room_m174 and room_m383 link it after the glow
 * orb's class. The script and the lists are each room's own data
 * (pe1/room_drop_field_class.h).
 *
 * The class and the set follow each other in the same order in all three
 * rooms, with the set's 0x70 bytes of read-only data as the unit's only
 * rodata, so they are one unit.
 */

#include "pe1/field_collision.h"
#include "pe1/field_script_context.h"
#include "pe1/gte.h"
#include "pe1/gte_short_vector.h"
#include "pe1/room_beam_pair.h"
#include "pe1/room_drop_field.h"
#include "pe1/room_drop_field_class.h"
#include "pe1/room_four_sprite.h"
#include "pe1/room_ring_pair.h"
#include "pe1/room_spark.h"
#include "pe1/room_staged_motion.h"

#include <string.h>
#include "pe1/field_actor.h"
#include "pe1/fieldfx_state.h"

void RoomEffect_DropFieldNop0(void) {}
void RoomEffect_DropFieldPlantScript(PmSlotHeader *object) {
    *FieldFx_GetSlot((FieldFxObject *)object) = g_RoomDropFieldScript;
}
int RoomEffect_DropFieldSpawn6(PmSlotHeader *slot, int b, int c, intptr_t d, intptr_t e, intptr_t f) {
    FieldFx_Spawn6((intptr_t)slot,b,c,d,(int)e,(int)f);return 0;
}
int RoomEffect_DropFieldRegister(PmSlotHeader *slot) {
    if (FieldFx_GetStatus(slot)==3) {
        ((FieldFxObjectPrefix *)slot)->actor->state->core_flags &= 0xBFFFFFFFu;
        FieldFx_Register((FieldFxObject *)slot,g_RoomDropFieldDrawList);
    }
    return 0;
}
int RoomEffect_DropFieldStart(PmSlotHeader *slot) {
    int result=-1;
    if (FieldFx_GetStatus(slot)==3) {
        result=FieldFx_UpdateSlots((FieldFxObject *)slot,g_RoomDropFieldUpdateList);
        result|=FieldFx_RunScript((FieldFxObject *)slot,g_RoomDropFieldSpawnData,g_RoomDropFieldSpawnScript);
    }
    if(result==-1)RoomEffect_DropFieldClose(slot);
    return 0;
}
int RoomEffect_DropFieldClose(PmSlotHeader *slot) {
    slot->state=4;
    if((unsigned)FieldFx_GetStatus(slot)>=2) {
        FieldActor *actor=((FieldFxObjectPrefix *)slot)->actor;
        actor->state->core_flags &= 0xC0FFFFFFu;
        *actor->state->action_state=4;
    }
    return 0;
}
void RoomEffect_DropFieldNop6(void) {}

/*
 * The drop field effect set: a controller (init, no-op, hold timer) and
 * twelve init/draw/update triples, two of them empty.
 *
 * room_m174, room_m348 and room_m383 link the same 39 functions in this
 * order, with the same 0x70 bytes of read-only data; this unit is that
 * object, compiled into each of them. The sprite and ring records, the
 * spawn tables and the images live in each room's own data. The object
 * starts after the no-op that ends the room prefix and, in room_m174 and
 * room_m383, ends where the room library begins.
 */

static const GteShortVector s_DropFieldTiltX = { 0x400, 0, 0, 0 };
static const GteShortVector s_DropFieldTiltY = { 0, 0x400, 0, 0 };
static const GteShortVector s_DropFieldNoTilt = { 0, 0, 0, 0 };
static const RoomFxVec4 s_DropFieldSpriteScale = { 0x400, 0x400, 0x400, 0 };
static const RoomFxVec4 s_DropFieldFlatScale = { 0x978, 0x978, 0x978, 0 };
static const RoomFxVec4 s_DropFieldModelScale = { 0x1190, 0x1190, 0x1190, 0 };
static const RoomFxVec4 s_DropFieldShadowScale = { 0xBE8, 0xBE8, 0xBE8, 0 };
static const GteShortVector s_EightParticleOffset = { 0, 0x28, -0x32, 0 };
static const GteShortVector s_EightParticleStepRight = { 0xA, 0, 0, 0 };
static const GteShortVector s_EightParticleStepLeft = { -0xA, 0, 0, 0 };

int RoomEffect_DropFieldInit(void *data0, void *unused, void *data2) {
    RoomDropFieldEntity * ent = data0;
    RoomDropFieldState * state = data2;
    RoomDropFieldLink *link;
    RoomDropFieldMatrixWords *matrix;

    FieldFx_SetScriptContext(state);
    g_RoomDropFieldModel = Asset_FindTable08ByU32Key(g_LoadedSceneAssetBlock[0], 0x118704);

    link = ent->prefix.actor;
    state->link = link;
    matrix = (RoomDropFieldMatrixWords *)link->render_object.matrices;
    memcpy(&state->matrix0, matrix, sizeof(state->matrix0));
    matrix = (RoomDropFieldMatrixWords *)&state->link->render_object.matrices[5];
    memcpy(&state->matrix1, matrix, sizeof(state->matrix1));
    state->asset.index = Asset_SearchByKeyType(9);

    state->holdTimer = 0;
    state->hit = 0;
    state->holdFrames = 0x96;
    state->particlePattern = *FieldFx_VarPtr(0);
    state->radiusRate = *FieldFx_VarPtr(1);
    state->fallRate = *FieldFx_VarPtr(2);
    state->initialFallStep = *FieldFx_VarPtr(3);

    g_RoomDropFieldSpinSprite.code = 0x22;
    g_RoomDropFieldSpinSprite.mode = 0x30;
    g_RoomDropFieldDoubleSprite.code = 0x40;
    g_RoomDropFieldDoubleSprite.mode = 5;
    g_RoomDropFieldModelSprite.offset = 0x64;
    g_RoomDropFieldDropSprite.code = 6;
    g_RoomDropFieldSpinSprite.offset = 0;
    g_RoomDropFieldSpinSprite.depth = 0x80;
    g_RoomDropFieldSpinSprite.r = 0x80;
    g_RoomDropFieldSpinSprite.g = 0x80;
    g_RoomDropFieldSpinSprite.b = 0x80;
    g_RoomDropFieldSpinSprite.zero = 0;
    g_RoomDropFieldDoubleSprite.offset = 0;
    g_RoomDropFieldDoubleSprite.depth = 0x80;
    g_RoomDropFieldDoubleSprite.r = 0x80;
    g_RoomDropFieldDoubleSprite.g = 0x80;
    g_RoomDropFieldDoubleSprite.b = 0x80;
    g_RoomDropFieldDoubleSprite.zero = 0;
    g_RoomDropFieldMotionSprite.offset = 0;
    g_RoomDropFieldMotionSprite.depth = 0x80;
    g_RoomDropFieldMotionSprite.r = 0x80;
    g_RoomDropFieldMotionSprite.g = 0x80;
    g_RoomDropFieldMotionSprite.b = 0x80;
    g_RoomDropFieldMotionSprite.zero = 0;
    g_RoomDropFieldModelSprite.code = 0x68;
    g_RoomDropFieldModelSprite.mode = 7;
    g_RoomDropFieldModelSprite.depth = 0x80;
    g_RoomDropFieldModelSprite.r = 0x80;
    g_RoomDropFieldModelSprite.g = 0x80;
    g_RoomDropFieldModelSprite.b = 0x80;
    g_RoomDropFieldModelSprite.zero = 0;
    g_RoomDropFieldDropSprite.mode = 0x60;
    g_RoomDropFieldQuadSprite.code = 2;
    g_RoomDropFieldDropSprite.offset = 0;
    g_RoomDropFieldDropSprite.depth = 0x60;
    g_RoomDropFieldDropSprite.r = 0x80;
    g_RoomDropFieldDropSprite.g = 0x80;
    g_RoomDropFieldDropSprite.b = 0x80;
    g_RoomDropFieldDropSprite.zero = 0;
    g_RoomDropFieldQuadSprite.mode = 0x20;
    g_RoomDropFieldQuadSprite.offset = 0;
    g_RoomDropFieldQuadSprite.depth = 0x60;
    g_RoomDropFieldQuadSprite.r = 0x80;
    g_RoomDropFieldQuadSprite.g = 0x80;
    g_RoomDropFieldQuadSprite.b = 0x80;
    g_RoomDropFieldQuadSprite.zero = 0;
    g_RoomDropFieldParticleSprite.code = 0x68;
    g_RoomDropFieldParticleSprite.mode = 7;
    g_RoomDropFieldParticleSprite.offset = 0;
    g_RoomDropFieldParticleSprite.depth = 0x80;
    g_RoomDropFieldParticleSprite.zero = 0;
    return 0;
}

int RoomEffect_DropFieldNop(void *unused0, void *unused1, void *unused2) {

    return 0;
}

int RoomEffect_DropFieldTick(void *arg0, void *data1, void *data2) {
    char* arg1 = data1;
    char* arg2 = data2;
    RoomDropFieldState *state = (RoomDropFieldState *)arg2;

    if (state->holdTimer != 0) {
        state->holdTimer--;
    }

    if (state->hit == 1) {
        int timer = state->holdTimer;

        state->hit = 0;
        if (timer == 0) {
            state->holdTimer = state->holdFrames;
            func_800C6C18(arg0);
        }
    }

    if (FieldFx_IsLastSlotHalted() == 1) {
        arg1[1] = 2;
    }
    return 0;
}

int RoomEffect_InitStageSpawn(void *unused0, void *unused1, void *unused2) {

    return 0;
}

int RoomEffect_DrawStageSpawn(void *unused0, void *unused1, void *unused2) {

    return 0;
}

int RoomEffect_UpdateStageSpawn(void *effect, void *data1, void *data2) {
    RoomDropSlot * state = data1;
    RoomFxSeed8 * seed = data2;
    RoomDropFieldState *owner = FieldFx_GetScriptContext();
    char *slot;

    if (state->step == 0) {
        slot = FieldFx_StartSlot(effect, 4, (unsigned short *)g_RoomDropFieldSpawnScript, (int (**)(void *, void *, void *))g_RoomDropFieldSpawnData);
        if (slot != 0) {
            *(RoomFxSeed8 *)slot = *seed;
        }
        if (state->step == 0) {
            slot = FieldFx_StartSlot(effect, 3, (unsigned short *)g_RoomDropFieldSpawnScript, (int (**)(void *, void *, void *))g_RoomDropFieldSpawnData);
            if (slot != 0) {
                *(RoomFxSeed8 *)slot = *seed;
            }
            if (state->step == 0) {
                slot = FieldFx_StartSlot(effect, 2, (unsigned short *)g_RoomDropFieldSpawnScript, (int (**)(void *, void *, void *))g_RoomDropFieldSpawnData);
                if (slot != 0) {
                    ((RoomDropFieldSprite *)slot)->phase = 1;
                    *(RoomFxSeed8 *)slot = *seed;
                }
            }
        }
    }
    if (state->step == 4) {
        slot = FieldFx_StartSlot(effect, 2, (unsigned short *)g_RoomDropFieldSpawnScript, (int (**)(void *, void *, void *))g_RoomDropFieldSpawnData);
        if (slot != 0) {
            ((RoomDropFieldSprite *)slot)->phase = 0;
            *(RoomFxSeed8 *)slot = *seed;
        }
    }
    if (state->step == 6) {
        slot = FieldFx_StartSlot(effect, 5, (unsigned short *)g_RoomDropFieldSpawnScript, (int (**)(void *, void *, void *))g_RoomDropFieldSpawnData);
        if (slot != 0) {
            unsigned int i;

            *(RoomFxSeed8 *)slot = *seed;
            if (owner->particlePattern == 0) {
                ((RoomStagedMotionState *)slot)->count = 8;
                ((RoomStagedMotionState *)slot)->remaining = 8;
                /* The original loop compares the signed halfword count as unsigned. */
                for (i = 0; i < ((RoomStagedMotionState *)slot)->count; i++) {
                    ((RoomStagedMotionState *)slot)->angle[i] = i << 9;
                }
            } else {
                ((RoomStagedMotionState *)slot)->count = 6;
                ((RoomStagedMotionState *)slot)->remaining = 6;
                for (i = 0; i < ((RoomStagedMotionState *)slot)->count; i++) {
                    ((RoomStagedMotionState *)slot)->angle[i] = i * 0x2A8;
                }
            }
        }
    }
    if (state->step >= 0x3D) {
        state->command = 2;
    }
    return 0;
}

int RoomEffect_InitSpinSprite(void *data0, void *data1, void *data2) {
    void* arg0 = data0;
    void* arg1 = data1;
    RoomDropFieldSprite * sprite = data2;
    sprite->scale = 0x1418;
    sprite->depth = 0xFF;
    sprite->rotation.x = 0;
    sprite->rotation.y = 0;
    sprite->rotation.z = 0;
    return 0;
}

int RoomEffect_DrawSpinSprite(void *arg0, void *arg1, void *data2) {
    RoomDropFieldSprite * sprite = data2;
    RoomDropFieldSpinFrame stack;
    s16 *ptr;

    FieldFx_SetSpriteTexturePreset(((RoomDropFieldState *)FieldFx_GetScriptContext())->asset.page);
    FieldFx_SetSpriteCellSize(0x20, 0x20);
    FieldFx_SetSpriteColorMode(0x100);
    FieldFx_SetSpriteBlendMode(2);
    RotMatrix(&sprite->rotation, &stack.sp10);
    memset(&stack.sp40, 0, 0x10);

    {
        s32 x = (s16)sprite->scale;
        stack.sp40.x = x;
    }
    {
        s32 y = (s16)sprite->scale;
        stack.sp40.z = 0x1000;
        stack.sp40.y = y;
    }
    stack.sp30 = stack.sp40;
    ScaleMatrix(&stack.sp10, &stack.sp30);

    ptr = &g_RoomDropFieldSpinSprite.depth;
    *ptr = sprite->depth;
    stack.sp24[0] = sprite->x;
    stack.sp24[1] = sprite->y;
    stack.sp24[2] = sprite->z;
    FieldFx_DrawGlowSprite((FieldGlowSprite *)(ptr - 5), (GteMatrix *)&stack.sp10, 1);
    return 0;
}

int RoomEffect_UpdateSpinSprite(void *arg0, void *data1, void *data2) {
    RoomDropSlot * slot = data1;
    RoomDropFieldSprite * sprite = data2;
    RoomDropFieldSprite *obj = sprite;
    u16 value;

    if ((s16)sprite->phase == 1) {
        sprite->scale += 0xC8;
    }

    if ((s16)sprite->phase == 0) {
        sprite->scale -= 0xC8;
    }

    value = obj->depth - 0x10;
    obj->depth = value;
    if ((s16)value < 0) {
        obj->depth = 0;
    }

    if (slot->step >= 0x15) {
        slot->command = 2;
    }
    return 0;
}

int RoomEffect_InitDoubleSprite(void *data0, void *data1, void *data2) {
    void* arg0 = data0;
    void* arg1 = data1;
    RoomDropFieldSprite * sprite = data2;
    sprite->scale = 0xAD4;
    sprite->depth = 0xFF;
    sprite->rotation.x = 0;
    sprite->rotation.y = 0;
    sprite->rotation.z = 0;
    sprite->phase = 0;
    return 0;
}

/* This view starts at the alpha byte inside the room's sprite state. */
#define ROOM_SPRITE_GLOBALS ((RoomDoubleSpriteGlobals *)&g_RoomDropFieldDoubleSprite.code)

int RoomEffect_DrawDoubleSprite(void *arg0, void *arg1, void *data2) {
    RoomDoubleSpriteFxParams * fx = data2;
    RoomSpriteMatrix matrix;
    RoomFxSeed8 seed;
    RoomFxVec4 scratch;
    RoomFxVec4 scale;
    RoomFxVec4 second_scale;
    RoomDropFieldState *owner;
    u16 *depth_slot;

    owner = FieldFx_GetScriptContext();
    seed = *(RoomFxSeed8 *)&s_DropFieldTiltX;
    FieldFx_SetSpriteTexturePreset(owner->asset.page);
    FieldFx_SetSpriteCellSize(0x20, 0x20);
    FieldFx_SetSpriteColorMode(0x10);
    FieldFx_SetSpriteBlendMode(2);
    RotMatrix(&fx->seed, &matrix);

    memset(&scale, 0, 0x10);
    scale.x = fx->scale;
    scale.y = fx->scale;
    scale.z = 0x1000;
    scratch = scale;
    ScaleMatrix(&matrix, &scratch);

    depth_slot = &ROOM_SPRITE_GLOBALS->depth;
    *depth_slot = fx->depth;
    ROOM_SPRITE_GLOBALS->alpha = fx->alpha * 2 + 0x40;
    matrix.t[0] = fx->x;
    matrix.t[1] = fx->y;
    matrix.t[2] = fx->z;
    FieldFx_DrawGlowSprite((FieldGlowSprite *)((char *)depth_slot - 0xA), &matrix, 1);

    RotMatrix(&seed, &matrix);
    memset(&second_scale, 0, 0x10);
    second_scale.x = fx->scale;
    second_scale.y = fx->scale;
    second_scale.z = 0x1000;
    scale = second_scale;
    ScaleMatrix(&matrix, &scale);

    *depth_slot = (s16)fx->depth >> 2;
    matrix.t[0] = fx->x;
    matrix.t[1] = g_RoomFloorY->y;
    matrix.t[2] = fx->z;
    FieldFx_DrawGlowSprite((FieldGlowSprite *)((char *)depth_slot - 0xA), &matrix, 0);
    return 0;
}

int RoomEffect_UpdateDoubleSprite(void *arg0, void *data1, void *data2) {
    RoomDropSlot * slot = data1;
    RoomDropFieldSprite * sprite = data2;
    u16 value;

    sprite->scale = sprite->scale + 0xC8;
    sprite->depth = sprite->depth - 0x10;
    value = sprite->phase + 1;
    sprite->phase = value;
    if ((s16)value >= 8) {
        slot->command = 2;
    }

    if ((s16)sprite->depth < 0) {
        sprite->depth = 0;
    }
    return 0;
}

int RoomEffect_InitRingPair(void *unused0, void *unused1, void *data2) {
    RoomRingPairState * bounds = data2;
    s16 *anchor;
    char *source;
    int mode;
    int intensity;
    int color1_0;
    int color1_1;
    int color1_2;
    int extent1;
    RoomFxEmitterParams *params;
    RoomFxEmitterParams *secondary;
    int primaryExtent;
    int secondaryExtent;

    anchor = &g_RoomDropFieldRings[0].mode;
    params = &g_RoomDropFieldRings[0];

    bounds->scale = 10;
    bounds->secondaryScale = 10;
    bounds->secondaryScaleStep = 20;
    bounds->depth = 255;

    mode = 16;
    intensity = 128;
    color1_0 = 120;
    color1_1 = 100;
    color1_2 = 30;
    primaryExtent = 0x6A4;
    extent1 = 0x514;
    source = g_RoomDropFieldRingImages;

    g_RoomDropFieldRings[0].mode = mode;
    g_RoomDropFieldRings[0].offset = 0;
    g_RoomDropFieldRings[0].intensity = intensity;
    g_RoomDropFieldRings[0].color1[0] = color1_0;
    g_RoomDropFieldRings[0].color1[1] = color1_1;
    g_RoomDropFieldRings[0].color1[2] = color1_2;
    g_RoomDropFieldRings[0].color0[0] = 0;
    g_RoomDropFieldRings[0].color0[1] = 0;
    g_RoomDropFieldRings[0].color0[2] = 0;
    g_RoomDropFieldRings[0].extent0 = primaryExtent;
    g_RoomDropFieldRings[0].extent1 = extent1;
    params->source = source;
    FieldFx_BuildRingGeometry(params);

    params = &g_RoomDropFieldRings[1];
    secondaryExtent = 0x226;
    source += 0x100;
    secondary = &g_RoomDropFieldRings[1];
    secondary->mode = mode;
    secondary->offset = 0;
    secondary->intensity = intensity;
    secondary->color1[0] = 0;
    secondary->color1[1] = 0;
    secondary->color1[2] = 0;
    secondary->color0[0] = color1_0;
    secondary->color0[1] = color1_1;
    secondary->color0[2] = color1_2;
    secondary->extent1 = secondaryExtent;
    secondary->extent0 = extent1;
    secondary->source = source;
    FieldFx_BuildRingGeometry(params);
    return 0;
}

int RoomEffect_DrawRingPair(void *unused0, void *unused1, void *data2) {
    RoomRingPairState * fx = data2;
    RoomSpriteMatrix matrix;
    RoomFxSeed8 seed0;
    RoomFxSeed8 seed1;
    RoomSpriteMatrix saved_matrix;
    RoomFxVec4 scratch;
    RoomFxVec4 scale;
    RoomFxVec4 second_scale;
    u16 *depth_slot;

    seed0 = *(RoomFxSeed8 *)&s_DropFieldTiltX;
    seed1 = *(RoomFxSeed8 *)&s_DropFieldTiltY;
    FieldFx_SetSpriteBlendMode(2);
    depth_slot = &g_RoomDropFieldRings[0].intensity;
    *depth_slot = fx->depth;
    g_RoomDropFieldRings[1].intensity = fx->depth;
    RotMatrix(&seed1, &matrix);

    memset(&scale, 0, 0x10);
    scale.x = fx->scale;
    scale.y = fx->scale;
    scale.z = fx->scale;
    scratch = scale;
    ScaleMatrix(&matrix, &scratch);

    matrix.t[0] = fx->x;
    matrix.t[1] = fx->y;
    matrix.t[2] = fx->z;
    saved_matrix = matrix;
    FieldFx_DrawShadedRing(&g_RoomDropFieldRings[0], &matrix, 0);
    matrix = saved_matrix;
    FieldFx_DrawShadedRing(&g_RoomDropFieldRings[1], &matrix, 0);

    RotMatrix(&seed0, &matrix);
    memset(&second_scale, 0, 0x10);
    second_scale.x = fx->scale * 2;
    second_scale.y = fx->scale * 2;
    second_scale.z = fx->scale * 2;
    scale = second_scale;
    ScaleMatrix(&matrix, &scale);

    matrix.t[0] = fx->x;
    matrix.t[1] = fx->y;
    matrix.t[2] = fx->z;
    saved_matrix = matrix;
    FieldFx_DrawShadedRing(&g_RoomDropFieldRings[0], &matrix, 0);
    matrix = saved_matrix;
    FieldFx_DrawShadedRing(&g_RoomDropFieldRings[1], &matrix, 0);
    return 0;
}

int RoomEffect_UpdateRingPair(void *unused, void *data1, void *data2) {
    u8 * signal = data1;
    RoomRingPairState * state = data2;
    u16 scale;
    u16 secondaryScale;
    u16 step;
    u16 depth;

    scale = state->scale;
    secondaryScale = state->secondaryScale;
    step = state->secondaryScaleStep;
    state->scale = scale + 0x64;
    state->secondaryScale = secondaryScale + step;
    state->secondaryScaleStep = state->secondaryScaleStep + 0x32;
    depth = state->depth - 0x10;
    state->depth = depth;
    if ((short)depth < 0) {
        signal[1] = 2;
        state->depth = 0;
    }
    return 0;
}

int RoomEffect_InitStagedMotion(void *unused0, void *unused1, void *data2) {
    RoomStagedMotionState * state = data2;
    RoomDropFieldState *owner;
    unsigned int i;

    owner = FieldFx_GetScriptContext();
    state->size = 0x80;
    state->fallStep = 0xFFEC0000;
    state->radiusStep = (s32)((u32)owner->initialFallStep << 8);

    for (i = 0; i < 8; i++) {
        state->phase[i] = 0;
        state->radius[i] = 0;
        state->record[i].y = (s32)((u32)state->y << 16);
        state->kind[i] = 1;
        state->depth[i] = 0x80;
    }
    return 0;
}

typedef struct RoomStagedMotionRenderOwner {
    u8 pad00[0x44];
    u8 owner;                         /* 0x44: render owner id */
} RoomStagedMotionRenderOwner;

/* Emitter parameters; only the y origin selects the sprite variant. */
typedef struct RoomStagedMotionEmitter {
    s16 x;
    u16 y;
} RoomStagedMotionEmitter;

/* Rotation seed for RotMatrix; the halfword at 4 is the per-record
 * spin phase. */
typedef struct RoomStagedMotionSeed {
    u8 bytes[4];
    u16 phase;
    u16 pad06;
} RoomStagedMotionSeed;

/* Sprite packet body that follows the 4-byte packet base. */
typedef struct RoomStagedMotionPacket {
    u8 code;
    u8 mode;
    u8 pad02[4];
    u16 depth;
} RoomStagedMotionPacket;

/* Draws every staged motion record: kind 1 is a spinning sprite plus its
 * ground shadow at the frame-counter height, kinds 2..0xA add a flat
 * sprite 200 units up, and every kind >= 2 draws the model scaled by the
 * stage. The loop entry test is written as the counter compare so the
 * retail frame keeps its spare 8-byte slot. */
int RoomEffect_DrawStagedMotion(void *unused, void *data1, void *data2) {
    RoomStagedMotionEmitter * emitter = data1;
    RoomStagedMotionState * state = data2;
    RoomStagedMotionSeed seedA;
    RoomStagedMotionSeed seedB;
    RoomSpriteMatrix matrix;
    RoomSpriteMatrix saved;
    RoomFxVec4 scale;
    RoomFxVec4 scale2;
    RoomFxVec4 scale3;
    RoomDropFieldState *owner;
    RoomStagedMotionPacket *packet;
    u16 *flatDepth;
    u16 *modelDepth;
    int i;
    int variant;
    int handle;

    owner = FieldFx_GetScriptContext();
    seedA = *(RoomStagedMotionSeed *)&s_DropFieldNoTilt;
    seedB = *(RoomStagedMotionSeed *)&s_DropFieldTiltX;
    FieldFx_SetSpriteTexturePreset(owner->asset.page);
    i = 0;
    FieldFx_SetSpriteCellSize(0x20, 0x20);
    FieldFx_SetSpriteColorMode(0x10);

    if ((unsigned int)i < state->count) {
        /* Both packet views are fixed before the loop: the sprite stage
         * then addresses the packet fields through the base and
         * the flat stage through the depth pointer, as retail does. */
        packet = (RoomStagedMotionPacket *)&g_RoomDropFieldMotionSprite.code;
        flatDepth = &packet->depth;
        do {
            if (state->kind[i] == 1) {
                FieldFx_SetSpriteBlendMode(2);
                seedA.phase = state->phase[i];
                RotMatrix(&seedA, &matrix);
                scale = s_DropFieldSpriteScale;
                ScaleMatrix((GteMatrix *)&matrix, (GteVector *)&scale);
                /* The int temporary keeps the remainder in full width. */
                variant = (s16)emitter->y;
                packet->code = (variant % 3) * 2 + 0x44;
                packet->mode = 5;
                packet->depth = state->size;
                matrix.t[0] = (state->record[i].x >> 16) + state->x;
                matrix.t[1] = (state->record[i].y >> 16) + state->y;
                matrix.t[2] = (state->record[i].z >> 16) + state->z;
                FieldFx_DrawGlowSprite((FieldGlowSprite *)&g_RoomDropFieldMotionSprite, &matrix, 1);
                packet->mode = 9;
                RotMatrix(&seedB, &matrix);
                scale2 = s_DropFieldSpriteScale;
                ScaleMatrix((GteMatrix *)&matrix, (GteVector *)&scale2);
                packet->depth = (s16)state->size >> 2;
                matrix.t[0] = (state->record[i].x >> 16) + state->x;
                matrix.t[1] = g_RoomFloorY->y;
                matrix.t[2] = (state->record[i].z >> 16) + state->z;
                FieldFx_DrawGlowSprite((FieldGlowSprite *)&g_RoomDropFieldMotionSprite, &matrix, 0);
                packet->mode = 0xB;
            }
            if (state->kind[i] >= 2) {
                FieldFx_SetSpriteBlendMode(2);
                if (state->kind[i] < 0xB) {
                    FieldFx_SetSpriteColorMode(0x10);
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
                    scale = s_DropFieldFlatScale;
                    ScaleMatrix((GteMatrix *)&matrix, (GteVector *)&scale);
                    *flatDepth = state->size;
                    *((u8 *)flatDepth - 6) = state->kind[i] * 2 + 0x3A;
                    *((u8 *)flatDepth - 5) = 5;
                    matrix.t[0] = (state->record[i].x >> 16) + state->x;
                    matrix.t[1] = (state->record[i].y >> 16) + state->y - 0xC8;
                    matrix.t[2] = (state->record[i].z >> 16) + state->z;
                    FieldFx_DrawGlowSprite((FieldGlowSprite *)((char *)flatDepth - 10), &matrix, 1);
                }
                FieldFx_SetSpriteColorMode(0x100);
                /* Taken here, not before the loop, so the depth pointer
                 * outranks the rematerialised packet base for $fp. */
                modelDepth = &g_RoomDropFieldSpinSprite.depth;
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
                scale = s_DropFieldModelScale;
                ScaleMatrix((GteMatrix *)&matrix, (GteVector *)&scale);
                *modelDepth = state->depth[i];
                matrix.t[0] = (state->record[i].x >> 16) + state->x;
                matrix.t[1] = (state->record[i].y >> 16) + state->y - 0xC8;
                matrix.t[2] = (state->record[i].z >> 16) + state->z;
                saved = matrix;
                FieldFx_DrawGlowSprite((FieldGlowSprite *)((char *)modelDepth - 10), &matrix, 1);
                matrix = saved;
                matrix.t[1] += 0xC8;
                memset(&scale3, 0, 0x10);
                scale3.x = state->kind[i] * 0x20 + 0x20C;
                scale3.y = state->kind[i] * 0x10;
                scale3.z = state->kind[i] * 0x20 + 0x20C;
                scale2 = scale3;
                ScaleMatrix((GteMatrix *)&matrix, (GteVector *)&scale2);
                FieldFx_ModelScrollUv(g_RoomDropFieldModel, 0, 0);
                FieldFx_ModelSetDepthBias(-0x64);
                handle = GetTPage(1, 1, 0x340, 0x100);
                FieldFx_ModelSetTexture(handle, GetClut(0, 0x1DB));
                FieldFx_ModelSetSemiTrans(1);
                FieldFx_ModelSaveColors(g_RoomDropFieldModel);
                FieldFx_ModelScaleColors(g_RoomDropFieldModel,
                              (u16)(state->depth[i] * 2));
                FieldFx_ModelDraw(g_RoomDropFieldModel, &matrix);
                FieldFx_ModelRestoreColors(g_RoomDropFieldModel);
            }
            i++;
        } while ((unsigned int)i < state->count);
    }
    return 0;
}

typedef struct RoomStagedMotionControl {
    u8 pad00;
    u8 state;                         /* 0x01: 2 once every record is spent */
} RoomStagedMotionControl;

typedef GteShortVector RoomStagedMotionPoint;

/* Advances every staged motion record: kind 1 spirals outwards and falls
 * until it leaves the floor polygon or reaches the frame-counter height,
 * spawning two child effects at that point; kinds 2.. count up to 0x14 and
 * fade their depth. Any live record within 200 units flags the owner. The
 * for loop keeps the entry compare on the counter so the retail frame keeps
 * its spare 8-byte slot and the counter wins $s6. */
int RoomEffect_UpdateStagedMotion(void *entity, void *data1, void *data2) {
    RoomStagedMotionControl * control = data1;
    RoomStagedMotionState * state = data2;
    RoomStagedMotionPoint radiusPoint;
    RoomStagedMotionPoint spawnPoint;
    RoomDropFieldState *owner;
    char *spawn;
    int i;

    owner = FieldFx_GetScriptContext();
    state->fallStep += (s32)((u32)owner->fallRate << 8);
    state->radiusStep += (state->radiusStep * owner->radiusRate) >> 12;
    for (i = 0; (unsigned int)i < state->count; i++) {
        if (state->kind[i] == 1) {
            state->phase[i] += 0x64;
            state->angle[i] += 0x14;
            state->radius[i] += state->radiusStep;
            state->record[i].x =
                (s32)((u32)((rsin(state->angle[i]) * (state->radius[i] >> 12)) >> 12) << 16);
            state->record[i].y += state->fallStep;
            state->record[i].z =
                (s32)((u32)((rcos(state->angle[i]) * (state->radius[i] >> 12)) >> 12) << 16);
            spawnPoint.x = (state->record[i].x >> 16) + state->x;
            spawnPoint.y = (state->record[i].y >> 16) + state->y;
            spawnPoint.z = (state->record[i].z >> 16) + state->z;
            if (Geo_PointInPoly((s32)((u32)spawnPoint.x << 16), (s32)((u32)spawnPoint.z << 16),
                                (const PolygonVertex *)g_FloorPolygonVertices,
                                g_FloorPolygonVertexCount) == 0) {
                state->kind[i] = 0;
                state->remaining--;
                if (state->remaining == 0) {
                    control->state = 2;
                }
                spawn = FieldFx_StartSlot(entity, 6,
                                      (unsigned short *)g_RoomDropFieldSpawnScript,
                                      (int (**)(void *, void *, void *))g_RoomDropFieldSpawnData);
                if (spawn != 0) {
                    *(RoomStagedMotionPoint *)spawn = spawnPoint;
                }
                spawn = FieldFx_StartSlot(entity, 6,
                                      (unsigned short *)g_RoomDropFieldSpawnScript,
                                      (int (**)(void *, void *, void *))g_RoomDropFieldSpawnData);
                if (spawn != 0) {
                    *(RoomStagedMotionPoint *)spawn = spawnPoint;
                }
            }
            if (g_RoomFloorY->y < (state->record[i].y >> 16) + state->y) {
                state->kind[i] = 2;
                spawn = FieldFx_StartSlot(entity, 6,
                                      (unsigned short *)g_RoomDropFieldSpawnScript,
                                      (int (**)(void *, void *, void *))g_RoomDropFieldSpawnData);
                if (spawn != 0) {
                    *(RoomStagedMotionPoint *)spawn = spawnPoint;
                }
                spawn = FieldFx_StartSlot(entity, 6,
                                      (unsigned short *)g_RoomDropFieldSpawnScript,
                                      (int (**)(void *, void *, void *))g_RoomDropFieldSpawnData);
                if (spawn != 0) {
                    *(RoomStagedMotionPoint *)spawn = spawnPoint;
                }
            }
        }
        if (state->kind[i] >= 2) {
            state->kind[i]++;
            state->depth[i] -= 8;
            if ((s16)state->depth[i] < 0) {
                state->depth[i] = 0;
            }
            if (state->kind[i] >= 0xB && state->kind[i] == 0x14) {
                state->remaining--;
                if (state->remaining == 0) {
                    control->state = 2;
                }
            }
        }
        if (state->kind[i] != 0) {
            radiusPoint.x = (state->record[i].x >> 16) + state->x;
            radiusPoint.y = (state->record[i].y >> 16) + state->y;
            radiusPoint.z = (state->record[i].z >> 16) + state->z;
            if (FieldFx_IsPlayerWithinRadius(&radiusPoint, 0xC8)) {
                owner->hit = 1;
            }
        }
    }
    return 0;
}

int RoomEffect_InitModelSprite(void *a, void *b, void *data2) {
    RoomDropFieldSprite * sprite = data2;
    sprite->scale = 0xB20;
    sprite->rotation.x = 0;
    sprite->rotation.y = 0;
    sprite->rotation.z = 0;
    sprite->depth = 0xFF;
    return 0;
}

int RoomEffect_DrawModelSprite(void *arg0, void *arg1, void *data2) {
    RoomDropFieldSprite * sprite = data2;
    RoomDropFieldSpinFrame stack;
    s16 *ptr;

    FieldFx_SetSpriteTexturePreset(((RoomDropFieldState *)FieldFx_GetScriptContext())->asset.page);
    FieldFx_SetSpriteCellSize(0x20, 0x20);
    FieldFx_SetSpriteColorMode(0x10);
    FieldFx_SetSpriteBlendMode(2);
    RotMatrix(&sprite->rotation, &stack.sp10);
    memset(&stack.sp40, 0, 0x10);

    {
        s32 x = (s16)sprite->scale;
        stack.sp40.x = x;
    }
    {
        s32 y = (s16)sprite->scale;
        stack.sp40.z = 0x1000;
        stack.sp40.y = y;
    }
    stack.sp30 = stack.sp40;
    ScaleMatrix(&stack.sp10, &stack.sp30);

    ptr = &g_RoomDropFieldModelSprite.depth;
    *ptr = sprite->depth;
    stack.sp24[0] = sprite->x;
    stack.sp24[1] = sprite->y;
    stack.sp24[2] = sprite->z;
    FieldFx_DrawGlowSprite((FieldGlowSprite *)(ptr - 5), (GteMatrix *)&stack.sp10, 1);
    return 0;
}

int RoomEffect_UpdateModelSprite(void *arg0, void *data1, void *data2) {
    RoomDropSlot * slot = data1;
    RoomDropFieldSprite * sprite = data2;
    sprite->depth -= 8;
    sprite->scale += 0x1E;
    sprite->y -= 0xF;

    if ((s16)sprite->depth < 0) {
        sprite->depth = 0;
        slot->command = 2;
    }
    return 0;
}

int RoomEffect_InitIdleSlot(void *unused0, void *unused1, void *unused2) {

    return 0;
}

int RoomEffect_DrawIdleSlot(void *unused0, void *unused1, void *unused2) {

    return 0;
}

int RoomEffect_UpdateIdleSlot(void *unused0, void *unused1, void *unused2) {

    return 0;
}

/* Lays out the sixteen drops in a column above the anchor and starts their
 * sound. */
int RoomEffect_InitDropField(void *arg0, void *arg1, void *data2) {
    RoomDropFieldDrops * state = data2;
    RoomDropFieldState *src;
    u32 i;

    src = FieldFx_GetScriptContext();
    state->anchor = src->matrix0.head;
    state->facing = src->matrix0.tail;
    state->h180 = *FieldFx_RegPtr(1);
    state->h182 = *FieldFx_VarPtr(8);
    state->h184 = 0;

    for (i = 0; i < 16; i++) {
        state->pos[i].x = 0;
        state->pos[i].y = -0x12C;
        state->pos[i].z = i * 0x78;
        state->vel[i].x = 0;
        state->vel[i].y = 0;
        state->vel[i].z = 0;
        state->scale[i] = 0x800;
        state->height[i] = 0x60 - i * 3;
        state->flag[i] = 1;
    }

    FieldFx_FindAssetAtShortPosition(arg0, 0x56D, state->pos);
    return 0;
}

/* Draw every live drop below the release height: the drop sprite at its
 * world position (retired once it leaves the walkable polygon; touching
 * the player flags the actor) and its shadow on the floor. */
int RoomEffect_DrawDropField(void *object, void *slot, void *data2) {
    RoomDropField * field = data2;
    GteMatrix matrix;
    GteMatrix saved;
    GteMatrix turn;
    RoomDropFieldState *actor = FieldFx_GetScriptContext();
    GteShortVector spin = s_DropFieldNoTilt;
    GteShortVector shadowSpin = s_DropFieldTiltX;
    GteShortVector rotation;
    GteShortVector point;
    GteVector scale;
    unsigned int i;

    FieldFx_SetSpriteTexturePreset(actor->asset.page);
    FieldFx_SetSpriteCellSize(0x20, 0x20);
    FieldFx_SetSpriteColorMode(0x100);
    FieldFx_SetSpriteBlendMode(2);
    rotation.x = 0x20;
    rotation.y = field->yaw;
    rotation.z = 0;
    RotMatrix(&rotation, &turn);
    turn.t[0] = 0;
    turn.t[1] = 0;
    turn.t[2] = 0;
    for (i = 0; i < 16; i++) {
        if (field->live[i] == 1 && field->drops[i].z < -0x28A) {
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
            matrix.t[0] = field->drops[i].x;
            matrix.t[1] = field->drops[i].y;
            matrix.t[2] = field->drops[i].z;
            gte_CompMatrix(&turn, &matrix, &matrix);
            gte_CompMatrix(&field->matrix, &matrix, &matrix);
            RotMatrix(&spin, &matrix);
            scale = (GteVector){ field->size[i], field->size[i], field->size[i] };
            ScaleMatrix(&matrix, &scale);
            g_RoomDropFieldDropSprite.depth = field->depth[i];
            saved = matrix;
            point.x = matrix.t[0];
            point.y = matrix.t[1];
            point.z = matrix.t[2];
            if (Geo_PointInPoly((s32)((u32)matrix.t[0] << 16), (s32)((u32)matrix.t[2] << 16), g_FloorPolygonVertices, g_FloorPolygonVertexCount) == 0) {
                field->live[i] = 0;
            }
            if (FieldFx_IsPlayerWithinRadius(&point, 200) != 0) {
                actor->hit = 1;
            }
            FieldFx_DrawGlowSprite((FieldGlowSprite *)&g_RoomDropFieldDropSprite, &matrix, 1);
            matrix = saved;
            RotMatrix(&shadowSpin, &matrix);
            matrix.t[1] = g_RoomFloorY->y;
            {
                GteVector shadow = *(GteVector *)&s_DropFieldShadowScale;

                ScaleMatrix(&matrix, &shadow);
            }
            g_RoomDropFieldDropSprite.depth = field->depth[i] >> 1;
            FieldFx_DrawGlowSprite((FieldGlowSprite *)&g_RoomDropFieldDropSprite, &matrix, 0);
        }
    }
    return 0;
}

/* Sink every drop; when the script step reaches a drop and flag 7 is set,
 * retire it and, for the first one, spawn the splash at its world
 * position (drop offset turned by the field yaw and the field matrix). */
int RoomEffect_UpdateDropField(void *object, void *data1, void *data2) {
    RoomDropSlot * slot = data1;
    RoomDropField * field = data2;
    GteMatrix matrix;
    GteMatrix turn;
    GteShortVector rotation;
    RoomDropPoint *splash;
    unsigned int i;

    for (i = 0; i < 16; i++) {
        field->drops[i].z -= 0x96;
        if (slot->step == field->first_step + i && *FieldFx_VarPtr(7) == 1) {
            field->live[i] = 0;
            if (++field->released == 1) {
                rotation.x = 0x20;
                rotation.y = field->yaw;
                rotation.z = 0;
                RotMatrix(&rotation, &turn);
                turn.t[0] = 0;
                turn.t[1] = 0;
                turn.t[2] = 0;
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
                matrix.t[0] = field->drops[i].x;
                matrix.t[1] = field->drops[i].y;
                matrix.t[2] = field->drops[i].z;
                gte_CompMatrix(&turn, &matrix, &matrix);
                gte_CompMatrix(&field->matrix, &matrix, &matrix);
                splash = FieldFx_StartSlot(object, 1, (unsigned short *)g_RoomDropFieldSpawnScript,
                                       (int (**)(void *, void *, void *))g_RoomDropFieldSpawnData);
                if (splash != 0) {
                    splash->x = matrix.t[0];
                    splash->y = matrix.t[1];
                    splash->z = matrix.t[2];
                }
            }
        }
    }
    if (slot->step >= 0x79) {
        slot->command = 2;
    }
    return 0;
}

int RoomEffect_InitBeamPair(void *unused0, void *unused1, void *output_arg) {

    RoomBeamPair *beam = output_arg;
    unsigned char *image;
    short *anchor;
    int mode;
    int offset;
    int intensity;
    int primary_color;
    int blue;
    int extent;
    RoomFxEmitterParams *params;
    int white;
    int primary_extent;
    FieldActor **context;
    FieldActor *actor;
    RenderMatrix *model;

    context = FieldFx_GetScriptContext();
    anchor = &g_RoomDropFieldRings[2].mode;
    params = &g_RoomDropFieldRings[2];
    mode = 0x10;
    offset = -500;
    intensity = 0x80;
    primary_color = 250;
    actor = *context;
    blue = 120;
    model = actor->render_object.matrices;
    extent = 1400;
    memcpy(&beam->matrix,&model[5],sizeof(beam->matrix));
    beam->scale = 1200;
    beam->brightness = 255;

    primary_extent = 3200;
    image = g_RoomDropFieldBeamImages;
    *anchor = mode;
    g_RoomDropFieldRings[2].offset = offset;
    g_RoomDropFieldRings[2].intensity = intensity;
    g_RoomDropFieldRings[2].color1[0] = primary_color;
    g_RoomDropFieldRings[2].color1[1] = primary_color;
    g_RoomDropFieldRings[2].color1[2] = blue;
    g_RoomDropFieldRings[2].color0[0] = 0;
    g_RoomDropFieldRings[2].color0[1] = 0;
    g_RoomDropFieldRings[2].color0[2] = 0;
    g_RoomDropFieldRings[2].extent0 = primary_extent;
    g_RoomDropFieldRings[2].extent1 = extent;
    params->source = image;
    FieldFx_BuildRingGeometry(params);

    params = &g_RoomDropFieldRings[3];
    white = 255;
    image += 0x100;
    g_RoomDropFieldRings[3].mode = mode;
    g_RoomDropFieldRings[3].offset = offset;
    g_RoomDropFieldRings[3].intensity = intensity;
    g_RoomDropFieldRings[3].color1[0] = white;
    g_RoomDropFieldRings[3].color1[1] = white;
    g_RoomDropFieldRings[3].color1[2] = white;
    g_RoomDropFieldRings[3].color0[0] = primary_color;
    g_RoomDropFieldRings[3].color0[1] = primary_color;
    g_RoomDropFieldRings[3].color0[2] = blue;
    g_RoomDropFieldRings[3].extent1 = 0;
    g_RoomDropFieldRings[3].extent0 = extent;
    params->source = image;
    FieldFx_BuildRingGeometry(params);
    return 0;
}

/* Draw both beam halves at the script actor's anchor, offset 100 up and
 * 100 back, spun by the room rotation and scaled uniformly. */
int RoomEffect_DrawBeamPair(void *object, void *slot, void *data2) {
    RoomBeamPair * beam = data2;
    GteMatrix matrix;
    GteMatrix second;
    FieldActor **actor = FieldFx_GetScriptContext();
    GteShortVector spin = s_DropFieldNoTilt;
    GteVector scale;

    memcpy(&beam->matrix,&(*actor)->render_object.matrices[5],sizeof(beam->matrix));
    g_RoomDropFieldRings[2].intensity = beam->brightness;
    g_RoomDropFieldRings[3].intensity = beam->brightness;
    FieldFx_SetSpriteBlendMode(2);
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
    matrix.t[0] = 0;
    matrix.t[1] = 100;
    matrix.t[2] = -100;
    gte_CompMatrix(&beam->matrix, &matrix, &matrix);
    RotMatrix(&spin, &matrix);
    scale = (GteVector){ beam->scale, beam->scale, beam->scale };
    ScaleMatrix(&matrix, &scale);
    second = matrix;
    FieldFx_DrawShadedRing(&g_RoomDropFieldRings[2], &matrix, 1);
    FieldFx_DrawShadedRing(&g_RoomDropFieldRings[3], &second, 1);
    return 0;
}

int RoomEffect_UpdateBeamPair(void *arg0, void *data1, void *data2) {
    RoomDropSlot * slot = data1;
    RoomBeamPair * beam = data2;
    RoomBeamPair *obj = beam;
    u16 value;

    value = beam->scale - 0x96;
    beam->scale = value;
    if ((s16)value < 0x190) {
        beam->scale = 0x190;
    }

    if (slot->step >= 8) {
        value = obj->brightness - 0x10;
        obj->brightness = value;
        if ((s16)value < 0) {
            obj->brightness = 0;
            slot->command = 2;
        }
    }
    return 0;
}

int RoomEffect_InitQuadSprites(void *unused0, void *unused1, void *output_arg) {

    RoomFourSpriteState *base = output_arg;
    int index;
    int red;
    int blue;
    int mid;
    RoomDropFieldState *clock;
    unsigned int speed;
    int value;
    int hasNonzeroIndex;

    clock = FieldFx_GetScriptContext();
    *(RoomDropFieldMatrixWords *)base = clock->matrix0;

    index = 0;
    red = 159;
    blue = 137;
    mid = 64;

    for (; (unsigned int)index < 4; index++) {
        speed = rand() % 40 + 10;
        rand();

        value = -200;
        base->position[index].y = value;
        base->position[index].z = value;
        value = -(speed >> 1);
        speed <<= 6;
        base->velocity[index].x = value;
        base->velocity[index].y = value;
        value = 160;
        base->position[index].x = 0;
        base->depth[index] = value;
        base->scale[index] = speed;

        if (index != 1) {
            hasNonzeroIndex = index != 0;
            if (hasNonzeroIndex) {
                if (index == 2) goto color2;
                if (index == 3) goto color3;
                continue;
            } else {
                base->color[index].r = red;
                base->color[index].g = 138;
                base->color[index].b = blue;
                continue;
            }
        } else {
            base->color[1].r = mid;
            base->color[1].g = 76;
            base->color[1].b = 6;
            continue;
        }
color2:
        base->color[2].r = 18;
        base->color[2].g = 32;
        base->color[2].b = mid;
        continue;
color3:
        base->color[3].r = red;
        base->color[3].g = 98;
        base->color[3].b = blue;
    }
    return 0;
}

typedef struct RoomFourSpriteStack {
    RoomFxSeed8 seed;
    RoomSpriteMatrix matrix;
    RoomFxVec4 scale;
    RoomFxVec4 sourceScale;
} RoomFourSpriteStack;

/* Draws four scaled sprites. The state begins with the shared rotation
 * matrix; each sprite reads its position, scale, depth and color from the
 * arrays that follow it. Retail marks this function handwritten; only its
 * COP2 windows remain ASM. */
int RoomEffect_DrawQuadSprites(void *unused0, void *unused1, void *data2) {
    RoomFourSpriteState * state = data2;
    RoomFourSpriteStack stack;
    RoomDropFieldState *clock;
    unsigned int i;
    /* Retail keeps the matrix base in s3 and the second column in s7; the
     * three sprite-array cursors are strength-reduced copies the compiler
     * places in s0..s2 after them. */
    RoomSpriteMatrix *matrix;
    short *column1;
    short *work;
    unsigned short *depthSlot;

    clock = FieldFx_GetScriptContext();
    stack.seed = *(RoomFxSeed8 *)&s_DropFieldNoTilt;
    i = 0;
    matrix = &stack.matrix;
    column1 = &stack.matrix.m[0][1];
    depthSlot = &g_RoomDropFieldQuadSprite.depth;
    FieldFx_SetSpriteTexturePreset(clock->asset.page);
    FieldFx_SetSpriteCellSize(0x20, 0x20);
    FieldFx_SetSpriteColorMode(0x100);
    FieldFx_SetSpriteBlendMode(2);

    for (i = 0; i < 4; i++) {
        stack.matrix.m[2][2] = 0x1000;
        stack.matrix.m[1][1] = 0x1000;
        stack.matrix.m[0][0] = 0x1000;
        stack.matrix.t[2] = 0;
        stack.matrix.t[1] = 0;
        stack.matrix.t[0] = 0;
        stack.matrix.m[2][1] = 0;
        stack.matrix.m[2][0] = 0;
        stack.matrix.m[1][2] = 0;
        stack.matrix.m[1][0] = 0;
        stack.matrix.m[0][2] = 0;
        stack.matrix.m[0][1] = 0;
        stack.matrix.t[0] = state->position[i].x;
        stack.matrix.t[1] = state->position[i].y;
        stack.matrix.t[2] = state->position[i].z;

        gte_ldrotmatrix(&state->rotation);
        gte_ldrtir12_matrix_column(matrix);
        gte_stir123_column(matrix);
        gte_ldrtir12_matrix_column(column1);
        gte_stir123_column(column1);
        work = &stack.matrix.m[0][2];
        gte_ldrtir12_matrix_column(work);
        gte_stir123_column(work);
        gte_ldtransmatrix(&state->rotation);
        {
            s32 *translation = stack.matrix.t;

            gte_ldv0_word3(translation);
            gte_cop2_hazard_slot();
            gte_cop2_hazard_slot();
            gte_mvmva_rotation_v0_translation_sf12();
            gte_swc2_9_0(translation);
            gte_swc2_10_4(translation);
            gte_swc2_11_8(translation);
        }

        RotMatrix(&stack.seed, matrix);
        memset(&stack.sourceScale, 0, 0x10);
        stack.sourceScale.x = state->scale[i];
        stack.sourceScale.y = state->scale[i];
        stack.sourceScale.z = state->scale[i];
        stack.scale = stack.sourceScale;
        ScaleMatrix(matrix, &stack.scale);
        *depthSlot = state->depth[i];
        ((u8 *)depthSlot)[-0xA] = state->color[i].r;
        ((u8 *)depthSlot)[-0x9] = state->color[i].g;
        ((u8 *)depthSlot)[-0x8] = state->color[i].b;
        FieldFx_DrawGlowSprite((FieldGlowSprite *)((char *)depthSlot - 0xA), matrix, 1);
    }
    return 0;
}

int RoomEffect_UpdateQuadSprites(void *unused, void *data1, void *data2) {
    RoomDropSlot * slot = data1;
    RoomFourSpriteState * p = data2;
    unsigned int i;

    for (i = 0; i < 4; i++) {

        p->position[i].x += (u16)p->velocity[i].x;
        p->position[i].y += (u16)p->velocity[i].y;
        p->position[i].z += (u16)p->velocity[i].z;
        p->depth[i] -= 8;
        if ((short)p->depth[i] < 0) {
            p->depth[i] = 0;
        }
    }

    if (slot->step >= 0x3D) {
        slot->command = 2;
    }
    return 0;
}

int RoomEffect_InitIdleSlot2(void *unused0, void *unused1, void *unused2) {

    return 0;
}

int RoomEffect_DrawIdleSlot2(void *unused0, void *unused1, void *unused2) {

    return 0;
}

int RoomEffect_UpdateIdleSlot2(void *unused0, void *unused1, void *unused2) {

    return 0;
}

int RoomEffect_InitEightParticles(void *data0, void *data1, void *data2) {
    void* arg0 = data0;
    void* arg1 = data1;
    RoomEightParticleState * state = data2;
    unsigned int i;

    state->activeCount = 0;

    for (i = 0; i < 8; i++) {
        state->active[i] = 0;
    }
    return 0;
}

int RoomEffect_DrawEightParticles(void *unused0, void *unused1, void *data2) {
    RoomEightParticleState * stateArg = data2;
    RoomEightParticleState *state;
    unsigned int i;
    int unitScale;
    u16 *depthOut;
    char *packet;
    RoomSpriteMatrix matrix;
    GteVector scaleArg;
    volatile GteVector sourceScale;
    RoomSpriteMatrix *matrixArg;
    int scale2;
    int scale3;
    RoomDropFieldState *context;
    int secondScale;

    state = stateArg;
    context = FieldFx_GetScriptContext();
    i = 0;
    unitScale = 0x1000;
    depthOut = &g_RoomDropFieldParticleSprite.depth;
    packet = (char *)depthOut - 10;

    FieldFx_SetSpriteTexturePreset(context->asset.page);
    FieldFx_SetSpriteCellSize(0x20, 0x20);
    FieldFx_SetSpriteColorMode(0x10);
    FieldFx_SetSpriteBlendMode(2);

    for (; i < 8; i++) {
        if (state->active[i] != 0) {
            matrix.m[2][2] = unitScale;
            matrix.m[1][1] = unitScale;
            matrix.m[0][0] = unitScale;
            matrix.t[2] = 0;
            matrix.t[1] = 0;
            matrix.t[0] = 0;
            matrix.m[2][1] = 0;
            matrix.m[2][0] = 0;
            matrix.m[1][2] = 0;
            matrix.m[1][0] = 0;
            matrix.m[0][2] = 0;
            matrix.m[0][1] = 0;

            memset((void *)&sourceScale, 0, sizeof(sourceScale));
            sourceScale.x = (s16)state->scale[i];
            matrixArg = &matrix;
            secondScale = (s16)state->scale[i];
            sourceScale.z = unitScale;
            sourceScale.y = secondScale;
            scaleArg.x = sourceScale.x;
            scaleArg.y = sourceScale.y;
            scale2 = sourceScale.z;
            scale3 = sourceScale.pad;
            scaleArg.z = scale2;
            scaleArg.pad = scale3;
            ScaleMatrix(matrixArg, &scaleArg);

            *depthOut = (u16)state->fade[i];
            matrix.t[0] = (s16)state->position[i].x;
            matrix.t[1] = (s16)state->position[i].y;
            matrix.t[2] = (s16)state->position[i].z;
            FieldFx_InterpolateColorRamp(g_RoomDropFieldParticleFrames,
                          (s16)state->angle[i], packet);
            FieldFx_DrawGlowSprite((FieldGlowSprite *)packet, &matrix, 1);
        }
    }
    return 0;
}

int RoomEffect_UpdateEightParticles(void *unused, void *data1, void *data2) {
    RoomEightParticleControl * control = data1;
    RoomEightParticleState * stateArg = data2;
    RoomEightParticleControl *clock;
    unsigned int i;
    RoomEightParticleContext *context;
    const GteMatrix *matrix;
    RoomFxSeed8 seed0;
    RoomFxSeed8 seed1;
    RoomFxSeed8 seed2;
    RoomFxSeed8 transformed0;
    RoomFxSeed8 transformed1;
    RoomFxSeed8 transformed2;
    RoomEightParticleContext *contextResult;
    RoomFxTransform *matrixBase;
    int frame;
    s16 slot;
    int random;
    RoomEightParticleState *particles;

    clock = control;
    particles = stateArg;
    contextResult = FieldFx_GetScriptContext();
    i = 0;
    context = contextResult;
    matrixBase = context->root->transforms;
    seed0 = *(RoomFxSeed8 *)&s_EightParticleOffset;
    seed1 = *(RoomFxSeed8 *)&s_EightParticleStepRight;
    seed2 = *(RoomFxSeed8 *)&s_EightParticleStepLeft;
    matrix = (const GteMatrix *)&matrixBase[5];

    for (; i < 8; i++) {
        if (particles->active[i] == 0) {
            frame = clock->frame;
            slot = frame % 8;
            if (slot == i && frame < 90) {
                particles->active[i] = 1;
                particles->activeCount++;
                ApplyMatrixSV(matrix,
                              (const GteShortVector *)&seed0,
                              (GteShortVector *)&transformed0);
                ApplyMatrixSV(matrix,
                              (const GteShortVector *)&seed1,
                              (GteShortVector *)&transformed1);
                ApplyMatrixSV(matrix,
                              (const GteShortVector *)&seed2,
                              (GteShortVector *)&transformed2);

                particles->position[i].x =
                    (u16)transformed0.vector.x + context->baseX;
                particles->position[i].y =
                    (u16)transformed0.vector.y + context->baseY;
                particles->position[i].z =
                    (u16)transformed0.vector.z + context->baseZ;

                random = rand();
                if ((random & 1) == 0) {
                    particles->velocity[i] = transformed2;
                } else {
                    particles->velocity[i] = transformed1;
                }
                particles->scale[i] = 0x800;
                particles->fade[i] = 0x80;
                particles->angle[i] = 0;
            }
        } else {
            int randomValue;
            s16 yValue;
            s16 zVelocity;
            s16 zPosition;
            s16 zValue;

            particles->position[i].x +=
                (u16)particles->velocity[i].vector.x;
            randomValue = rand();
            randomValue %= 20;
            zPosition = particles->position[i].z;
            zVelocity = (u16)particles->velocity[i].vector.z;
            yValue = particles->position[i].y;
            yValue -= 20;
            zValue = zPosition + zVelocity;
            particles->position[i].z = zValue;
            yValue -= randomValue;
            particles->position[i].y = yValue;
            particles->fade[i] -= 8;
            (particles->angle[i])++;
            if (particles->fade[i] <= 0) {
                particles->fade[i] = 0;
                particles->active[i] = 0;
                particles->activeCount--;
            }
        }

    }

    if (particles->activeCount == 0 && clock->frame >= 91) {
        clock->state = 2;
    }
    return 0;
}

FieldFxHandler g_RoomDropFieldSpawnData[14] = {
    RoomEffect_DropFieldInit,
    RoomEffect_InitStageSpawn,
    RoomEffect_InitSpinSprite,
    RoomEffect_InitDoubleSprite,
    RoomEffect_InitRingPair,
    RoomEffect_InitStagedMotion,
    RoomEffect_InitModelSprite,
    RoomEffect_InitIdleSlot,
    RoomEffect_InitDropField,
    RoomEffect_InitBeamPair,
    RoomEffect_InitQuadSprites,
    RoomEffect_InitIdleSlot2,
    RoomEffect_InitEightParticles,
    (FieldFxHandler)(intptr_t)-1};

FieldFxHandler g_RoomDropFieldDrawList[14] = {
    RoomEffect_DropFieldNop,
    RoomEffect_DrawStageSpawn,
    RoomEffect_DrawSpinSprite,
    RoomEffect_DrawDoubleSprite,
    RoomEffect_DrawRingPair,
    RoomEffect_DrawStagedMotion,
    RoomEffect_DrawModelSprite,
    RoomEffect_DrawIdleSlot,
    RoomEffect_DrawDropField,
    RoomEffect_DrawBeamPair,
    RoomEffect_DrawQuadSprites,
    RoomEffect_DrawIdleSlot2,
    RoomEffect_DrawEightParticles,
    (FieldFxHandler)(intptr_t)-1};

FieldFxHandler g_RoomDropFieldUpdateList[14] = {
    RoomEffect_DropFieldTick,
    RoomEffect_UpdateStageSpawn,
    RoomEffect_UpdateSpinSprite,
    RoomEffect_UpdateDoubleSprite,
    RoomEffect_UpdateRingPair,
    RoomEffect_UpdateStagedMotion,
    RoomEffect_UpdateModelSprite,
    RoomEffect_UpdateIdleSlot,
    RoomEffect_UpdateDropField,
    RoomEffect_UpdateBeamPair,
    RoomEffect_UpdateQuadSprites,
    RoomEffect_UpdateIdleSlot2,
    RoomEffect_UpdateEightParticles,
    (FieldFxHandler)(intptr_t)-1};

u16 g_RoomDropFieldSpawnScript[14] = {sizeof(RoomDropFieldState),8,24,24,24,240,20,4,392,56,132,96,188,65535};
int g_RoomDropFieldScript[48] = {
    0x00010000,0x00100000,0x00410007,0x11030000,0x11140001,0x111e0002,
    0x30280000,0x100f0000,0x00410004,0x1008001a,0x00410005,0x10060033,
    0x00410006,0x10040047,0x10200064,0x00200001,0x00020000,0x30f50000,
    0x00010009,0x0001000a,0x00020002,0x00010008,0x30f80000,0x0001000c,
    0x30f60000,0x00110000,0x1004001e,0x1013001f,0x00200001,0x00020000,
    0x30fa0000,0x00010009,0x0001000a,0x00020002,0x00010008,0x30f80000,
    0x00110000,0x1004001e,0x1008001f,0x00200001,0x00020000,0x30fa0000,
    0x00010009,0x0001000a,0x00020002,0x00010008,0x30f80000,0xffffffff,
};
PmCommand g_RoomDropFieldHandlers = {RoomEffect_DropFieldNop0,RoomEffect_DropFieldPlantScript,
    RoomEffect_DropFieldSpawn6,RoomEffect_DropFieldRegister,RoomEffect_DropFieldStart,
    RoomEffect_DropFieldClose,RoomEffect_DropFieldNop6};
RoomFxSpritePacket g_RoomDropFieldSpinSprite,g_RoomDropFieldDoubleSprite,g_RoomDropFieldModelSprite;
RoomFxSpritePacket g_RoomDropFieldDropSprite,g_RoomDropFieldQuadSprite,g_RoomDropFieldMotionSprite,g_RoomDropFieldParticleSprite;
void *g_RoomDropFieldModel;
RoomFxEmitterParams g_RoomDropFieldRings[4];
_Alignas(4) char g_RoomDropFieldRingImages[512];
_Alignas(4) unsigned char g_RoomDropFieldBeamImages[512];
u8 g_RoomDropFieldParticleFrames[20] = {250,180,100,10,180,120,10,25,80,40,20,10,40,0,0,10,0,0,0,255};
