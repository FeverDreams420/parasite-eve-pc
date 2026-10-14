#ifndef PE1_COLLISION_DATABASE_H
#define PE1_COLLISION_DATABASE_H

#include "common.h"
#include "pe1/game_state.h"

/* Packed vertex tables used by the flat and sloped triangle formats.
 * Coordinates are loaded as halfwords and interpreted as signed for geometry. */
typedef struct CollisionVertexXZ {
    u16 x, z;
} CollisionVertexXZ;
typedef struct CollisionVertexXYZ {
    u16 x, y, z;
} CollisionVertexXYZ;
/* The vertex table pointer, also read as the 32-bit address retail adds
 * the scaled vertex index to (index first). */
typedef union CollisionVertexTable {
    void *pointer;
    CollisionVertexXZ *xz;
    CollisionVertexXYZ *xyz;
    uintptr_t word;
} CollisionVertexTable;
/* The first word is byte-addressed in both triangle formats. Keeping this
 * two-byte view separate from the sloped face also describes the 22-byte
 * flat records, whose addresses need not be aligned to a 32-bit word. */
typedef struct CollisionTriangleHeader {
    u8 kind;
    u8 region; /* flat mode: index into the height records */
} CollisionTriangleHeader;

/* Sloped triangle prefix. Only kind and region are shared with flat triangles. */
typedef struct CollisionFace {
    u8 kind;
    u8 region;    /* flat mode: index into g_RegionHeightTable */
    u16 plane;    /* sloped mode: index into g_CollisionPlaneTable */
    s32 distance; /* sloped mode: plane D term */
} CollisionFace;

/* Walkable triangles. Both formats are walked as halfword arrays: the
 * flat one keeps its vertex indices at [1..3] and its edge neighbours at
 * [7..9], the sloped one at [4..6] and [10..12]. */
/* Three vertices, three edge ids and three neighbouring triangle ids.
 * Both formats keep the same links after their different headers. */
typedef struct CollisionTriangleLinks {
    u16 vertices[3];
    u16 edgeIds[3];
    u16 neighbours[3];
    u16 reserved;
} CollisionTriangleLinks;

typedef union CollisionTriangleXZ {
    CollisionTriangleHeader header;
    u16 words[11];
    struct { CollisionTriangleHeader header; CollisionTriangleLinks links; } layout;
    u8 kind; /* bit 0x80: not walkable from a neighbour */
} CollisionTriangleXZ;
typedef union CollisionTriangleXYZ {
    CollisionTriangleHeader header;
    CollisionFace face;
    u16 words[14];
    struct { CollisionFace face; CollisionTriangleLinks links; } layout;
    u8 kind;
} CollisionTriangleXYZ;
PE1_STATIC_ASSERT(PE1_OFFSETOF(CollisionFace, plane) == 2, collision_face_plane_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(CollisionFace, distance) == 4, collision_face_distance_offset);

PE1_STATIC_ASSERT(PE1_OFFSETOF(CollisionTriangleXZ, layout.links.vertices) == 2,
                  collision_xz_vertex_indices_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(CollisionTriangleXYZ, layout.links.vertices) == 8,
                  collision_xyz_vertex_indices_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(CollisionTriangleXZ, layout.links.neighbours) == 14,
                  collision_xz_neighbours_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(CollisionTriangleXYZ, layout.links.neighbours) == 20,
                  collision_xyz_neighbours_offset);

typedef union CollisionTriangleTable {
    void *pointer;
    CollisionTriangleHeader *header; /* Common prefix of either triangle format. */
    CollisionTriangleXZ *xz;
    CollisionTriangleXYZ *xyz;
    uintptr_t word;
} CollisionTriangleTable;
/* Per-region triangle index lists; the database selects one height mode. */
typedef struct CollisionFlatRegion {
    s16 height;
    u16 triangleCount;
    u16 triangleIndices[];
} CollisionFlatRegion;
typedef struct CollisionPlaneRegion {
    /* Disc regions are halfword-aligned, including this opaque prefix. */
    u16 reserved[2];
    u16 triangleCount;
    u16 triangleIndices[];
} CollisionPlaneRegion;

PE1_STATIC_ASSERT(PE1_OFFSETOF(CollisionFlatRegion, triangleIndices) == 4,
                  collision_flat_region_indices_offset);
_Static_assert(_Alignof(CollisionPlaneRegion) == 2, "disc region halfword alignment");
PE1_STATIC_ASSERT(PE1_OFFSETOF(CollisionPlaneRegion, triangleIndices) == 6,
                  collision_plane_region_indices_offset);

/* A relative byte offset on disk, or a region pointer after relocation. */
typedef union CollisionRegionEntry {
    uintptr_t word;
    s16 *height;
    CollisionFlatRegion *flat;
    CollisionPlaneRegion *plane;
} CollisionRegionEntry;

typedef struct CollisionDatabase {
    u16 reserved00;
    u16 regionCount;                  /* 0x02: inline height-table references. */
    u16 triangleCount;                /* 0x04: floor triangle array length. */
    u16 planeCount;                   /* 0x06: slope plane array length. */
    u16 visitedWordCount;             /* 0x08: traversal bitmap bits on disk; words after relocation. */
    u16 reserved0A;
    u32 reserved0C[3];
    CollisionVertexTable vertices;   /* 0x18: XZ or XYZ array. */
    CollisionTriangleTable triangles; /* 0x1C: XZ or XYZ triangles. */
    union { struct CollisionPlane *pointer; uintptr_t word; } planes; /* 0x20 */
    union { struct RampEdge *pointer; uintptr_t word; } rampEdges; /* 0x24 */
    CollisionRegionEntry *regions; /* 0x28: variable-length tail. */
} CollisionDatabase;

/* The database address as the word added to each relative region offset. */
typedef union CollisionDatabaseAddress {
    CollisionDatabase *db;
    uintptr_t word;
} CollisionDatabaseAddress;


extern CollisionDatabase *g_CollisionDb;
/* Loaded scene blocks; entry 0 is the room collision database. */
#define g_RoomCollisionData (g_GameState.bank_reset_948)
/* Active views published by the scene-data relocator. */
extern struct CollisionPlane *g_CollisionPlaneTable;

PE1_STATIC_ASSERT(sizeof(CollisionTriangleHeader) == 2, collision_triangle_header_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(CollisionTriangleHeader, region) == 1,
                  collision_triangle_region_offset);

/* Immutable 32-bit offset header stored on disc, distinct from runtime pointers. */
typedef struct CollisionDatabaseFile {
    u16 reserved00, regionCount, triangleCount, planeCount;
    u16 visitedBits, reserved0A;
    u32 reserved0C[3];
    u32 vertices, triangles, planes, rampEdges;
    u32 regions[];
} CollisionDatabaseFile;
PE1_STATIC_ASSERT(sizeof(CollisionDatabaseFile) == 0x28, collision_database_file_header_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(CollisionDatabase, vertices) ==
                  PE1_OFFSETOF(CollisionDatabaseFile, vertices), collision_database_count_prefix_match);
PE1_STATIC_ASSERT(PE1_OFFSETOF(CollisionDatabaseFile, regions) == 0x28,
                  collision_database_file_regions_offset);

/* Script word that receives the wall slot hit by the current entity. */
extern short *g_EntityCollisionWallSlot;

#endif
