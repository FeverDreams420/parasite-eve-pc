#include "pe1/gte.h"
#include "pe1/render_object.h"
#include <stdio.h>

static int skinned_checks(void)
{
    RenderMatrix matrices[2] = {
        {{{4096,0,0},{0,4096,0},{0,0,4096}},0,{100,200,512}},
        {{{4096,0,0},{0,4096,0},{0,0,4096}},0,{20,30,512}}
    };
    RenderObjectHeader header = {0};
    RenderVec3s origin={0}, bounds[4]={{0},{0},{100,0,0,0},{0}};
    u16 section[4]={1,2,3,0};
    GteMatrix view={{{4096,0,0},{0,4096,0},{0,0,4096}},{0,0,0}};
    RenderObjectEntity object={0}, child={0};
    header.anchor_matrix_index=1; header.anchor_y=50;
    object.header=&header; object.matrices=matrices;
    object.model_section14=section; object.projection_origin=&origin;
    object.bounds_vertices=bounds; object.table_index=1;
    object.hit_body.value0=10; object.hit_body.value1=20; object.hit_body.value2=30;
    object.hit_body.radius=44;
    gte_SetGeomOffset(160,112); gte_ctc2_26(256); gte_lddqa(0); gte_lddqb(0);
    Render_TransformSkinnedVertices(&object,(u32 *)&view);
    if (object.hit_cylinder.value0!=20 || object.hit_cylinder.value1!=80 ||
        object.hit_cylinder.value2!=512 || object.hit_body.value0!=111 ||
        object.hit_body.value1!=222 || object.hit_body.value2!=545 ||
        object.target_x!=120 || object.target_y!=30 || object.target_z!=512)
        return 6;
    if (object.projected_x!=210 || object.projected_y!=212 ||
        object.projected_target_x!=220 || object.projected_target_y!=127) return 7;
    child.animation_source=&object; child.table_index=1;
    child.hit_cylinder.radius=10;
    Render_TransformSkinnedVertices(&child,(u32 *)&view);
    if (child.hit_cylinder.value0!=120 || child.hit_cylinder.value1!=30 ||
        child.hit_cylinder.value2!=512 || child.target_x!=120 ||
        child.rotation_overrides[0].x!=120 || child.hit_body.radius!=44 ||
        child.hit_body.value0!=111 || child.table_value2c!=0) return 8;
    if (child.projected_target_x!=170 || child.projected_target_y!=127 ||
        child.projected_x!=175 || child.projected_y!=127) return 9;
    return 0;
}

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
    if (D_800B1644[0] != g_RenderClutLookupTable[3] ||
        D_800B1640[1] != D_800B1644[0] || D_800A636C[0] != 512) return 4;
    D_800B1640[1] = 12345;
    if (D_800B1644[0] != 12345) return 5;
    parts[0].visible = parts[1].visible = 0;
    g_RenderClutLookupTable[0] = 123;
    Render_TransformMorphVertices(&object,(u32 *)&view);
    if (g_RenderClutLookupTable[0] != 123) return 3;
    { int error=skinned_checks(); if (error) return error; }
    puts("Original RTPT part tails, overlapping stores and hidden parts passed");
    return 0;
}
