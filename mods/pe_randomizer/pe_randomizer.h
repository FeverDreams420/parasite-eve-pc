#ifndef PE_RANDOMIZER_H
#define PE_RANDOMIZER_H

/* Item randomizer: the spot table (pe_rand_spots.h, generated from the disc by
 * tools/spottable.py) and the seed generator (pe_rand_gen.c). The generator has no game
 * code in it, so the tests run it on its own. */

#include <stdint.h>

/* Raise PE_RAND_LOGIC_VERSION whenever the same seed would give a different layout (the generator,
 * its rules or PE_RAND_TABLE_VERSION). A save keeps the versions it was made with; a build that
 * doesn't know them leaves that game's items vanilla rather than moving them under the player. */
#define PE_RAND_LOGIC_VERSION 1
#define PE_RAND_TABLE_VERSION 1

enum { PE_RAND_EASY, PE_RAND_MEDIUM, PE_RAND_HARD, PE_RAND_DIFFICULTIES };

/* How a spot hands its item out. Only chests take a key from elsewhere or any item: on a full
 * bag Aya can swap and the chest keeps the item. Pickups and gifts take items that can't be lost
 * there (gift-safe ones), or their own vanilla item. */
enum { PE_RAND_CHEST, PE_RAND_PICKUP, PE_RAND_GIFT };
enum { PE_RAND_KEY = 1 };   /* spot flag: its vanilla item is a key item that unlocks something */

/* Key logic, from real play (tools/keylogic.py). Bit k is the k-th key spot of the table
 * (table order) and means both: key k is held, and key k's own spot has been reached (picking a key up
 * can also move the story on, and that spot still does it when it gives something else).
 *   needs   bits that reach the spot: an over-estimate (a run reached it with them).
 *           Unknown: no key goes there; a key spot with unknown needs counts as needing every key of
 *           the days before it, and keeps its own key (and every key of its day stays put).
 *   before  keys whose lock was first used after a run had reached this spot: only those may be put
 *           here, so a key never sits in a spot the player could already have walked past for good. */
#define PE_RAND_NEEDS_UNKNOWN 0xFFFFFFFFu
#define PE_RAND_MAX_KEYS 32

typedef struct PeRandSpot {
    uint16_t vanilla;
    uint8_t kind, day, area, flags;   /* day: story day 1-6, 0 unknown; area: save label id */
    uint32_t needs, before;
} PeRandSpot;

/* One script word to rewrite: room map, offset of the step in the room data, which operand holds
 * the item, and the step's 16 bytes as scanned (header, mode word, operands 0 and 1). */
typedef struct PeRandSite {
    uint16_t map, spot;
    uint32_t at;
    uint8_t operand;
    uint32_t expect[4];
} PeRandSite;

/* A GiveItem step of a spot: the mod logs which spot gives (rand_give rows), for keylogic.py. */
typedef struct PeRandGive {
    uint16_t map, spot;
    uint32_t at;
} PeRandGive;

typedef struct PeRandResult {
    unsigned attempts;    /* tries used, 1 or more */
    unsigned keys_moved;
    int fallback;         /* 1: keys stayed vanilla after the tries ran out; 2: everything vanilla */
    int keys_locked;      /* keys can't move: no story day has logic data for all its key spots */
} PeRandResult;

/* Fills out[0..count) with the item for each spot. Same inputs, same layout on every machine.
 * gift_safe(item): the item can go to a pickup or gift spot. Always returns a usable layout:
 * when no try passes the checks, keys stay vanilla, and then everything does. */
void pe_rand_generate(const PeRandSpot *spots, unsigned count, uint32_t seed, int difficulty,
                      int (*gift_safe)(unsigned item), uint16_t *out, PeRandResult *result);

/* The checks every layout passes (also used by the tests). 0 = fine, else a reason. */
const char *pe_rand_check(const PeRandSpot *spots, unsigned count, int difficulty,
                          int (*gift_safe)(unsigned item), const uint16_t *layout);

#endif
