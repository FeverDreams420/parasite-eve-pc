#ifndef PE1_FIELD_SHADED_QUAD_H
#define PE1_FIELD_SHADED_QUAD_H

#include "common.h"
#include "pe1/field_sprite_state.h"
#include "pe1/fieldfx_scratch.h"
#include "pe1/gte_types.h"
#include "pe1/render_tint.h"

/* Field engine shaded quad (FieldFx_DrawShadedQuad): four vertex colours scaled by
 * a brightness, a fixed four-corner shape transformed by a placed matrix
 * and drawn as a POLY_G4 (semi-transparent when g_FieldFxSpriteSemiTrans is set). */

typedef struct FieldShadedQuadColors {
    /* 0x00 */ u8 rgb[4][4];
    /* 0x10 */ s16 depth;
    /* 0x12 */ s16 brightness;
} FieldShadedQuadColors;

/* PSY-Q POLY_G4. */
typedef struct FieldShadedQuadPacket {
    RenderGpuTag tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 r1, g1, b1, pad1;
    s16 x1, y1;
    u8 r2, g2, b2, pad2;
    s16 x2, y2;
    u8 r3, g3, b3, pad3;
    s16 x3, y3;
} FieldShadedQuadPacket;

extern char *D_800B0E58[];

/* Corners of the sprite quad around its centre, 16 units per texel; set by
 * FieldFx_SetSpriteCellSize: top left, top right, bottom left, bottom right. */
extern GteShortVector g_FieldFxCellQuad[4];

/* A packet tag seen as the 32-bit address the DMA chain links through. */
typedef union FieldShadedQuadLink {
    RenderGpuTag *tag;
    u32 word;
} FieldShadedQuadLink;

/* Ordering-table entry `depth` of the active buffer. */
#define SHADED_QUAD_OT(depth) \
    ((RenderGpuTag *)(D_800B0E38.ordering[g_ActiveDrawSlot] + (depth)))

void FieldFx_ScaleColor(int scale, u8 *src, u8 *dst);

#endif
