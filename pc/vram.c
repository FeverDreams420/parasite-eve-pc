#include "vram.h"
#include <string.h>

static uint32_t read32(const uint8_t *p) {
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 |
           (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}

static int image(const PeRoomBlob *blob, uint64_t offset, uint32_t geometry,
                  unsigned height, PeVram *target, PeVramStats *stats) {
    unsigned width = geometry & 1023;
    unsigned x = (geometry >> 10) & 2047, y = geometry >> 21;
    if (!width || !height) return 1; /* LoadImage's empty rectangle */
    if (x >= PE_VRAM_WIDTH || y >= PE_VRAM_HEIGHT ||
        width > PE_VRAM_WIDTH - x || height > PE_VRAM_HEIGHT - y ||
        offset > blob->size || (uint64_t)width * height * 2 > blob->size - offset)
        return 0;
    if (target) {
        const uint8_t *source = blob->bytes + (size_t)offset;
        for (unsigned row = 0; row < height; ++row) {
            uint16_t *dest = target->words + (y + row) * PE_VRAM_WIDTH + x;
            for (unsigned col = 0; col < width; ++col) {
                dest[col] = (uint16_t)source[0] | (uint16_t)source[1] << 8;
                source += 2;
            }
        }
        ++stats->uploads;
        stats->words += (uint64_t)width * height;
    }
    return 1;
}

static int bank(const PeRoomBlob *blob, PeVram *target, PeVramStats *stats) {
    const uint8_t *records;
    unsigned count;
    if (!pe_room_blob_table(blob, 10, 20, &records, &count)) return 0;
    for (unsigned i = 0; i < count; ++i) {
        const uint8_t *record = records + i * 20;
        uint32_t primary = read32(record + 4), secondary = read32(record + 12);
        uint64_t offset = primary & UINT32_C(0xffffff);
        unsigned height = primary >> 24;
        if (!image(blob, offset, read32(record + 8), height ? height : 256,
                    target, stats)) return 0;
        if (secondary & UINT32_C(0xffffff)) {
            if (!image(blob, offset + (secondary & UINT32_C(0xffffff)),
                        read32(record + 16), secondary >> 24, target, stats)) return 0;
        }
    }
    return 1;
}

int pe_room_upload_vram(const PeRoom *room, PeVram *vram, PeVramStats *stats,
                        char *error, size_t error_size) {
    const PeRoomBlob *banks[] = {&room->scratch, &room->textures};
    PeVramStats next = {0};
    for (unsigned i = 0; i < 2; ++i) {
        if (!bank(banks[i], NULL, NULL)) {
            snprintf(error, error_size, "Map %u: invalid %s texture upload",
                     room->map, i == 0 ? "scratch" : "texture-bank");
            return 0;
        }
    }
    for (unsigned i = 0; i < 2; ++i) bank(banks[i], vram, &next);
    if (stats) *stats = next;
    if (error_size) error[0] = 0;
    return 1;
}
int pe_blob_upload_vram(const PeRoomBlob *blob,PeVram *vram,PeVramStats *stats,char *error,size_t error_size){
    if(!bank(blob,NULL,NULL)){snprintf(error,error_size,"Invalid asset-bank texture upload");return 0;}
    PeVramStats next={0};bank(blob,vram,&next);
    if(stats)*stats=next;if(error_size)error[0]=0;return 1;
}
