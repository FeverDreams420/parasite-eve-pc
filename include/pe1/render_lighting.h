#ifndef PE1_RENDER_LIGHTING_H
#define PE1_RENDER_LIGHTING_H

#include "pe1/gte_types.h"
#include "pe1/scene_room_load.h"

/* MATRIX arithmetic and the packed word transfers used by the GTE. */
typedef union RenderLightingMatrix {
    GteMatrix matrix;
    u32 words[8];
} RenderLightingMatrix;

extern RenderLightingMatrix g_RenderLightingState, g_RenderLightColorMatrix;

/* Lighting block at g_RenderLightingState: the light-direction matrix, the light
 * color matrix (one column per light) and the per-light 8-bit colors. */
typedef struct RenderLightingState {
    RenderLightingMatrix light;  /* 0x00 */
    RenderLightingMatrix color;  /* 0x20 */
    u8 light_colors[3][4];       /* 0x40: r, g, b per light */
} RenderLightingState;
/* One-field views of g_RenderLightingState.matrix.m[0][1] (light 0
 * direction y) and m[1][0] (light 1 direction x). Draw_InitPipeline takes
 * the light row addresses from them; written as fields, cc1 reuses the
 * that already holds the matrix base and retail does not. */
extern s16 g_RenderLight0DirY, g_RenderLight1DirX;
typedef struct RenderLightColor {
    u8 r, g, b;
} RenderLightColor;

PE1_STATIC_ASSERT(sizeof(RenderLightColor) == 3, render_light_color_size);
extern RenderLightColor g_RoomLightTint;

#endif
