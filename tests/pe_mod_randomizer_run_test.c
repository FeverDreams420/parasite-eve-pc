/* Room scripts run headless, for the randomizer's story order. The step listing (mod-randomizer-rooms)
 * can't tell which characters a room places at a story value: that is decided when the room starts,
 * by its own scripts. Here each room is loaded through Scene_LoadRoom, its starting actors spawn from
 * the scene table (Script_DrawSyncAndFlush), and their scripts run frame by frame with the player
 * standing still, at the story variables a run line gives.
 *
 * The interpreter loop is Script_RunQueue's, copied so the test knows each step it runs. Logic steps
 * run their real handlers (jumps, tests, variables, child routines, event handlers and sends, kills,
 * spawns, bits, scene flags, entity fields, positions, zone tests). Anything else (models, animation,
 * camera, sound, menus, movement, waits) is skipped: those steps only touch what this test doesn't read.
 * A skipped step goes straight on, as most do in the game in the same frame; the second time a routine
 * reaches the same skipped step in one frame (a wait loop with nothing to wait for) it yields there
 * until the next frame. GiveItem succeeds and is noted; CountItems reads 0 (no keys held) unless the run says keys=all.
 * A door ends the run. A frame that runs more than SPIN_STEPS steps ends it too (noted as spin).
 *
 * Usage: test-mod-randomizer-run <out.txt> [runs.txt]. Each runs.txt line: M114 0x88 0x0 [V0x41=0x20 ...]
 * [keys=all] (room, V0x4a, V0x1, more variables). Without runs.txt every room runs once at V0x4a = 0.
 * One output line per run:
 *   run M114 V0x4a=0x88 V0x1=0x0 end=<idle|door M115|spin|load> frames=N alive=<types> gave=<offsets>
 *       (end=noroom and nothing more for M5000/M8888: scenes of the main loop, not rooms)
 *       set=<V0x4a:0x88>0x190,...> steps=<e3:+1a2c,+1a40;e5:...>
 * Steps are the listing's offsets (from the task block, as in randomizer-rooms.txt) of every step run.
 *
 * Walkable floor: ops 128/129 mark collision triangles closed (bit 0x80: not walkable from a neighbour)
 * and open again. After the run the floor is flooded from Aya's triangle over open neighbours, and each
 * zone a PointInPoly step (op 119) tested is checked against it. Appended to the run line:
 *   start=<triangle|none> closed=<triangles with 0x80 at the end> cut=<+offset of each zone tested
 *   whose polygon touches no reached triangle> reach=<+offset@x,z: a reached point inside each other zone>
 * A door whose zone is cut can't be walked to in that state. A runs line may end in at=x,z[,angle]: from
 * frame AT_FRAME on, Aya stands there (facing angle), as if she had walked into that zone, for AT_FRAMES
 * frames. Up to AT_MAX at= in a row walk her through several zones in turn (AT_FRAMES each); the run
 * ends after the last. The output line then ends in at=x,z,angle;x,z,angle... too.
 *
 * A runs line with press (before its at=) also presses the examine / talk button while Aya stands at
 * each stop: one frame down every PRESS_EVERY frames from PRESS_AT on. The button is the pad's Circle,
 * which the boot pad table maps to field bits 0x100, 0x200 and 0x20000000 (the scripts' tests: op 17
 * modes 0-3, whose real handler runs in these runs). The output line then ends in press=1.
 *
 * Op 85 (Boot_MemCardStepInit, misnamed: the battle transition, CD reads and fade) is skipped, so the
 * battle never runs or ends; an encounter routine has often told the room's door controller to die
 * just before it. A run that reaches op 85 ends its line in battle=1: its alive= is not the room's. */
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
#include "pe1/field_collision.h"
#include "pe1/field_sfx.h"
#include "pe1/game_state.h"
#include "pe1/inventory.h"
#include "pe1/pad_state.h"
#include "pe1/player_entity.h"
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

enum { AT_FRAME = 60, AT_FRAMES = 240, AT_MAX = 8, FRAMES = 600, SPIN_STEPS = 200000, VARS = 512, TAKE = 0x458 };
enum { PRESS_AT = 20, PRESS_EVERY = 30 };
static const u32 press_bits = 0x20000300;   /* Circle: field bits 8, 9 and 29 */
static unsigned failures;
#define FAIL(...) do { if (++failures <= 40) { fprintf(stderr, __VA_ARGS__); fputc('\n', stderr); } } while (0)

/* Opcodes whose real handler runs: logic over script variables, nodes and entity fields. */
static const u16 real_ops[] = {
    0, 1, 2, 4, 5, 9, 10, 11, 12, 18, 19, 20, 24, 25, 26, 28, 29, 30, 31, 32, 35, 36, 38, 40, 41, 42, 43,
    45, 50, 55, 57, 63, 64, 65, 66, 79, 80, 81, 82, 83, 94, 119, 120, 121, 128, 129, 136, 139, 170, 171, 199,
    222, 240,   /* 240: 1 unless a battle is set up (D_8009D2E8 & 1): zone routines wait on it */
};
static u8 is_real[8192];

/* The run in progress. */
static const u8 *block;          /* task block start: listing offsets count from here */
static size_t block_bytes;
static u8 *ran;                  /* one per word of the block: step started there */
static u32 gave[64]; static unsigned gave_count;
static int keys_all, press, battle, spin, trace;   /* trace: PE_RUN_TRACE set, each step to stderr */
static unsigned long frame_steps;
/* Zones PointInPoly tested: listing offset and the polygon's whole-unit corners (x, z). */
typedef struct Zone { u32 at; s16 x[4], z[4]; } Zone;
static Zone zones[256]; static unsigned zone_count;

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
        size_t skipped[64]; unsigned skipped_count = 0;
        int result;
        do {
            u32 *step = g_ScriptCursor, header = step[0], tail = step[1], modes = header >> 17;
            u32 op = header & 0x1FFF, count = (header >> 13) & 15, *values = step + 2;
            g_ScriptCursor = values + count;
            size_t at = (size_t)((const u8 *)step - block);
            if (at < block_bytes) ran[at / 4] = 1;
            if (++frame_steps > SPIN_STEPS) { spin = 1; return; }
            if (op == 85) battle = 1;
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
            if (op == 119 && at < block_bytes) {
                unsigned z = 0;
                while (z < zone_count && zones[z].at != at) ++z;
                if (z == zone_count && zone_count < 256) zones[zone_count++].at = (u32)at;
                if (z < 256)
                    for (int i = 0; i < 4; ++i) { zones[z].x[i] = (s16)(*args[i * 2] >> 16); zones[z].z[i] = (s16)(*args[i * 2 + 1] >> 16); }
            }
            if (op == 139 && *args[0] != 0) {   /* enemy field: actors here have no battle state, as if absent */
                FieldActor *a = g_FieldActorListHead;
                while (a && (a->type_id != *args[0] || a->sub_id != *args[1] || (a->flags & 0x10))) a = a->next;
                result = a && a->state ? g_ScriptQueueOpcodeHandlers[op](args) : 1;
            } else if (is_real[op]) result = g_ScriptQueueOpcodeHandlers[op](args);
            else if (op == 49) result = g_ScriptQueueOpcodeHandlers[op](args);   /* door: ends the run */
            else if (op == 17 && press && (u32)*args[0] <= 3) result = g_ScriptQueueOpcodeHandlers[op](args);   /* pad */
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
            } else {                                                              /* skipped */
                unsigned k = 0;
                while (k < skipped_count && skipped[k] != at) ++k;
                if (k < skipped_count || skipped_count == 64) { result = 0; g_ScriptNodePool->active = 1; }
                else { skipped[skipped_count++] = at; result = 1; }
            }
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

/* The collision floor: triangle i's header, links and corner c. */
static u8 *tri_at(unsigned i) {
    return g_CollisionPlaneTable ? (u8 *)&g_CollisionDb->triangles.xyz[i] : (u8 *)&g_CollisionDb->triangles.xz[i];
}
static const CollisionTriangleLinks *tri_links(unsigned i) {
    return g_CollisionPlaneTable ? &g_CollisionDb->triangles.xyz[i].layout.links : &g_CollisionDb->triangles.xz[i].layout.links;
}
static void tri_corner(unsigned i, int c, s16 *x, s16 *z) {
    CollisionVertexTable v;
    if (g_CollisionPlaneTable) { COLLISION_VERTEX(v, tri_links(i)->vertices[c], CollisionVertexXYZ); *x = (s16)v.xyz->x; *z = (s16)v.xyz->z; }
    else { COLLISION_VERTEX(v, tri_links(i)->vertices[c], CollisionVertexXZ); *x = (s16)v.xz->x; *z = (s16)v.xz->z; }
}

/* Point (x, z) in the quad, by crossing count; points on an edge count as in. */
static int in_quad(const Zone *q, int x, int z) {
    int in = 0;
    for (int i = 0, j = 3; i < 4; j = i++) {
        int xi = q->x[i], zi = q->z[i], xj = q->x[j], zj = q->z[j];
        long long cross = (long long)(xj - xi) * (z - zi) - (long long)(zj - zi) * (x - xi);
        if (cross == 0 && x >= (xi < xj ? xi : xj) && x <= (xi > xj ? xi : xj) && z >= (zi < zj ? zi : zj) && z <= (zi > zj ? zi : zj)) return 1;
        if ((zi > z) != (zj > z) && x < xi + (long long)(xj - xi) * (z - zi) / (zj - zi)) in = !in;
    }
    return in;
}
static long long side(int ax, int az, int bx, int bz, int px, int pz) { return (long long)(bx - ax) * (pz - az) - (long long)(bz - az) * (px - ax); }
static int segments_cross(int ax, int az, int bx, int bz, int cx, int cz, int dx, int dz) {
    long long d1 = side(cx, cz, dx, dz, ax, az), d2 = side(cx, cz, dx, dz, bx, bz);
    long long d3 = side(ax, az, bx, bz, cx, cz), d4 = side(ax, az, bx, bz, dx, dz);
    return ((d1 > 0 && d2 < 0) || (d1 < 0 && d2 > 0)) && ((d3 > 0 && d4 < 0) || (d3 < 0 && d4 > 0));
}
/* Triangle i and the zone share any area (a corner of one inside the other, or crossing edges). */
static int tri_meets_zone(unsigned i, const Zone *q) {
    s16 x[3], z[3];
    for (int c = 0; c < 3; ++c) { tri_corner(i, c, &x[c], &z[c]); if (in_quad(q, x[c], z[c])) return 1; }
    u8 *t = tri_at(i);
    for (int k = 0; k < 4; ++k) if (Geo_PointInTri(t, q->x[k], q->z[k])) return 1;
    for (int c = 0; c < 3; ++c)
        for (int k = 0, l = 3; k < 4; l = k++)
            if (segments_cross(x[c], z[c], x[(c + 1) % 3], z[(c + 1) % 3], q->x[l], q->z[l], q->x[k], q->z[k])) return 1;
    return 0;
}

/* A point inside both triangle i and the zone, or 0. */
static int stand_point(unsigned i, const Zone *q, int *px, int *pz) {
    s16 x[3], z[3];
    for (int c = 0; c < 3; ++c) tri_corner(i, c, &x[c], &z[c]);
    int cx = (q->x[0] + q->x[1] + q->x[2] + q->x[3]) / 4, cz = (q->z[0] + q->z[1] + q->z[2] + q->z[3]) / 4;
    int tx = (x[0] + x[1] + x[2]) / 3, tz = (z[0] + z[1] + z[2]) / 3;
    int cand[16][2], n = 0;
    cand[n][0] = cx; cand[n++][1] = cz;
    cand[n][0] = tx; cand[n++][1] = tz;
    for (int k = 0; k < 4; ++k) { cand[n][0] = (q->x[k] + 3 * cx) / 4; cand[n++][1] = (q->z[k] + 3 * cz) / 4; }
    for (int c = 0; c < 3; ++c) { cand[n][0] = (x[c] + 3 * tx) / 4; cand[n++][1] = (z[c] + 3 * tz) / 4; }
    for (int k = 0; k < 4; ++k) { cand[n][0] = (q->x[k] + tx) / 2; cand[n++][1] = (q->z[k] + tz) / 2; }
    for (int k = 0; k < n; ++k)
        if (in_quad(q, cand[k][0], cand[k][1]) && Geo_PointInTri(tri_at(i), (s16)cand[k][0], (s16)cand[k][1])) {
            *px = cand[k][0]; *pz = cand[k][1]; return 1;
        }
    return 0;
}

/* Flood the floor from Aya's triangle over neighbours without 0x80; prints start=, closed=, cut= and reach=. */
static void print_floor(FILE *out) {
    unsigned n = g_CollisionDb ? g_CollisionDb->triangleCount : 0, start = n;
    FieldActor *aya = g_PlayerEntity;
    if (aya && n)
        for (unsigned i = 0; i < n && start == n; ++i)
            if (Geo_PointInTri(tri_at(i), (s16)(aya->pos_x >> 16), (s16)(aya->pos_z >> 16))) start = i;
    if (aya) fprintf(out, "\taya=%d,%d", aya->pos_x >> 16, aya->pos_z >> 16); else fputs("\taya=none", out);
    if (aya && n && start == n) {   /* off the floor (no spawn for this V0x1): the triangle nearest her, start=~N */
        long long best = -1;
        for (unsigned i = 0; i < n; ++i) {
            s16 x, z; long long sx = 0, sz = 0;
            for (int c = 0; c < 3; ++c) { tri_corner(i, c, &x, &z); sx += x; sz += z; }
            long long dx = sx / 3 - (aya->pos_x >> 16), dz = sz / 3 - (aya->pos_z >> 16), d = dx * dx + dz * dz;
            if (!(*tri_at(i) & 0x80) && (best < 0 || d < best)) { best = d; start = i; }
        }
        if (start < n) fprintf(out, "\tstart=~%u", start);
    } else if (start < n) fprintf(out, "\tstart=%u", start);
    if (start == n) fputs("\tstart=none", out);
    fputs("\tclosed=", out);
    int first = 1;
    for (unsigned i = 0; i < n; ++i) if (*tri_at(i) & 0x80) { fprintf(out, "%s%u", first ? "" : ",", i); first = 0; }
    fputs("\tcut=", out);
    if (start == n) return;
    u8 *reached = calloc(n, 1); unsigned *queue = malloc(n * sizeof(*queue)), head = 0, tail = 0;
    reached[start] = 1; queue[tail++] = start;
    while (head < tail) {
        const CollisionTriangleLinks *l = tri_links(queue[head++]);
        for (int s = 0; s < 3; ++s) {
            unsigned m = l->neighbours[s];
            if (m < n && !reached[m] && !(*tri_at(m) & 0x80)) { reached[m] = 1; queue[tail++] = m; }
        }
    }
    static int px[256], pz[256]; static u8 has[256];
    first = 1;
    for (unsigned z = 0; z < zone_count; ++z) {
        unsigned i = 0;
        has[z] = 0;
        while (i < n && !(reached[i] && tri_meets_zone(i, &zones[z]))) ++i;
        if (i == n) { fprintf(out, "%s+%X", first ? "" : ",", zones[z].at); first = 0; continue; }
        for (; i < n && !has[z]; ++i) if (reached[i] && stand_point(i, &zones[z], &px[z], &pz[z])) has[z] = 1;
    }
    fputs("\treach=", out);
    first = 1;
    for (unsigned z = 0; z < zone_count; ++z)
        if (has[z]) { fprintf(out, "%s+%X@%d,%d", first ? "" : ",", zones[z].at, px[z], pz[z]); first = 0; }
    free(reached); free(queue);
}

/* Room number a scene token names, or -1 (M0000, M5000 and M8888 are scenes of the main loop). */
static int token_room(u32 token) {
    char name[8] = {0};
    Text_EncodeBase32(name, token);
    if (name[0] != 'M' || name[5] != 'I') return -1;
    for (int i = 1; i < 5; ++i) if (name[i] < '0' || name[i] > '9') return -1;
    return atoi(name + 1);
}

typedef struct Run {
    unsigned room; u32 area, left; unsigned nvars; u16 var[32]; u32 val[32]; int keys, press;
    int at, at_x[AT_MAX], at_z[AT_MAX], at_angle[AT_MAX];   /* Aya's stops from AT_FRAME on, at of them */
} Run;

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
        if (!strcmp(tok, "press")) { r->press = 1; continue; }
        if (!strncmp(tok, "at=", 3)) {
            if (r->at == AT_MAX || sscanf(tok + 3, "%d,%d,%d", &r->at_x[r->at], &r->at_z[r->at], &r->at_angle[r->at]) < 2) return 0;
            ++r->at;
            continue;
        }
        char *eq = strchr(tok, '=');
        if (tok[0] != 'V' || !eq || r->nvars == 32) return 0;
        r->var[r->nvars] = (u16)strtoul(tok + 1, NULL, 0); r->val[r->nvars++] = (u32)strtoul(eq + 1, NULL, 0);
    }
    return r->room >= 1 && r->room <= SCENE_ROOM_COUNT ? 1 : 2;   /* 2: M5000, M8888 (main-loop scenes) */
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
    keys_all = r->keys; press = r->press; battle = 0; spin = 0; gave_count = 0; zone_count = 0;
    g_FieldPadBits = g_FieldPadPressed = g_FieldPadReleased = 0;
    memset(g_FieldPadHeldFrameCounts, 0, 32 * sizeof(*g_FieldPadHeldFrameCounts));
    g_GameStateFlags &= ~0x2100u; g_SceneDispatchToken = token; g_FieldSfxQueueCount = 0;
    g_EntitySerialCounter = 1;
    /* No models: actors spawn on Scene_LoadMap's modelless path (no render or animation blocks). */
    memset(g_GameState.bank_slots, 0, sizeof(g_GameState.bank_slots));
    Entity_InitFreePool(); Script_InitNodeFreeList(); Entity_RelocateSceneData();
    Script_DrawSyncAndFlush();

    unsigned frame;
    for (frame = 0; frame < (r->at ? AT_FRAME + r->at * AT_FRAMES : FRAMES) && !spin && !(g_GameStateFlags & 0x2000); ++frame) {
        frame_steps = 0;
        if (r->at && frame >= AT_FRAME && g_PlayerEntity) {
            FieldActor *aya = g_PlayerEntity;
            int stop = (int)(frame - AT_FRAME) / AT_FRAMES;
            aya->pos_x = aya->base_x = r->at_x[stop] << 16; aya->pos_z = aya->base_z = r->at_z[stop] << 16;
            aya->rot_y = (short)r->at_angle[stop];
        }
        if (press) {   /* the button, as Field_HandleStateTransition leaves it for this frame */
            unsigned k = frame >= AT_FRAME ? (frame - AT_FRAME) % AT_FRAMES : 0;
            u32 down = r->at && frame >= AT_FRAME && k >= PRESS_AT && (k - PRESS_AT) % PRESS_EVERY == 0 ? press_bits : 0;
            g_FieldPadPressed = down & ~g_FieldPadBits; g_FieldPadReleased = g_FieldPadBits & ~down; g_FieldPadBits = down;
            for (int b = 0; b < 32; ++b) if (press_bits >> b & 1) g_FieldPadHeldFrameCounts[b] = down ? g_FieldPadHeldFrameCounts[b] + 1 : 0;
        }
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
    print_floor(out);
    for (int i = 0; i < r->at; ++i) fprintf(out, "%s%d,%d,%d", i ? ";" : "\tat=", r->at_x[i], r->at_z[i], r->at_angle[i]);
    if (press) fputs("\tpress=1", out);
    if (battle) fputs("\tbattle=1", out);
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
            int read = parse_run(line, &r);
            if (!read) { FAIL("runs line %u can't be read", n); continue; }
            if (read == 2) {   /* no room to run, but one output line per runs line all the same */
                if (out) { fprintf(out, "run\tM%03u\tV0x4a=%#x\tV0x1=%#x\tend=noroom\n", r.room, r.area, r.left); fflush(out); }
                continue;
            }
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
