#ifndef PE1_DRAW_AREA_H
#define PE1_DRAW_AREA_H

#include "common.h"
#include "pe1/psyq_gpu.h"
#include "pe1/render_packets.h"

typedef struct DrawAreaRect {
    s16 x, y;
    u16 w, h;
} DrawAreaRect;

void SetDrawArea(GpuCmdPacket *packet, DrawAreaRect *rect);

/* PsyZ keeps the address and command count in separate native words. */
static inline void Draw_LinkDrawArea(u32 *entry, GpuCmdPacket *packet)
{
    if (!packet) return;
    RenderNativeTag *ordering = (RenderNativeTag *)entry;
    packet->u0.tag = ordering->address;
    ordering->address = (uintptr_t)packet;
}

#endif
