#include "collision.h"
#include "room.h"
#include <stdio.h>
static uint32_t word(const uint8_t *p) {
    return p[0] | (uint32_t)p[1]<<8 | (uint32_t)p[2]<<16 | (uint32_t)p[3]<<24;
}
int pe_room_collision(const PeRoom *room, PeCollision *db, char *error, size_t error_size) {
    const uint8_t *records; unsigned count;
    if (!pe_room_blob_table(&room->data, 6, 8, &records, &count) || !count) {
        snprintf(error, error_size, "Map %u: missing collision record", room->map); return 0;
    }
    size_t size = word(records) & UINT32_C(0xffffff);
    uint32_t offset = word(records+4) & UINT32_C(0xffffff);
    if (offset > room->data.size || size > room->data.size-offset) {
        snprintf(error, error_size, "Map %u: collision payload exceeds room", room->map); return 0;
    }
    return pe_collision_open(db, room->data.bytes+offset, size, error, error_size);
}
