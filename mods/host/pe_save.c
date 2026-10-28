/* Mods' per-save data, kept by the host. See pe_save.h. */
#include <stdio.h>
#include <string.h>

#include "pe_mod_host.h"
#include "pe_save.h"

#define PE_INV_CMD_START_NEW_GAME 1117
#define PE_SAVE_MAX_RECORDS 32
#define PE_SAVE_MAX_ENTRIES 64      /* records in one tail: 8 header bytes each, so 108 at most */
#define PE_SAVE_MAGIC "PEMC"
#define PE_SAVE_FORMAT 1

typedef struct {
    uint32_t id;
    uint16_t version, size;
    const uint8_t *bytes;
} PeSaveEntry;

/* Registered records: the running mods' state. kept = the loaded save holds a newer layout than
 * this build reads, so its carried bytes are written back instead of the (reset) state. */
static struct {
    const PeSaveRecord *record;
    const char *mod_id;
    uint32_t id;
    int kept;
} s_reg[PE_SAVE_MAX_RECORDS];
static int s_reg_count;
static unsigned s_reg_bytes = PE_SAVE_HEADER_SIZE;

/* Carried: the records of the save loaded or last written, pointing into s_tail. foreign = the
 * loaded tail is a container format newer than this build: s_tail is written back as it is. */
static uint8_t s_tail[PE_SAVE_TAIL_SIZE], s_lin[PE_SAVE_ROOM];
static PeSaveEntry s_car[PE_SAVE_MAX_ENTRIES];
static int s_car_count, s_foreign;

/* Saves from before: each mod's fixed block, read as a version 0 record holding the block. */
static const struct { const char *name; unsigned at, size; char magic[5]; } s_legacy[] = {
    { "pe_economy.loot", 0x000, 0x020, "PELT" },
    { "pe_economy.catalog", 0x040, 0x148, "PECT" },
    { "pe_economy.shop", 0x1C0, 0x014, "PESH" },
    { "pe_qol.keys", 0x200, 0x054, "PKEY" },
    { "pe_qol.sort", 0x260, 0x028, "SORT" },
};
#define PE_LEGACY_COUNT ((int)(sizeof s_legacy / sizeof s_legacy[0]))

static uint32_t pe_fnv(const char *name) {
    uint32_t h = 0x811C9DC5u;
    for (; *name; ++name) h = (h ^ (uint8_t)*name) * 0x01000193u;
    return h;
}

static uint32_t pe_crc32(const uint8_t *p, unsigned n) {
    uint32_t crc = 0xFFFFFFFFu;
    unsigned i, bit;
    for (i = 0; i < n; ++i) {
        crc ^= p[i];
        for (bit = 0; bit < 8; ++bit) crc = crc & 1u ? (crc >> 1) ^ 0xEDB88320u : crc >> 1;
    }
    return ~crc;
}

static uint16_t pe_get16(const uint8_t *p) { return (uint16_t)(p[0] | p[1] << 8); }
static uint32_t pe_get32(const uint8_t *p) { return (uint32_t)pe_get16(p) | (uint32_t)pe_get16(p + 2) << 16; }
static void pe_put16(uint8_t *p, unsigned v) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); }
static void pe_put32(uint8_t *p, uint32_t v) { pe_put16(p, v & 0xFFFFu); pe_put16(p + 2, v >> 16); }

static const char *pe_name_of(uint32_t id, const char **mod_id) {
    int i;
    for (i = 0; i < s_reg_count; ++i)
        if (s_reg[i].id == id) {
            if (mod_id) *mod_id = s_reg[i].mod_id;
            return s_reg[i].record->name;
        }
    for (i = 0; i < PE_LEGACY_COUNT; ++i)
        if (pe_fnv(s_legacy[i].name) == id) {
            if (mod_id) *mod_id = NULL;
            return s_legacy[i].name;
        }
    if (mod_id) *mod_id = NULL;
    return NULL;
}

/* The container's bytes run through the tail around the reserved window (pe_save.h). */
static void pe_gather(const uint8_t *tail, uint8_t *lin) {
    memcpy(lin, tail, PE_SAVE_WINDOW_AT);
    memcpy(lin + PE_SAVE_WINDOW_AT, tail + PE_SAVE_WINDOW_END, PE_SAVE_TAIL_SIZE - PE_SAVE_WINDOW_END);
}
static void pe_scatter(const uint8_t *lin, uint8_t *tail) {
    memcpy(tail, lin, PE_SAVE_WINDOW_AT);
    memcpy(tail + PE_SAVE_WINDOW_END, lin + PE_SAVE_WINDOW_AT, PE_SAVE_TAIL_SIZE - PE_SAVE_WINDOW_END);
}
static int pe_in_window(int i) { return s_legacy[i].at >= PE_SAVE_WINDOW_AT && s_legacy[i].at < PE_SAVE_WINDOW_END; }

/* The records in a tail: a container (its bytes gathered into lin), else the old fixed blocks.
 * -1: a damaged container; -2: a container in a format (or with flags) newer than this build. */
static int pe_parse(const uint8_t *tail, uint8_t *lin, PeSaveEntry *out) {
    unsigned total, at;
    uint32_t crc;
    int n = 0, i, j;
    if (!memcmp(tail, PE_SAVE_MAGIC, 4)) {
        total = pe_get16(tail + 6);
        if (tail[4] > PE_SAVE_FORMAT || (tail[4] == PE_SAVE_FORMAT && tail[5] != 0)) return -2;
        if (tail[4] != PE_SAVE_FORMAT || total < PE_SAVE_HEADER_SIZE || total > PE_SAVE_ROOM) return -1;
        pe_gather(tail, lin);
        crc = pe_get32(lin + 8);
        memset(lin + 8, 0, 4);
        i = pe_crc32(lin, total) != crc;
        pe_put32(lin + 8, crc);
        if (i) return -1;
        for (at = PE_SAVE_HEADER_SIZE; at < total;) {
            if (total - at < PE_SAVE_RECORD_HEADER_SIZE || n == PE_SAVE_MAX_ENTRIES) return -1;
            out[n].id = pe_get32(lin + at);
            out[n].version = pe_get16(lin + at + 4);
            out[n].size = pe_get16(lin + at + 6);
            at += PE_SAVE_RECORD_HEADER_SIZE;
            if (out[n].size > total - at) return -1;
            out[n].bytes = lin + at;
            at += out[n].size;
            for (i = 0; i < n; ++i)
                if (out[i].id == out[n].id) return -1;
            ++n;
        }
        /* The window changed since this container was written: a build that still writes the old
         * blocks saved here since. Its blocks are newer than our records of them. */
        if (pe_crc32(tail + PE_SAVE_WINDOW_AT, PE_SAVE_WINDOW_SIZE) != pe_get32(lin + 12))
            for (i = 0; i < PE_LEGACY_COUNT; ++i) {
                uint32_t id = pe_fnv(s_legacy[i].name);
                if (!pe_in_window(i) || memcmp(tail + s_legacy[i].at, s_legacy[i].magic, 4)) continue;
                for (j = 0; j < n && out[j].id != id; ++j) {}
                if (j == n) {
                    if (n == PE_SAVE_MAX_ENTRIES) continue;
                    ++n;
                }
                out[j].id = id;
                out[j].version = 0;
                out[j].size = (uint16_t)s_legacy[i].size;
                out[j].bytes = tail + s_legacy[i].at;
            }
        return n;
    }
    for (i = 0; i < PE_LEGACY_COUNT; ++i)
        if (!memcmp(tail + s_legacy[i].at, s_legacy[i].magic, 4)) {
            out[n].id = pe_fnv(s_legacy[i].name);
            out[n].version = 0;
            out[n].size = (uint16_t)s_legacy[i].size;
            out[n].bytes = tail + s_legacy[i].at;
            ++n;
        }
    return n;
}

static const PeSaveEntry *pe_carried(uint32_t id) {
    int i;
    for (i = 0; i < s_car_count; ++i)
        if (s_car[i].id == id) return &s_car[i];
    return NULL;
}

/* The window a save made now holds: each old block as its owner just wrote it into live (the
 * save's own tail) while that owner runs, else as carried from the loaded save. */
static void pe_window(uint8_t *win, const uint8_t *live) {
    int i, j;
    memcpy(win, s_tail + PE_SAVE_WINDOW_AT, PE_SAVE_WINDOW_SIZE);
    if (!live) return;
    for (i = 0; i < PE_LEGACY_COUNT; ++i) {
        uint32_t id = pe_fnv(s_legacy[i].name);
        if (!pe_in_window(i)) continue;
        for (j = 0; j < s_reg_count && s_reg[j].id != id; ++j) {}
        if (j < s_reg_count && pe_mod_active(s_reg[j].mod_id))
            memcpy(win + s_legacy[i].at - PE_SAVE_WINDOW_AT, live + s_legacy[i].at, s_legacy[i].size);
    }
}

/* The records a save made now holds, into out (a whole tail): running mods' state first, then
 * carried records (mods off, newer layouts, mods not in this build) while room is left, around
 * the window (live: the save's own tail, whose window the old blocks' owners wrote; NULL: the
 * carried window). Reads memory only, so the crash report can use it. Returns the record count. */
static int pe_pack(uint8_t *out, const uint8_t *live, int *dropped) {
    PeSaveEntry list[PE_SAVE_MAX_ENTRIES];
    uint8_t lin[PE_SAVE_ROOM], win[PE_SAVE_WINDOW_SIZE];
    unsigned total = PE_SAVE_HEADER_SIZE, at;
    int n = 0, i, j;
    *dropped = 0;
    pe_window(win, live);            /* first: live may be out itself */
    if (s_foreign) {                 /* a newer build's container: back as it was, nothing added */
        memcpy(out, s_tail, PE_SAVE_TAIL_SIZE);
        memcpy(out + PE_SAVE_WINDOW_AT, win, sizeof win);
        return 0;
    }
    for (i = 0; i < s_reg_count; ++i) {
        if (s_reg[i].kept || !pe_mod_active(s_reg[i].mod_id)) continue;
        list[n].id = s_reg[i].id;
        list[n].version = (uint16_t)s_reg[i].record->version;
        list[n].size = (uint16_t)s_reg[i].record->size;
        list[n].bytes = (const uint8_t *)s_reg[i].record->data;
        total += PE_SAVE_RECORD_HEADER_SIZE + list[n].size;
        ++n;
    }
    for (i = 0; i < s_car_count; ++i) {
        for (j = 0; j < n && list[j].id != s_car[i].id; ++j) {}
        if (j < n) continue;
        if (n == PE_SAVE_MAX_ENTRIES || total + PE_SAVE_RECORD_HEADER_SIZE + s_car[i].size > PE_SAVE_ROOM) {
            ++*dropped;
            continue;
        }
        list[n++] = s_car[i];
        total += PE_SAVE_RECORD_HEADER_SIZE + s_car[i].size;
    }
    for (i = 1; i < n; ++i)   /* by id: the same state always gives the same bytes */
        for (j = i; j > 0 && list[j - 1].id > list[j].id; --j) {
            PeSaveEntry t = list[j];
            list[j] = list[j - 1];
            list[j - 1] = t;
        }
    memset(lin, 0, sizeof lin);
    memcpy(lin, PE_SAVE_MAGIC, 4);
    lin[4] = PE_SAVE_FORMAT;
    pe_put16(lin + 6, total);
    pe_put32(lin + 12, pe_crc32(win, sizeof win));
    for (i = 0, at = PE_SAVE_HEADER_SIZE; i < n; ++i) {
        pe_put32(lin + at, list[i].id);
        pe_put16(lin + at + 4, list[i].version);
        pe_put16(lin + at + 6, list[i].size);
        memcpy(lin + at + PE_SAVE_RECORD_HEADER_SIZE, list[i].bytes, list[i].size);
        at += PE_SAVE_RECORD_HEADER_SIZE + list[i].size;
    }
    pe_put32(lin + 8, pe_crc32(lin, total));
    memset(out, 0, PE_SAVE_TAIL_SIZE);
    pe_scatter(lin, out);
    memcpy(out + PE_SAVE_WINDOW_AT, win, sizeof win);
    return n;
}

static void pe_carry(const uint8_t *tail) {
    int n;
    if (tail != s_tail) memcpy(s_tail, tail, sizeof s_tail);
    n = pe_parse(s_tail, s_lin, s_car);
    s_car_count = n < 0 ? 0 : n;
    s_foreign = n == -2;
    if (n == -1) pe_mod_log(NULL, "save: the mods' save data is damaged; mods start from their defaults");
    if (n == -2) {
        pe_mod_log(NULL, "save: the mods' save data is format %u, newer than this build's %u: kept as it is, "
                   "mods start from their defaults and their progress isn't saved", (unsigned)s_tail[4], PE_SAVE_FORMAT);
        pe_toast("This save's mod data is from a newer build: kept, but mods start fresh");
    }
}

static void pe_reset(int i) {
    const PeSaveRecord *r = s_reg[i].record;
    if (r->reset) r->reset(r->data);
    else memset(r->data, 0, r->size);
}

static int pe_call_check(int i) {
    const PeSaveRecord *r = s_reg[i].record;
    return r->check ? r->check(r->data) : 0;
}

static void pe_load_one(int i) {
    const PeSaveRecord *r = s_reg[i].record;
    const PeSaveEntry *e = pe_carried(s_reg[i].id);
    const char *mod = s_reg[i].mod_id;
    s_reg[i].kept = 0;
    if (!e) {
        pe_reset(i);
    } else if (e->version == r->version && e->size == r->size) {
        memcpy(r->data, e->bytes, r->size);
        if (pe_call_check(i)) {
            pe_mod_log(mod, "save: %s didn't pass its checks, reset", r->name);
            pe_reset(i);
        }
    } else if (e->version < r->version && r->upgrade) {
        if (r->upgrade(r->data, e->bytes, e->size, e->version) || pe_call_check(i)) {
            pe_mod_log(mod, "save: %s v%u couldn't be read, reset", r->name, (unsigned)e->version);
            pe_reset(i);
        } else {
            pe_mod_log(mod, "save: %s upgraded from v%u", r->name, (unsigned)e->version);
        }
    } else if (e->version > r->version) {
        pe_mod_log(mod, "save: %s is v%u, newer than this build's v%u: kept as is, mod starts from defaults",
                   r->name, (unsigned)e->version, r->version);
        pe_reset(i);
        s_reg[i].kept = 1;
    } else {
        pe_mod_log(mod, "save: %s v%u (%u bytes) doesn't match v%u (%u bytes), reset", r->name,
                   (unsigned)e->version, (unsigned)e->size, r->version, r->size);
        pe_reset(i);
    }
}

int pe_save_register(const PeSaveRecord *r) {
    const char *mod = pe_mod_current();
    uint32_t id;
    int i;
    if (!r || !r->name || !*r->name || !r->data || !r->size || r->version < 1 || r->version > 0xFFFF) {
        pe_mod_log(mod, "save: bad record, not registered");
        return -1;
    }
    id = pe_fnv(r->name);
    for (i = 0; i < s_reg_count; ++i)
        if (s_reg[i].id == id) {
            pe_mod_log(mod, "save: %s clashes with %s (%s), not registered", r->name, s_reg[i].record->name,
                       s_reg[i].mod_id ? s_reg[i].mod_id : "host");
            return -1;
        }
    if (s_reg_count == PE_SAVE_MAX_RECORDS ||
        s_reg_bytes + PE_SAVE_RECORD_HEADER_SIZE + r->size > PE_SAVE_ROOM) {
        pe_mod_log(mod, "save: no room for %s (%u bytes; %u of %u in use), not registered", r->name, r->size,
                   s_reg_bytes, (unsigned)PE_SAVE_ROOM);
        return -1;
    }
    s_reg[s_reg_count].record = r;
    s_reg[s_reg_count].mod_id = mod;
    s_reg[s_reg_count].id = id;
    s_reg[s_reg_count].kept = 0;
    ++s_reg_count;
    s_reg_bytes += PE_SAVE_RECORD_HEADER_SIZE + r->size;
    return 0;
}

void pe_save_on_read(const uint8_t *block) {
    int i;
    pe_carry(block + PE_SAVE_TAIL_OFFSET);
    for (i = 0; i < s_reg_count; ++i)
        if (pe_mod_active(s_reg[i].mod_id)) pe_load_one(i);
}

void pe_save_on_write(uint8_t *block) {
    int dropped;
    pe_pack(block + PE_SAVE_TAIL_OFFSET, block + PE_SAVE_TAIL_OFFSET, &dropped);
    if (dropped) {
        pe_mod_log(NULL, "save: no room for %d record(s) of mods not running; left out of this save", dropped);
        pe_toast("Mod data of a mod that is off didn't fit in this save");
    }
    if (!s_foreign) pe_carry(block + PE_SAVE_TAIL_OFFSET);   /* foreign: s_tail already holds it */
}

void pe_save_new_game(void) {
    int i;
    memset(s_tail, 0, sizeof s_tail);
    s_car_count = 0;
    s_foreign = 0;
    for (i = 0; i < s_reg_count; ++i) {
        s_reg[i].kept = 0;
        if (pe_mod_active(s_reg[i].mod_id)) pe_reset(i);
    }
}

/* ---- crash report ---- */
static uint8_t s_crash_tail[PE_SAVE_TAIL_SIZE], s_crash_lin[PE_SAVE_ROOM];
static PeSaveEntry s_crash_list[PE_SAVE_MAX_ENTRIES];
static int s_crash_count = -1, s_crash_dropped;

void pe_save_crash_tail(uint8_t *block) {
    pe_pack(s_crash_tail, NULL, &s_crash_dropped);
    memcpy(block + PE_SAVE_TAIL_OFFSET, s_crash_tail, sizeof s_crash_tail);
    s_crash_count = pe_parse(s_crash_tail, s_crash_lin, s_crash_list);
}

void pe_save_crash_describe(char *out, size_t size, size_t *used) {
    int i, j;
#define PE_OUT(...)                                                              \
    do {                                                                         \
        if (*used < size) {                                                      \
            int w_ = snprintf(out + *used, size - *used, __VA_ARGS__);           \
            if (w_ > 0) *used += (size_t)w_ < size - *used ? (size_t)w_ : size - *used - 1; \
        }                                                                        \
    } while (0)
    if (s_crash_count == -2) {
        PE_OUT("format %u from a newer build, kept as it is (mods run from their defaults).\r\n",
               (unsigned)s_crash_tail[4]);
        return;
    }
    if (s_crash_count < 0) {
        PE_OUT("Not taken.\r\n");
        return;
    }
    PE_OUT("%d record(s), %u of %u bytes%s\r\n", s_crash_count, (unsigned)pe_get16(s_crash_tail + 6),
           (unsigned)PE_SAVE_ROOM, s_crash_dropped ? "; some carried records didn't fit" : "");
    for (i = 0; i < s_crash_count; ++i) {
        const char *mod = NULL, *name = pe_name_of(s_crash_list[i].id, &mod), *from = "carried from the loaded save";
        for (j = 0; j < s_reg_count; ++j)
            if (s_reg[j].id == s_crash_list[i].id) {
                from = s_reg[j].kept ? "kept: newer than this build" :
                       pe_mod_active(s_reg[j].mod_id) ? "live" : "carried: mod off";
                break;
            }
        if (j == s_reg_count && name) from = "carried: mod not in this build";
        PE_OUT("  %08X %s%s%s%s v%u, %u bytes, %s\r\n", (unsigned)s_crash_list[i].id, name ? name : "(unknown)",
               mod ? " (" : "", mod ? mod : "", mod ? ")" : "", (unsigned)s_crash_list[i].version,
               (unsigned)s_crash_list[i].size, from);
    }
#undef PE_OUT
}

static int pe_on_command(void *ctx, void *user) {
    (void)user;
    if (ctx && ((const PeHookInvCommand *)ctx)->command == PE_INV_CMD_START_NEW_GAME) pe_save_new_game();
    return 0;
}

void pe_save_install(void) {
    pe_hook_add(PE_HOOK_INV_COMMAND, pe_on_command, NULL, -1000, "host");
}
