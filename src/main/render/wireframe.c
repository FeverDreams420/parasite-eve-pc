/* Wireframe model pass: projects a scaled, yawed model and draws its edges
 * as LINE_G2 packets. Back faces only contribute their floor edge (both
 * vertices at y 0, cyan); front faces draw all three edges, cyan on the
 * floor and green elsewhere. Returns the packet bytes written.
 *
 * Matching debt (not counted by crutch_debt.py): the four unused
 * scratch-word initialisers reproduce retail's unused 32-byte frame range
 * sp+0x50..0x6F; the dead "line_b = scale" store, the identical-arm screen
 * tests, the do-while(0) around the tpage link, and the shared far_edge
 * tail keep GCC's allocation. */

#include "common.h"
#include "pe1/gte.h"
#include "pe1/render_anim_position.h"
#include "pe1/render_animation_frame.h"
#include "pe1/render_object.h"

typedef struct RenderAnimFrame {
    u8 reserved0;
    u8 vertex_count;
    u8 pad02[4];
    u16 data_offset_words;
    u8 count4_a;
    u8 count4_b;
    u8 face_count;
    u8 count4_c;
    u8 count8_a;
    u8 count8_b;
    u8 count8_c;
    u8 count8_d;
    u8 count4_d;
    u8 count2_a;
    u8 count2_b;
    u8 modifier_count;
    u8 pad14[8];
} RenderAnimFrame;

typedef union RenderAnimScratchWord {
    u32 raw;
    u8 bytes[4];
} RenderAnimScratchWord;

typedef struct RenderAnimVertex {
    s16 x;
    s16 y;
    s16 z;
    u8 material_index;
    u8 pad7;
} RenderAnimVertex;

typedef struct RenderAnimMaterialRecord {
    u8 reserved[3];
    u8 flags;
} RenderAnimMaterialRecord;

typedef union RenderAnimTag {
    u32 raw;
    struct {
        u32 addr : 24;
        u32 len : 8;
    } bits;
} RenderAnimTag;

typedef struct RenderAnimLineG2 {
    RenderAnimTag tag;
    u8 r0;
    u8 g0;
    u8 b0;
    u8 code;
    u32 xy0;
    u8 r1;
    u8 g1;
    u8 b1;
    u8 pad0F;
    u32 xy1;
} RenderAnimLineG2;

typedef struct RenderAnimFace {
    u8 vertex0;
    u8 vertex1;
    u8 vertex2;
    u8 pad3;
} RenderAnimFace;

/* An output packet as a line primitive, as bytes and as a link address. */
typedef union RenderAnimPacket {
    RenderAnimLineG2 *line;
    u8 *bytes;
    u32 word;
} RenderAnimPacket;

/* Two-byte modifier record; the frame resets bit 7 of each flags byte. */
typedef struct RenderAnimModifier {
    u8 flags;
    u8 value;
} RenderAnimModifier;

PE1_STATIC_ASSERT(sizeof(RenderAnimFrame) == 0x1C, render_anim_frame_size);
PE1_STATIC_ASSERT(sizeof(RenderAnimVertex) == 8, render_anim_vertex_size);
PE1_STATIC_ASSERT(sizeof(RenderAnimMaterialRecord) == 4,
                  render_anim_material_record_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RenderAnimMaterialRecord, flags) == 3,
                  render_anim_material_flags_offset);
PE1_STATIC_ASSERT(sizeof(RenderAnimLineG2) == 0x14, render_anim_line_g2_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RenderAnimLineG2, xy0) == 8,
                  render_anim_line_g2_xy0_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RenderAnimLineG2, xy1) == 0x10,
                  render_anim_line_g2_xy1_offset);
PE1_STATIC_ASSERT(sizeof(RenderAnimFace) == 4, render_anim_face_size);

GteMatrix *RotMatrixY(s32, GteMatrix *);
void SetDrawTPage(void *, s32, s32, s32);

#define AnimLoadRotMatrix(matrix) gte_ldrotmatrix((const GteMatrixWords *)(matrix))

#define AnimLoadTrans(matrix) gte_ldtransmatrix((const GteMatrixWords *)(matrix))


#define Anim_LoadAxis(src) \
    { \
        gte_ldclmv((src)); \
        gte_cop2_hazard_slot(); \
        gte_cop2_hazard_slot(); \
        gte_mvmva_light_ir_sf0(); \
    }

#define Anim_StoreAxis(dst) gte_stclmv((dst))


#define AnimLoadVector(source) gte_ldlv0(source)

s32 Render_DrawWireframeModel(RenderAnimFrame *frame, void *position, s32 scale, s32 yaw,
                              void *ot, u8 *buffer) {
    RenderMatrix transform;
    s32 winding;
    s32 y0;
    s16 y0_short;
    u32 *ot_line;
    u32 *ot_page;
    u32 *ot_edge;
    u32 *ot_edge_page;
    RenderAnimPacket negative_line;
    RenderAnimPacket positive_line;
    s16 in_range;
    u32 screen1;
    s32 link_word;
    u32 line_b;
    s32 positive_depth;
    s32 edge;
    s32 line_a;
    u32 screen2;
    u32 screen0;
    s32 i;
    u32 cyan;
    u32 address_mask;
    u32 command_mask;
    u16 y1;
    u16 y2;
    u32 line_c;
    s16 diagonal;
    u32 ot_word;
    u8 negative_flags;
    u8 material_flags;
    u8 tpage_code;
    s32 index2;
    s32 index0;
    s32 index1;
    s32 modifier_count;
    RenderAnimMaterialRecord *materials;
    s32 count;
    s16 edge_end;
    s32 primitive_code;
    s16 edge_start;
    u8 near_vertex;
    RenderAnimFace *faces;
    RenderAnimModifier *modifier;
    u8 *code_byte;
    RenderAnimPacket cursor;
    u8 *output;
    /* Matching debt: see the file comment (unused sp+0x50..0x6F range). */
    RenderAnimScratchWord spill0 = {0}, spill1 = {0}, spill2 = {0}, spill3 = {0};
    RenderAnimVertex *vertices;
    RenderAnimFace *face;
    GteMatrix **matrix_slot;
    GteMatrix *matrix_value;
    RenderAnimPositionInput *position_input;

    cursor.bytes = buffer;
    line_b = scale;
    output = cursor.bytes;
    diagonal = (scale >> 16) / 4;
    transform.rotation[2][2] = diagonal;
    transform.rotation[1][1] = diagonal;
    transform.rotation[0][0] = diagonal;
    transform.rotation[2][1] = 0;
    transform.rotation[2][0] = 0;
    transform.rotation[1][2] = 0;
    transform.rotation[1][0] = 0;
    transform.rotation[0][2] = 0;
    transform.rotation[0][1] = 0;
    position_input = position;
    transform.translation[0] = position_input->x;
    transform.translation[1] = position_input->y;
    transform.translation[2] = position_input->z;
    RotMatrixY(yaw & 0xFFFF, (GteMatrix *)&transform);
    matrix_slot = &g_ViewMatrixSlot.value;
    matrix_value = *matrix_slot;
    gte_ldrotmatrix(matrix_value);
    {
        short *column = &transform.rotation[0][0];
        Anim_LoadAxis(column);
        Anim_StoreAxis(column);
        {
            short *src = &transform.rotation[0][1];
            Anim_LoadAxis(src);
            Anim_StoreAxis(src);
        }
        {
            short *src = &transform.rotation[0][2];
            Anim_LoadAxis(src);
            Anim_StoreAxis(src);
        }
        {
            GteMatrix *transMatrix = *matrix_slot;
            AnimLoadTrans(transMatrix);
        }
        {
            s32 *translation = transform.translation;
            AnimLoadVector(translation);
            gte_cop2_hazard_slot();
            gte_cop2_hazard_slot();
            gte_rtv0tr_sf0();
            gte_swc2_9_0(translation);
            gte_swc2_10_4(translation);
            gte_swc2_11_8(translation);
        }
        AnimLoadRotMatrix(column);
        AnimLoadTrans(column);
    }
    i = 0;
    count = frame->vertex_count;
    vertices = (RenderAnimVertex *)(frame + 1);
    if (count != 0) {
        RenderAnimVertex *vertex = vertices;
        do {
            gte_ldv0(vertex);
            gte_rtps();
            gte_stsxy2(&g_WireScratch.screenXY[i]);
            {
                /* Matching debt: retail reads SZ3 into t4 and stores it
                 * through v0 (lev 5 unpinned). */
                s16 *depth = &g_WireScratch.depth[i];
                int z;

                gte_getsz3_for_store(z, depth);
                *depth = z;
            }
            vertex++;
            i += 1;
        } while (i < count);
        i = 0;
    }
    faces = (RenderAnimFace *)&vertices[frame->vertex_count] + (frame->count4_a + frame->count4_b);
    modifier = (RenderAnimModifier *)(faces + (frame->face_count + frame->count4_c) +
                       (frame->count8_a + frame->count8_b + frame->count8_c + frame->count8_d) * 2 +
                       frame->count4_d) + frame->count2_a + frame->count2_b;
    modifier_count = frame->modifier_count;
    materials = (RenderAnimMaterialRecord *)frame + frame->data_offset_words;
    if (modifier_count != 0) {
        do {
            i += 1;
            modifier->flags &= 0x7F;
            modifier++;
        } while (i < modifier_count);
    }
    i = 0;
    count = frame->face_count;
    if (count != 0) {
        cyan = 0xFF;
        address_mask = 0xFFFFFF;
        command_mask = 0xFF000000;
        face = faces;
        do {
            index0 = face->vertex0;
            index1 = face->vertex1;
            index2 = face->vertex2;
            screen0 = g_WireScratch.screenXY[index0];
            screen1 = g_WireScratch.screenXY[index1];
            screen2 = g_WireScratch.screenXY[index2];
            gte_ldsxy0(screen0);
            gte_ldsxy2(screen2);
            gte_ldsxy1(screen1);
            gte_nclip();
            gte_stmac0(&winding);
            edge = 0;
            primitive_code = 0x50;
            if (winding <= 0) {
                s32 negative_depth;

                edge_end = -1;
                edge_start = -1;
                line_b = -1;
                line_a = -1;
                y0 = vertices[index0].y;
                y1 = (u16)vertices[index1].y;
                y2 = (u16)vertices[index2].y;
                y0_short = y0;
                if (y0 == 0 && y1 == 0) {
                    edge_start = index0;
                    edge_end = index1;
                    line_a = screen0;
                    line_b = screen1;
                } else if (y1 == 0 && y2 == 0) {
                    edge_end = index2;
                    line_a = screen1;
                    goto far_edge;
                } else if (y0_short == 0 && y2 == 0) {
                    edge_start = index0;
                    edge_end = index2;
                    line_a = screen0;
                far_edge:
                    line_b = screen2;
                }
                if (edge_start != -1 && edge_end != -1) {
                    negative_line = cursor;
                    negative_flags = materials[vertices[edge_start].material_index].flags;
                    negative_line.line->tag.bits.len = 4;
                    negative_line.line->code = primitive_code;
                    if (negative_flags & 0x20) {
                        if (screen0 || screen2) {
                            tpage_code = 0x52;
                        } else {
                            tpage_code = 0x52;
                        }
                        cursor.line->code = tpage_code;
                    }
                    negative_line.line->r0 = 0;
                    negative_line.line->g0 = cyan;
                    negative_line.line->b0 = cyan;
                    negative_line.line->r1 = 0;
                    negative_line.line->g1 = cyan;
                    negative_line.line->b1 = cyan;
                    negative_line.line->xy0 = line_a;
                    negative_line.line->xy1 = line_b;
                    negative_depth = (g_WireScratch.depth[edge_start] +
                                      g_WireScratch.depth[edge_end]) / 8;
                    in_range = negative_depth < 0x1000;
                    if (in_range != 0) {
                        cursor.bytes += 0x14;
                        ot_line = (u32 *)ot - (-negative_depth);
                        negative_line.line->tag.raw = (negative_line.line->tag.raw & command_mask) | (*ot_line & address_mask);
                        *ot_line = (*ot_line & command_mask) | (negative_line.word & address_mask);
                    }
                    if (negative_flags & 0x20) {
                        SetDrawTPage(cursor.bytes, 0, 1, (negative_flags & 3) << 5);
                        if (in_range != 0) {
                            ot_page = (u32 *)ot - (-negative_depth);
                            cursor.line->tag.raw = (cursor.line->tag.raw & command_mask) | (*ot_page & address_mask);
                            *ot_page = (*ot_page & command_mask) | (cursor.word & address_mask);
                            cursor.bytes += 8;
                        }
                    }
                } else {
                    cursor.line->tag.raw = 0;
                    cursor.bytes += 4;
                }
            } else {
                code_byte = cursor.bytes + 7;
                do {
                    near_vertex = 0;
                    line_a = 0;
                    line_b = 0;
                    line_c = 0;
                    switch (edge) {
                        case 0:
                            near_vertex = index0;
                            if (index0 < index1) {
                                line_a = index1;
                                line_b = screen0;
                                line_c = screen1;
                            } else {
                                near_vertex = index1;
                                line_a = index0;
                                line_b = screen1;
                                line_c = screen0;
                            }
                            break;
                        case 1:
                            near_vertex = index0;
                            if (index0 < index2) {
                                line_a = index2;
                                line_b = screen0;
                                line_c = screen2;
                            } else {
                                near_vertex = index2;
                                line_a = index0;
                                line_b = screen2;
                                line_c = screen0;
                            }
                            break;
                        case 2:
                            near_vertex = index1;
                            if (index1 < index2) {
                                line_a = index2;
                                line_b = screen1;
                                line_c = screen2;
                            } else {
                                near_vertex = index2;
                                line_a = index1;
                                line_b = screen2;
                                line_c = screen1;
                            }
                            break;
                    }
                    positive_line = cursor;
                    material_flags = materials[vertices[near_vertex].material_index].flags;
                    code_byte[-4] = 4; /* tag length byte, code_byte tracks the code byte */
                    *code_byte = primitive_code;
                    if (material_flags & 0x20) {
                        if (screen2) {
                            *code_byte = 0x52;
                        } else {
                            *code_byte = 0x52;
                        }
                    }
                    if ((vertices[near_vertex].y == 0) && (vertices[line_a].y == 0)) {
                        positive_line.line->r0 = 0;
                        positive_line.line->g0 = cyan;
                        positive_line.line->b0 = cyan;
                        positive_line.line->r1 = 0;
                        positive_line.line->g1 = cyan;
                        positive_line.line->b1 = cyan;
                    } else {
                        positive_line.line->r0 = 0;
                        positive_line.line->g0 = 0x80;
                        positive_line.line->b0 = 0;
                        positive_line.line->r1 = 0;
                        positive_line.line->g1 = 0x80;
                        positive_line.line->b1 = 0;
                    }
                    positive_line.line->xy0 = line_b;
                    positive_line.line->xy1 = line_c;
                    positive_depth = (g_WireScratch.depth[near_vertex] + g_WireScratch.depth[line_a]) / 8;
                    in_range = positive_depth < 0x1000;
                    if (in_range != 0) {
                        code_byte += 0x14;
                        cursor.bytes += 0x14;
                        ot_edge = (u32 *)ot - (-positive_depth);
                        ot_word = *ot_edge & address_mask;
                        link_word = positive_line.line->tag.raw & command_mask;
                        link_word |= ot_word;
                        positive_line.line->tag.raw = link_word;
                        *ot_edge = (*ot_edge & command_mask) | (positive_line.word & address_mask);
                    }
                    if (material_flags & 0x20) {
                        SetDrawTPage(cursor.bytes, 0, 1, (material_flags & 3) << 5);
                        if (in_range != 0) {
                            do {
                                code_byte += 8;
                                ot_edge_page = (u32 *)ot - (-positive_depth);
                                cursor.line->tag.raw = (cursor.line->tag.raw & command_mask) | (*ot_edge_page & address_mask);
                            } while (0);

                                link_word = cursor.word & address_mask;
                                ot_word = *ot_edge_page;
                                cursor.bytes += 8;
                                ot_word &= command_mask;
                                ot_word |= link_word;
                                *ot_edge_page = ot_word;

                        }
                    }
                    edge += 1;
                } while (edge < 3);
            }
            face += 1;
            i += 1;
        } while (i < count);
    }
    return cursor.bytes - output;
}
