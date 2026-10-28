/* Item randomizer: New Game shuffles what room chests, pickups and character gifts hand out, from a
 * six-digit seed and a difficulty picked under F1 > Randomizer. The game keeps the seed it started
 * with (a save record), so loading always gives the same layout; F1 changes only reach the next New
 * Game. Battle drops, Wayne, the Chrysler Building and the Theater keys stay vanilla.
 *
 * How items move: when a room has loaded (PE_HOOK_SCENE_START, before its first frame; the room data
 * is in g_GameState.loaded_scene_assets in the disc's layout), the item operands of its spots are
 * rewritten in the loaded script, all or none per room. The game's own code then runs unchanged:
 * the full-bag swap, the item a chest keeps for the player, the opened bit, the "You got" popup.
 * Changing the id in Inv_AddItem instead would also catch steals, drops, unequips and the New Game
 * kit, and would turn the item a chest keeps for the player into the seed's item. A room whose
 * script isn't byte for byte what the table expects stays vanilla. */
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "pe_game.h"
#include "pe_item_table.h"
#include "pe_mod_host.h"
#include "pe_mod_menu.h"
#include "pe_save.h"
#include "pe1/game_state.h"
#include "pe1/inventory.h"
#include "pe1/scene_room.h"
#include "pe1/script_state.h"
#include "pe_randomizer.h"
#include "pe_rand_spots.h"

#define MOD_ID "pe_randomizer"
#define PE_INV_CMD_START_NEW_GAME 1117

/* What this game was started with: saved as the "pe_randomizer.run" record. */
typedef struct PeRandRun {
    uint8_t on, difficulty, logic, table;
    uint32_t seed;
} PeRandRun;
_Static_assert(sizeof(PeRandRun) == 8, "the saved record's layout");

static PeRandRun s_run, s_layout_for;
static uint16_t s_layout[PE_RAND_SPOTS];
static PeRandResult s_result;
static int s_layout_ready;
static int s_enabled, s_difficulty = PE_RAND_MEDIUM, s_digit[6];
static unsigned char s_room_warned[SCENE_ROOM_COUNT + 1];
static const char *const s_difficulty_names[] = {"Easy", "Medium", "Hard"};
static const char *const s_digit_labels[] = {"Digit 1", "Digit 2", "Digit 3", "Digit 4", "Digit 5", "Digit 6"};

static uint32_t pe_rand_seed(void) {
    uint32_t seed = 0;
    for (int d = 0; d < 6; ++d) seed = seed * 10 + (uint32_t)(s_digit[d] >= 0 && s_digit[d] <= 9 ? s_digit[d] : 0);
    return seed;
}

static int pe_rand_supported(void) {
    return s_run.logic == PE_RAND_LOGIC_VERSION && s_run.table == PE_RAND_TABLE_VERSION &&
           s_run.difficulty < PE_RAND_DIFFICULTIES;
}

static int pe_is_key_item(unsigned id) { return (id >= 0x38 && id <= 0x3E) || (id >= 0xC8 && id <= 0xDC); }

/* Can sit in a pickup or gift: bag items and ammo, whose add fails only on a full bag, the same way
 * the spot's own item does. Not equipment (it also needs a free equipment record) or keys. */
static int pe_gift_safe(unsigned id) {
    ItemDataRecord *item;
    if (!id || id > 0xFF || pe_is_key_item(id) || !(item = Item_LookupBaseData(id - 1))) return 0;
    return item->kind == 10 || (item->kind >= 12 && item->kind <= 18);
}

static const char *pe_item_name(unsigned id) {
    for (unsigned i = 0; i < PE_ITEM_COUNT; ++i)
        if (pe_item_ids[i] == id) return pe_item_names[i];
    return "?";
}

static int pe_rand_ready(void) {
    if (!s_run.on || !pe_rand_supported()) return 0;
    if (!s_layout_ready || memcmp(&s_layout_for, &s_run, sizeof s_run)) {
        pe_rand_generate(g_pe_rand_spots, PE_RAND_SPOTS, s_run.seed, s_run.difficulty, pe_gift_safe, s_layout, &s_result);
        s_layout_for = s_run;
        s_layout_ready = 1;
        memset(s_room_warned, 0, sizeof s_room_warned);
        pe_log("rand_layout", "seed=%06u difficulty=%u tries=%u keys_moved=%u fallback=%d keys_locked=%d",
               (unsigned)s_run.seed, (unsigned)s_run.difficulty, s_result.attempts, s_result.keys_moved,
               s_result.fallback, s_result.keys_locked);
    }
    return 1;
}

static uint32_t pe_get32(const uint8_t *p) { return p[0] | p[1] << 8 | p[2] << 16 | (uint32_t)p[3] << 24; }
static void pe_put32(uint8_t *p, uint32_t v) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); p[2] = (uint8_t)(v >> 16); p[3] = (uint8_t)(v >> 24); }

static int pe_rand_scene(void *ctx, void *user) {
    uint8_t *room = (uint8_t *)g_GameState.loaded_scene_assets;
    int map = pe_room_number();
    unsigned first = 0, end, patched = 0;
    size_t bytes;
    (void)ctx;
    (void)user;
    if (!room || map < 1 || map > SCENE_ROOM_COUNT || !pe_rand_ready()) return 0;
    while (first < PE_RAND_SITES && g_pe_rand_sites[first].map < map) ++first;
    for (end = first; end < PE_RAND_SITES && g_pe_rand_sites[end].map == map; ++end) {}
    if (first == end) return 0;
    bytes = (size_t)g_RoomSectorTable[map - 1].roomSectors * 2048;
    /* all or none: a half-patched room could give one version of a spot's item and not the other */
    for (unsigned i = first; i < end; ++i) {
        const PeRandSite *s = &g_pe_rand_sites[i];
        int same = s->at <= bytes && bytes - s->at >= 16;
        for (unsigned w = 0; same && w < 4; ++w) {
            uint32_t have = pe_get32(room + s->at + 4 * w);
            same = have == s->expect[w] || (w == 2u + s->operand && have == s_layout[s->spot]);
        }
        if (!same) {
            if (!s_room_warned[map]) {
                s_room_warned[map] = 1;
                pe_mod_log(MOD_ID, "M%03d: script differs from the table at +%06X; its items stay vanilla",
                           map, (unsigned)s->at);
            }
            return 0;
        }
    }
    for (unsigned i = first; i < end; ++i) {
        const PeRandSite *s = &g_pe_rand_sites[i];
        pe_put32(room + s->at + 8 + 4 * s->operand, s_layout[s->spot]);
        patched += s_layout[s->spot] != g_pe_rand_spots[s->spot].vanilla;
    }
    pe_log("rand_room", "map=%d sites=%u changed=%u", map, end - first, patched);
    return 0;
}

/* A table spot giving its item: a rand_give row (play log), on or off, for tools/keylogic.py,
 * which works out from a run which keys each spot needs. During a script step the cursor is just past
 * it, so a GiveItem (two operands, 16 bytes) starts 16 bytes back. Only rows for the table's own give
 * steps are written; steals, drops and menus aren't script gives. */
static int pe_rand_item_add(void *ctx, void *user) {
    const uint8_t *room = (const uint8_t *)g_GameState.loaded_scene_assets;
    const int *item = ctx;
    int map = pe_room_number();
    uintptr_t base = (uintptr_t)room, cursor = (uintptr_t)g_ScriptCursor;
    size_t bytes, at;
    (void)user;
    if (!room || !item || map < 1 || map > SCENE_ROOM_COUNT || pe_in_battle() || cursor < base + 16) return 0;
    bytes = (size_t)g_RoomSectorTable[map - 1].roomSectors * 2048;
    if (cursor - base > bytes) return 0;
    at = cursor - base - 16;
    if ((pe_get32(room + at) & 0x1FFFu) != 167u) return 0;
    for (unsigned i = 0; i < PE_RAND_GIVES && g_pe_rand_gives[i].map <= map; ++i)
        if (g_pe_rand_gives[i].map == map && g_pe_rand_gives[i].at == at) {
            pe_log("rand_give", "map=%d at=%06X spot=%u item=0x%02X", map, (unsigned)at,
                   (unsigned)g_pe_rand_gives[i].spot, (unsigned)*item);
            break;
        }
    return 0;
}

/* The game asking whether Aya holds a movable key (a lock, or the script near one): a rand_lock row,
 * once per key and room per launch and run, for keylogic.py's before= (where a key may go). */
static unsigned char s_lock_logged[PE_RAND_MAX_KEYS][SCENE_ROOM_COUNT + 1];

static void pe_rand_lock_seen(unsigned item) {
    int map = pe_room_number();
    unsigned k = 0;
    if (map < 1 || map > SCENE_ROOM_COUNT) return;
    for (unsigned i = 0; i < PE_RAND_SPOTS && k < PE_RAND_MAX_KEYS; ++i) {
        if (!(g_pe_rand_spots[i].flags & PE_RAND_KEY)) continue;
        if (g_pe_rand_spots[i].vanilla == item) {
            if (!s_lock_logged[k][map]) pe_log("rand_lock", "item=0x%02X map=%d", item, map);
            s_lock_logged[k][map] = 1;
            return;
        }
        ++k;
    }
}

static int pe_rand_item_count(void *ctx, void *user) {
    int **args = ctx;
    (void)user;
    if (args && args[0]) pe_rand_lock_seen((unsigned)*args[0]);
    return 0;
}

static int pe_rand_inv_command(void *ctx, void *user) {
    const PeHookInvCommand *c = ctx;
    (void)user;
    if (c && (c->command == 1102 || c->command == 1112)) pe_rand_lock_seen((unsigned)c->value);
    return 0;
}

/* randomizer-<seed>.txt beside the preferences: every spot that changed. Easy and Medium only. */
static void pe_rand_spoiler(void) {
    char name[64], path[1100], temp[1110];
    FILE *f;
    snprintf(name, sizeof name, "randomizer-%06u.txt", (unsigned)s_run.seed);
    if (pe_mod_path(path, sizeof path, name) != 0) return;
    snprintf(temp, sizeof temp, "%s.tmp", path);
    if (!(f = fopen(temp, "w"))) { pe_toast("Randomizer: couldn't write the spoiler file"); return; }
    fprintf(f, "Parasite Eve randomizer, seed %06u, %s (logic %u, table %u)\n", (unsigned)s_run.seed,
            s_difficulty_names[s_run.difficulty], (unsigned)s_run.logic, (unsigned)s_run.table);
    if (s_result.keys_locked) fprintf(f, "Key items stay in their places (logic data not filled in yet).\n");
    else if (s_result.fallback) fprintf(f, "No layout passed its checks: %s.\n", s_result.fallback == 1 ? "key items stay in their places" : "nothing moved");
    fprintf(f, "\n");
    for (unsigned n = 0; n < PE_RAND_SPOTS; ++n) {
        unsigned i = 0;
        if (s_layout[n] == g_pe_rand_spots[n].vanilla) continue;
        while (i < PE_RAND_SITES && g_pe_rand_sites[i].spot != n) ++i;
        fprintf(f, "M%03u day %u: %s (was %s)\n", i < PE_RAND_SITES ? (unsigned)g_pe_rand_sites[i].map : 0,
                (unsigned)g_pe_rand_spots[n].day, pe_item_name(s_layout[n]), pe_item_name(g_pe_rand_spots[n].vanilla));
    }
    if (fclose(f) != 0) { remove(temp); pe_toast("Randomizer: couldn't write the spoiler file"); return; }
    remove(path);
    if (rename(temp, path) != 0) { remove(temp); pe_toast("Randomizer: couldn't write the spoiler file"); }
}

static int pe_rand_command(void *ctx, void *user) {
    const PeHookInvCommand *c = ctx;
    char line[96];
    (void)user;
    if (!c || c->command != PE_INV_CMD_START_NEW_GAME) return 0;
    memset(s_lock_logged, 0, sizeof s_lock_logged);   /* a new run: its locks are logged again */
    /* the host has just reset the record (off); this is the only place a run's settings are taken */
    s_run.on = s_enabled != 0;
    s_run.difficulty = (uint8_t)(s_difficulty >= 0 && s_difficulty < PE_RAND_DIFFICULTIES ? s_difficulty : PE_RAND_MEDIUM);
    s_run.logic = PE_RAND_LOGIC_VERSION;
    s_run.table = PE_RAND_TABLE_VERSION;
    s_run.seed = pe_rand_seed();
    if (!s_run.on || !pe_rand_ready()) {
        pe_log("rand_newgame", "on=0 keys_moved=0");
        return 0;
    }
    pe_log("rand_newgame", "on=1 seed=%06u difficulty=%u keys_moved=%u", (unsigned)s_run.seed,
           (unsigned)s_run.difficulty, s_result.keys_moved);
    if (s_run.difficulty != PE_RAND_HARD) pe_rand_spoiler();
    snprintf(line, sizeof line, "Randomizer: seed %06u, %s", (unsigned)s_run.seed, s_difficulty_names[s_run.difficulty]);
    pe_toast(line);
    return 0;
}

static int pe_rand_save_read(void *ctx, void *user) {
    (void)ctx;
    (void)user;
    s_layout_ready = 0;
    if (s_run.on && !pe_rand_supported()) {
        pe_toast("Randomizer: this save's layout is from another version; items stay vanilla");
        pe_mod_log(MOD_ID, "save from logic %u / table %u, this build %u / %u: items stay vanilla",
                   (unsigned)s_run.logic, (unsigned)s_run.table, PE_RAND_LOGIC_VERSION, PE_RAND_TABLE_VERSION);
    }
    return 0;
}

/* A layout from an unknown version is kept as it is (saved back unchanged), only not used. */
static int pe_rand_check_record(void *data) {
    const PeRandRun *run = data;
    return run->on > 1;
}
static const PeSaveRecord s_record = {"pe_randomizer.run", 1, &s_run, sizeof s_run, pe_rand_check_record, NULL, NULL, NULL};

static void pe_rand_random_seed(void) {
    uint32_t x = (uint32_t)time(NULL) ^ (uint32_t)clock() * 2654435761u;
    x ^= x >> 15; x *= 2246822519u; x ^= x >> 13;
    x %= 1000000u;
    for (int d = 5; d >= 0; --d) { s_digit[d] = (int)(x % 10); x /= 10; }
}

static void pe_rand_seed_text(char *out, unsigned size) { snprintf(out, size, "%06u", (unsigned)pe_rand_seed()); }

static void pe_rand_run_text(char *out, unsigned size) {
    if (!pe_save_loaded() || !s_run.on) { snprintf(out, size, "Not randomized"); return; }
    if (!pe_rand_supported()) { snprintf(out, size, "Other version: vanilla"); return; }
    pe_rand_ready();
    snprintf(out, size, "%06u %s, %s", (unsigned)s_run.seed, s_difficulty_names[s_run.difficulty],
             s_result.keys_locked || s_result.fallback ? "keys vanilla" : "keys moved");
}

static int pe_rand_init(void) {
    pe_menu_begin("Randomizer");
    pe_menu_top_page("Randomizer", "Shuffle items from a seed. Set before New Game.", 50);
    pe_menu_register_toggle(0, "Randomize new games", "New Game shuffles items with this seed. A game keeps the seed it began with.",
                            &s_enabled, NULL);
    pe_menu_register_choice(0, "Difficulty", "Easy: same story day. Medium: anywhere, keys in their area. Hard: keys anywhere logic allows.",
                            &s_difficulty, s_difficulty_names, PE_RAND_DIFFICULTIES);
    pe_menu_register_info(0, "Seed", "Six digits: the same seed and difficulty give the same items.", pe_rand_seed_text);
    pe_menu_register_action(0, "Random seed", "Pick a new seed.", pe_rand_random_seed);
    pe_menu_page("Seed digits", "Type the seed one digit at a time.", 0);
    for (int d = 0; d < 6; ++d)
        pe_menu_register_int("Seed digits", s_digit_labels[d], "Left and right change this digit.", &s_digit[d], 0, 9, 1);
    pe_menu_register_info(0, "This game", "The seed this game was started with. F1 changes only reach the next New Game.",
                          pe_rand_run_text);
    pe_menu_begin(0);
    pe_hook_add(PE_HOOK_SCENE_START, pe_rand_scene, NULL, PE_PRIORITY_NORMAL, MOD_ID);
    pe_hook_add(PE_HOOK_INV_COMMAND, pe_rand_command, NULL, PE_PRIORITY_NORMAL, MOD_ID);
    pe_hook_add(PE_HOOK_SAVE_READ, pe_rand_save_read, NULL, PE_PRIORITY_NORMAL, MOD_ID);
    /* these read only: rows for keylogic.py */
    pe_hook_add(PE_HOOK_ITEM_ADD, pe_rand_item_add, NULL, PE_PRIORITY_EARLY, MOD_ID);
    pe_hook_add(PE_HOOK_ITEM_COUNT, pe_rand_item_count, NULL, PE_PRIORITY_EARLY, MOD_ID);
    pe_hook_add(PE_HOOK_INV_COMMAND, pe_rand_inv_command, NULL, PE_PRIORITY_EARLY, MOD_ID);
    return pe_save_register(&s_record) < 0;
}

const PeMod pe_mod_pe_randomizer = {MOD_ID, "Randomizer", "0.1", NULL, pe_rand_init, NULL};
