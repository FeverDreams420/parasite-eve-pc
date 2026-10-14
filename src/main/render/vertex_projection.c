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
