#include "room.h"
#include "vram.h"
#include "script.h"
#include "spu_bank.h"
#include "scene.h"
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
    PeSpuBank *spu = calloc(1, sizeof(*spu));
    CHECK(spu);
    PeSpuBankStats bank_stats;
    CHECK(catalog.sector_bounds[6] == 131 && catalog.sector_bounds[7] == 153);
    CHECK(pe_spu_bank_load_startup(&catalog, spu, &bank_stats));
    CHECK(bank_stats.address == 4144 && bank_stats.bytes == 28176);
    CHECK(bank_stats.first == 0 && bank_stats.end == 256);
    CHECK(spu->valid[0] && spu->valid[255]);
    CHECK(spu->instruments[0].start == 4144 && spu->instruments[0].loop == 4176);
    CHECK(spu->instruments[0].pitch[0] == 2722);
    free(spu);
    CHECK(catalog.ranges[0].start == 8945);
    CHECK(catalog.ranges[0].scratch_sectors == 33);
    CHECK(catalog.ranges[0].texture_sectors == 74);
    CHECK(catalog.ranges[0].room_sectors == 82);
    PeRoom room = {0};
    PeVram *vram = calloc(1, sizeof(*vram));
    CHECK(vram);
    uint64_t uploaded_words = 0;
    unsigned script_entries = 0, initial_actors = 0;
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
        PeVramStats stats;
        char error[256];
        if (!pe_room_upload_vram(&room, vram, &stats, error, sizeof(error))) {
            fprintf(stderr, "%s\n", error);
            return 1;
        }
        CHECK(stats.uploads > 0 && stats.words > 0);
        uploaded_words += stats.words;
        PeScriptProgram program;
        if (!pe_script_program_open(&room, &program, error, sizeof(error))) {
            fprintf(stderr, "%s\n", error);
            return 1;
        }
        script_entries += program.entries;
        initial_actors += program.actors;
        for (unsigned actor = 0; actor < program.actors; ++actor) {
            unsigned entry, sub_id;
            uint32_t offset;
            PeScriptCommand command;
            CHECK(pe_script_actor(&program, actor, &entry, &sub_id));
            CHECK(pe_script_entry(&program, entry, &offset));
            if (!pe_script_decode(&program, offset, &command, error, sizeof(error))) {
                fprintf(stderr, "Map %u actor %u: %s\n", map, actor, error);
                return 1;
            }
        }
        ++loaded;
    }
    CHECK(loaded > 300 && loaded + absent == PE_USA_ROOM_SLOTS);
    PeScene scene = {0};
    char scene_error[256];
    CHECK(pe_scene_prepare(&catalog, 431, &scene, scene_error, sizeof(scene_error)));
    CHECK(scene.scripts.blob == &scene.room.data);
    CHECK(scene.sample_stats.bytes == 28176 && scene.spu->valid[255]);
    uint8_t *scene_data = scene.room.data.bytes;
    PeSpuBank *scene_spu = scene.spu;
    CHECK(!pe_scene_prepare(&catalog, PE_USA_ROOM_SLOTS, &scene, scene_error, sizeof(scene_error)));
    CHECK(scene.room.map == 431 && scene.room.data.bytes == scene_data && scene.spu == scene_spu);
    scene.spu->ram[0x2000] = 123;
    CHECK(pe_scene_prepare(&catalog, 353, &scene, scene_error, sizeof(scene_error)));
    CHECK(scene.scripts.blob == &scene.room.data && scene.spu->ram[0x2000] == 123);
    /* Failure in startup audio occurs after room allocation; roll it all back. */
    PeScene failed_scene = {0};
    catalog.sector_bounds[7] = catalog.sector_bounds[6];
    CHECK(!pe_scene_prepare(&catalog, 431, &failed_scene, scene_error, sizeof(scene_error)));
    CHECK(!failed_scene.room.data.bytes && !failed_scene.vram && !failed_scene.spu);
    catalog.sector_bounds[7] = 153;
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
    for (unsigned start = 0; start < 2; ++start) {
        CHECK(pe_room_load(&catalog, start ? 353 : 431, &room));
        PeScriptProgram program;
        char error[256];
        CHECK(pe_script_program_open(&room, &program, error, sizeof(error)));
        CHECK(program.actors == 1);
        unsigned entry, sub_id;
        uint32_t offset;
        CHECK(pe_script_actor(&program, 0, &entry, &sub_id));
        CHECK(entry == (start ? 2u : 1u) && sub_id == 0);
        CHECK(pe_script_entry(&program, entry, &offset));
        PeScriptCommand command;
        CHECK(pe_script_decode(&program, offset, &command, error, sizeof(error)));
        CHECK(command.opcode == 234 && command.count == 6);
        CHECK(command.args[0].value == (start ? 200u : 217u));
    }
    CHECK(pe_room_load(&catalog, 1, &room));
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
    /* Owned scene bytes remain valid after closing their input image. */
    unsigned scene_entry, scene_sub_id;
    uint32_t scene_offset;
    PeScriptCommand scene_command;
    CHECK(pe_script_actor(&scene.scripts, 0, &scene_entry, &scene_sub_id));
    CHECK(pe_script_entry(&scene.scripts, scene_entry, &scene_offset));
    CHECK(pe_script_decode(&scene.scripts, scene_offset, &scene_command, scene_error, sizeof(scene_error)));
    CHECK(scene_command.opcode == 234);
    pe_scene_close(&scene);
    pe_scene_close(&scene);
    free(vram);
    printf("USA Disc 1: %u rooms, %u empty slots, %llu texture words, %u script entries, %u initial actors; no scripts executed\n",
           loaded, absent, (unsigned long long)uploaded_words, script_entries, initial_actors);
    return 0;
}
