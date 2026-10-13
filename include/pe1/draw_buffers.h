#ifndef PE1_DRAW_BUFFERS_H
#define PE1_DRAW_BUFFERS_H

#include "common.h"
#include "pe1/psyq_gpu.h"

/* Game double-buffer records: SDK environments followed by the ordering-table
 * and packet-arena pointers. Selection advances by one 0x78-byte record. */
typedef struct DrawFrameBuffer {
    DRAWENV draw;
    DISPENV display;
    u32 *orderingTableBase;
    u8 *frontBufferBase;
} DrawFrameBuffer;

PE1_STATIC_ASSERT(sizeof(DrawFrameBuffer) == 0x78, draw_frame_buffer_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(DrawFrameBuffer, display) == 0x5C,
                  draw_frame_buffer_display_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(DrawFrameBuffer, orderingTableBase) == 0x70,
                  draw_frame_buffer_ordering_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(DrawFrameBuffer, frontBufferBase) == 0x74,
                  draw_frame_buffer_packets_offset);
/* Double buffer: environments, ordering table and packet arena per frame. */
extern DrawFrameBuffer g_DrawFrameBuffers[2];
extern DrawFrameBuffer *g_DrawCurrentFrameBuffer;
/* Frame buffer the game draws into this frame, and the one presented. */
extern int g_ActiveDrawSlot;
extern int g_DrawBufferIndex;
extern int g_DrawPresentEnabled;
/* Image uploaded over the presented frame when nonzero. */
extern int g_DrawPresentImage;
extern u32 *g_DrawOrderingTableBase;

/* Main renderer keeps two SDK environments alongside its buffer pointers. */
extern DISPENV g_RenderDispEnvArray[2];
extern DRAWENV g_RenderDrawEnvArray[2];
/* Screen vertical offset (0..15) mirrored into both display environments. */
extern signed char g_DrawBaseY;

#endif
