/* Every room through the real loader, for the randomizer's room map. Scene_LoadRoom must accept each
 * room on the disc; the task block it relocates must walk cleanly from every entity entry: each step
 * reached has a native handler, no step overlaps data a step points at, and every door (op 49) names
 * a room on the disc. With a path argument the walk is written there, one line per step, for
 * mods/pe_randomizer/tools. PE_TEST_DISC2, when set, must hold the same rooms as disc 1.
 *
 * Walk rules (from the native handlers): a step is two words (opcode | count << 13 | modes << 17, then
 * the modes of operands 6+) and its operands. Script offsets are halfwords from the entry of the entity
 * running them. Code targets: op 0 goto, 5 jump if zero (not on a constant nonzero), 29 branch, 18 child
 * routine, 20 event routine (selector 1-3). Data targets: ops 172, 226, 227. A routine ends at op 0,
 * 1 or 32, at the FFFFFFFF word between routines, or at a door followed by that word. Door lines end
 * with the target: M0000, M5000 and M8888 are the scenes named at door_room. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL3/SDL.h>
#include "common.h"
#include "pe1/asset_table.h"
#include "pe1/boot_disc_check.h"
#include "pe1/boot_services.h"
#include "pe1/cdrom.h"
#include "pe1/game_state.h"
#include "pe1/reloc_block.h"
#include "pe1/scene_room.h"
#include "pe1/scene_room_load.h"
#include "pe1/script_queue.h"

/* Fixture I/O: the test-disc object renames the real image reads to these. */
int CdRom_ImageReadSectorsFromLba(u32 lba, void *buffer, u32 count);
int CdRom_ImagePollReady(void);
int CdRom_ReadSectorsFromLba(u32 lba, void *buffer, u32 count) { return CdRom_ImageReadSectorsFromLba(lba, buffer, count); }
int CdRom_PollReady(void) { return CdRom_ImagePollReady(); }
int Asset_Find08w(int id, int mode, int x, int y, int z) { (void)id; (void)mode; (void)x; (void)y; (void)z; return 0; }

enum { WORD_NONE, WORD_STEP, WORD_DATA, WORD_END };
static unsigned failures;
#define FAIL(...) do { if (++failures <= 40) { fprintf(stderr, __VA_ARGS__); fputc('\n', stderr); } } while (0)

static u32 word_at(const u8 *block, size_t offset) { u32 v; memcpy(&v, block + offset, 4); return v; }

typedef struct Step { u32 op, count, modes[15], values[15]; size_t bytes; } Step;
static void decode(const u8 *block, size_t at, Step *step) {
    u32 header = word_at(block, at), modes = header >> 17;
    step->op = header & 0x1FFF; step->count = (header >> 13) & 15; step->bytes = 8 + 4 * (size_t)step->count;
    for (u32 k = 0; k < step->count; ++k) {
        if (k == 5) modes = word_at(block, at + 4);
        step->modes[k] = modes & 7; step->values[k] = word_at(block, at + 8 + 4 * (size_t)k); modes >>= 3;
    }
}

/* Three door tokens are scenes of the main loop, not rooms (boot/main.c): M0000 runs the effect-scene
 * loop, M5000 the title screen, M8888 the boot display (then M431). */
enum { DOOR_NONE = -1, DOOR_EFFECT_SCENE = 0, DOOR_TITLE = 5000, DOOR_BOOT_DISPLAY = 8888 };

/* Room number a door token names, a scene above, or DOOR_NONE. */
static int door_room(u32 token) {
    char name[8] = {0};
    Text_EncodeBase32(name, token); /* upper case: M0005I */
    if (name[0] != 'M' || name[5] != 'I') return DOOR_NONE;
    for (int i = 1; i < 5; ++i) if (name[i] < '0' || name[i] > '9') return DOOR_NONE;
    int room = atoi(name + 1);
    if (room == DOOR_EFFECT_SCENE || room == DOOR_TITLE || room == DOOR_BOOT_DISPLAY) return room;
    return room >= 1 && room <= SCENE_ROOM_COUNT && g_RoomSectorTable[room - 1].roomSectors ? room : DOOR_NONE;
}

static void walk_room(unsigned room, FILE *out, unsigned *steps_total, unsigned *doors_total) {
    const u8 *assets = (const u8 *)g_GameState.loaded_scene_assets;
    size_t room_bytes = (size_t)g_RoomSectorTable[room - 1].roomSectors * 2048;
    const SceneRoomDirectory *directory = (const void *)(assets + word_at(assets, 4));
    u32 table = directory->taskBlock & 0x3FFFFF;
    size_t size = word_at(assets, table) & 0xFFFFFF, offset = word_at(assets, table + 4) & 0xFFFFFF;
    const u8 *block = assets + offset;
    u32 base = word_at(block, 0), count = word_at(block, 4);
    const RelocBlock *tasks = g_GameState.bank_reset_944[0];
    if (offset + size > room_bytes || base > size || 8 + 4 * (size_t)count > size) { FAIL("M%03u task block out of range", room); return; }
    if (!tasks || tasks->count != count || tasks->u0.base != block + base) { FAIL("M%03u relocated task block differs from the room's", room); return; }
    u32 *entries = malloc(count * sizeof(u32));
    u8 *kind = calloc(size / 4 + 1, 1);
    size_t *work = malloc((size / 4 + count + 1) * sizeof(size_t));
    unsigned doors = 0, steps = 0;
    for (u32 t = 0; t < count; ++t) {
        entries[t] = word_at(block, 8 + 4 * (size_t)t);
        if (tasks->offsets[t] != block + entries[t]) FAIL("M%03u entry %u differs from the relocated one", room, t);
    }
    if (out) {
        fprintf(out, "room\tM%03u\ttype=%u\tbank=%u\tentries=%u\tcode_end=%#x", room, g_GameState.room_type,
                g_GameState.requested_entity_bank, count, base);
        for (u32 t = 0; t < count; ++t) fprintf(out, "%s%#x", t ? "," : "\t", entries[t]);
        fputc('\n', out);
    }
    for (u32 t = 0; t < count; ++t) {
        size_t top = 0; work[top++] = entries[t];
        while (top) {
            size_t at = work[--top];
            for (;;) {
                if (at % 4 || at + 4 > base) { FAIL("M%03u entity %u: step at %#zx outside the code", room, t, at); break; }
                if (kind[at / 4] == WORD_STEP || kind[at / 4] == WORD_END) break;
                if (word_at(block, at) == 0xFFFFFFFFu) { kind[at / 4] = WORD_END; break; }
                if (at + 8 > base) { FAIL("M%03u entity %u: step at %#zx outside the code", room, t, at); break; }
                if (kind[at / 4] == WORD_DATA) { FAIL("M%03u entity %u: step at %#zx is data", room, t, at); break; }
                Step s; decode(block, at, &s);
                if (at + s.bytes > base) { FAIL("M%03u entity %u: step at %#zx runs past the code", room, t, at); break; }
                if (!g_ScriptQueueOpcodeHandlers[s.op]) { FAIL("M%03u entity %u: op %u at %#zx has no handler", room, t, s.op, at); break; }
                kind[at / 4] = WORD_STEP; ++steps;
                int target = -1, data = -1;
                if (s.op == 0 || s.op == 18) target = 0;
                else if (s.op == 5 && !(s.modes[0] == 0 && s.values[0])) target = 1;
                else if (s.op == 29) target = 2;
                else if (s.op == 20 && s.count > 1 && s.modes[0] == 0 && s.values[0] >= 1 && s.values[0] <= 3) target = 1;
                else if (s.op == 172 || s.op == 227) data = 0;
                else if (s.op == 226) data = 3;
                if (target >= 0 && target < (int)s.count && s.modes[target] == 0 && s.values[target] < 0x80000000u)
                    work[top++] = entries[t] + (size_t)s.values[target] * 2;
                if (data >= 0 && data < (int)s.count && s.modes[data] == 0) {
                    size_t d = entries[t] + (size_t)s.values[data] * 2;
                    if (d + 4 > size) FAIL("M%03u op %u at %#zx: data %#zx outside the block", room, s.op, at, d);
                    else if (kind[d / 4] == WORD_STEP) FAIL("M%03u op %u at %#zx: data %#zx is a step", room, s.op, at, d);
                    else if (d % 4 == 0) kind[d / 4] = WORD_DATA;
                }
                if (s.op == 49) {
                    if (s.count != 1) FAIL("M%03u door at %#zx has %u operands", room, at, s.count);
                    else if (s.modes[0] == 0 && door_room(s.values[0]) == DOOR_NONE) FAIL("M%03u door at %#zx: token %08x is no room", room, at, s.values[0]);
                    else if (s.modes[0] == 0) ++doors;
                }
                if (out) {
                    fprintf(out, "step\tM%03u\t%u\t%#zx\t%u", room, t, at, s.op);
                    for (u32 k = 0; k < s.count; ++k) fprintf(out, "%s%u:%#x", k ? "," : "\t", s.modes[k], s.values[k]);
                    if (s.op == 49 && s.count == 1 && s.modes[0] == 0) fprintf(out, "\tM%03d", door_room(s.values[0]));
                    fputc('\n', out);
                }
                if (s.op == 0 || s.op == 1 || s.op == 32) break;
                if (s.op == 49 && at + s.bytes + 4 <= size && word_at(block, at + s.bytes) == 0xFFFFFFFFu) break;
                at += s.bytes;
            }
        }
    }
    *steps_total += steps; *doors_total += doors;
    free(entries); free(kind); free(work);
}

/* Disc 2 must carry the same room table and room bytes as disc 1. */
static int same_rooms(PeDisc *one, const char *path) {
    PeDisc two = {0}; PeDiscFile exe1, exe2, img1, img2; int ok = 1;
    static u8 a[2048 * 64], b[2048 * 64]; u8 ta[SCENE_ROOM_COUNT * 8], tb[SCENE_ROOM_COUNT * 8];
    if (!pe_disc_open(&two, path) || !pe_disc_find(one, "SLUS_006.62", &exe1) || !pe_disc_find(&two, "SLUS_006.68", &exe2) ||
        !pe_disc_find(one, "PE.IMG", &img1) || !pe_disc_find(&two, "PE.IMG", &img2) ||
        !pe_disc_read(one, &exe1, 0x800u + 0x80093378u - 0x80010000u, ta, sizeof(ta)) ||
        !pe_disc_read(&two, &exe2, 0x800u + 0x80093378u - 0x80010000u, tb, sizeof(tb))) { pe_disc_close(&two); return 0; }
    if (memcmp(ta, tb, sizeof(ta))) { FAIL("disc 2 room table differs"); ok = 0; }
    for (unsigned m = 0; ok && m < SCENE_ROOM_COUNT; ++m) {
        SceneRoomSectorRange r; memcpy(&r, ta + m * 8, 8);
        u64 at = (u64)r.start * 2048, left = (u64)(r.scratchSectors + r.textureSectors + r.roomSectors) * 2048;
        while (left) {
            size_t n = left > sizeof(a) ? sizeof(a) : (size_t)left;
            if (!pe_disc_read(one, &img1, at, a, n) || !pe_disc_read(&two, &img2, at, b, n)) { ok = 0; break; }
            if (memcmp(a, b, n)) { FAIL("disc 2 M%03u differs", m + 1); ok = 0; break; }
            at += n; left -= n;
        }
    }
    pe_disc_close(&two);
    return ok;
}

int main(int argc, char **argv) {
    const char *path = getenv("PE_TEST_DISC"), *path2 = getenv("PE_TEST_DISC2");
    if (!path || !*path) return getenv("PE_REQUIRE_GAME_ASSETS") ? 1 : 77;
    PeDisc disc = {0};
    if (!SDL_Init(SDL_INIT_EVENTS) || !pe_disc_open(&disc, path)) return 1;
    ResetGraph(0); CdRom_BindImage(&disc, NULL);
    if (Asset_OpenPeImage() != 0) return 1;
    Boot_InitGameState(); D_8009CDC8 = 'm';
    FILE *out = argc > 1 ? fopen(argv[1], "w") : NULL;
    if (argc > 1 && !out) return 1;
    unsigned rooms = 0, steps = 0, doors = 0;
    for (unsigned room = 1; room <= SCENE_ROOM_COUNT; ++room) {
        if (!g_RoomSectorTable[room - 1].roomSectors) continue;
        char name[16]; snprintf(name, sizeof(name), "m%04ui", room);
        int status = Scene_LoadRoom((u32)Text_ParseBase32Id((signed char *)name));
        if (status) { FAIL("M%03u: Scene_LoadRoom %d", room, status); continue; }
        ++rooms; walk_room(room, out, &steps, &doors);
    }
    if (out) fclose(out);
    int discs = 1;
    if (path2 && *path2) { if (!same_rooms(&disc, path2)) FAIL("disc 2 rooms differ from disc 1"); else discs = 2; }
    CdRom_BindImage(NULL, NULL); pe_disc_close(&disc);
    if (failures) { fprintf(stderr, "%u failures\n", failures); return 1; }
    printf("Randomizer rooms: %u rooms loaded through Scene_LoadRoom, %u steps walked, %u doors to rooms or main-loop scenes, %u disc(s) checked\n",
           rooms, steps, doors, discs);
    return 0;
}
