#ifndef PE_PC_COLLISION_H
#define PE_PC_COLLISION_H
#include "collision_math.h"
#include <stddef.h>
typedef struct PeCollision {
    uint8_t *bytes;
    size_t size;
    uint32_t vertices, triangles, planes;
    uint16_t region_count, triangle_count;
} PeCollision;
typedef struct PeFloorHit { uint16_t triangle, region; uint32_t height; } PeFloorHit;
/* Zero initialize. Checked relative data, copied and owned; failure preserves
 * the previous database. Room record selection and wall/ramp walking pending. */
int pe_collision_open(PeCollision *db, const uint8_t *bytes, size_t size, char *error, size_t error_size);
void pe_collision_close(PeCollision *db);
/* First matching region/triangle in original order. Flags 0x80 bypass lookup;
 * flag 2 preserves supplied Y. No match preserves output. */
int pe_collision_find_floor(const PeCollision *db, uint32_t x, uint32_t y, uint32_t z,
                            uint32_t flags, PeFloorHit *hit);
#endif
