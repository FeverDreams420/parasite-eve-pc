#include "pe1/gte.h"
#include "pe1/render_object.h"
#include <stdio.h>

int main(void)
{
    RenderMatrix matrices[2] = {
        {{{4096,0,0},{0,4096,0},{0,0,4096}},0,{0,0,0}},
        {{{4096,0,0},{0,4096,0},{0,0,4096}},0,{20,0,0}}
    };
    RenderObjectHeader header = {0};
    RenderObjectPart parts[2] = {{0,1,1,{0},0},{1,1,1,{0},0}};
    RenderVec3s vertices[4] = {{0,0,512,0},{100,0,512,0},{0,100,512,0},{0,0,512,0}};
    RenderObjectEntity object = {0};
    GteMatrix view = {{{4096,0,0},{0,4096,0},{0,0,4096}},{0,0,0}};
    header.part_count = 2;
    object.header = &header;
    object.parts = parts;
    object.vertices = vertices;
    object.matrices = matrices;
    gte_SetGeomOffset(160,112);
    gte_ctc2_26(256);
    gte_lddqa(0);
    gte_lddqb(0);
    for (unsigned i=0; i<8; ++i) {
        g_RenderClutLookupTable[i] = 0x12345678;
        g_RenderVertexWorkWords[i] = 0x87654321;
    }
    Render_TransformMorphVertices(&object,(u32 *)&view);
    /* First part writes slots 3..5; second overwrites 4..5 and writes 6.
     * Tail vertices still use RTPT and are not truncated to the part count. */
    if (g_RenderClutLookupTable[3] != (112u<<16|160u) ||
        g_RenderClutLookupTable[4] != (112u<<16|220u) ||
        g_RenderClutLookupTable[5] != (162u<<16|170u) ||
        g_RenderClutLookupTable[6] != (112u<<16|170u) ||
        g_RenderClutLookupTable[0] != 0x12345678 ||
        g_RenderClutLookupTable[7] != 0x12345678) return 1;
    for (unsigned i=3; i<7; ++i) if (g_RenderVertexWorkWords[i] != 512) return 2;
    parts[0].visible = parts[1].visible = 0;
    g_RenderClutLookupTable[0] = 123;
    Render_TransformMorphVertices(&object,(u32 *)&view);
    if (g_RenderClutLookupTable[0] != 123) return 3;
    puts("Original RTPT part tails, overlapping stores and hidden parts passed");
    return 0;
}
