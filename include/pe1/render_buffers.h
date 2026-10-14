#ifndef PE1_RENDER_BUFFERS_H
#define PE1_RENDER_BUFFERS_H

#include "pe1/scene_archive.h"

/* Original absolute buffer symbols are views of the canonical game state.
 * Array syntax preserves the original callers without separate storage. */
#define g_RenderOtBufferBaseAlt (&g_GameState.shared_buffers.ordering_tables[1])
#define g_RenderScratchBufferBase (g_GameState.render_buffer_names.scratch)
#define g_RenderFrontBufferBase (g_GameState.render_buffer_names.front)
#define g_RenderBackBufferBase (g_GameState.render_buffer_names.back)

/* Double-buffer slot (0 or 1) selecting the per-slot tables below and the
 * display/draw environments. */
extern int g_ActiveDrawSlot;
/* Per-slot ordering tables and animation buffers. */
#define g_OtBufferTable (g_GameState.shared_buffers.ordering_tables)
#define g_RenderAnimBufferBase (g_GameState.render_buffer_names.animation)

#endif
