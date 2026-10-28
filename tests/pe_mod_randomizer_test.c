/* Randomizer: the spot table is sane, and every seed on every difficulty gives a layout that passes
 * the checks, the same one each time. Golden hashes catch a change that moves items for the same
 * seed: if it's meant, raise PE_RAND_LOGIC_VERSION and update them. */
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "pe_randomizer.h"
#include "pe_rand_spots.h"

static int test_gift_safe(unsigned id) { return id >= 1 && id <= 0x16; }   /* medicine, ammo, Junk */

static uint32_t layout_hash(const uint16_t *layout, unsigned count) {
    uint32_t h = 2166136261u;
    for (unsigned i = 0; i < count; ++i) { h = (h ^ (layout[i] & 0xFF)) * 16777619u; h = (h ^ (layout[i] >> 8)) * 16777619u; }
    return h;
}

static void table_is_sane(void) {
    unsigned keys = 0, sites_of[PE_RAND_SPOTS] = {0}, vanilla_of[PE_RAND_SPOTS] = {0};
    for (unsigned i = 0; i < PE_RAND_SPOTS; ++i) {
        const PeRandSpot *s = &g_pe_rand_spots[i];
        assert(s->vanilla && s->vanilla <= 0xFF && s->kind <= PE_RAND_GIFT && s->day <= 6);
        if (s->flags & PE_RAND_KEY) {
            ++keys;
            for (unsigned j = 0; j < i; ++j) assert(g_pe_rand_spots[j].vanilla != s->vanilla);   /* one spot per key */
        }
    }
    assert(keys == 15 && keys <= PE_RAND_MAX_KEYS);
    for (unsigned i = 0; i < PE_RAND_SITES; ++i) {
        const PeRandSite *s = &g_pe_rand_sites[i];
        assert(s->spot < PE_RAND_SPOTS && s->operand <= 1 && s->at % 4 == 0);
        /* the operand holds an item: the spot's own, or the other one of a roll */
        assert(s->expect[2 + s->operand] >= 1 && s->expect[2 + s->operand] <= 0xFF);
        vanilla_of[s->spot] += s->expect[2 + s->operand] == g_pe_rand_spots[s->spot].vanilla;
        assert((s->expect[0] & 0x1FFF) == 10 || (s->expect[0] & 0x1FFF) == 167 || (s->expect[0] & 0x1FFF) == 232);
        if (i) {
            const PeRandSite *p = &g_pe_rand_sites[i - 1];
            assert(p->map < s->map || (p->map == s->map && p->at < s->at));   /* sorted, no duplicates */
        }
        ++sites_of[s->spot];
    }
    for (unsigned i = 0; i < PE_RAND_SPOTS; ++i) assert(sites_of[i] && vanilla_of[i]);
}

static void every_seed_passes(void) {
    static uint16_t a[PE_RAND_SPOTS], b[PE_RAND_SPOTS];
    for (int d = 0; d < PE_RAND_DIFFICULTIES; ++d) {
        unsigned changed = 0;
        for (uint32_t seed = 0; seed < 2000; ++seed) {
            PeRandResult ra, rb;
            pe_rand_generate(g_pe_rand_spots, PE_RAND_SPOTS, seed, d, test_gift_safe, a, &ra);
            pe_rand_generate(g_pe_rand_spots, PE_RAND_SPOTS, seed, d, test_gift_safe, b, &rb);
            const char *why = pe_rand_check(g_pe_rand_spots, PE_RAND_SPOTS, d, test_gift_safe, a);
            if (why) { fprintf(stderr, "difficulty %d seed %u: %s\n", d, (unsigned)seed, why); assert(0); }
            assert(!memcmp(a, b, sizeof a) && !memcmp(&ra, &rb, sizeof ra));
            assert(!ra.fallback && ra.attempts >= 1);
            /* the table has no logic data yet: keys stay on every difficulty */
            assert(ra.keys_locked && !ra.keys_moved);
            for (unsigned i = 0; i < PE_RAND_SPOTS; ++i) changed += a[i] != g_pe_rand_spots[i].vanilla;
        }
        assert(changed > 2000 * 20);   /* it does shuffle */
    }
}

static void golden_layouts(void) {
    static const uint32_t golden[PE_RAND_DIFFICULTIES][4] = {
        {0xC82C39F6, 0x8AE72E56, 0xD7C1CDB6, 0xC7760AA6},
        {0xE25D9ABE, 0xD2C3AE9E, 0x5DF5804E, 0x2458569E},
        {0x8E84A746, 0x834D19AE, 0x5A9A45E6, 0x297319D6},
    };
    static uint16_t out[PE_RAND_SPOTS];
    int print = !golden[0][0];
    for (int d = 0; d < PE_RAND_DIFFICULTIES; ++d)
        for (uint32_t s = 0; s < 4; ++s) {
            PeRandResult r;
            pe_rand_generate(g_pe_rand_spots, PE_RAND_SPOTS, s * 111111u, d, test_gift_safe, out, &r);
            uint32_t h = layout_hash(out, PE_RAND_SPOTS);
            if (print) printf("golden difficulty %d seed %06u: 0x%08X\n", d, (unsigned)(s * 111111u), (unsigned)h);
            else assert(h == golden[d][s]);
        }
}

#define ALL 0xFFFFFFFFu
#define UNKNOWN PE_RAND_NEEDS_UNKNOWN

/* A small table with logic data: keys move, and every layout can be finished. Key spots 0-2 hold
 * keys A, B, C (bits 0-2); chests need nothing, A, or A and B. */
static void keys_move_with_logic_data(void) {
    static const PeRandSpot t[] = {
        {0xC8, PE_RAND_GIFT, 3, 6, PE_RAND_KEY, 0, 0},
        {0xC9, PE_RAND_GIFT, 3, 6, PE_RAND_KEY, 1, 0},
        {0xCA, PE_RAND_GIFT, 3, 6, PE_RAND_KEY, 3, 0},
        {0x01, PE_RAND_CHEST, 3, 6, 0, 0, ALL}, {0x02, PE_RAND_CHEST, 3, 6, 0, 0, ALL}, {0x03, PE_RAND_CHEST, 3, 6, 0, 1, ALL},
        {0x04, PE_RAND_CHEST, 3, 6, 0, 1, ALL}, {0x05, PE_RAND_CHEST, 3, 6, 0, 3, ALL}, {0x06, PE_RAND_CHEST, 3, 6, 0, 3, ALL},
        {0x40, PE_RAND_CHEST, 3, 7, 0, 0, ALL},       /* another area: Hard only */
        {0x07, PE_RAND_PICKUP, 3, 6, 0, 0, ALL},
        {0x08, PE_RAND_CHEST, 3, 6, 0, 0, 0},         /* reached only after every lock: no key */
        {0x09, PE_RAND_CHEST, 3, 6, 0, UNKNOWN, ALL}, /* no run reached it: no key */
        {0x0A, PE_RAND_CHEST, 2, 4, 0, 0, ALL},       /* another day: no key */
    };
    enum { N = sizeof t / sizeof t[0] };
    uint16_t out[N];
    unsigned moved = 0, other_area = 0;
    for (int d = PE_RAND_MEDIUM; d <= PE_RAND_HARD; ++d)
        for (uint32_t seed = 0; seed < 3000; ++seed) {
            PeRandResult r;
            pe_rand_generate(t, N, seed, d, test_gift_safe, out, &r);
            assert(!r.keys_locked && !r.fallback && !pe_rand_check(t, N, d, test_gift_safe, out));
            moved += r.keys_moved;
            for (unsigned i = 0; i < N; ++i) {
                if (out[i] >= 0xC8) assert(t[i].kind == PE_RAND_CHEST || t[i].vanilla == out[i]);
                if (out[i] == 0x40) assert(t[i].kind == PE_RAND_CHEST);   /* not gift-safe */
            }
            assert(out[11] < 0xC8 && out[12] < 0xC8 && out[13] < 0xC8);
            if (d == PE_RAND_MEDIUM) assert(out[9] < 0xC8);
            else other_area += out[9] >= 0xC8;
        }
    assert(moved > 1000 && other_area > 0);
    /* the checks catch a key that can't be reached: A behind A */
    uint16_t bad[N];
    for (unsigned i = 0; i < N; ++i) bad[i] = t[i].vanilla;
    bad[0] = 0x03; bad[5] = 0xC8;
    assert(pe_rand_check(t, N, PE_RAND_MEDIUM, test_gift_safe, bad));
    /* and one that moves on Easy, or a lost item */
    bad[0] = 0x01; bad[3] = 0xC8; bad[5] = 0x03;
    assert(!pe_rand_check(t, N, PE_RAND_MEDIUM, test_gift_safe, bad));
    assert(pe_rand_check(t, N, PE_RAND_EASY, test_gift_safe, bad));
    for (unsigned i = 0; i < N; ++i) bad[i] = t[i].vanilla;
    bad[4] = 0x01;
    assert(pe_rand_check(t, N, PE_RAND_HARD, test_gift_safe, bad));
    /* a key in a chest without its before bit, unknown needs or another day */
    for (unsigned i = 0; i < N; ++i) bad[i] = t[i].vanilla;
    bad[0] = 0x08; bad[11] = 0xC8;
    assert(pe_rand_check(t, N, PE_RAND_HARD, test_gift_safe, bad));
    bad[0] = 0x09; bad[11] = 0x08; bad[12] = 0xC8;
    assert(pe_rand_check(t, N, PE_RAND_HARD, test_gift_safe, bad));
}

/* A needs bit is met by holding the key AND reaching the key's own spot (its pickup may move the
 * story on). Key A's spot needs B; chest X was first reached after A's pickup, so it needs A. With A
 * moved into chest C and B into X, holding A isn't enough: X needs A's spot, which needs B. */
static void needs_mean_the_key_spot_too(void) {
    static const PeRandSpot t[] = {
        {0xC8, PE_RAND_GIFT, 3, 6, PE_RAND_KEY, 2, 0},   /* A, needs B */
        {0xC9, PE_RAND_GIFT, 3, 6, PE_RAND_KEY, 0, 0},   /* B */
        {0x01, PE_RAND_CHEST, 3, 6, 0, 0, ALL},          /* C */
        {0x02, PE_RAND_CHEST, 3, 6, 0, 1, ALL},          /* X, needs A */
    };
    enum { N = sizeof t / sizeof t[0] };
    uint16_t layout[N] = {0x01, 0x02, 0xC8, 0xC9};
    uint16_t out[N];
    assert(pe_rand_check(t, N, PE_RAND_HARD, test_gift_safe, layout));
    for (uint32_t seed = 0; seed < 2000; ++seed) {
        PeRandResult r;
        pe_rand_generate(t, N, seed, PE_RAND_HARD, test_gift_safe, out, &r);
        assert(!pe_rand_check(t, N, PE_RAND_HARD, test_gift_safe, out));
        assert(!(out[2] == 0xC8 && out[3] == 0xC9));
    }
}

/* Day 3 has data for all its key spots, day 4 doesn't: day 4's key stays, day 3's move. The key spot
 * without data counts as needing day 3's keys. */
static void partial_logic_data(void) {
    static const PeRandSpot t[] = {
        {0xC8, PE_RAND_GIFT, 3, 6, PE_RAND_KEY, 0, 0},
        {0xC9, PE_RAND_GIFT, 3, 6, PE_RAND_KEY, 1, 0},
        {0xCA, PE_RAND_GIFT, 4, 7, PE_RAND_KEY, UNKNOWN, 0},
        {0x01, PE_RAND_CHEST, 3, 6, 0, 0, ALL}, {0x02, PE_RAND_CHEST, 3, 6, 0, 0, ALL},
        {0x03, PE_RAND_CHEST, 4, 7, 0, 3, ALL}, {0x04, PE_RAND_CHEST, 4, 7, 0, 3, ALL},
    };
    enum { N = sizeof t / sizeof t[0] };
    uint16_t out[N];
    unsigned moved = 0;
    for (uint32_t seed = 0; seed < 2000; ++seed) {
        PeRandResult r;
        pe_rand_generate(t, N, seed, PE_RAND_MEDIUM, test_gift_safe, out, &r);
        assert(!r.keys_locked && !r.fallback && !pe_rand_check(t, N, PE_RAND_MEDIUM, test_gift_safe, out));
        assert(out[2] == 0xCA);
        moved += r.keys_moved;
    }
    assert(moved > 500);
}

int main(void) {
    table_is_sane();
    every_seed_passes();
    keys_move_with_logic_data();
    needs_mean_the_key_spot_too();
    partial_logic_data();
    golden_layouts();
    puts("pe_randomizer: ok");
    return 0;
}
