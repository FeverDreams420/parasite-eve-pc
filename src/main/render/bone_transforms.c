#include "pe1/gte.h"
#include "pe1/render_object.h"

/* Original object.c routines, separated from draw-packet dependencies.
 * The serialized command-byte count is u16; this ordinary memory buffer
 * replaces the shared PS1 scratchpad stack for every possible valid depth. */
static RenderMatrix g_RenderTransformStack[65536];

void Render_TransformVertices(RenderObjectEntity *actor) {
    s32 origin[3];
    s32 *originY = &origin[1];
    s32 *originZ = &origin[2];
    RenderMatrix *stack = g_RenderTransformStack;
    s8 *commands;
    RenderMatrix *out;
    RenderAnimationLookupEntry *centres;
    RenderMatrix *current;
    int mode;
    int i;

    mode = actor->animation_state;
    if (mode == 1) {
        Render_CopyFrameData(actor, actor->animation_source, actor->animation_id);
        current = &actor->model_matrix;
    } else if (mode == 3) {
        if (actor->flags_9C & 0x400) {
            actor->matrices[1].rotation[0][0] = actor->model_matrix.rotation[0][0];
            actor->matrices[1].rotation[0][1] = actor->model_matrix.rotation[0][1];
            actor->matrices[1].rotation[0][2] = actor->model_matrix.rotation[0][2];
            actor->matrices[1].rotation[1][0] = actor->model_matrix.rotation[1][0];
            actor->matrices[1].rotation[1][1] = actor->model_matrix.rotation[1][1];
            actor->matrices[1].rotation[1][2] = actor->model_matrix.rotation[1][2];
            actor->matrices[1].rotation[2][0] = actor->model_matrix.rotation[2][0];
            actor->matrices[1].rotation[2][1] = actor->model_matrix.rotation[2][1];
            actor->matrices[1].rotation[2][2] = actor->model_matrix.rotation[2][2];
            actor->matrices[1].translation[0] = actor->model_matrix.translation[0];
            actor->matrices[1].translation[1] = actor->model_matrix.translation[1];
            actor->matrices[1].translation[2] = actor->model_matrix.translation[2];
        } else {
            Render_CopyFrameDataDouble(actor, actor->animation_source, actor->animation_id);
        }
        return;
    } else {
        if (mode == 4) {
            gte_CompMatrix(&actor->animation_source->matrices[actor->animation_id],
                           &actor->model_matrix, &actor->model_matrix);
        }
        current = &actor->model_matrix;
    }

    *origin = 0;
    *originY = 0;
    out = actor->matrices;
    commands = actor->matrix_commands;
    centres = actor->animation_entries;
    gte_ldrotmatrix(current);
    gte_ldtransmatrix(current);
    for (i = 0; i < actor->header->matrix_command_bytes; i++, commands++) {
        int command = *commands;
        RenderMatrix *source;

        if (command == -1) {
            gte_ReadMatrix(stack);
            stack++;
            continue;
        }
        if (command == -2) {
            stack--;
            gte_ldrotmatrix(stack);
            gte_ldtransmatrix(stack);
            continue;
        }
        source = &actor->active_matrix[command];
        gte_ldclmv(source);
        gte_rtir();
        *originZ = actor->parts[command].translation_z;
        gte_stclmv(out);
        gte_ldclmv((u16 *)source + 1);
        gte_rtir();
        gte_stclmv((s16 *)out + 1);
        gte_ldclmv((u16 *)source + 2);
        gte_rtir();
        gte_stclmv((s16 *)out + 2);
        if (*commands == 0)
            gte_ldlv0(source->translation);
        else
            gte_ldlv0(origin);
        gte_rt();
        gte_stlvnl(out->translation);
        gte_ldrotmatrix(out);
        gte_ldtransmatrix(out);
        out++;
        if (actor->parts[*commands].visible == 1 &&
            actor->bounds_vertices[*commands * 2 + 1].pad >= 0) {
            gte_ldv0(&actor->bounds_vertices[*commands * 2]);
            gte_rt();
            gte_stsv(centres);
            centres++;
        }
    }
}

void Render_CopyFrameData(RenderObjectEntity *dst, RenderObjectEntity *src, s32 frame) {
    s16 index = frame;

    dst->model_matrix.rotation[0][0] = src->matrices[index].rotation[0][0];
    dst->model_matrix.rotation[0][1] = src->matrices[index].rotation[0][1];
    dst->model_matrix.rotation[0][2] = src->matrices[index].rotation[0][2];
    dst->model_matrix.rotation[1][0] = src->matrices[index].rotation[1][0];
    dst->model_matrix.rotation[1][1] = src->matrices[index].rotation[1][1];
    dst->model_matrix.rotation[1][2] = src->matrices[index].rotation[1][2];
    dst->model_matrix.rotation[2][0] = src->matrices[index].rotation[2][0];
    dst->model_matrix.rotation[2][1] = src->matrices[index].rotation[2][1];
    dst->model_matrix.rotation[2][2] = src->matrices[index].rotation[2][2];
    dst->model_matrix.translation[0] = src->matrices[index].translation[0];
    dst->model_matrix.translation[1] = src->matrices[index].translation[1];
    dst->model_matrix.translation[2] = src->matrices[index].translation[2];
}

void Render_CopyFrameDataDouble(RenderObjectEntity *dst, RenderObjectEntity *src, s32 frame) {
    s16 index = frame;

    dst->matrices[0].rotation[0][0] = src->matrices[index].rotation[0][0];
    dst->matrices[0].rotation[0][1] = src->matrices[index].rotation[0][1];
    dst->matrices[0].rotation[0][2] = src->matrices[index].rotation[0][2];
    dst->matrices[0].rotation[1][0] = src->matrices[index].rotation[1][0];
    dst->matrices[0].rotation[1][1] = src->matrices[index].rotation[1][1];
    dst->matrices[0].rotation[1][2] = src->matrices[index].rotation[1][2];
    dst->matrices[0].rotation[2][0] = src->matrices[index].rotation[2][0];
    dst->matrices[0].rotation[2][1] = src->matrices[index].rotation[2][1];
    dst->matrices[0].rotation[2][2] = src->matrices[index].rotation[2][2];
    dst->matrices[0].translation[0] = src->matrices[index].translation[0];
    dst->matrices[0].translation[1] = src->matrices[index].translation[1];
    dst->matrices[0].translation[2] = src->matrices[index].translation[2];

    dst->matrices[1].rotation[0][0] = src->matrices[index].rotation[0][0];
    dst->matrices[1].rotation[0][1] = src->matrices[index].rotation[0][1];
    dst->matrices[1].rotation[0][2] = src->matrices[index].rotation[0][2];
    dst->matrices[1].rotation[1][0] = src->matrices[index].rotation[1][0];
    dst->matrices[1].rotation[1][1] = src->matrices[index].rotation[1][1];
    dst->matrices[1].rotation[1][2] = src->matrices[index].rotation[1][2];
    dst->matrices[1].rotation[2][0] = src->matrices[index].rotation[2][0];
    dst->matrices[1].rotation[2][1] = src->matrices[index].rotation[2][1];
    dst->matrices[1].rotation[2][2] = src->matrices[index].rotation[2][2];
    dst->matrices[1].translation[0] = src->matrices[index].translation[0];
    dst->matrices[1].translation[1] = src->matrices[index].translation[1];
    dst->matrices[1].translation[2] = src->matrices[index].translation[2];
}
