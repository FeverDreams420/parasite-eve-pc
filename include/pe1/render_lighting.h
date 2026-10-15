#ifndef PE1_RENDER_LIGHTING_H
#define PE1_RENDER_LIGHTING_H

#include "pe1/gte_types.h"
#include "pe1/geom_state.h"

/* MATRIX arithmetic and the packed word transfers used by the GTE. */
typedef union RenderLightingMatrix {
    GteMatrix matrix;
    u32 words[8];
} RenderLightingMatrix;

/* Lighting block at g_RenderLightingState: the light-direction matrix, the light
 * color matrix (one column per light) and the per-light 8-bit colors. */
typedef struct RenderLightingState {
    RenderLightingMatrix light;  /* 0x00 */
    RenderLightingMatrix color;  /* 0x20 */
    u8 light_colors[3][4];       /* 0x40: r, g, b per light */
} RenderLightingState;
extern RenderLightingState g_RenderLightingBlock;
#define g_RenderLightingState (g_RenderLightingBlock.light)
#define g_RenderLightColorMatrix (g_RenderLightingBlock.color)
#define g_RenderLight0DirY (g_RenderLightingBlock.light.matrix.m[0][1])
#define g_RenderLight1DirX (g_RenderLightingBlock.light.matrix.m[1][0])
void Boot_SetBackColor(u16 *destination, int r, int g, int b);
void Boot_SetLightColor(RenderLightingState *lighting, int index, int r, int g, u16 b);
/* One tint shared by view reset, scripts and model lighting. */
#define g_RoomLightTint (g_GeomScroll.state.position.lightTint)

#endif
