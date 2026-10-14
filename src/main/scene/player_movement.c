#include "pe1/geom_state.h"
#include "pe1/asset_tim.h"
#include "pe1/battle_runtime.h"
#include "pe1/field_movement.h"
#include "pe1/gte.h"
#include "pe1/gte_types.h"
#include "pe1/pad_state.h"
#include "pe1/render_camera.h"

/* Game-side TIM container helpers: upload a TIM's image and optional CLUT,
 * and return its CLUT/image rectangles and pixel pointers. No Psy-Q
 * signature of any SDK version matches these bytes; they are linked as the
 * last game object, directly before the LIBC veneers. */

int *Asset_LoadTimImage(TimFile *tim) {
    TimBlock *clut;
    TimBlock *image;
    int *pixels;

    clut = 0;
    if ((tim->flags & 8) != 0) {
        clut = &tim->first_block;
        image = (TimBlock *)&clut->bytes[clut->header.length];
    } else {
        image = &tim->first_block;
    }

    pixels = image->header.pixels;
    LoadImage(&image->header.rect, pixels);
    if (clut != 0) {
        LoadImage(&clut->header.rect, clut->header.pixels);
    }
    return pixels;
}

RECT *Asset_GetTimClutRect(TimFile *tim) {
    if (tim->flags & 8) {
        return &tim->first_block.header.rect;
    }
    return 0;
}

static inline TimBlock *ImageBlock(TimFile *tim) {
    TimBlock *image;

    if (tim->flags & 8) {
        TimBlock *clut = &tim->first_block;
        image = (TimBlock *)&clut->bytes[clut->header.length];
    } else {
        image = &tim->first_block;
    }
    return image;
}

RECT *Asset_GetTimImageRect(TimFile *tim) {
    return &ImageBlock(tim)->header.rect;
}

int *Asset_GetTimImagePixels(TimFile *tim) {
    return ImageBlock(tim)->header.pixels;
}

int *Asset_GetTimClutPixels(TimFile *tim) {
    if (tim->flags & 8) {
        return tim->first_block.header.pixels;
    }
    return 0;
}
