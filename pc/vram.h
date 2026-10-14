#ifndef PE_PC_VRAM_H
#define PE_PC_VRAM_H

#include "room.h"

#define PE_VRAM_WIDTH 1024u
#define PE_VRAM_HEIGHT 512u

typedef struct PeVram {
    uint16_t words[PE_VRAM_WIDTH * PE_VRAM_HEIGHT];
} PeVram;

typedef struct PeVramStats {
    unsigned uploads;
    uint64_t words;
} PeVramStats;

/* Apply the original scratch then texture-bank upload order. The complete
 * batch is validated before touching VRAM, including secondary offsets
 * relative to the primary image (not relative to the archive). */
int pe_room_upload_vram(const PeRoom *room, PeVram *vram, PeVramStats *stats,
                        char *error, size_t error_size);

#endif
