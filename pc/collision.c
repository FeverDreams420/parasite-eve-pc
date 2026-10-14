#include "collision.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
static uint16_t u16(const uint8_t *p) { return (uint16_t)(p[0] | (unsigned)p[1] << 8); }
static uint32_t u32(const uint8_t *p) { return u16(p) | (uint32_t)u16(p+2) << 16; }
static int16_t s16(uint16_t v) { return (int16_t)(v & 32768 ? (int)v - 65536 : (int)v); }
static int64_t s32(uint32_t v) { return v & UINT32_C(0x80000000) ? (int64_t)v - INT64_C(0x100000000) : v; }
static uint32_t mul(uint32_t a, uint32_t b) { return (uint32_t)((uint64_t)(s32(a)*s32(b)) >> 16); }
static int span(size_t size, uint32_t offset, size_t bytes) { return offset <= size && bytes <= size-offset; }

void pe_collision_close(PeCollision *db) { free(db->bytes); memset(db, 0, sizeof(*db)); }

int pe_collision_open(PeCollision *db, const uint8_t *bytes, size_t size, char *error, size_t error_size) {
    if (size < 40 || size > UINT32_MAX) goto malformed;
    PeCollision next = {0}; next.size = size;
    next.region_count = u16(bytes+2);
    /* +04 counts floor triangles. +08 sizes a traversal bitmap and may
     * also cover later collision data; it is not the floor array length. */
    next.triangle_count = u16(bytes+4); /* +06 is a separate plane count. */
    next.vertices = u32(bytes+24); next.triangles = u32(bytes+28); next.planes = u32(bytes+32);
    unsigned triangle_stride = next.planes ? 28 : 22, vertex_stride = next.planes ? 6 : 4;
    if (!span(size, 40, (size_t)next.region_count*4) ||
        !span(size, next.triangles, (size_t)next.triangle_count*triangle_stride)) goto malformed;
    for (unsigned i=0; i<next.triangle_count; ++i) {
        const uint8_t *triangle = bytes + next.triangles + i*triangle_stride;
        unsigned links = next.planes ? 8 : 2;
        for (unsigned vertex=0; vertex<3; ++vertex)
            if (!span(size, next.vertices, ((size_t)u16(triangle+links+vertex*2)+1)*vertex_stride)) goto malformed;
        if (next.planes && !span(size, next.planes, ((size_t)u16(triangle+2)+1)*12)) goto malformed;
    }
    for (unsigned i=0; i<next.region_count; ++i) {
        uint32_t region = u32(bytes+40+i*4); unsigned header = next.planes ? 6 : 4;
        if (!span(size, region, header)) goto malformed;
        unsigned count = u16(bytes+region+(next.planes ? 4 : 2));
        if (!span(size, region, header+(size_t)count*2)) goto malformed;
        for (unsigned j=0; j<count; ++j)
            if (u16(bytes+region+header+j*2) >= next.triangle_count) goto malformed;
    }
    next.bytes = malloc(size);
    if (!next.bytes) { snprintf(error, error_size, "Cannot allocate collision database"); return 0; }
    memcpy(next.bytes, bytes, size); pe_collision_close(db); *db = next;
    if (error_size) error[0]=0;
    return 1;
malformed:
    snprintf(error, error_size, "Malformed relative collision database"); return 0;
}

int pe_collision_triangle_xz(const PeCollision *db, unsigned id, PeCollisionXZ vertices[3]) {
    if (id >= db->triangle_count) return 0;
    const uint8_t *triangle = db->bytes+db->triangles+id*(db->planes ? 28 : 22);
    for (unsigned v=0; v<3; ++v) {
        unsigned index = u16(triangle+(db->planes ? 8 : 2)+v*2);
        const uint8_t *vertex = db->bytes+db->vertices+index*(db->planes ? 6 : 4);
        vertices[v] = (PeCollisionXZ){s16(u16(vertex)),s16(u16(vertex+(db->planes ? 4 : 2)))};
    }
    return 1;
}

int pe_collision_triangle_in_regions(const PeCollision *db, unsigned id) {
    if (id >= db->triangle_count) return 0;
    for (unsigned region=0; region<db->region_count; ++region) {
        const uint8_t *record = db->bytes+u32(db->bytes+40+region*4);
        unsigned header = db->planes ? 6 : 4, count = u16(record+(db->planes ? 4 : 2));
        for (unsigned i=0; i<count; ++i) if (u16(record+header+i*2) == id) return 1;
    }
    return 0;
}

int pe_collision_find_floor(const PeCollision *db, uint32_t x, uint32_t y, uint32_t z,
                            uint32_t flags, PeFloorHit *hit) {
    if (flags & 0x80) return 0;
    unsigned triangle_stride = db->planes ? 28 : 22;
    for (unsigned region=0; region<db->region_count; ++region) {
        const uint8_t *record = db->bytes + u32(db->bytes+40+region*4);
        unsigned header = db->planes ? 6 : 4, count = u16(record+(db->planes ? 4 : 2));
        for (unsigned i=0; i<count; ++i) {
            unsigned id = u16(record+header+i*2);
            const uint8_t *triangle = db->bytes+db->triangles+id*triangle_stride;
            PeCollisionXZ vertices[3];
            if (!pe_collision_triangle_xz(db, id, vertices)) return 0;
            if (!pe_collision_point_in_tri(vertices, s16((uint16_t)(x>>16)), s16((uint16_t)(z>>16)))) continue;
            uint32_t height = y;
            if (!(flags & 2)) {
                if (!db->planes) height = (uint32_t)u16(record)<<16;
                else {
                    const uint8_t *plane = db->bytes+db->planes+u16(triangle+2)*12;
                    height = mul(u32(triangle+4)-mul(u32(plane),x)-mul(u32(plane+8),z),u32(plane+4));
                }
            }
            *hit = (PeFloorHit){(uint16_t)id,(uint16_t)region,height}; return 1;
        }
    }
    return 0;
}
