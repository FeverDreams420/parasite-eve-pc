#ifndef PE_PC_ROOM_H
#define PE_PC_ROOM_H

#include "disc.h"

/* Table ends at 0x80094188, the documented PM handler array. The last
 * eight-byte slot is all zero, but is still included in the data audit. */
#define PE_USA_ROOM_SLOTS 450u

typedef struct PeRoomRange {
    uint32_t start;
    uint16_t scratch_sectors, texture_sectors, room_sectors;
} PeRoomRange;

typedef struct PeRoomCatalog {
    PeDisc *disc; /* borrowed; must remain open */
    PeDiscFile archive;
    uint16_t sector_bounds[81]; /* resident/startup/UI data ranges */
    PeRoomRange ranges[PE_USA_ROOM_SLOTS];
    char error[256];
} PeRoomCatalog;

typedef struct PeRoomBlob {
    uint8_t *bytes;
    size_t size;
    uint32_t directory[13]; /* decoded words, never cast to host pointers */
} PeRoomBlob;

typedef struct PeRoom {
    unsigned map;
    PeRoomBlob scratch, textures, data;
} PeRoom;

/* Read only data tables from the supplied USA executable inside the image.
 * No PS1 code runs, no extracted executable or linker addresses are needed. */
int pe_room_catalog_open(PeRoomCatalog *catalog, PeDisc *disc);
/* Room starts zero-initialized. On failure the previous room is preserved. */
int pe_room_load(PeRoomCatalog *catalog, unsigned map, PeRoom *room);
void pe_room_close(PeRoom *room);
/* Packed count/offset table, with explicit record stride and bounds checks. */
int pe_room_blob_table(const PeRoomBlob *blob, unsigned word, size_t stride,
                       const uint8_t **records, unsigned *count);

#endif
