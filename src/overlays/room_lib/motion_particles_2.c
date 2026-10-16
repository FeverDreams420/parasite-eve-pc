/*
 * The motion particle set, part 2 of 3: the particle draw.
 *
 * room_m075, room_m080, room_m082, scene_e09 and scene_e10 link the same
 * fifteen functions in the same order, after the room prelude and before
 * the room library, with the same 0x18 bytes of read-only data. The object
 * remains in its three original source units. Native builds use ordinary
 * C11 and pointer-bearing runtime records. Trails, glyphs and lookup codes live
 * in each room's own data (pe1/room_motion_init.h).
 */

#include "common.h"
#include "pe1/field_script_context.h"
#include "pe1/room_fx.h"
#include "pe1/room_motion_init.h"

#include "motion_particles.h"
#include "room_lib.h"

static const RoomFxVec4 s_MotionScale = { 0x590, 0x590, 0x590, 0 };

typedef struct RoomLibMotionScaleScratch {
    RoomFxVec4 value;
} RoomLibMotionScaleScratch;

int RoomEffect_DrawMotionParticles(void *owner_data, void *control_data, void *data) {
    RoomStatePair *control = control_data; RoomMotionState *state = data;
    RoomSpriteMatrix matrix;
    RoomLibMotionScaleScratch scale;
    RoomMotionContext *context;
    RoomSpriteMatrix livePrimary, liveSecondary;
    RoomSpriteMatrix *primary;
    RoomSpriteMatrix *secondary;
    u16 *depth;
    char *packet;
    RoomMotionPoint *cursor;
    int i;

    context = FieldFx_GetScriptContext();
    FieldFx_SetSpriteTexturePreset(context->allocation & 255);
    FieldFx_SetSpriteCellSize(0x20, 0x20);
    FieldFx_SetSpriteColorMode(0x10);
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
    scale.value = s_MotionScale;
    ScaleMatrix(&matrix, &scale.value);

    if (control->h2 == 30) {
        memcpy(&state->matrix[1], &context->link->render_object.matrices[13], sizeof(state->matrix[1]));
        memcpy(&state->matrix[0], &context->link->render_object.matrices[9], sizeof(state->matrix[0]));
    }

    if (control->h2 >= 31) {
        g_RoomMotionGlyph.offset = 0;
        primary = &state->matrix[0];
        secondary = &state->matrix[1];
    } else {
        memcpy(&livePrimary, &context->link->render_object.matrices[9], sizeof(livePrimary));
        memcpy(&liveSecondary, &context->link->render_object.matrices[13], sizeof(liveSecondary));
        primary = &livePrimary;
        secondary = &liveSecondary;
    }

    i = 0;
    if (state->count != 0) {
        depth = &g_RoomMotionGlyph.depth;
        packet = (char *)depth - 10;
        cursor = state->particle;
        do {
            *depth = state->depth;
            *(u8 *)((char *)depth - 6) =
                g_RoomMotionLookupCodes[state->lookup[i].id];
            matrix.t[0] = primary->t[0] + cursor->primaryX;
            matrix.t[1] = primary->t[1] + cursor->primaryY;
            matrix.t[2] = primary->t[2] + cursor->primaryZ;
            FieldFx_DrawGlowSprite((FieldGlowSprite *)packet, &matrix, 1);
            matrix.t[0] = secondary->t[0] + cursor->secondaryX;
            matrix.t[1] = secondary->t[1] + cursor->secondaryY;
            matrix.t[2] = secondary->t[2] + cursor->secondaryZ;
            FieldFx_DrawGlowSprite((FieldGlowSprite *)packet, &matrix, 1);
            i++;
            ++cursor;
        } while ((unsigned int)i < state->count);
    }
    return 0;
}
