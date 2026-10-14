#include "pe1/gte.h"
#include "pe1/render_object.h"

/* Shared original render work buffers. Vertex indices/counts are u16;
 * RTPT stores a whole triplet at part tails, including two extra slots after a three-slot output bias. */
u32 g_RenderClutLookupTable[65541];
u32 g_RenderVertexWorkWords[65541];

/* Original object.c projection procedure, with host matrix workspace. */
void Render_TransformMorphVertices(RenderObjectEntity *entity, u32 *view) {
    GteMatrix workspace;
    GteMatrix *scratch = &workspace;
    RenderMatrix *matrix = entity->matrices;
    int i;

    for (i = 0; i < entity->header->part_count; i++) {
        RenderObjectPart *part = &entity->parts[i];

        gte_CompMatrixMac(view, matrix, scratch);
        gte_ldrotmatrix(scratch);
        gte_ldtransmatrix(scratch);
        matrix++;
        if (part->visible == 1) {
            RenderVec3s *vertices = &entity->vertices[part->vertex_start];
            u32 *screen = &g_RenderClutLookupTable[part->vertex_start];
            u32 *depth = &g_RenderVertexWorkWords[part->vertex_start];
            int j;

            for (j = 0; j < part->vertex_count; ) {
                gte_ldv3c(vertices);
                gte_rtpt_padded();
                vertices += 3;
                j += 3;
                screen += 3;
                depth += 3;
                gte_stsxy3c(screen);
                gte_stsz3c(depth);
            }
        }
    }
}

/* Shared anchor: Y is a view into the original zero vector. */
GteShortVector g_SkinAnchor = {0};

void Render_TransformSkinnedVertices(RenderObjectEntity *actor, u32 *view) {
    u16 offset[4] = {0};
    GteMatrix workspace;
    GteMatrix *scratch = &workspace;
    RenderObjectHeader *header = actor->header;

    if (!header) {
        Render_SetupBoneTransforms(actor, (s32 *)view);
        return;
    }
    gte_ldrotmatrix(&actor->matrices[header->anchor_matrix_index]);
    gte_ldtransmatrix(&actor->matrices[header->anchor_matrix_index]);
    g_SkinAnchorY = header->anchor_y;
    gte_ldv0(g_SkinAnchorVector);
    gte_rt();
    gte_stsv(&actor->hit_cylinder.value0);
    gte_ldrotmatrix(actor->matrices);
    gte_ldtransmatrix(actor->matrices);
    offset[0] = ((u16 *)actor->model_section14)[0] + (u16)actor->hit_body.value0;
    offset[1] = ((u16 *)actor->model_section14)[1] + (u16)actor->hit_body.value1;
    offset[2] = ((u16 *)actor->model_section14)[2] + (u16)actor->hit_body.value2;
    gte_ldv0(offset);
    gte_rt();
    gte_stsv(&actor->hit_body.value0);
    gte_ldrotmatrix(&actor->matrices[(s16)actor->table_index]);
    gte_ldtransmatrix(&actor->matrices[(s16)actor->table_index]);
    gte_ldv0(&actor->bounds_vertices[(s16)actor->table_index * 2]);
    gte_rt();
    gte_stsv(&actor->target_x);
    gte_CompMatrixMac(view, actor->matrices, scratch);
    gte_ldrotmatrix(scratch);
    gte_ldtransmatrix(scratch);
    gte_ldv0(actor->projection_origin);
    gte_rtps();
    gte_stsxy2(&actor->projected_x);
    gte_CompMatrixMac(view, &actor->matrices[(s16)actor->table_index], scratch);
    gte_ldrotmatrix(scratch);
    gte_ldtransmatrix(scratch);
    gte_ldv0(&actor->bounds_vertices[(s16)actor->table_index * 2]);
    gte_rtps();
    gte_stsxy2(&actor->projected_target_x);
}

void Render_SetupBoneTransforms(RenderObjectEntity *actor, s32 *view) {
    RenderMatrix workspace;
    RenderMatrix *scratch = &workspace;
    RenderObjectEntity *source = actor->animation_source;
    int index = (s16)actor->table_index;
    RenderMatrix *matrix = &source->matrices[index];

    gte_ldrotmatrix(matrix);
    gte_ldtransmatrix(matrix);
    gte_ldv0(&source->bounds_vertices[index * 2]);
    gte_rt();
    /* Headerless objects reuse the first override XYZ as a position output. */
    gte_stsv(&actor->rotation_overrides[0].x);
    gte_stsv(&actor->hit_cylinder.value0);
    gte_stsv(&actor->target_x);
    gte_CompMatrixMac(view, matrix, scratch);
    gte_ldrotmatrix(scratch);
    gte_ldtransmatrix(scratch);
    /* In this mode the halfwords at +0x2C form the projection input vector. */
    gte_ldv0(&actor->table_value2c);
    gte_rtps();
    actor->hit_body.radius = source->hit_body.radius;
    gte_stsxy2(&actor->projected_target_x);
    actor->table_value2c += actor->hit_cylinder.radius;
    gte_ldv0(&actor->table_value2c);
    gte_rtps();
    actor->hit_body.value0 = source->hit_body.value0;
    actor->hit_body.value1 = source->hit_body.value1;
    actor->hit_body.value2 = source->hit_body.value2;
    gte_stsxy2(&actor->projected_x);
    actor->table_value2c -= actor->hit_cylinder.radius;
}
