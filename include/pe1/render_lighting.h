#ifndef PE1_RENDER_LIGHTING_H
#define PE1_RENDER_LIGHTING_H

#include "pe1/gte_types.h"
#include "pe1/scene_room_load.h"

/* MATRIX arithmetic and the packed word transfers used by the GTE. */
typedef union RenderLightingMatrix {
    GteMatrix matrix;
    u32 words[8];
} RenderLightingMatrix;

extern RenderLightingMatrix g_RenderLightingState, D_800BEA60;

/* Lighting block at g_RenderLightingState: the light-direction matrix, the light
 * color matrix (one column per light) and the per-light 8-bit colors. */
typedef struct RenderLightingState {
    RenderLightingMatrix light;  /* 0x00 */
    RenderLightingMatrix color;  /* 0x20 */
    u8 light_colors[3][4];       /* 0x40: r, g, b per light */
} RenderLightingState;
/* Existing absolute symbols expose individual matrix elements. */
extern s16 D_800BEA42, D_800BEA44, D_800BEA46, D_800BEA48, D_800BEA4A;
extern s16 D_800BEA4C, D_800BEA4E, D_800BEA50;
extern s16 D_800BEA62, D_800BEA64, D_800BEA66, D_800BEA68, D_800BEA6A;
extern s16 D_800BEA6C, D_800BEA6E, D_800BEA70;
typedef struct RenderLightColor {
    u8 r, g, b;
} RenderLightColor;

PE1_STATIC_ASSERT(sizeof(RenderLightColor) == 3, render_light_color_size);
extern RenderLightColor g_RoomLightTint;

#endif
