#include "pe1/scene_assets.h"
#include <libgpu.h>

int Draw_LoadTimAsset(TimUploadRecord *asset, void *base) {
    RECT rect;
    int mask;
    u8 *saved_base;
    unsigned int packed;
    int h;
    unsigned int offset;
    int secondary;
    unsigned int image_offset;
    u8 *image_address;

    packed = asset->image_rectangle;
    rect.x = (packed >> 10) & 0x7FF;
    packed = asset->image_rectangle;
    rect.y = packed >> 21;
    packed = asset->image_rectangle;
    rect.w = packed & 0x3FF;
    saved_base = base;
    packed = asset->image_source.bytes.height;
    if (packed != 0) {
        h = packed & 0xFF;
    } else {
        h = 0x100;
    }

    mask = 0xFFFFFF;
    rect.h = h;
    LoadImage(&rect, (u_long *)(saved_base + (asset->image_source.offsetAndHeight & mask)));

    secondary = asset->secondary_source.offsetAndHeight & mask;
    if (secondary != 0) {
        packed = asset->secondary_rectangle;
        rect.x = (packed >> 10) & 0x7FF;
        packed = asset->secondary_rectangle;
        rect.y = packed >> 21;
        packed = asset->secondary_rectangle;
        rect.w = packed & 0x3FF;
        rect.h = asset->secondary_source.bytes.height;
        image_offset = asset->image_source.offsetAndHeight & mask;
        image_address = saved_base + image_offset;
        offset = asset->secondary_source.offsetAndHeight & mask;
        LoadImage(&rect, (u_long *)(offset + image_address));
    }

    return 0;
}

