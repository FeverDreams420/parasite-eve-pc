#ifndef PE1_RENDER_TINT_H
#define PE1_RENDER_TINT_H

#include "common.h"

/* Native PsyZ DMA header; command payload words retain their PS1 layout. */
#include "pe1/render_packets.h"
typedef RenderNativeTag RenderGpuTag;

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

PE1_STATIC_ASSERT(sizeof(RenderGpuTag) == 2 * sizeof(uintptr_t), render_gpu_tag_size);
PE1_STATIC_ASSERT(sizeof(RenderTintTile) == ((sizeof(RenderGpuTag) + 12 + _Alignof(RenderTintTile) - 1) / _Alignof(RenderTintTile)) * _Alignof(RenderTintTile), render_tint_tile_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RenderTintTile, r) == sizeof(RenderGpuTag), render_tint_tile_rgb_offset);
PE1_STATIC_ASSERT(sizeof(RenderTintMode) == ((sizeof(RenderGpuTag) + 4 + _Alignof(RenderTintMode) - 1) / _Alignof(RenderTintMode)) * _Alignof(RenderTintMode), render_tint_mode_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RenderTintMode, command) == sizeof(RenderGpuTag), render_tint_command_offset);
PE1_STATIC_ASSERT(sizeof(RenderTintState) == ((PE1_OFFSETOF(RenderTintState, frame) + 4 + _Alignof(RenderTintState) - 1) / _Alignof(RenderTintState)) * _Alignof(RenderTintState), render_tint_state_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RenderTintState, modes) == 2 * sizeof(RenderTintTile), render_tint_modes_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RenderTintState, targetR) == 2 * (sizeof(RenderTintTile) + sizeof(RenderTintMode)), render_tint_target_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RenderTintState, fadeMode) == PE1_OFFSETOF(RenderTintState, targetR) + 6, render_tint_flags_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RenderTintState, startR) == PE1_OFFSETOF(RenderTintState, targetR) + 8, render_tint_start_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RenderTintState, duration) == PE1_OFFSETOF(RenderTintState, startR) + 6, render_tint_duration_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RenderTintState, frame) == PE1_OFFSETOF(RenderTintState, duration) + 2, render_tint_frame_offset);

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
