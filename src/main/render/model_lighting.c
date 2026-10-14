#include "pe1/gte.h"
#include "pe1/render_lighting.h"
#include "pe1/render_object.h"

u32 g_RenderFadeColour = 0x808080;

/* Lights an object's visible parts: each part's light matrix is the view
 * light matrix times the part matrix, every vertex triple gets an NCCT colour
 * from the base colour, and the first coded vertex colour of the triple gets
 * a second NCCT into the alternate colour table. */
void Render_DrawObject(RenderObjectEntity *entity, union RenderLightingMatrix *view) {
    u32 baseColourStorage;
    GteMatrix lightMatrixStorage = {0};
    u32 *baseColour = &baseColourStorage;
    s16 *lightMatrix = &lightMatrixStorage.m[0][0];
    RenderMatrix *matrix;
    int i, j;

    if (!entity->header || !entity->draw_count)
        return;
    *baseColour = g_RenderFadeColour;
    matrix = entity->matrices;
    gte_SetBackColor(entity->shade, entity->shade, entity->shade);
    for (i = 0; i < entity->header->part_count; matrix++, i++) {
        RenderObjectPart *part = &entity->parts[i];
        int first;
        u32 *mainColours;
        u32 *altColours;
        RenderVec3s *vertex;

        if (part->visible != 1)
            continue;
        gte_MulMatrix0(view, matrix, lightMatrix);
        gte_SetLightMatrix(lightMatrix);
        first = part->vertex_start;
        mainColours = &g_RenderClutLookupTable[first];
        altColours = &g_RenderVertexWorkWords[first];
        vertex = &entity->vertices[first];
        for (j = 0; j < part->vertex_count; j += 3, mainColours += 3, altColours += 3) {
            RenderColor *colours;
            int k;

            gte_ldv3(&g_RenderNormalTable[vertex[0].pad], &g_RenderNormalTable[vertex[1].pad],
                     &g_RenderNormalTable[vertex[2].pad]);
            gte_ldrgb(baseColour);
            gte_ncct();
            vertex += 3;
            gte_strgb3(mainColours, mainColours + 1, mainColours + 2);
            k = first + j;
            colours = (RenderColor *)entity->vertex_colours;
            if (colours[k].code)
                gte_ldrgb(&colours[k]);
            else if (colours[k + 1].code)
                gte_ldrgb(&colours[k + 1]);
            else if (colours[k + 2].code)
                gte_ldrgb(&colours[k + 2]);
            else
                continue;
            gte_ncct();
            gte_strgb3(altColours, altColours + 1, altColours + 2);
        }
    }
}


void Render_UpdateClutTable(RenderObjectEntity *arg0, s16 arg1, s16 arg2) {
    RenderObjectHeader *hdr;
    u32 *clutTbl;
    u32 *tpageTbl;
    RenderPrimitiveDescriptor *src;
    u8 *prim;
    u8 *p;
    s32 i;
    u8 cmd;

    hdr = arg0->header;
    if ((hdr != NULL) && (arg0->draw_count != 0)) {
        clutTbl = g_RenderClutLookupTable;
        tpageTbl = g_RenderVertexWorkWords;
        src = arg0->primitive_descriptors;
        prim = arg0->primitive_buffer;
        for (i = 0; i < arg0->header->packet34_count; src++) {
            p = prim + (arg2 * sizeof(RenderPacket34));
            if ((((RenderPacket34 *)p)->tag) != 0 || (arg1 != 0)) {
                cmd = ((RenderPacket34 *)p)->values0.bytes.command;
                ((RenderPacket34 *)p)->values0.value = clutTbl[src->lookup_indices[0]];
                ((RenderPacket34 *)p)->value1 = clutTbl[src->lookup_indices[1]];
                ((RenderPacket34 *)p)->value2 = clutTbl[src->lookup_indices[2]];
                ((RenderPacket34 *)p)->value3 = clutTbl[src->lookup_indices[3]];
                ((RenderPacket34 *)p)->values0.bytes.command = cmd;
            }
            prim += 2 * sizeof(RenderPacket34);
            i++;
        }
        for (i = 0; i < arg0->header->packet28_count; src++) {
            p = prim + (arg2 * sizeof(RenderPacket28));
            if ((((RenderPacket28 *)p)->tag) != 0 || (arg1 != 0)) {
                cmd = ((RenderPacket28 *)p)->values0.bytes.command;
                ((RenderPacket28 *)p)->values0.value = clutTbl[src->lookup_indices[0]];
                ((RenderPacket28 *)p)->value1 = clutTbl[src->lookup_indices[1]];
                ((RenderPacket28 *)p)->value2 = clutTbl[src->lookup_indices[2]];
                ((RenderPacket28 *)p)->values0.bytes.command = cmd;
            }
            prim += 2 * sizeof(RenderPacket28);
            i++;
        }
        for (i = 0; i < arg0->header->packet24_count; src++) {
            p = prim + (arg2 * sizeof(RenderPacket24));
            if ((((RenderPacket24 *)p)->tag) != 0 || (arg1 != 0)) {
                cmd = ((RenderPacket24 *)p)->values0.bytes.command;
                ((RenderPacket24 *)p)->values0.value = tpageTbl[src->lookup_indices[0]];
                ((RenderPacket24 *)p)->value1 = tpageTbl[src->lookup_indices[1]];
                ((RenderPacket24 *)p)->value2 = tpageTbl[src->lookup_indices[2]];
                ((RenderPacket24 *)p)->value3 = tpageTbl[src->lookup_indices[3]];
                ((RenderPacket24 *)p)->values0.bytes.command = cmd;
            }
            prim += 2 * sizeof(RenderPacket24);
            i++;
        }
        for (i = 0; i < arg0->header->packet1c_count; src++) {
            p = prim + (arg2 * sizeof(RenderPacket1C));
            if ((((RenderPacket1C *)p)->tag) != 0 || (arg1 != 0)) {
                cmd = ((RenderPacket1C *)p)->values0.bytes.command;
                ((RenderPacket1C *)p)->values0.value = tpageTbl[src->lookup_indices[0]];
                ((RenderPacket1C *)p)->value1 = tpageTbl[src->lookup_indices[1]];
                ((RenderPacket1C *)p)->value2 = tpageTbl[src->lookup_indices[2]];
                ((RenderPacket1C *)p)->values0.bytes.command = cmd;
            }
            prim += 2 * sizeof(RenderPacket1C);
            i++;
        }
    }
}


static inline int ClampLight(int value)
{
    if (value < 0) return 0;
    if (value > 255) return 255;
    return value;
}

/* Build two opposite Y lights and upload their color matrix to the GTE. */
int Render_InitRoomPrimState(void *objectArg)
{
    RenderObjectEntity *object = objectArg;
    int intensity, r, g, b;

    g_RenderLightingState.matrix.m[0][0] = 0;
    g_RenderLightingState.matrix.m[0][1] = 4096;
    g_RenderLightingState.matrix.m[0][2] = 0;
    intensity = object->shade + object->lightPositiveY;
    r = g_RoomLightTint.r;
    g = g_RoomLightTint.g;
    b = g_RoomLightTint.b;
    intensity = ClampLight(intensity) << 4;
    g_RenderLightColorMatrix.matrix.m[0][0] = intensity * r / 256;
    g_RenderLightColorMatrix.matrix.m[1][0] = intensity * g / 256;
    g_RenderLightColorMatrix.matrix.m[2][0] = intensity * b / 256;

    g_RenderLightingState.matrix.m[1][0] = 0;
    g_RenderLightingState.matrix.m[1][1] = -4096;
    g_RenderLightingState.matrix.m[1][2] = 0;
    intensity = object->shade + object->lightNegativeY;
    intensity = ClampLight(intensity) << 4;
    g_RenderLightColorMatrix.matrix.m[0][1] = intensity * r / 256;
    g_RenderLightColorMatrix.matrix.m[1][1] = intensity * g / 256;
    g_RenderLightColorMatrix.matrix.m[2][1] = intensity * b / 256;
    g_RenderLightingState.matrix.m[2][0] = 0;
    g_RenderLightingState.matrix.m[2][1] = 0;
    g_RenderLightingState.matrix.m[2][2] = 0;
    g_RenderLightColorMatrix.matrix.m[0][2] = 0;
    g_RenderLightColorMatrix.matrix.m[1][2] = 0;
    g_RenderLightColorMatrix.matrix.m[2][2] = 0;
    gte_SetColorMatrix(g_RenderLightColorMatrix.words);
    return 0;
}

