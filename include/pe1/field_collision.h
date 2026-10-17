#ifndef PE1_FIELD_COLLISION_H
#define PE1_FIELD_COLLISION_H

#include "common.h"
#include "pe1/collision_database.h"
#include "pe1/floor_edge_point.h"
#include "pe1/floor_polygon.h"
#include "pe1/gte_types.h"

/* Up axis (0, -4096, 0) the placement basis (fieldfx/collision.c)
 * crosses with the facing direction. */
extern GteVector g_FieldPlacementUpAxis;

/* vertices must contain count > 0 entries. x and z are 16.16 coordinates. */
int Geo_PointInPoly(int x, int z, const PolygonVertex *vertices,
                    unsigned short count);

struct BattleEntity;
void Entity_FindFloor(struct BattleEntity *actor);
void Entity_ResolvePosition(struct BattleEntity *actor, int index);
void Entity_UpdateAndRender(struct BattleEntity *actor);

/* 16.16 edge length; the slide reads only the integer half. */
typedef union RampEdgeLength {
    s32 fixed;
    struct {
        u16 fraction;
        s16 integer;
    } parts;
} RampEdgeLength;

/* Floor edge record (g_SceneDataTable2, one per edge id): 16.16 length and unit
 * direction in the X/Z plane. */
typedef struct RampEdge {
    RampEdgeLength length;
    s32 directionX;
    s32 directionZ;
} RampEdge;

extern RampEdge *g_SceneDataTable2;
extern u16 g_FieldCollisionTriIndex;
void Entity_SlideOnRamp(struct BattleEntity *entity);

PE1_STATIC_ASSERT(sizeof(RampEdge) == 12, ramp_edge_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RampEdge, length.parts.integer) == 2, ramp_edge_length_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RampEdge, directionX) == 4, ramp_edge_x_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RampEdge, directionZ) == 8, ramp_edge_z_offset);

/* Shared collision query state. The response routine publishes the scaled
 * half-width before querying and sliding against this polygon. */
extern unsigned short g_CollisionRadius;

int Geo_FindNearestEdge(int x, int z, PolygonVertex *vertices,
                        unsigned short count);
void Entity_SlideOnWall(struct BattleEntity *actor, const PolygonVertex *vertices,
                       unsigned short count, short edge, int oldX, int oldZ);
/* The retail caller passes an argument; this routine uses D_8009D254. */
struct FieldActor;
void Entity_ApplyCollisionResponse(struct FieldActor *player);

/* Vertex `index` of the active table, addressed as retail does. */
#define COLLISION_VERTEX(address, index, type) \
    ((address).word = (index) * sizeof(type) + g_CollisionDb->vertices.word)

extern CollisionDatabase *g_CollisionDb;

/* Sloped-mode plane A*x + B*y + C*z = D, with B stored inverted. */
typedef struct CollisionPlane {
    s32 a;
    s32 inverseB;
    s32 c;
} CollisionPlane;

/* Plane-table pointer; existing floor queries require a fresh read. */
extern CollisionPlane *g_CollisionPlaneTable;
/* Native per-region pointers, indexed in both flat and sloped modes. */
typedef union CollisionRegionTable {
    CollisionRegionEntry *entries;
    uintptr_t address;
} CollisionRegionTable;
extern CollisionRegionTable g_RegionHeightTable;
/* Bounding box of the ramp edge the player last crossed. */
extern u16 g_FloorBlockedEdgeMaxX, g_FloorBlockedEdgeMinX, g_FloorBlockedEdgeMaxZ, g_FloorBlockedEdgeMinZ;
/* One bit per triangle already visited by the floor search. */
extern u32 g_FloorEdgeVisitedBitmap[];
int Geo_PointInTri(void *triangle, s16 x, s16 z);
void *Geo_ClipToFloorBoundarySub(void *triangle, void *previous, s16 x0, s16 z0,
                                 s16 x1, s16 z1);

PE1_STATIC_ASSERT(sizeof(CollisionVertexXZ) == 4, collision_vertex_xz_size);
PE1_STATIC_ASSERT(sizeof(CollisionVertexXYZ) == 6, collision_vertex_xyz_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(CollisionDatabase, vertices) == 0x18,
                  collision_database_vertices_offset);
PE1_STATIC_ASSERT(sizeof(CollisionTriangleXZ) == 22, collision_triangle_xz_size);
PE1_STATIC_ASSERT(sizeof(CollisionTriangleXYZ) == 28, collision_triangle_xyz_size);

/* Field floor / collision geometry (what keeps Aya on the walkable mesh and
 * sets her ground height Y). See field_movement.h for how motion drives pos;
 * after integration, Entity_ResolvePosition() snaps pos_y to the floor and the
 * current triangle is found by point-in-polygon test.
 *
 * Verified live via DuckStation GDB on the first field map (flat/region mode).
 *
 * The walkable area is a 2D mesh (in the X/Z plane) of triangles/regions:
 *   g_CollisionDb  collision DB base [live: 0x801aa85c]
 *     +0x18  vertex table  (vertices: u16 x@+0, u16 z; stride 6 plane-mode / 4 flat-mode)
 *     +0x1C  triangle/region entry table (stride 28 plane-mode / 22 flat-mode)
 *   g_CollisionPlaneTable  plane-equation table, OR 0 -> selects the height mode (see below)
 *   g_RegionHeightTable  region -> height table (flat mode only)
 *
 * Entry (one per walkable triangle/region):
 *   +0x01  u8   region/height id        (flat mode: index into g_RegionHeightTable)
 *   +0x02  u16  plane id                (plane mode: index into g_CollisionPlaneTable)
 *   +0x04  s32  plane D term            (plane mode)
 *   +0x02/+0x06/+0x08/+0x0C  vertex/neighbor indices walked by Geo_PointInTri
 *
 * TWO HEIGHT MODES (chosen by g_CollisionPlaneTable):
 *   1) FLAT / region mode  (g_CollisionPlaneTable == 0)  [this map]:
 *        Y = g_RegionHeightTable[entry[1]] << 16     (per-region constant height; here Y=0)
 *   2) SLOPED / plane mode (g_CollisionPlaneTable != 0):
 *        plane[id] = g_CollisionPlaneTable + id*12 : { s32 A@+0, s32 invB@+4, s32 C@+8 }
 *        Y = FixedMul( entryD - FixedMul(A, pos_x) - FixedMul(C, pos_z), invB )
 *        i.e. solve the triangle's plane equation A*x + B*y + C*z = D for y.
 *   The resolved Y is written to pos_y (FieldActor +0x2C), then base (+0x40..) is
 *   refreshed for rollback.
 *
 * The current triangle is cached on the actor:
 *   FieldActor +0x1A4 / +0x1A8 : pointer(s) to the current entry.
 *
 * Geo_PointInTri(entry, x, z): point-in-polygon edge-walk over the entry's
 * vertices (looked up in the +0x18 vertex table). Uses the INTEGER world coords
 * pos_x>>16 / pos_z>>16 (FieldActor reads them as s16 at +0x2A / +0x32). Drives
 * which region the actor is in and rejects positions outside the walkable mesh.
 *
 * COLLISION RESPONSE (wall blocking) -- Entity_RollbackPositionHierarchy:
 *   When the post-move position is off the walkable mesh, the actor is REVERTED
 *   to the position saved before the move:
 *     pos(+0x28/+0x2C/+0x30) <- base(+0x40/+0x44/+0x48)   (saved by Entity_IntegratePositionFull)
 *     current-tri(+0x1A4)    <- prev-tri(+0x1A8)
 *     flags(+0x98) |= 0x40000                              ("blocked this frame")
 *   So walls work by "move, test containment, revert if outside" (no slide in
 *   the flat map). Actor groups link via +0x18C (parent/child); the rollback
 *   recurses over the child and every entity sharing the same +0x18C parent,
 *   so a whole grouped formation reverts together.
 *   The per-frame actor contact pass Scene_UpdateEntityPositions walks every
 *   pair in the entity list g_FieldActorListHead (skipping actors with flags(+0x98)&0x20),
 *   tests body spheres and x/z cylinders, and rolls back actors that move into
 *   each other through Entity_RollbackPositionHierarchy.
 *
 * Functions:
 *   void Entity_ResolvePosition(struct BattleEntity *a, int triIndex);  // snap Y + cache tri
 *   int  Entity_ResolveCurrentPosition(int **idx);             // resolve D_8009D2F0
 *   int  Geo_PointInTri(void *entry, s16 x, s16 z);              // containment test
 *   void Entity_RollbackPositionHierarchy(FieldActor *a);      // revert pos<-base on block
 *   void Scene_UpdateEntityPositions(void);                    // per-frame actor contact pass
 *   int  Math_FixedMul(int a, int b);                          // (a*b)>>12 fixed-point
 */

#endif /* PE1_FIELD_COLLISION_H */
