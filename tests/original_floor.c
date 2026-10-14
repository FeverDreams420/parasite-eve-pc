#include "pe1/battle.h"
#include "pe1/collision_database.h"
#include "pe1/field_collision.h"
#include <stdio.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "line %d: %s\n", __LINE__, #x); return 1; } } while (0)
int main(void)
{
    CollisionVertexXZ vertices[] = {{0, 0}, {100, 0}, {0, 100}};
    CollisionTriangleXZ triangle = {0};
    triangle.layout.links.vertices[1] = 1;
    triangle.layout.links.vertices[2] = 2;
    struct { s16 height; u16 count, indices[1]; } empty = {99, 0, {0}}, flat = {-123, 1, {0}};
    CollisionRegionEntry regions[2] = {{.flat = (void *)&empty}, {.flat = (void *)&flat}};
    CollisionDatabase db = {0};
    db.regionCount = 2;
    db.triangleCount = 1;
    db.vertices.xz = vertices;
    db.triangles.xz = &triangle;
    db.regions = regions;
    g_CollisionDb = &db;
    g_RegionHeightTable.entries = regions;
    g_CollisionPlaneTable = NULL;
    BattleEntity actor = {0};
    actor.posX.fixed = 20 * 65536;
    actor.posZ.fixed = 30 * 65536;
    Entity_FindFloor(&actor);
    CHECK(actor.posY.fixed == -123 * 65536);
    CHECK(actor.collisionFace == &triangle && actor.collisionFaceMirror == &triangle);
    actor.entityFlags = 2;
    actor.posY.fixed = 17;
    actor.collisionFace = NULL;
    Entity_FindFloor(&actor);
    CHECK(actor.posY.fixed == 17 && actor.collisionFace == &triangle);
    actor.entityFlags = 0x80;
    actor.collisionFace = NULL;
    Entity_FindFloor(&actor);
    CHECK(actor.posY.fixed == 17 && actor.collisionFace == NULL);
    actor.entityFlags = 0;
    actor.posX.fixed = 200 * 65536;
    Entity_FindFloor(&actor);
    CHECK(actor.posY.fixed == 17 && actor.collisionFace == NULL);
    CollisionVertexXYZ sloped_vertices[] = {{0, 0, 0}, {100, 0, 0}, {0, 0, 100}};
    CollisionTriangleXYZ sloped = {0};
    sloped.layout.links.vertices[1] = 1;
    sloped.layout.links.vertices[2] = 2;
    sloped.face.distance = -10 * 65536;
    CollisionPlane plane = {32768, 65536, -16384};
    struct { u32 reserved; u16 count, indices[1]; } no_plane = {0, 0, {0}}, slope = {0, 1, {0}};
    regions[0].plane = (void *)&no_plane;
    regions[1].plane = (void *)&slope;
    db.vertices.xyz = sloped_vertices;
    db.triangles.xyz = &sloped;
    g_CollisionPlaneTable = &plane;
    actor.posX.fixed = 20 * 65536;
    actor.posZ.fixed = 12 * 65536;
    Entity_FindFloor(&actor);
    CHECK(actor.posY.fixed == -17 * 65536);
    CHECK(actor.collisionFace == &sloped.face && actor.collisionFaceMirror == &sloped.face);
    actor.posX.fixed = 21 * 65536 + 32768;
    actor.posZ.fixed = 14 * 65536 + 16384;
    Entity_FindFloor(&actor);
    CHECK(actor.posY.fixed == -17 * 65536 - 12288);
    puts("original floor: native second region, negative height, slope, miss and actor flags pass");
    return 0;
}
