#include "room.h"
#include "vram.h"
#include "script.h"
#include "spu_bank.h"
#include "scene.h"
#include "script_vm.h"
#include "akao_sequence.h"
#include "akao_channel.h"
#include "akao_player.h"
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
    PeSpuBank *record_spu = malloc(sizeof(*record_spu)); CHECK(record_spu);
    CHECK(catalog.ranges[0].start == 8945);
    CHECK(catalog.ranges[0].scratch_sectors == 33);
    CHECK(catalog.ranges[0].texture_sectors == 74);
    CHECK(catalog.ranges[0].room_sectors == 82);
    PeRoom room = {0};
    PeVram *vram = calloc(1, sizeof(*vram));
    CHECK(vram);
    uint64_t uploaded_words = 0;
    unsigned script_entries = 0, initial_actors = 0;
    unsigned music_records = 0;
    unsigned short_music_records = 0;
    uint8_t referenced_banks[PE_USA_AUDIO_BANKS] = {0};
    unsigned channel_notes = 0, channel_stops = 0, channel_missing = 0;
    unsigned player_prefix_ok = 0, player_prefix_missing = 0;
    unsigned unsupported[512] = {0};
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
        const uint8_t *tracks;
        unsigned tracks_count;
        CHECK(pe_room_blob_table(&room.data, 12, 12, &tracks, &tracks_count));
        for (unsigned i = 0; i < tracks_count; ++i) {
            PeRoomMusic music;
            if (!pe_room_music(&room, i, &music, error, sizeof(error))) {
                fprintf(stderr, "Map %u track %u: %s\n", map, i, error);
                return 1;
            }
            ++music_records;
            short_music_records += music.unresolved_short_record != 0;
            CHECK(music.bank < PE_USA_AUDIO_BANKS);
            referenced_banks[music.bank] = 1;
            if (!music.unresolved_short_record) {
                memcpy(record_spu, spu, sizeof(*spu));
                uint8_t *bank_bytes = NULL; size_t bank_size = 0;
                CHECK(pe_audio_bank_read(&catalog, music.bank, &bank_bytes, &bank_size));
                CHECK(pe_spu_bank_upload_rebased(record_spu, bank_bytes, bank_size, 0x8000, 0x20,
                                                NULL, error, sizeof(error)));
                free(bank_bytes);
                PeAkaoPlayer player = {0};
                PeAkaoControl initial = {0};
                CHECK(pe_akao_player_start(&player, &music.sequence, &catalog.akao, &initial));
                CHECK(pe_akao_player_bind_samples(&player, record_spu));
                int success = 1;
                for (unsigned tick = 0; tick < 4; ++tick) {
                    if (!pe_akao_player_tick(&player, 0, 1, 4096)) {
                        CHECK(strstr(player.error, "Unimplemented AKAO opcode"));
                        success = 0;
                        break;
                    }
                }
                if (success) ++player_prefix_ok;
                else ++player_prefix_missing;
                PeAkaoControl control = {0};
                for (unsigned voice = 0; voice < 24; ++voice) {
                    if (music.sequence.channels[voice] == UINT32_MAX) continue;
                    PeAkaoChannel channel = {0};
                    channel.control = &control;
                    channel.grammar = &catalog.akao;
                    channel.samples = record_spu;
                    CHECK(pe_akao_channel_start(&channel, &music.sequence, voice));
                    /* Elapsed initial delay, before command-set lengths. */
                    channel.note_length = 0;
                    PeAkaoChannelResult result = pe_akao_channel_next(&channel, 1024);
                    if (result == PE_AKAO_CHANNEL_NOTE) {
                        if (!pe_akao_channel_begin_note(&channel, 4096)) {
                            fprintf(stderr, "Map %u voice %u: %s\n", map, voice, channel.error);
                            return 1;
                        }
                        CHECK(pe_akao_channel_note_pitch(&channel));
                        ++channel_notes;
                    }
                    else if (result == PE_AKAO_CHANNEL_STOP) ++channel_stops;
                    else {
                        CHECK(strstr(channel.error, "Unimplemented AKAO opcode"));
                        ++channel_missing;
                        unsigned opcode = channel.sequence.bytes[channel.pc];
                        unsigned index = opcode == 0xfc ? 256u + channel.sequence.bytes[channel.pc + 1] : opcode;
                        ++unsupported[index];
                    }
                }
            }
        }
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
    CHECK(music_records == 600 && short_music_records == 1);
    CHECK(player_prefix_ok + player_prefix_missing == 599);
    printf("Four forced AKAO timer ticks: %u sequence prefixes completed, %u unimplemented (not full songs)\n",
           player_prefix_ok, player_prefix_missing);
    printf("Initial AKAO channel prefixes: %u raw notes, %u stops, %u unimplemented commands\n",
           channel_notes, channel_stops, channel_missing);
    for (unsigned i = 0; i < 512; ++i) {
        if (!unsupported[i]) continue;
        if (i < 256) printf("Unsupported initial opcode %02X: %u channels\n", i, unsupported[i]);
        else printf("Unsupported initial opcode FC/%02X: %u channels\n", i - 256, unsupported[i]);
    }
    CHECK(catalog.audio_base == 2224);
    CHECK(catalog.audio_base + catalog.audio_bounds[PE_USA_AUDIO_BANKS] == catalog.ranges[0].start);
    PeSpuBank *music_spu = calloc(1, sizeof(*music_spu)); CHECK(music_spu);
    unsigned verified_banks = 0;
    for (unsigned index = 0; index < PE_USA_AUDIO_BANKS; ++index) {
        if (!referenced_banks[index]) continue;
        uint8_t *bytes = NULL; size_t size = 0;
        CHECK(pe_audio_bank_read(&catalog, index, &bytes, &size));
        PeSpuBankStats stats;
        char error[256];
        if (!pe_spu_bank_upload_rebased(music_spu, bytes, size, 0x8000, 0x20,
                                        &stats, error, sizeof(error))) {
            fprintf(stderr, "Music bank %u: %s\n", index, error);
            return 1;
        }
        CHECK(stats.address == 0x8000 && stats.first == 0x20);
        free(bytes);
        ++verified_banks;
    }
    free(music_spu);
    printf("Referenced music sample banks uploaded: %u (not synthesized)\n", verified_banks);
    printf("Room music records: %u, including %u unresolved short records (not played)\n",
           music_records, short_music_records);
    PeScene scene = {0};
    char scene_error[256];
    CHECK(pe_scene_prepare(&catalog, 431, &scene, scene_error, sizeof(scene_error)));
    CHECK(scene.scripts.blob == &scene.room.data);
    CHECK(scene.akao.note_lengths[0] == 192 && scene.akao.note_lengths[10] == 4);
    CHECK(scene.sample_stats.bytes == 28176 && scene.spu->valid[255]);
    CHECK(scene.script_actor_count == 1 && scene.scheduler->live == 1);
    CHECK(scene.scheduler->slots[0].task.actor == &scene.script_actors[0]);
    CHECK(scene.scheduler->slots[0].task.program.blob == &scene.room.data);
    CHECK(scene.scheduler->slots[0].task.banks == &scene.script_banks);
    CHECK(!pe_script_scheduler_tick(scene.scheduler, &scene.script_actors[0], 100));
    CHECK(strstr(scene.scheduler->error, "Unimplemented script opcode 234"));
    PeScriptBanks banks = {0};
    PeScriptTask task;
    /* Actual M431 entry zero sets its event cursor/flag and waits one tick. */
    CHECK(pe_script_task_init(&task, &scene.scripts, 0, &banks));
    CHECK(pe_script_task_tick(&task, 100) == PE_SCRIPT_WAIT);
    CHECK(task.instructions == 3 && task.active == 1 && task.actor->flags == 0x40);
    CHECK(task.actor->cursors[1] == task.base + 36);
    task.target14 = 123;
    CHECK(pe_script_task_tick(&task, 100) == PE_SCRIPT_STOPPED);
    CHECK(task.instructions == 7 && task.actor->locals[3] == 123 && banks.operands[0] == 0);
    CHECK(task.flags == 0x10 && task.actor->flags == 0x40);
    CHECK(pe_script_task_init(&task, &scene.scripts, 0, &banks));
    CHECK(pe_script_task_tick(&task, 100) == PE_SCRIPT_WAIT);
    task.target14 = 251;
    CHECK(pe_script_task_tick(&task, 100) == PE_SCRIPT_STOPPED);
    CHECK(task.instructions == 7 && banks.operands[0] == 1);
    CHECK(!task.flags && task.actor->flags == 0x50);
    /* The active scene actor requires audio; report the missing opcode. */
    CHECK(pe_script_task_init(&task, &scene.scripts, 1, &banks));
    CHECK(pe_script_task_tick(&task, 100) == PE_SCRIPT_ERROR);
    CHECK(strstr(task.error, "Unimplemented script opcode 234"));
    uint8_t *scene_data = scene.room.data.bytes;
    PeSpuBank *scene_spu = scene.spu;
    CHECK(!pe_scene_prepare(&catalog, PE_USA_ROOM_SLOTS, &scene, scene_error, sizeof(scene_error)));
    CHECK(scene.room.map == 431 && scene.room.data.bytes == scene_data && scene.spu == scene_spu);
    scene.spu->ram[0x2000] = 123;
    scene.script_banks.variables[7] = 12345;
    CHECK(pe_scene_prepare(&catalog, 353, &scene, scene_error, sizeof(scene_error)));
    CHECK(scene.scripts.blob == &scene.room.data && scene.spu->ram[0x2000] == 123);
    CHECK(scene.script_banks.variables[7] == 12345);
    CHECK(scene.scheduler->slots[0].task.banks == &scene.script_banks);
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
    free(record_spu);
    free(spu);
    printf("USA Disc 1: %u rooms, %u empty slots, %llu texture words, %u script entries, %u initial actors; M431 entry-zero task branches verified, full scene execution pending\n",
           loaded, absent, (unsigned long long)uploaded_words, script_entries, initial_actors);
    return 0;
}
