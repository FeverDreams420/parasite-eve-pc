#include "pe1/collision_database.h"
#include "pe1/battle.h"
#include "collision.h"
#include "pe1/field_collision.h"
#include "room.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
void Entity_RelocateSceneData(void);
void Entity_ResetStateGlobals(void);
static u32 word(const u8 *p) { return p[0]|(u32)p[1]<<8|(u32)p[2]<<16|(u32)p[3]<<24; }
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"line %d: %s\n",__LINE__,#x);return 1; } } while (0)
int main(void)
{
    const char *path=getenv("PE_TEST_DISC");
    PeDisc disc={0};PeRoomCatalog catalog;PeRoom room={0};unsigned maps=0;
    unsigned long queries=0, hits=0, sloped_hits=0;
    if (!path || !*path) return getenv("PE_REQUIRE_GAME_ASSETS")?1:77;
    CHECK(pe_disc_open(&disc,path));CHECK(pe_room_catalog_open(&catalog,&disc));
    for (unsigned map=1;map<=PE_USA_ROOM_SLOTS;++map) {
        const u8 *records;unsigned count;size_t size;u32 offset;u8 *copy;
        if (!catalog.ranges[map-1].room_sectors) continue;
        CHECK(pe_room_load(&catalog,map,&room));
        CHECK(pe_room_blob_table(&room.data,6,8,&records,&count) && count);
        size=word(records)&0xffffff;offset=word(records+4)&0xffffff;
        CHECK(size>=40 && offset<=room.data.size && size<=room.data.size-offset);
        const CollisionDatabaseFile *file=(const CollisionDatabaseFile *)(room.data.bytes+offset);
        CHECK(file->regionCount <= (size-40)/4);
        copy=malloc(size);CHECK(copy);memcpy(copy,file,size);
        g_RoomCollisionData[0]=(void *)file;
        Entity_ResetStateGlobals();Entity_RelocateSceneData();
        CHECK(g_CollisionDb && g_CollisionDb->regionCount==file->regionCount &&
              g_CollisionDb->triangleCount==file->triangleCount);
        CHECK(g_CollisionDb->vertices.pointer==(u8 *)file+file->vertices);
        CHECK(g_CollisionDb->triangles.pointer==(u8 *)file+file->triangles);
        CHECK(g_CollisionDb->visitedWordCount==(file->visitedBits>>5)+1);
        for (unsigned i=0;i<file->regionCount;++i)
            CHECK(g_CollisionDb->regions[i].height==(s16 *)((u8 *)file+file->regions[i]));
        Entity_RelocateSceneData();CHECK(!memcmp(copy,file,size));
        /* The byte-decoding PC implementation is a test oracle only. Runtime
         * floor queries continue through the original Entity_FindFloor. */
        PeCollision reference = {0};
        char error[256];
        CHECK(pe_collision_open(&reference, (const u8 *)file, size, error, sizeof(error)));
        for (unsigned triangle=0; triangle<file->triangleCount; ++triangle) {
            PeCollisionXZ vertices[3];
            CHECK(pe_collision_triangle_xz(&reference, triangle, vertices));
            for (unsigned point=0; point<4; ++point) {
                int x = point < 3 ? vertices[point].x :
                    (vertices[0].x + vertices[1].x + vertices[2].x) / 3;
                int z = point < 3 ? vertices[point].z :
                    (vertices[0].z + vertices[1].z + vertices[2].z) / 3;
                for (unsigned mode=0; mode<3; ++mode) {
                    BattleEntity actor = {0};
                    actor.posX.fixed = (s32)(((u32)x << 16) | 0x4000);
                    actor.posZ.fixed = (s32)(((u32)z << 16) | 0x8000);
                    actor.posY.fixed = -1234567;
                    actor.entityFlags = mode == 0 ? 0 : mode == 1 ? 2 : 0x80;
                    PeFloorHit expected;
                    int found = pe_collision_find_floor(&reference, actor.posX.fixed,
                        actor.posY.fixed, actor.posZ.fixed, actor.entityFlags, &expected);
                    Entity_FindFloor(&actor);
                    ++queries;
                    if (found) {
                        void *face = g_CollisionPlaneTable ?
                            (void *)&g_CollisionDb->triangles.xyz[expected.triangle].face :
                            (void *)&g_CollisionDb->triangles.xz[expected.triangle];
                        CHECK(actor.collisionFace == face && actor.collisionFaceMirror == face);
                        CHECK((u32)actor.posY.fixed == expected.height);
                        ++hits;
                        sloped_hits += g_CollisionPlaneTable != NULL;
                    } else {
                        CHECK(actor.collisionFace == NULL && actor.collisionFaceMirror == NULL);
                        CHECK(actor.posY.fixed == -1234567);
                    }
                }
            }
        }
        CHECK(!memcmp(copy,file,size));
        pe_collision_close(&reference);
        free(copy);pe_room_close(&room);++maps;
    }
    g_RoomCollisionData[0]=0;Entity_RelocateSceneData();CHECK(!g_CollisionDb);
    pe_disc_close(&disc);CHECK(maps==414);
    CHECK(queries && hits && sloped_hits);
    printf("Original floor queries: %lu queries, %lu hits, %lu sloped hits\n", queries, hits, sloped_hits);
    printf("Original collision relocation: %u rooms, immutable disc records and native region pointers\n",maps);
    return 0;
}
