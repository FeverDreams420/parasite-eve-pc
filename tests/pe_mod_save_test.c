/* Mods' per-save data kept by the host (mods/host/pe_save.h): saved and loaded for every mod the
 * same way; a mod that is off, or a mod not in this build, keeps its record; a newer layout survives
 * an older build; saves from before still load; New Game clears; a damaged tail resets; and the
 * crash copy holds what a save would, without calling any mod code. The game's own save path is
 * modelled as it is: a cleared 8 KB buffer, then PE_HOOK_SAVE_WRITE. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "pe_mod_host.h"
#include "pe_save.h"

static PeHookFn s_new_game;
int pe_hook_add(int id, PeHookFn fn, void *user, int priority, const char *mod_id) {
    (void)user; (void)priority; (void)mod_id;
    assert(id == PE_HOOK_INV_COMMAND);
    s_new_game = fn;
    return 0;
}
static const char *s_current = "junk_mod";
static int s_junk_on = 1, s_keys_on = 1;
const char *pe_mod_current(void) { return s_current; }
int pe_mod_active(const char *mod_id) {
    return !mod_id || (!strcmp(mod_id, "junk_mod") ? s_junk_on : !strcmp(mod_id, "keys_mod") ? s_keys_on : 1);
}
void pe_mod_log(const char *mod_id, const char *fmt, ...) { (void)mod_id; (void)fmt; }
void pe_toast(const char *text) { (void)text; }

/* Two stand-in mods. "junk" counts its callbacks, so the crash copy can be shown to call none. */
static struct { unsigned junk, pity; } s_junk;
static unsigned short s_keys[4];
static int s_calls;
static int junk_check(void *d) { (void)d; ++s_calls; return s_junk.junk > 99999; }
static void junk_reset(void *d) { (void)d; ++s_calls; s_junk.junk = 0; s_junk.pity = 7; }
static int junk_upgrade(void *d, const unsigned char *old, unsigned size, unsigned version) {
    (void)d; ++s_calls;
    if (version != 0 || size < 12 || memcmp(old, "PELT", 4)) return 1;
    memcpy(&s_junk.junk, old + 8, 4);
    s_junk.pity = 0;
    return 0;
}
static const PeSaveRecord s_junk_rec = { "pe_economy.loot", 2, &s_junk, sizeof s_junk, junk_check, junk_upgrade, junk_reset };
static const PeSaveRecord s_keys_rec = { "test.keys", 1, s_keys, sizeof s_keys, NULL, NULL, NULL };

static unsigned char s_card[0x2000];
static void game_save(void) { memset(s_card, 0, sizeof s_card); pe_save_on_write(s_card); }   /* Save_BuildCardFile */
static void game_load(const unsigned char *card) { pe_save_on_read(card); }

static unsigned fnv(const char *name) {
    unsigned h = 0x811C9DC5u;
    for (; *name; ++name) h = (h ^ (unsigned char)*name) * 0x01000193u;
    return h;
}
/* Byte-level writer for tails the build under test didn't make (other builds, damage). */
static unsigned crc32(const unsigned char *p, unsigned n) {
    unsigned c = 0xFFFFFFFFu, i, b;
    for (i = 0; i < n; ++i) for (c ^= p[i], b = 0; b < 8; ++b) c = c & 1 ? (c >> 1) ^ 0xEDB88320u : c >> 1;
    return ~c;
}
static unsigned s_at;
static void tail_begin(unsigned char *card) { memset(card, 0, 0x2000); memcpy(card + 0x1C90, "PEMC\1\0", 6); s_at = 12; }
static void tail_add(unsigned char *card, unsigned id, unsigned version, const void *data, unsigned size) {
    unsigned char *t = card + 0x1C90 + s_at;
    memcpy(t, &id, 4); t[4] = (unsigned char)version; t[5] = (unsigned char)(version >> 8);
    t[6] = (unsigned char)size; t[7] = (unsigned char)(size >> 8); memcpy(t + 8, data, size);
    s_at += 8 + size;
}
static void tail_end(unsigned char *card) {
    unsigned char *t = card + 0x1C90;
    unsigned c;
    t[6] = (unsigned char)s_at; t[7] = (unsigned char)(s_at >> 8); memset(t + 8, 0, 4);
    c = crc32(t, s_at); memcpy(t + 8, &c, 4);
}
/* A record in a saved tail, NULL if absent. */
static const unsigned char *find(const unsigned char *card, const char *name, unsigned *version, unsigned *size) {
    const unsigned char *t = card + 0x1C90;
    unsigned at, total = t[6] | t[7] << 8, id;
    if (memcmp(t, "PEMC", 4)) return NULL;
    for (at = 12; at < total; at += 8 + (t[at + 6] | t[at + 7] << 8)) {
        memcpy(&id, t + at, 4);
        if (id == fnv(name)) {
            if (version) *version = t[at + 4] | t[at + 5] << 8;
            if (size) *size = t[at + 6] | t[at + 7] << 8;
            return t + at + 8;
        }
    }
    return NULL;
}

int main(void) {
    unsigned char other[0x2000], crash[0x2000];
    PeHookInvCommand new_game = {1117, 0, 0};
    unsigned v, n;
    const unsigned char *p;
    char text[512];
    size_t used = 0;

    pe_save_install();
    assert(pe_save_register(&s_junk_rec) == 0);
    s_current = "keys_mod";
    assert(pe_save_register(&s_keys_rec) == 0);
    assert(pe_save_register(&s_keys_rec) == -1);                      /* same name twice */
    {
        static unsigned char big[0x370];
        static const PeSaveRecord too_big = { "test.big", 1, big, sizeof big, NULL, NULL, NULL };
        assert(pe_save_register(&too_big) == -1);                      /* no room: refused at start-up */
    }

    /* Save and load: every mod's state, the same way. */
    s_junk.junk = 120; s_junk.pity = 3; s_keys[0] = 0xC8;
    game_save();
    assert(!memcmp(s_card + 0x1C90, "PEMC", 4) && s_card[0x1C8F] == 0);
    assert((p = find(s_card, "pe_economy.loot", &v, &n)) && v == 2 && n == sizeof s_junk);
    assert((p = find(s_card, "test.keys", &v, &n)) && v == 1 && p[0] == 0xC8);
    memset(&s_junk, 0, sizeof s_junk); memset(s_keys, 0, sizeof s_keys);
    game_load(s_card);
    assert(s_junk.junk == 120 && s_junk.pity == 3 && s_keys[0] == 0xC8);

    /* Saving twice gives the same bytes. */
    memcpy(other, s_card, sizeof other);
    game_save();
    assert(!memcmp(other, s_card, sizeof other));

    /* A mod that's off keeps its record through saves; turned back on it reads it. */
    s_keys_on = 0;
    s_keys[0] = 0;                                                     /* not running: its memory doesn't count */
    s_junk.junk = 130;
    game_save();
    assert((p = find(s_card, "test.keys", NULL, NULL)) && p[0] == 0xC8);
    assert((p = find(s_card, "pe_economy.loot", NULL, NULL)) && p[0] == 130);
    game_load(s_card);
    s_keys_on = 1;
    game_load(s_card);
    assert(s_keys[0] == 0xC8);

    /* A record nobody here knows (a mod not in this build) and a newer layout both survive. */
    {
        unsigned char extra[3] = {1, 2, 3}, newer[12] = {9, 9, 9};
        tail_begin(other);
        tail_add(other, fnv("someone.else"), 4, extra, sizeof extra);
        tail_add(other, fnv("pe_economy.loot"), 3, newer, sizeof newer);
        tail_end(other);
        s_junk.junk = 55;
        game_load(other);
        assert(s_junk.junk == 0 && s_junk.pity == 7);                  /* can't read v3: defaults */
        s_junk.junk = 77;                                              /* played on */
        game_save();
        assert((p = find(s_card, "someone.else", &v, &n)) && v == 4 && n == 3 && p[2] == 3);
        assert((p = find(s_card, "pe_economy.loot", &v, &n)) && v == 3 && n == 12 && p[0] == 9);   /* not overwritten */
    }

    /* A save from before: the old fixed blocks go to their owners' upgrade(). */
    memset(other, 0, sizeof other);
    memcpy(other + 0x1C90, "PELT", 4);
    other[0x1C90 + 8] = 42;
    game_load(other);
    assert(s_junk.junk == 42 && s_junk.pity == 0);
    game_save();
    assert((p = find(s_card, "pe_economy.loot", &v, NULL)) && v == 2 && p[0] == 42);   /* now in the new layout */

    /* A record that fails its owner's checks, and a damaged tail, both reset. */
    {
        struct { unsigned junk, pity; } bad = {100000, 1};
        tail_begin(other);
        tail_add(other, fnv("pe_economy.loot"), 2, &bad, sizeof bad);
        tail_end(other);
        game_load(other);
        assert(s_junk.junk == 0 && s_junk.pity == 7);
        game_save();
        memcpy(other, s_card, sizeof other);
        other[0x1C90 + 20] ^= 1;
        s_junk.junk = 5;
        game_load(other);
        assert(s_junk.junk == 0 && s_keys[0] == 0);
    }

    /* New Game: everything resets and nothing carries over. */
    s_junk.junk = 300; s_keys[0] = 0xC9;
    game_save();
    game_load(s_card);
    s_keys_on = 0;
    s_new_game(&new_game, NULL);
    assert(s_junk.junk == 0);
    game_save();
    assert(!find(s_card, "test.keys", NULL, NULL));
    s_keys_on = 1;

    /* The crash copy: what a save made now holds, with no mod code called and nothing changed. */
    s_junk.junk = 900; s_keys[0] = 0xCA;
    s_calls = 0;
    memset(crash, 0, sizeof crash);
    pe_save_crash_tail(crash);
    assert(s_calls == 0);
    game_save();
    assert(!memcmp(crash + 0x1C90, s_card + 0x1C90, 0x370));
    pe_save_crash_describe(text, sizeof text, &used);
    assert(strstr(text, "2 record(s)") && strstr(text, "pe_economy.loot (junk_mod) v2, 8 bytes, live"));

    puts("Save record checks passed: one way for every mod, mods off and unknown mods keep theirs, newer "
         "layouts survive, old saves load, damage resets, New Game clears, crash copy runs no mod code.");
    return 0;
}
