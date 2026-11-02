/* Battle randomizer, AI: an enemy actor of one room runs another actor's script.
 *
 * An actor of type E spawns with script_base = bank_reset_944[0]->offsets[E] (frame_update.c), the room's
 * task-block RelocBlock built at each room load. Each actor type has its own block there; every jump, label
 * and data ref is script_base + v*2, so a block runs from anywhere: no relocation. So a swap, done when the
 * room's banks are filled (PE_HOOK_PLAYER_ROWS, every room load, before the actors spawn): read the source
 * room from the disc, copy the source actor's block (and the rest of its task block after it) and point
 * offsets[E] at the copy. The spawner stays this room's (its spawns, formation, win counter, story flags).
 *
 * In the copy, the entity operands (tools/enemyai.py ENTITY_ARG: the type operand of ops 8, 14, 28, 54, 69,
 * 75, 84, 94, 139, 152, 164, 197, 206, 212; ops 17/237 sub commands 2500/3000; op 234 command 0x160) that name
 * the source actor's own number (its siblings) become E, and those that name its spawner become this room's
 * spawner of E (op 28 is an entity event, how an enemy signals its spawner). The block is read step by step
 * (header, tail word, operands): a block whose last step runs past its end is left alone.
 *
 * The script slots (PM commands, effect programs) the source room installs and this room doesn't are
 * installed too, from the source room's records as its own load does it (each slot number has one handler in
 * every room); at the next room load, each one that room doesn't install is set back as it was.
 */
#include <stdlib.h>
#include <string.h>
#include "pe_mod_host.h"
#include "pe_game.h"
#include "pe_hooks.h"
#include "pe_disc_assets.h"
#include "pe1/game_state.h"
#include "pe1/scene_room.h"
#include "pe_randomizer.h"

#define MOD_ID "pe_randomizer"
enum { PE_RAND_AIS = 8, PE_RAND_TYPES = 10, PE_RAND_AI_SLOTS = 32 };

typedef struct PeRandAiMap {
    uint16_t room, from_room;
    uint8_t entity, from_entity;
} PeRandAiMap;
static PeRandAiMap s_map[PE_RAND_AIS];
static unsigned s_map_count;
/* The copies given at the last room load (per actor type), and the slots installed with them. */
static uint8_t *s_copy[PE_RAND_TYPES];
static unsigned s_copy_count;
static struct { unsigned slot; const void *was, *now; } s_slot[PE_RAND_AI_SLOTS];
static unsigned s_slot_count;

static uint32_t pe_word(const uint8_t *p, uint32_t at) { uint32_t v; memcpy(&v, p + at, 4); return v; }

/* A room blob's task block: its bytes and length. */
static const uint8_t *pe_tasks(const uint8_t *blob, uint32_t *bytes) {
    const SceneRoomDirectory *dir = (const void *)(blob + pe_word(blob, 4));
    const uint32_t record = dir->taskBlock & 0x3FFFFF;
    *bytes = pe_word(blob, record) & 0xFFFFFF;
    return blob + (pe_word(blob, record + 4) & 0xFFFFFF);
}

/* Actor type t's block in a task block: [start, end) by the entries (end: the next entry, or the block's end). */
static int pe_entry(const uint8_t *tasks, uint32_t bytes, unsigned t, uint32_t *start, uint32_t *end) {
    const uint32_t count = pe_word(tasks, 4);
    if (t >= count) return 0;
    *start = pe_word(tasks, 8 + 4 * t);
    *end = bytes;
    for (uint32_t i = 0; i < count; ++i) {
        const uint32_t at = pe_word(tasks, 8 + 4 * i);
        if (at > *start && at < *end) *end = at;
    }
    return *start < *end;
}

/* The operand of a step that names an entity type, or -1. */
static int pe_entity_arg(const uint32_t *step) {
    const uint32_t op = step[0] & 0x1FFF, count = (step[0] >> 13) & 15;
    int idx;
    switch (op) {
    case 8: case 28: case 54: case 69: case 75: case 84: case 139: case 152: case 164: case 197: case 206: case 212:
        idx = 0; break;
    case 14: case 94: idx = 1; break;
    case 17: case 237: idx = count > 1 && !(step[0] >> 17 & 7) && (step[2] == 2500 || step[2] == 3000) ? 1 : -1; break;
    case 234: idx = count > 2 && !(step[0] >> 17 & 7) && step[2] == 0x160 ? 2 : -1; break;
    default: idx = -1;
    }
    /* a constant only (mode 0); the type operands sit in the header's modes (operands 0-4) */
    return idx >= 0 && (uint32_t)idx < count && !(step[0] >> (17 + 3 * idx) & 7) ? idx : -1;
}

/* Steps in [start, end): calls fn on each; 0 if the last one runs past the end (a pad word may follow it). */
static int pe_steps(const uint8_t *tasks, uint32_t start, uint32_t end, void (*fn)(uint32_t *, uint32_t, void *), void *user) {
    uint32_t at = start;
    while (at + 8 <= end) {
        uint32_t *step = (uint32_t *)(tasks + at);
        if (fn) fn(step, at, user);
        at += 8 + 4 * ((step[0] >> 13) & 15);
    }
    return at <= end;
}

typedef struct PeSpawner { unsigned type, found; } PeSpawner;
static void pe_spawn_step(uint32_t *step, uint32_t at, void *user) {
    PeSpawner *s = user;
    const uint32_t op = step[0] & 0x1FFF;
    (void)at;
    if ((op == 8 && pe_entity_arg(step) == 0 && step[2] == s->type) ||
        (op == 17 && pe_entity_arg(step) == 1 && step[2] == 2500 && step[3] == s->type)) s->found = 1;
}

/* The one actor that spawns type t (op 8, op 17 2500) in a task block; -1 if none or more than one. */
static int pe_spawner(const uint8_t *tasks, uint32_t bytes, unsigned t) {
    const uint32_t count = pe_word(tasks, 4);
    int who = -1;
    for (unsigned e = 0; e < count; ++e) {
        uint32_t start, end;
        PeSpawner s = {t, 0};
        if (!pe_entry(tasks, bytes, e, &start, &end)) continue;
        pe_steps(tasks, start, end, pe_spawn_step, &s);
        if (s.found) { if (who >= 0) return -1; who = (int)e; }
    }
    return who;
}

typedef struct PePatch { unsigned self, to_self; int spawner, to_spawner; unsigned done; } PePatch;
static void pe_patch_step(uint32_t *step, uint32_t at, void *user) {
    PePatch *p = user;
    const int idx = pe_entity_arg(step);
    (void)at;
    if (idx < 0) return;
    uint32_t *v = &step[2 + idx];
    if (*v == p->self && p->self != p->to_self) { *v = p->to_self; ++p->done; }
    else if (p->spawner >= 0 && *v == (uint32_t)p->spawner && p->spawner != p->to_spawner) { *v = (uint32_t)p->to_spawner; ++p->done; }
}

/* The slots a room blob's directory installs. */
static int pe_room_has_slot(const uint8_t *blob, unsigned slot) {
    const SceneRoomDirectory *dir = (const void *)(blob + pe_word(blob, 4));
    const uint8_t *r = blob + (dir->scripts & 0x3FFFFF);
    for (unsigned i = 0; i < dir->scripts >> 22; ++i, r += 12) if (r[7] == slot) return 1;
    return 0;
}

int pe_rand_ai_force(unsigned room, unsigned entity, unsigned from_room, unsigned from_entity) {
    if (s_map_count == PE_RAND_AIS || entity >= PE_RAND_TYPES || from_entity >= PE_RAND_TYPES) return 0;
    s_map[s_map_count++] = (PeRandAiMap){(uint16_t)room, (uint16_t)from_room, (uint8_t)entity, (uint8_t)from_entity};
    return 1;
}

void pe_rand_ai_clear(void) { s_map_count = 0; }

/* A new load: the copies go (the last room's actors are gone), and each slot installed for them that this
 * room doesn't install is set back. */
static void pe_rand_ai_release(const uint8_t *assets) {
    for (unsigned i = 0; i < PE_RAND_TYPES; ++i) { free(s_copy[i]); s_copy[i] = NULL; }
    s_copy_count = 0;
    for (unsigned i = s_slot_count; i-- > 0;)
        if (SceneRoom_ScriptSlot(s_slot[i].slot) == s_slot[i].now && !pe_room_has_slot(assets, s_slot[i].slot))
            SceneRoom_SetScriptSlot(s_slot[i].slot, s_slot[i].was);
    s_slot_count = 0;
}

int pe_rand_ai_apply(int room) {
    const uint8_t *assets = (const uint8_t *)g_GameState.loaded_scene_assets;
    RelocBlock *tasks = g_GameState.bank_reset_944[0];
    unsigned done = 0;
    if (!assets || !tasks) return 0;
    /* Still the same load (the hook runs twice a load): done already. */
    for (unsigned e = 0; e < PE_RAND_TYPES; ++e)
        if (s_copy[e]) { if (e < tasks->count && tasks->offsets[e] == s_copy[e]) return 0; break; }
    pe_rand_ai_release(assets);
    for (unsigned m = 0; m < s_map_count; ++m) {
        const PeRandAiMap *map = &s_map[m];
        uint32_t bytes, from_bytes, start, end, task_bytes;
        const uint8_t *mine;
        PeRoomBlobs src;
        if (map->room != room || s_copy[map->entity] || map->entity >= tasks->count) continue;
        mine = pe_tasks(assets, &bytes);
        if (!pe_disc_room(map->from_room, &src)) continue;
        const uint8_t *theirs = pe_tasks(src.data, &from_bytes);
        const int spawner = pe_spawner(theirs, from_bytes, map->from_entity), to_spawner = pe_spawner(mine, bytes, map->entity);
        if (!pe_entry(theirs, from_bytes, map->from_entity, &start, &end) || !pe_steps(theirs, start, end, NULL, NULL) ||
            (spawner >= 0) != (to_spawner >= 0)) {
            pe_log("rand_ai", "map=%d e%u: M%03u e%u doesn't fit (block, spawner e%d / e%d)", room, map->entity,
                   map->from_room, map->from_entity, spawner, to_spawner);
            pe_disc_room_free(&src);
            continue;
        }
        task_bytes = from_bytes - start;
        uint8_t *copy = malloc(task_bytes);
        if (!copy) { pe_disc_room_free(&src); continue; }
        memcpy(copy, theirs + start, task_bytes);
        PePatch patch = {map->from_entity, map->entity, spawner, to_spawner, 0};
        pe_steps(copy, 0, end - start, pe_patch_step, &patch);
        /* the source room's slots this room lacks */
        const SceneRoomDirectory *dir = (const void *)(src.data + pe_word(src.data, 4));
        const uint8_t *r = src.data + (dir->scripts & 0x3FFFFF);
        unsigned slots = 0;
        for (unsigned i = 0; i < dir->scripts >> 22 && s_slot_count < PE_RAND_AI_SLOTS; ++i, r += 12) {
            const void *was = SceneRoom_ScriptSlot(r[7]), *now;
            if (pe_room_has_slot(assets, r[7])) continue;
            unsigned seen = 0;
            for (unsigned k = 0; k < s_slot_count; ++k) seen |= s_slot[k].slot == r[7];
            if (seen || !(now = SceneRoom_InstallRoomScript(map->from_room, r[7], pe_word(r, 8)))) continue;
            s_slot[s_slot_count].slot = r[7]; s_slot[s_slot_count].was = was; s_slot[s_slot_count++].now = now;
            ++slots;
        }
        tasks->offsets[map->entity] = s_copy[map->entity] = copy;
        ++s_copy_count;
        pe_log("rand_ai", "map=%d e%u script of M%03u e%u, %u numbers patched (spawner e%d > e%d), %u slots installed",
               room, map->entity, map->from_room, map->from_entity, patch.done, spawner, to_spawner, slots);
        pe_disc_room_free(&src);
        ++done;
    }
    return (int)done;
}

static int pe_rand_ai_rows(void *ctx, void *user) {
    (void)ctx;
    (void)user;
    if (s_map_count || s_slot_count || s_copy_count) pe_rand_ai_apply(pe_room_number());
    return 0;
}

void pe_rand_ai_register(void) {
    pe_hook_add(PE_HOOK_PLAYER_ROWS, pe_rand_ai_rows, NULL, PE_PRIORITY_NORMAL, MOD_ID);
}
