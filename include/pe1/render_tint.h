#ifndef PE1_RENDER_TINT_H
#define PE1_RENDER_TINT_H

#include "common.h"

/* Little-endian PSX DMA packet tag. */
typedef struct RenderGpuTag {
    u32 address : 24;
    u32 length : 8;
} RenderGpuTag;

typedef struct RenderTintTile {
    RenderGpuTag tag;
    u8 r, g, b, command;
    s16 x, y, width, height;
} RenderTintTile;

typedef struct RenderTintMode {
    RenderGpuTag tag;
    u32 command;
} RenderTintMode;

/* Embedded at 0x30 in the state at 0x800BCF88. */
typedef struct RenderTintState {
    RenderTintTile tiles[2];
    RenderTintMode modes[2];
    s16 targetR, targetG, targetB;
    u8 fadeMode, blendMode;
    s16 startR, startG, startB;
    u16 duration, frame;
    /* Screen fade length and elapsed frames (g_RenderFadeFrameCount and
     * g_RenderFadeFrameCounter, Render_StartFadeIn/StepFade). */
    u8 fadeFrameCount, fadeFrameCounter;
} RenderTintState;

PE1_STATIC_ASSERT(sizeof(RenderGpuTag) == 4, render_gpu_tag_size);
PE1_STATIC_ASSERT(sizeof(RenderTintTile) == 16, render_tint_tile_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RenderTintTile, r) == 4, render_tint_tile_rgb_offset);
PE1_STATIC_ASSERT(sizeof(RenderTintMode) == 8, render_tint_mode_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RenderTintMode, command) == 4, render_tint_command_offset);
PE1_STATIC_ASSERT(sizeof(RenderTintState) == 0x44, render_tint_state_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RenderTintState, modes) == 0x20, render_tint_modes_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RenderTintState, targetR) == 0x30, render_tint_target_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RenderTintState, fadeMode) == 0x36, render_tint_flags_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RenderTintState, startR) == 0x38, render_tint_start_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RenderTintState, duration) == 0x3E, render_tint_duration_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RenderTintState, frame) == 0x40, render_tint_frame_offset);

/* The target colour at RenderTintState.targetR, also tested as one word:
 * red and green together are 0x00FF00FF when both are full. */
typedef union RenderTintTargetView {
    struct {
        s16 r, g, b;
        u8 fadeMode, blendMode;
    } colour;
    struct {
        u32 red_green;
        s16 blue;
        u8 fadeMode;
    } packed;
} RenderTintTargetView;

int Render_SetCDDCSlot(void);

#endif
