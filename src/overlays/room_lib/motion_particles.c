/*
 * The motion particle set, part 1 of 3: the setup, the timer and anchor handlers, the
 * anchored sprite quad and pair with their step handlers, and the particle
 * seeding.
 *
 * room_m075, room_m080, room_m082, scene_e09 and scene_e10 link the same
 * fifteen functions in the same order, after the room prelude and before
 * the room library, with the same 0x18 bytes of read-only data. The object
 * is split in three units only because its particle draw needs
 * -fno-strength-reduce, which the setup loop must not get; they are
 * merged once they compile together. Trails, glyphs and lookup codes live
 * in each room's own data (pe1/room_motion_init.h).
 */

#include "common.h"
#include "pe1/field_script_context.h"
#include "pe1/gte.h"
#include "pe1/psyq_libc.h"
#include "pe1/room_floor.h"
#include "pe1/room_fx.h"
#include "pe1/room_motion_init.h"
#include "pe1/room_rearm_timer.h"

#include "motion_particles.h"
#include "render_layouts.h"
#include "room_lib.h"

static const RoomFxSeed8 s_MotionQuadSeed = { { 0, 4, 0, 0, 0, 0, 0, 0 } };

/* Work area of the anchored sprite quad and pair: the uniform scale, the
 * sprite depth (the pair's ring intensity), the ring image source and the
 * ring the pair draws. */
typedef struct RoomAnchoredSpriteState {
    u8 pad00[0x10];
    s16 scale;                    /* 0x10 */
    u16 depth;                    /* 0x12 */
    u8 sourceData[0x100];         /* 0x14 */
    FieldRingGeometry ring;       /* 0x114 */
} RoomAnchoredSpriteState;

/* Motion particle setup: the two trails of ten particles and the four
 * glyphs. */
void RoomEffect_MotionParticlesInit(RoomMotionInitOwner *owner, void *unused, RoomMotionInitState *state) {
    RoomMotionInitLink *link;
    RoomMotionParticle *particle;
    unsigned int i;

    FieldFx_SetScriptContext(state);
    state->allocation = Asset_SearchByKeyType(12);
    link = owner->link;
    state->link = link;
    state->base = link->model->base;
    state->primary = state->link->model->primary;
    state->secondary = state->link->model->secondary;
    state->timer = 30;
    state->field66 = 0;
    state->field68 = 0;

    g_RoomMotionGlyphs[1].cell = 44;
    g_RoomMotionGlyphs[1].clut = 1;
    g_RoomMotionGlyphs[1].offset = -30;
    g_RoomMotionGlyphs[1].depth = 0;
    g_RoomMotionGlyphs[1].r = 128;
    g_RoomMotionGlyphs[1].g = 128;
    g_RoomMotionGlyphs[1].b = 128;
    g_RoomMotionGlyphs[1].flip = 0;

    g_RoomMotionGlyphs[2].cell = 44;
    g_RoomMotionGlyphs[2].clut = 1;
    g_RoomMotionGlyphs[2].offset = 0;
    g_RoomMotionGlyphs[2].depth = 32;
    g_RoomMotionGlyphs[2].r = 128;
    g_RoomMotionGlyphs[2].g = 128;
    g_RoomMotionGlyphs[2].b = 128;
    g_RoomMotionGlyphs[2].flip = 0;

    g_RoomMotionGlyphs[0].cell = 44;
    g_RoomMotionGlyphs[0].clut = 1;
    g_RoomMotionGlyphs[0].offset = -30;
    g_RoomMotionGlyphs[0].depth = 128;
    g_RoomMotionGlyph.offset = -31;
    g_RoomMotionGlyphs[3].cell = 32;
    g_RoomMotionGlyphs[0].r = 128;
    g_RoomMotionGlyphs[0].g = 128;
    g_RoomMotionGlyphs[0].b = 128;
    g_RoomMotionGlyphs[0].flip = 0;

    g_RoomMotionGlyph.cell = 0;
    g_RoomMotionGlyph.clut = 1;
    g_RoomMotionGlyph.depth = 128;
    g_RoomMotionGlyph.r = 128;
    g_RoomMotionGlyph.g = 128;
    g_RoomMotionGlyph.b = 128;
    g_RoomMotionGlyph.flip = 0;

    g_RoomMotionGlyphs[3].clut = 1;
    g_RoomMotionGlyphs[3].offset = 100;
    g_RoomMotionGlyphs[3].depth = 128;
    g_RoomMotionGlyphs[3].r = 128;
    g_RoomMotionGlyphs[3].g = 128;
    g_RoomMotionGlyphs[3].b = 128;
    g_RoomMotionGlyphs[3].flip = 0;

    for (i = 0; i < 10; i++) {
        particle = &g_RoomMotionTrailA[i];
        particle->mode = 4;
        particle->alpha = 128;
        particle->kind = 4;
        particle->life = 30;
        particle->speed = 0;
        if (i == 0) particle->drift = -64;
        else particle->drift = 0;
        particle->red = 255;
        particle->green = 255;
        particle->blue = 255;
        particle = &g_RoomMotionTrailB[i];
        particle->mode = 4;
        particle->alpha = 128;
        particle->kind = 4;
        particle->life = 30;
        particle->speed = 0;
        if (i == 0) particle->drift = 64;
        else particle->drift = 0;
        particle->red = 255;
        particle->green = 255;
        particle->blue = 255;
    }
}

void RoomEffect_MotionParticlesNop(void) {
}

ROOM_REARM_TIMER_TICK(RoomEffect_MotionParticlesTick, 0x64)

void RoomEffect_MotionParticlesAnchor(int a0, int a1, short *out) {
    RoomMotionInitState *ctx = FieldFx_GetScriptContext();

    out[0] = ctx->primary.t[0];
    out[1] = ctx->primary.t[1];
    out[2] = ctx->primary.t[2];
    out[4] = ctx->secondary.t[0];
    out[5] = ctx->secondary.t[1];
    out[6] = ctx->secondary.t[2];
    out[9] = 0;
    out[8] = 0x400;
}

typedef struct RoomLibSpriteQuadStack {
    RoomSpriteMatrix scaleMatrix;
    RoomFxSeed8 seed;
    RoomSpriteMatrix matrix;
    RoomFxVec4 scale;
    RoomFxVec4 sourceScale;
} RoomLibSpriteQuadStack;

/* Builds a uniformly scaled rotation from the shared seed and draws the
 * sprite twice at each of two map anchors: once unrotated at the anchor
 * height and once rotated on the shared floor height. Retail marks this
 * function handwritten; only its COP2 windows remain ASM. Like
 * RoomEffect_DrawAnchoredSpritePair, the matrix pointer is still pinned.
 * RotMatrix receives the stack addresses directly; the pointer used by
 * the following GTE operations starts its lifetime after that call. */
void RoomEffect_DrawAnchoredSpriteQuad(void *unused0, void *unused1,
                                            char *params) {
    RoomLibSpriteQuadStack stack;
    /* The params and packet pointers share one pinned variable. */
    char *fx;
    RoomMotionInitModel *map;
    RoomSpriteMatrix *scaleMatrix;
    RoomSpriteMatrix *matrix;
    RoomMotionContext *root;
    short *floorY;
    int sprite;

    fx = params;
    root = FieldFx_GetScriptContext();
    stack.seed = s_MotionQuadSeed;
    map = ((RoomMotionInitLink *)root->link)->model;
    FieldFx_SetSpriteTexturePreset(root->owner);
    FieldFx_SetSpriteCellSize(0x20, 0x20);
    FieldFx_SetSpriteColorMode(0x10);
    FieldFx_SetSpriteBlendMode(2);

    stack.scaleMatrix.m[2][2] = 0x1000;
    stack.scaleMatrix.m[1][1] = 0x1000;
    stack.scaleMatrix.m[0][0] = 0x1000;
    stack.scaleMatrix.t[2] = 0;
    stack.scaleMatrix.t[1] = 0;
    stack.scaleMatrix.t[0] = 0;
    stack.scaleMatrix.m[2][1] = 0;
    stack.scaleMatrix.m[2][0] = 0;
    stack.scaleMatrix.m[1][2] = 0;
    stack.scaleMatrix.m[1][0] = 0;
    stack.scaleMatrix.m[0][2] = 0;
    stack.scaleMatrix.m[0][1] = 0;
    memset(&stack.sourceScale, 0, 0x10);

    stack.sourceScale.x = ((RoomAnchoredSpriteState *)fx)->scale;
    stack.sourceScale.y = ((RoomAnchoredSpriteState *)fx)->scale;
    stack.sourceScale.z = ((RoomAnchoredSpriteState *)fx)->scale;
    stack.scale = stack.sourceScale;
    ScaleMatrix(&stack.scaleMatrix, &stack.scale);
    RotMatrix(&stack.seed.vector, &stack.matrix);
    matrix = &stack.matrix;

    scaleMatrix = &stack.scaleMatrix;
    gte_ldrotmatrix(scaleMatrix);
    gte_ldrtir12_matrix_column(&stack.matrix.m[0][0]);
    gte_stir123_column(matrix);
    gte_ldrtir12_matrix_column(&stack.matrix.m[0][1]);
    gte_stir123_column_at(&stack.matrix.m[0][1]);
    gte_ldrtir12_matrix_column(&stack.matrix.m[0][2]);
    gte_stir123_column_at(&stack.matrix.m[0][2]);
    gte_ldtransmatrix(scaleMatrix);
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

    /* Retail reuses the params for the packet pointers. */
    sprite = ((RoomAnchoredSpriteState *)fx)->depth;
    fx = (char *)&g_RoomMotionQuadDepth;
    *(u16 *)fx = sprite;
    stack.scaleMatrix.t[0] = map->primary.t[0];
    stack.scaleMatrix.t[1] = map->primary.t[1];
    fx -= 10;
    stack.scaleMatrix.t[2] = map->primary.t[2];
    FieldFx_DrawGlowSprite((FieldGlowSprite *)fx, scaleMatrix, 1);
    stack.scaleMatrix.t[0] = map->secondary.t[0];
    stack.scaleMatrix.t[1] = map->secondary.t[1];
    stack.scaleMatrix.t[2] = map->secondary.t[2];
    FieldFx_DrawGlowSprite((FieldGlowSprite *)fx, scaleMatrix, 1);

    fx = (char *)&g_RoomMotionQuadPacket;
    floorY = &g_RoomFloorY->y;
    stack.matrix.t[0] = map->primary.t[0];
    stack.matrix.t[1] = *floorY;
    stack.matrix.t[2] = map->primary.t[2];
    FieldFx_DrawGlowSprite((FieldGlowSprite *)fx, matrix, 1);
    stack.matrix.t[0] = map->secondary.t[0];
    stack.matrix.t[1] = *floorY;
    stack.matrix.t[2] = map->secondary.t[2];
    FieldFx_DrawGlowSprite((FieldGlowSprite *)fx, matrix, 1);
}

void RoomEffect_UpdateTimedFade(void *arg0, RoomFxControl *state, RoomAnchoredSpriteState *obj) {
    RoomAnchoredSpriteState *p = obj;
    if ((unsigned short)((unsigned short)state->frame - 1) < 7U) {
        obj->depth += 0x10;
    }
    if ((unsigned short)((unsigned short)state->frame - 0x15) < 0x1DU) {
        obj->scale += 0x28;
    }
    if (state->frame >= 0x33 && (s16)p->depth >= 0x11) {
        p->depth -= 0x10;
    }
    if (state->frame == 0x3A) {
        state->state = 2;
    }
}

void RoomEffect_InitSpritePair(void *arg0, void *arg1, RoomAnchoredSpriteState *obj) {
    FieldRingGeometry *ring = &obj->ring;
    int c80 = 0x80;
    int v = 0x10;

    ring->mode = v;
    v = 0xC8;
    obj->depth = c80;
    ring->intensity = c80;
    c80 = 0xFF;
    ring->offset = v;
    v = 0x40;
    ring->color0[0] = v;
    v = 0x80;
    ring->color0[1] = v;
    v = 0x4B0;
    ring->extent0 = v;
    obj->scale = 0;
    ring->color1[0] = c80;
    ring->color1[1] = c80;
    ring->color1[2] = c80;
    ring->color0[2] = c80;
    ring->extent1 = 0;
    ring->source = obj->sourceData;
    FieldFx_BuildRingGeometry(ring);
}

/* Builds a uniformly scaled rotation from the shared seed and draws the
 * sprite at two map anchors. Retail marks this function handwritten; only
 * its COP2 windows remain ASM. */
void RoomEffect_DrawAnchoredSpritePair(void *unused0, void *unused1,
                                            RoomAnchoredSpriteState *params) {
    RoomLibSpriteTransformStack stack;
    /* The GTE and draw matrix pointer still needs its pin. */
    RoomSpriteMatrix *scaleMatrix;
    RoomSpriteMatrix *matrix;
    RoomMotionContext *root;
    RoomMotionInitModel *map;
    short *floorY;
    FieldRingGeometry *renderState;
    int firstY;
    int firstX;
    int secondX;
    int secondY;

    root = FieldFx_GetScriptContext();
    stack.seed = s_MotionQuadSeed;

    stack.scaleMatrix.m[2][2] = 0x1000;
    stack.scaleMatrix.m[1][1] = 0x1000;
    stack.scaleMatrix.m[0][0] = 0x1000;
    stack.scaleMatrix.t[2] = 0;
    stack.scaleMatrix.t[1] = 0;
    stack.scaleMatrix.t[0] = 0;
    stack.scaleMatrix.m[2][1] = 0;
    stack.scaleMatrix.m[2][0] = 0;
    stack.scaleMatrix.m[1][2] = 0;
    stack.scaleMatrix.m[1][0] = 0;
    stack.scaleMatrix.m[0][2] = 0;
    stack.scaleMatrix.m[0][1] = 0;
    memset(&stack.sourceScale, 0, 0x10);

    stack.sourceScale.x = params->scale;
    stack.sourceScale.y = params->scale;
    stack.sourceScale.z = params->scale;
    stack.scale = stack.sourceScale;
    scaleMatrix = &stack.scaleMatrix;
    ScaleMatrix(scaleMatrix, &stack.scale);
    RotMatrix(&stack.seed.vector, &stack.matrix);
    matrix = &stack.matrix;

    gte_ldrotmatrix(scaleMatrix);
    gte_ldrtir12_matrix_column(&stack.matrix.m[0][0]);
    gte_stir123_column(matrix);
    gte_ldrtir12_matrix_column(&stack.matrix.m[0][1]);
    gte_stir123_column_at(&stack.matrix.m[0][1]);
    gte_ldrtir12_matrix_column(&stack.matrix.m[0][2]);
    gte_stir123_column_at(&stack.matrix.m[0][2]);
    gte_ldtransmatrix(scaleMatrix);
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

    FieldFx_SetSpriteBlendMode(2);
    map = ((RoomMotionInitLink *)root->link)->model;
    renderState = &params->ring;
    renderState->intensity = params->depth;
    /* Both anchors sit on the shared floor height; the first block stores
     * the height before the x coordinate, which fixes retail's * choice. */
    floorY = &g_RoomFloorY->y;
    firstX = map->primary.t[0];
    firstY = *floorY;
    stack.matrix.t[1] = firstY;
    stack.matrix.t[0] = firstX;
    stack.matrix.t[2] = map->primary.t[2];
    FieldFx_DrawShadedRing(renderState, matrix, 1);
    secondX = map->secondary.t[0];
    secondY = *floorY;
    stack.matrix.t[0] = secondX;
    stack.matrix.t[1] = secondY;
    stack.matrix.t[2] = map->secondary.t[2];
    FieldFx_DrawShadedRing(renderState, matrix, 1);
}

void RoomEffect_StepSpritePair(void *arg0, RoomFxControl *state, RoomAnchoredSpriteState *obj) {
    if (state->frame < 0x10) {
        obj->scale += 0xF0;
        if (state->frame < 0x10) {
            obj->depth -= 8;
        }
    }
    if (state->frame == 0x10) {
        state->state = 2;
    }
}

s32 *FieldFx_RegPtr(s32 index);

/* Seeds the motion particles: three or six of them depending on the first
 * scene flag, each with a random anchor offset, a random sprite lookup and
 * a random drift that is slower and sideways when the flag is set. */
void RoomEffect_SeedMotionParticles(void *unused0, void *unused1,
                                        RoomMotionState *state)
{
    /* Retail reserves one 8-byte local that nothing in the shipped code
     * reads; keep it so the frame matches. */
    s16 unusedVector[4];
    unsigned int i;

    if (*FieldFx_RegPtr(1)) {
        state->count = 3;
    } else {
        state->count = 6;
    }
    state->depth = 0;
    i = 0;
    if (state->count != 0) {
        do {
            state->scale[i].x = 0x400;
            state->scale[i].y = 0x400;
            state->particle[i].primaryX = rand() % 140 - 70;
            state->particle[i].primaryY = rand() % 140 - 70;
            state->particle[i].primaryZ = rand() % 140 - 70;
            state->particle[i].secondaryX = rand() % 140 - 70;
            state->particle[i].secondaryY = rand() % 140 - 70;
            state->particle[i].secondaryZ = rand() % 140 - 70;
            state->lookup[i].id = rand() % 8;
            state->lookup[i].pad = rand() % 8;
            if (*FieldFx_RegPtr(1) == 0) {
                state->velocity[i].primaryX = rand() % 40 - 20;
                state->velocity[i].primaryY = 0;
                state->velocity[i].primaryZ = rand() % 40 - 20;
                state->velocity[i].secondaryX = rand() % 40 - 20;
                state->velocity[i].secondaryY = 0;
                state->velocity[i].secondaryZ = rand() % 40 - 20;
            } else {
                state->velocity[i].primaryX = rand() % 26 - 13;
                state->velocity[i].primaryY = -rand() % 10;
                state->velocity[i].primaryZ = rand() % 26 - 13;
                state->velocity[i].secondaryX = rand() % 26 - 13;
                state->velocity[i].secondaryY = -rand() % 10;
                state->velocity[i].secondaryZ = rand() % 26 - 13;
            }
            i++;
        } while (i < state->count);
    }
}
