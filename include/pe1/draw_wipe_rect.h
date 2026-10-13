#ifndef PE1_DRAW_WIPE_RECT_H
#define PE1_DRAW_WIPE_RECT_H

#include "common.h"
#include "pe1/bounds_check.h"
#include "pe1/draw_state.h"
#include "pe1/psyq_gpu.h"
#include "pe1/render_prim.h"

/* Rectangular wipe bars: four corner vertices pushed onto the halfword
 * vertex stack, then the shared wipe-bar edge list. The blended palette
 * is the inline form of Draw_BlendColor. */

extern int g_DrawGradientBlendColor;
extern int g_DrawBlendColor;
extern int g_SavedDrawBlendColor[];
extern int g_DrawScratchBuffer[];
extern u16 *g_DrawVertexWritePtr;
extern u8 g_WipeRectEdgeList[];

/* A packet in the 16 KiB packet arena, or an ordering-table link word. */
typedef union DrawPacketAddress {
    char *bytes;
    u32 *tag;
    GpuCmdPacket *window;
    RenderSpritePacket *sprite;
    u32 word;
} DrawPacketAddress;

extern u8 *D_8009D100;                  /* packet cursor address */
extern u8 *D_8009D104;                  /* packet arena base address */
extern int g_DrawTextDimmed;                 /* colour select */
extern unsigned int g_DrawPrimColor;                 /* primary colour */
extern unsigned int g_DrawColorShaded;                 /* alternate colour */
extern u32 *D_8009D11C;                 /* ordering-table entry */

void SetTexWindow(GpuCmdPacket *packet, RECT *window);
void Draw_AllocColorTri(int width, int height, int pulse);

static inline void Draw_BlendColorInline(int color)
{
    int *dst;
    int value;

    g_DrawBlendColor = color & 0xFFFFFF;

    value = (((color >> 16) & 0xFF) + ((color >> 8) & 0xFF)) >> 1;
    dst = g_DrawScratchBuffer;
    if (value >= 0x100) {
        value = 0xFF;
    }

    {
        int half = (((color >> 16) & 0xFF) + (color & 0xFF)) >> 1;
        if (half < 0x100) {
            value |= half << 8;
        } else {
            value |= 0xFF00;
        }
    }
    {
        int half = (((color >> 8) & 0xFF) + (color & 0xFF)) >> 1;
        *dst = value | ((half < 0x100) ? (half << 16) : 0xFF0000);
    }
}

typedef struct DrawWipeVertex {
    s16 y, x;
} DrawWipeVertex;

PE1_STATIC_ASSERT(sizeof(DrawWipeVertex) == 4, draw_wipe_vertex_size);

/* Push one (x, y) vertex, stored y first, while below `limit`. */
#define DRAW_PUSH_WIPE_VERTEX(xValue, yValue, limit) \
    {                                                \
        int x = (xValue);                            \
        int y = (yValue);                            \
        DrawWipeVertex *out = g_DrawVertexWritePtr;             \
                                                     \
        if (out < (DrawWipeVertex *)(limit)) {                         \
            out->x = x;                              \
            g_DrawVertexWritePtr = (u16 *)(out + 1);          \
            out->y = y;                              \
        } else {                                     \
            Inv_BoundsCheckAssertStub(4);               \
        }                                            \
    }

/* Reserve `size` bytes of the packet arena into `packet` (0 on overflow). */
#define DRAW_ALLOC_PACKET(packet, size)                                  \
    {                                                                    \
        u32 old = (u32)D_8009D100;                                            \
        u32 next = old + (size);                                         \
                                                                         \
        (packet).word = 0;                                               \
        if (next < (u32)D_8009D104 + 0x4000) {                                \
            D_8009D100 = next;                                           \
            (packet).word = old;                                         \
        } else {                                                         \
            Inv_BoundsCheckAssertStub(1);                                   \
        }                                                                \
    }

/* PSY-Q addPrim on the current ordering-table entry. */
#define DRAW_LINK_PACKET(packet)                                         \
    {                                                                    \
        u32 mask24 = 0xFFFFFF, maskTop = 0xFF000000;                     \
        DrawPacketAddress *ot = D_8009D11C;                              \
        u32 tag = *(packet).tag;                                         \
        tag &= maskTop;                                                  \
        tag |= ot->word & mask24;                                        \
        *(packet).tag = tag;                                             \
        ot->word = (ot->word & maskTop) | ((packet).word & mask24);      \
    }

#endif /* PE1_DRAW_WIPE_RECT_H */
