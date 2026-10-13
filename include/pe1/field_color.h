#ifndef PE1_FIELD_COLOR_H
#define PE1_FIELD_COLOR_H

#include "common.h"

/* RGB triple plus a primitive code byte, as copied into GPU packets. */
typedef struct RenderColor {
    u8 r, g, b, code;
} RenderColor;

PE1_STATIC_ASSERT(sizeof(RenderColor) == 4, render_color_size);

/* In a color-track key, color.code stores the encoded segment duration. */
typedef struct RenderColorKeyTiming {
    u16 length;
    u16 start;
} RenderColorKeyTiming;

typedef struct RenderColorKey {
    RenderColor color;
    RenderColorKeyTiming timing;
} RenderColorKey;

typedef struct RenderColorTrack {
    int duration; /* Zero means the timing cache has not been initialized. */
    int count;
    RenderColorKey keys[0];
} RenderColorTrack;

PE1_STATIC_ASSERT(sizeof(RenderColorKey) == 8, render_color_key_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RenderColorKey, timing) == 4,
                  render_color_key_timing_offset);
PE1_STATIC_ASSERT(sizeof(RenderColorTrack) == 8, render_color_track_header_size);

#endif /* PE1_FIELD_COLOR_H */
