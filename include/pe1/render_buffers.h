#ifndef PE1_RENDER_BUFFERS_H
#define PE1_RENDER_BUFFERS_H

/* Work-buffer bases carved out by Draw_InitDisplay. The -G8 menu units read
 * them with absolute addresses, so they are incomplete arrays used at
 * element 0. */
extern unsigned char *g_RenderOtBufferBaseAlt[];
extern unsigned char *g_RenderScratchBufferBase[];
extern unsigned char *g_RenderFrontBufferBase[];
extern unsigned char *g_RenderBackBufferBase[];
#include "pe1/scene_archive.h"

/* Double-buffer slot (0 or 1) selecting the per-slot tables below and the
 * display/draw environments. */
extern int g_ActiveDrawSlot;
/* Per-slot ordering tables and animation buffers. */
extern unsigned char *g_OtBufferTable[];
extern unsigned char *g_RenderAnimBufferBase[];

#endif
