#include "pe1/gte.h"
#include "pe1/render_object.h"
#include "pe1/render_buffers.h"
extern RenderBufferPrefix D_800B0E38;

/* Original ordering-table insertion using native PsyZ packet links. */
/* A packet seen as bytes and as the address the DMA chain links through. */
typedef union RenderPacketAddress {
    u8 *bytes;
    uintptr_t word;
} RenderPacketAddress;

#define RENDER_TAG(p) ((RenderNativeTag *)(p))
#define RENDER_ADDPRIM(ot, packet) \
    (RENDER_TAG((packet).bytes)->address = RENDER_TAG(ot)->address, \
     RENDER_TAG(ot)->address = (packet).word)

/* NCLIP-tests each model face on the projected vertices and links the
 * faces with positive winding into the active ordering table at their
 * average depth; culled faces get a null link. Quads and triangles of both
 * packet classes are walked in turn through one shared descriptor cursor.
 * The room overlays carry a copy that keeps the negative winding instead. */
void Render_DrawTexturedQuads(RenderObjectEntity *entity) {
    int i;
    s32 clip_area;
    s32 *area = &clip_area;
    u32 *xy = D_800B1644;
    RenderPrimitiveDescriptor *record = entity->primitive_descriptors;
    u8 *packets = entity->primitive_buffer;
    int slot = g_ActiveDrawSlot;
    RenderNativeTag *ordering = D_800B0E38.ordering[slot];
    s32 *z = D_800A636C;
    u32 xy0, xy1, xy2;
    int a, b, c, d;
    RenderPacketAddress packet;

    i = 0;
    while (i < entity->header->packet34_count) {
        a = record->lookup_indices[0];
        b = record->lookup_indices[1];
        c = record->lookup_indices[2];
        xy0 = xy[a];
        xy1 = xy[b];
        xy2 = xy[c];
        gte_ldsxy0(xy0);
        gte_ldsxy2(xy2);
        gte_ldsxy1(xy1);
        gte_cop2_hazard_slot();
        gte_cop2_hazard_slot();
        gte_nclip_now();
        packet.bytes = (u8 *)((RenderPacket34 *)packets + slot);
        d = record->lookup_indices[3];
        gte_stmac0(area);
        if (*area > 0) {
            int depth = z[a] + z[b];
            depth += z[c];
            depth += z[d];
            depth >>= 4;
            if (depth < 0x1000) {
                RENDER_ADDPRIM(&ordering[depth], packet);
                ((RenderPacket34 *)packet.bytes)->sxy0 = xy0;
                ((RenderPacket34 *)packet.bytes)->sxy1 = xy1;
                ((RenderPacket34 *)packet.bytes)->sxy2 = xy2;
                ((RenderPacket34 *)packet.bytes)->sxy3 = xy[d];
            }
        } else {
            RENDER_TAG(packet.bytes)->address = 0;
        }
        packets += 2 * sizeof(RenderPacket34);
        i++;
        record++;
    }

    i = 0;
    while (i < entity->header->packet28_count) {
        a = record->lookup_indices[0];
        b = record->lookup_indices[1];
        c = record->lookup_indices[2];
        xy0 = xy[a];
        xy1 = xy[b];
        xy2 = xy[c];
        gte_ldsxy0(xy0);
        gte_ldsxy2(xy2);
        gte_ldsxy1(xy1);
        gte_cop2_hazard_slot();
        gte_cop2_hazard_slot();
        gte_nclip_now();
        packet.bytes = (u8 *)((RenderPacket28 *)packets + slot);
        gte_stmac0(area);
        if (*area > 0) {
            int depth = z[a] + z[b] + z[c];
            depth = depth / 3 >> 2;
            if (depth < 0x1000) {
                RENDER_ADDPRIM(&ordering[depth], packet);
                ((RenderPacket28 *)packet.bytes)->sxy0 = xy0;
                ((RenderPacket28 *)packet.bytes)->sxy1 = xy1;
                ((RenderPacket28 *)packet.bytes)->sxy2 = xy2;
            }
        } else {
            RENDER_TAG(packet.bytes)->address = 0;
        }
        packets += 2 * sizeof(RenderPacket28);
        i++;
        record++;
    }

    i = 0;
    while (i < entity->header->packet24_count) {
        a = record->lookup_indices[0];
        b = record->lookup_indices[1];
        c = record->lookup_indices[2];
        xy0 = xy[a];
        xy1 = xy[b];
        xy2 = xy[c];
        gte_ldsxy0(xy0);
        gte_ldsxy2(xy2);
        gte_ldsxy1(xy1);
        gte_cop2_hazard_slot();
        gte_cop2_hazard_slot();
        gte_nclip_now();
        packet.bytes = (u8 *)((RenderPacket24 *)packets + slot);
        d = record->lookup_indices[3];
        gte_stmac0(area);
        if (*area > 0) {
            int depth = z[a] + z[b];
            depth += z[c];
            depth += z[d];
            depth >>= 4;
            if (depth < 0x1000) {
                RENDER_ADDPRIM(&ordering[depth], packet);
                ((RenderPacket24 *)packet.bytes)->sxy0 = xy0;
                ((RenderPacket24 *)packet.bytes)->sxy1 = xy1;
                ((RenderPacket24 *)packet.bytes)->sxy2 = xy2;
                ((RenderPacket24 *)packet.bytes)->sxy3 = xy[d];
            }
        } else {
            RENDER_TAG(packet.bytes)->address = 0;
        }
        packets += 2 * sizeof(RenderPacket24);
        i++;
        record++;
    }

    i = 0;
    while (i < entity->header->packet1c_count) {
        a = record->lookup_indices[0];
        b = record->lookup_indices[1];
        c = record->lookup_indices[2];
        xy0 = xy[a];
        xy1 = xy[b];
        xy2 = xy[c];
        gte_ldsxy0(xy0);
        gte_ldsxy2(xy2);
        gte_ldsxy1(xy1);
        gte_cop2_hazard_slot();
        gte_cop2_hazard_slot();
        gte_nclip_now();
        packet.bytes = (u8 *)((RenderPacket1C *)packets + slot);
        gte_stmac0(area);
        if (*area > 0) {
            int depth = z[a] + z[b] + z[c];
            depth = depth / 3 >> 2;
            if (depth < 0x1000) {
                RENDER_ADDPRIM(&ordering[depth], packet);
                ((RenderPacket1C *)packet.bytes)->sxy0 = xy0;
                ((RenderPacket1C *)packet.bytes)->sxy1 = xy1;
                ((RenderPacket1C *)packet.bytes)->sxy2 = xy2;
            }
        } else {
            RENDER_TAG(packet.bytes)->address = 0;
        }
        packets += 2 * sizeof(RenderPacket1C);
        i++;
        record++;
    }
}


