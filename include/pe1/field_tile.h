#ifndef PE1_FIELD_TILE_H
#define PE1_FIELD_TILE_H

#include "common.h"
#include "pe1/field_sprite_draw.h"
#include "pe1/field_sprite_params.h"
#include "pe1/gte_types.h"
#include "pe1/psyq_gpu.h"
#include "pe1/psyq_libc.h"
#include "pe1/render_prim.h"
#include "pe1/render_tint.h"

/* Flat colour tiles linked straight into the active ordering table. */

/* An ordering-table entry or packet seen both as its tag and as the 32-bit
 * address the GPU DMA chain links through. */
typedef union FieldTileAddress {
    RenderGpuTag *tag;
    char *bytes;
    u32 word;
} FieldTileAddress;

/* Entry `depth` of the table held in `base`: retail adds the scaled depth
 * to the table address as an integer, index first. */
#define TILE_OT_ENTRY(entry, base, table, depth) \
    ((base).bytes = (table), (entry).word = (depth) * 4 + (base).word)

/* PSY-Q addPrim: link `packet` (with a RenderGpuTag tag) in front of the
 * chain at `entry`, storing the low 24 address bits in each tag. */
#define TILE_OT_ADDPRIM(entry, packet, link) \
    ((packet)->tag.address = (entry).tag->address, \
     (link).tag = &(packet)->tag, \
     (entry).tag->address = (link).word)

/* One-pixel TILE_1 packet. */
typedef struct FieldTilePoint {
    RenderGpuTag tag;
    u8 r, g, b, command;
    s16 x, y;
} FieldTilePoint;

/* PSY-Q POLY_G4 (history trail and ring band quads). */
typedef struct FieldG4Packet {
    /* 0x00 */ RenderGpuTag tag;
    /* 0x04 */ u8 r0, g0, b0, code;
    /* 0x08 */ s16 x0, y0;
    /* 0x0C */ u8 r1, g1, b1, p1;
    /* 0x10 */ s16 x1, y1;
    /* 0x14 */ u8 r2, g2, b2, p2;
    /* 0x18 */ s16 x2, y2;
    /* 0x1C */ u8 r3, g3, b3, p3;
    /* 0x20 */ s16 x3, y3;
} FieldG4Packet;

PE1_STATIC_ASSERT(sizeof(FieldG4Packet) == 0x24, field_g4_packet_size);

/* Matches render_object.h, for includers that do not include that header. */

void SetSemiTrans(void *packet, int dither);
void SetTile1(FieldTilePoint *tile);

#endif
