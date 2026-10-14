#include "room.h"
#include <stdlib.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "line %d: %s\n", __LINE__, #expr); exit(1); } } while (0)

int main(void) {
    const char *path = getenv("PE_TEST_DISC");
    if (!path || !*path) {
        puts("Set PE_TEST_DISC to the USA Disc 1 image for room asset verification.");
        return getenv("PE_REQUIRE_GAME_ASSETS") ? 1 : 77;
    }
    PeDisc disc;
    CHECK(pe_disc_open(&disc, path));
    PeRoomCatalog catalog;
    CHECK(pe_room_catalog_open(&catalog, &disc));
    CHECK(catalog.ranges[0].start == 8945);
    CHECK(catalog.ranges[0].scratch_sectors == 33);
    CHECK(catalog.ranges[0].texture_sectors == 74);
    CHECK(catalog.ranges[0].room_sectors == 82);
    PeRoom room = {0};
    unsigned loaded = 0, absent = 0;
    for (unsigned map = 1; map <= PE_USA_ROOM_SLOTS; ++map) {
        const PeRoomRange *range = &catalog.ranges[map - 1];
        if (!range->room_sectors) {
            unsigned previous = room.map;
            uint8_t *previous_data = room.data.bytes;
            CHECK(!pe_room_load(&catalog, map, &room));
            CHECK(room.map == previous && room.data.bytes == previous_data);
            ++absent;
            continue;
        }
        if (!pe_room_load(&catalog, map, &room)) {
            fprintf(stderr, "Map %u: %s\n", map, catalog.error);
            return 1;
        }
        CHECK(room.map == map && room.data.size);
        ++loaded;
    }
    CHECK(loaded > 300 && loaded + absent == PE_USA_ROOM_SLOTS);
    CHECK(pe_room_load(&catalog, 1, &room));
    CHECK(room.data.size == 166084);
    const uint8_t *records;
    unsigned count;
    CHECK(pe_room_blob_table(&room.scratch, 10, 20, &records, &count));
    CHECK(count == 4);
    CHECK(pe_room_blob_table(&room.textures, 10, 20, &records, &count));
    CHECK(count == 12);
    CHECK(pe_room_blob_table(&room.data, 3, 12, &records, &count));
    CHECK(count == 3);
    uint8_t *previous_data = room.data.bytes;
    catalog.ranges[0].start = catalog.archive.size / 2048;
    CHECK(!pe_room_load(&catalog, 1, &room));
    CHECK(room.map == 1 && room.data.bytes == previous_data);
    CHECK(!pe_room_load(&catalog, 0, &room));
    CHECK(!pe_room_load(&catalog, PE_USA_ROOM_SLOTS + 1, &room));
    /* A mutated packed descriptor must never escape the loaded buffer. */
    room.data.directory[3] = UINT32_C(0xffc00000) | (uint32_t)(room.data.size - 1);
    CHECK(!pe_room_blob_table(&room.data, 3, 12, &records, &count));
    CHECK(!pe_room_blob_table(&room.data, 13, 12, &records, &count));
    CHECK(!pe_room_blob_table(&room.data, 3, 0, &records, &count));
    pe_room_close(&room);
    pe_room_close(&room);
    pe_disc_close(&disc);
    printf("USA Disc 1 room directories verified: %u loaded, %u empty slots; no scripts executed\n",
           loaded, absent);
    return 0;
}
