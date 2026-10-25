#ifndef PE_ECONOMY_H
#define PE_ECONOMY_H

/* The economy's services, for its own files and for other mods
 * (pe_service_get("junk", 1) and so on). */

#include <stddef.h>
#include <stdint.h>

/* Economy armor mods in the game's unused armor effect numbers 17-20. */
#define PE_FX_BARGAIN    17 /* prices -15% */
#define PE_FX_SCAVENGER  18 /* battle Junk +50% */
#define PE_FX_LUCKY_FIND 19 /* pity builds twice as fast */
#define PE_FX_SALVAGER   20 /* dismantle Junk +25% */

/* "junk" v1: the Junk counter (pe_loot.c). */
typedef struct PeJunkApi {
    int (*enabled)(void);              /* "Junk economy" switch */
    uint32_t (*junk)(void);
    int (*spend)(uint32_t n);          /* 0 (and nothing spent) if short */
    void (*give)(uint32_t n);
    int (*armor_has)(int effect);      /* on the equipped armor */
    uint32_t (*price)(uint32_t base);  /* after preset and Bargain; callers cap after */
    int (*wayne_waiting)(void);        /* he holds 300+ but can't build before Day 4 */
    int (*handin_blocked)(void);       /* a hand-in needs one free bag slot */
} PeJunkApi;

/* "catalog" v1: armor effects unlocked in the Mod Catalog (pe_loot_catalog.c). */
typedef struct PeCatalogApi {
    int (*armor_unlocked)(int effect);
    int (*unlock_armor)(int effect);   /* 1 if it was new */
} PeCatalogApi;

/* "keys" v1: the key item pocket. */
typedef struct PeKeysApi {
    int (*take)(uint16_t item);        /* found-list key with the bag full: into the pocket */
} PeKeysApi;

/* Direct calls inside the economy mod. */
int pe_loot_enabled(void);
uint32_t pe_loot_junk(void);
int pe_loot_spend_junk(uint32_t n);
void pe_loot_give_junk(uint32_t n);
int pe_loot_armor_has(int effect);
uint32_t pe_loot_price(uint32_t base);
int pe_loot_wayne_waiting(void);
int pe_loot_handin_blocked(void);

#endif
