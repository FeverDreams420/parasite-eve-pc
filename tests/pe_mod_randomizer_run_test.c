/* Room scripts run headless, for the randomizer's story order. The step listing (mod-randomizer-rooms)
 * can't tell which characters a room places at a story value: that is decided when the room starts,
 * by its own scripts. Here each room is loaded through Scene_LoadRoom, its starting actors spawn from
 * the scene table (Script_DrawSyncAndFlush), and their scripts run frame by frame with the player
 * standing still, at the story variables a run line gives.
 *
 * The interpreter loop is Script_RunQueue's, copied so the test knows each step it runs. Logic steps
 * run their real handlers (jumps, tests, variables, child routines, event handlers and sends, kills,
 * spawns, bits, scene flags, entity fields). Anything else (models, animation, camera, sound, menus,
 * movement, waits) is skipped as a one-frame yield: those steps only touch what this test doesn't
 * read. GiveItem succeeds and is noted; CountItems reads 0 (no keys held) unless the run says keys=all.
 * A door ends the run. A frame that runs more than SPIN_STEPS steps ends it too (noted as spin).
 *
 * Usage: test-mod-randomizer-run <out.txt> [runs.txt]. Each runs.txt line: M114 0x88 0x0 [V0x41=0x20 ...]
 * [keys=all] (room, V0x4a, V0x1, more variables). Without runs.txt every room runs once at V0x4a = 0.
 * One output line per run:
 *   run M114 V0x4a=0x88 V0x1=0x0 end=<idle|door M115|spin|load> frames=N alive=<types> gave=<offsets>
 *       set=<V0x4a:0x88>0x190,...> steps=<e3:+1a2c,+1a40;e5:...>
 * Steps are the listing's offsets (from the task block, as in randomizer-rooms.txt) of every step run. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL3/SDL.h>
#include "common.h"
#include "pe1/asset_table.h"
#include "pe1/boot.h"
#include "pe1/boot_disc_check.h"
#include "pe1/boot_services.h"
#include "pe1/cdrom.h"
#include "pe1/entity_allocation.h"
#include "pe1/field_actor.h"
#include "pe1/field_sfx.h"
#include "pe1/game_state.h"
#include "pe1/inventory.h"
#include "pe1/reloc_block.h"
#include "pe1/scene_room.h"
#include "pe1/scene_room_load.h"
#include "pe1/script_node.h"
#include "pe1/script_queue.h"
#include "pe1/script_state.h"

/* Fixture I/O: the test-disc object renames the real image reads to these. */
int CdRom_ImageReadSectorsFromLba(u32 lba, void *buffer, u32 count);
int CdRom_ImagePollReady(void);
int CdRom_ReadSectorsFromLba(u32 lba, void *buffer, u32 count) { return CdRom_ImageReadSectorsFromLba(lba, buffer, count); }
int CdRom_PollReady(void) { return CdRom_ImagePollReady(); }
int Asset_Find08w(int id, int mode, int x, int y, int z) { (void)id; (void)mode; (void)x; (void)y; (void)z; return 0; }

void Entity_RelocateSceneData(void);
void Script_GarbageCollectNodes(void);
void Entity_CollectGarbage(void);
FieldActor *Scene_LoadMap(u8 *scene, FieldActor *after, int allocateFull);

enum { FRAMES = 600, SPIN_STEPS = 200000, VARS = 512, TAKE = 0x458 };
static unsigned failures;
#define FAIL(...) do { if (++failures <= 40) { fprintf(stderr, __VA_ARGS__); fputc('\n', stderr); } } while (0)

/* Opcodes whose real handler runs: logic over script variables, nodes and entity fields. */
static const u16 real_ops[] = {
    0, 1, 2, 4, 5, 9, 10, 18, 19, 20, 24, 25, 26, 28, 29, 30, 31, 32, 35, 36, 38, 40, 41, 42, 43, 45,
    50, 55, 57, 63, 64, 65, 66, 79, 80, 81, 82, 83, 120, 121, 136, 139, 170, 171, 199, 222,
};
static u8 is_real[8192];

/* The run in progress. */
static const u8 *block;          /* task block start: listing offsets count from here */
static size_t block_bytes;
static u8 *ran;                  /* one per word of the block: step started there */
static u32 gave[64]; static unsigned gave_count;
static int keys_all, spin, trace;   /* trace: PE_RUN_TRACE set, each step to stderr */
static unsigned long frame_steps;

static u32 word_at(const u8 *p, size_t offset) { u32 v; memcpy(&v, p + offset, 4); return v; }

/* Script_RunQueue (script/flow_opcodes.c), with the step policy above. */
static void run_queue(void) {
    int *args[16];
    do {
        ScriptNode *node = g_ScriptNodePool;
        FieldActor *entity = g_CurrentEntity;
        if (node->flags & 0x50) continue;
        if ((entity->flags & 0x1000) && !(node->flags & 0x80)) continue;
        if ((g_GameStateFlags & 0x100) && entity != g_PlayerEntity && !(node->flags & 0x80)) continue;
        if (node->active == 0) continue;
        node->active--;
        if (node->active != 0) continue;
        g_ScriptCursor = node->current.script;
        int result;
        do {
            u32 *step = g_ScriptCursor, header = step[0], tail = step[1], modes = header >> 17;
            u32 op = header & 0x1FFF, count = (header >> 13) & 15, *values = step + 2;
            g_ScriptCursor = values + count;
            size_t at = (size_t)((const u8 *)step - block);
            if (at < block_bytes) ran[at / 4] = 1;
            if (++frame_steps > SPIN_STEPS) { spin = 1; return; }
            if (trace) { fprintf(stderr, "e%u +%#zx op %u\n", entity->type_id, at, op); fflush(stderr); }
            for (u32 i = 0; i < count; i++) {
                switch (modes & 7) {
                case 0: args[i] = (int *)&values[i]; break;
                case 3: args[i] = (int *)&g_ScriptOperandTable[values[i]]; break;
                case 1: args[i] = (int *)&g_CurrentEntity->pad_0AC[values[i]]; break;
                case 2: args[i] = (int *)&g_ScriptVariableBank[values[i]]; break;
                case 4: args[i] = &g_ScriptNodeActiveFlags[values[i]]; break;
                }
                modes >>= 3;
                if (i == 4) modes = tail;
            }
            if (is_real[op]) result = g_ScriptQueueOpcodeHandlers[op](args);
            else if (op == 49) result = g_ScriptQueueOpcodeHandlers[op](args);   /* door: ends the run */
            else if (op == 167) {                                                 /* GiveItem: given */
                if (gave_count < 64) gave[gave_count++] = (u32)at;
                *args[1] = 0; result = 1;
            } else if (op == 168) { *args[1] = keys_all; result = 1; }            /* CountItems */
            else if (op == 8) {                                                   /* spawn at a point */
                char scene[2] = { (char)*args[0], (char)*args[1] };
                if (trace) { fprintf(stderr, "spawn type %d sub %d\n", scene[0], scene[1]); fflush(stderr); }
                FieldActor *a = Scene_LoadMap((u8 *)scene, g_CurrentEntity, 1);
                if (a) { a->pos_x = *args[2]; a->pos_y = *args[3]; a->pos_z = *args[4]; }
                result = 1;
            } else if (op == 17 && *args[0] == 2500) {                            /* spawn command */
                char scene[2] = { (char)*args[1], (char)*args[2] };
                FieldActor *a = Scene_LoadMap((u8 *)scene, g_CurrentEntity, 0);
                if (a) { a->pos_x = *args[3]; a->pos_y = *args[4]; a->pos_z = *args[5]; }
                result = 1;
            } else { result = 0; g_ScriptNodePool->active = 1; }                  /* skipped: yield */
            if (result < 0) { result = 0; g_ScriptNodePool->active = 1; }
        } while (result && !(g_GameStateFlags & 0x2000));
        g_ScriptNodePool->current.script = g_ScriptCursor;
        if (g_GameStateFlags & 0x2000) return;
    } while ((g_ScriptNodePool = g_ScriptNodePool->next) != 0);
}

/* Entity_DispatchCallbacks: the actor's three node lists. */
static void dispatch(FieldActor *a) {
    g_CurrentEntity = a;
    for (int i = 0; i < 3 && !spin && !(g_GameStateFlags & 0x2000); ++i) {
        g_ScriptNodePool = a->script_node_lists[i];
        if (g_ScriptNodePool) run_queue();
    }
}

/* Room number a scene token names, or -1 (M0000, M5000 and M8888 are scenes of the main loop). */
static int token_room(u32 token) {
    char name[8] = {0};
    Text_EncodeBase32(name, token);
    if (name[0] != 'M' || name[5] != 'I') return -1;
    for (int i = 1; i < 5; ++i) if (name[i] < '0' || name[i] > '9') return -1;
    return atoi(name + 1);
}

typedef struct Run { unsigned room; u32 area, left; unsigned nvars; u16 var[32]; u32 val[32]; int keys; } Run;

static int parse_run(char *line, Run *r) {
    memset(r, 0, sizeof(*r));
    char *tok = strtok(line, " \t\r\n");
    if (!tok || (tok[0] != 'M' && tok[0] != 'm')) return 0;
    r->room = (unsigned)atoi(tok + 1);
    if (!(tok = strtok(NULL, " \t\r\n"))) return 0;
    r->area = (u32)strtoul(tok, NULL, 0);
    if (!(tok = strtok(NULL, " \t\r\n"))) return 0;
    r->left = (u32)strtoul(tok, NULL, 0);
    while ((tok = strtok(NULL, " \t\r\n"))) {
        if (!strcmp(tok, "keys=all")) { r->keys = 1; continue; }
        char *eq = strchr(tok, '=');
        if (tok[0] != 'V' || !eq || r->nvars == 32) return 0;
        r->var[r->nvars] = (u16)strtoul(tok + 1, NULL, 0); r->val[r->nvars++] = (u32)strtoul(eq + 1, NULL, 0);
    }
    return r->room >= 1 && r->room <= SCENE_ROOM_COUNT;
}

static unsigned runs_done, runs_door, runs_spin;

static void do_run(const Run *r, FILE *out) {
    char name[16]; snprintf(name, sizeof(name), "m%04ui", r->room);
    u32 token = (u32)Text_ParseBase32Id((signed char *)name);
    if (!g_RoomSectorTable[r->room - 1].roomSectors) { FAIL("M%03u is no room on the disc", r->room); return; }
    Entity_ResetAllPools();   /* zeroes the variables, node flags and actors, as a new game does */
    int status = Scene_LoadRoom(token);
    if (status) { if (out) fprintf(out, "run\tM%03u\tV0x4a=%#x\tV0x1=%#x\tend=load %d\n", r->room, r->area, r->left, status); return; }
    const u8 *assets = (const u8 *)g_GameState.loaded_scene_assets;
    const SceneRoomDirectory *directory = (const void *)(assets + word_at(assets, 4));
    u32 table = directory->taskBlock & 0x3FFFFF;
    block_bytes = word_at(assets, table) & 0xFFFFFF;
    block = assets + (word_at(assets, table + 4) & 0xFFFFFF);
    free(ran); ran = calloc(block_bytes / 4 + 1, 1);

    static u32 before[VARS];
    g_ScriptVariableBank[SCRIPT_VAR_AREA] = r->area; g_ScriptVariableBank[SCRIPT_VAR_ROOM] = r->left;
    for (unsigned i = 0; i < r->nvars; ++i) if (r->var[i] < VARS) g_ScriptVariableBank[r->var[i]] = r->val[i];
    memcpy(before, g_ScriptVariableBank, sizeof(before));
    keys_all = r->keys; spin = 0; gave_count = 0;
    g_GameStateFlags &= ~0x2100u; g_SceneDispatchToken = token; g_FieldSfxQueueCount = 0;
    g_EntitySerialCounter = 1;
    /* No models: actors spawn on Scene_LoadMap's modelless path (no render or animation blocks). */
    memset(g_GameState.bank_slots, 0, sizeof(g_GameState.bank_slots));
    Entity_InitFreePool(); Script_InitNodeFreeList(); Entity_RelocateSceneData();
    Script_DrawSyncAndFlush();

    unsigned frame;
    for (frame = 0; frame < FRAMES && !spin && !(g_GameStateFlags & 0x2000); ++frame) {
        frame_steps = 0;
        Scene_UpdateEntityList();
        for (FieldActor *a = g_FieldActorListHead; a && !spin && !(g_GameStateFlags & 0x2000); a = a->next)
            if (!(a->flags & 0x10)) dispatch(a);
        /* End of Entity_FrameUpdate: freed nodes and dead actors back to their pools, new actors settled. */
        Script_GarbageCollectNodes();
        Entity_CollectGarbage();
        for (FieldActor *a = g_FieldActorListHead; a; a = a->next) a->flags &= ~0x10000000;
    }
    ++runs_done;
    if (!out) return;
    fprintf(out, "run\tM%03u\tV0x4a=%#x\tV0x1=%#x\tend=", r->room, r->area, r->left);
    if (g_GameStateFlags & 0x2000) {
        int to = token_room(g_SceneDispatchToken); ++runs_door;
        if (to >= 0) fprintf(out, "door M%03d", to); else fprintf(out, "door %08x", (u32)g_SceneDispatchToken);
    } else if (spin) { fputs("spin", out); ++runs_spin; }
    else fputs("idle", out);
    fprintf(out, "\tframes=%u\talive=", frame);
    int first = 1;
    for (FieldActor *a = g_FieldActorListHead; a; a = a->next)
        if (!(a->flags & 0x10)) { fprintf(out, "%s%u", first ? "" : ",", a->type_id); first = 0; }
    fputs("\tgave=", out);
    for (unsigned i = 0; i < gave_count; ++i) fprintf(out, "%s+%06X", i ? "," : "", gave[i]);
    fputs("\tset=", out);
    first = 1;
    for (unsigned v = 0; v < VARS; ++v)
        if (g_ScriptVariableBank[v] != before[v]) { fprintf(out, "%sV%#x:%#x>%#x", first ? "" : ",", v, before[v], g_ScriptVariableBank[v]); first = 0; }
    fputs("\tsteps=", out);
    first = 1;
    for (size_t w = 0; w <= block_bytes / 4; ++w)
        if (ran[w]) { fprintf(out, "%s%#zx", first ? "" : ",", w * 4); first = 0; }
    fputc('\n', out);
}

int main(int argc, char **argv) {
    const char *path = getenv("PE_TEST_DISC");
    if (!path || !*path) return getenv("PE_REQUIRE_GAME_ASSETS") ? 1 : 77;
    PeDisc disc = {0};
    /* Nothing is drawn, but ResetGraph opens psyz: keep it windowless whichever way the test runs. */
    if (!SDL_SetEnvironmentVariable(SDL_GetEnvironment(), "PSYZ_HEADLESS", "1", true) ||
        !SDL_SetEnvironmentVariable(SDL_GetEnvironment(), "SDL_AUDIODRIVER", "dummy", true)) return 1;
    if (!SDL_Init(SDL_INIT_EVENTS) || !pe_disc_open(&disc, path)) return 1;
    ResetGraph(0); CdRom_BindImage(&disc, NULL);
    if (Asset_OpenPeImage() != 0) return 1;
    Boot_InitGameState(); D_8009CDC8 = 'm';
    /* The resident data and a new game's bag: spawning Aya builds her weapon and armour lists. */
    if (Scene_LoadSceneData() < 0) { fputs("Scene_LoadSceneData failed\n", stderr); return 1; }
    Inv_InitNewGameInventory();
    for (size_t i = 0; i < sizeof(real_ops) / sizeof(*real_ops); ++i) is_real[real_ops[i]] = 1;
    trace = getenv("PE_RUN_TRACE") != NULL;
    /* A third argument: start at that runs line (1-based) and append, after a run that crashed. */
    unsigned from = argc > 3 ? (unsigned)atoi(argv[3]) : 1;
    FILE *out = argc > 1 ? fopen(argv[1], from > 1 ? "a" : "w") : NULL;
    if (argc > 1 && !out) return 1;
    if (argc > 2) {
        FILE *in = fopen(argv[2], "r");
        if (!in) return 1;
        char line[1024]; unsigned n = 0;
        while (fgets(line, sizeof(line), in)) {
            ++n; Run r;
            if (n < from || line[0] == '#' || line[0] == '\n') continue;
            if (!parse_run(line, &r)) { FAIL("runs line %u can't be read", n); continue; }
            if (trace) fprintf(stderr, "line %u\n", n);
            do_run(&r, out);
            if (out) fflush(out);   /* one line per run on disk: a crash names the next line */
        }
        fclose(in);
    } else {
        for (unsigned room = 1; room <= SCENE_ROOM_COUNT; ++room) {
            if (!g_RoomSectorTable[room - 1].roomSectors) continue;
            Run r = { .room = room };
            do_run(&r, out);
        }
    }
    if (out) fclose(out);
    free(ran);
    CdRom_BindImage(NULL, NULL); pe_disc_close(&disc);
    if (failures) { fprintf(stderr, "%u failures\n", failures); return 1; }
    printf("Randomizer runs: %u room runs, %u ended at a door, %u spun\n", runs_done, runs_door, runs_spin);
    return 0;
}
