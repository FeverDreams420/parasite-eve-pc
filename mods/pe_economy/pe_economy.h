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

/* The Mod Catalog (pe_loot_catalog.c). which: 0 = the equipped gun, 1 = the equipped armor.
 * Records are PS1 addresses of equipment records (pe_ps1.h). */
enum {
    PE_CAT_OK, PE_CAT_NO_GAME, PE_CAT_OFF, PE_CAT_NO_ITEM, PE_CAT_NO_SLOTS, PE_CAT_LOCKED,
    PE_CAT_ALREADY, PE_CAT_BAG, PE_CAT_JUNK, PE_CAT_TOOL, PE_CAT_EMPTY,
};
uint32_t pe_cat_record(int which);
int pe_cat_slots(int which);
int pe_cat_slot_effect(int which, int slot);
int pe_cat_board_count(int which);
int pe_cat_board_effect(int which, int i);
int pe_cat_unlocked(int which, int e);
uint32_t pe_cat_price(int which, int e);
int pe_cat_needs_tool(int which, int e);
int pe_cat_is_bag_mod(int which, int e);
int pe_cat_install(int which, int slot, int e, int dry_run);
int pe_cat_remove(int which, int slot, int dry_run);
int pe_cat_replaces(int which, int slot, int e, int *out, int max);
int pe_catalog_armor_unlocked(int effect);
int pe_catalog_unlock_armor(int effect);
uint32_t pe_catalog_dismantle_preview(uint32_t rec);
uint32_t pe_catalog_dismantle_reward(uint32_t rec, int *unlocks_out);
int pe_rebuild_count(void);
uint8_t pe_rebuild_item(int i);
uint32_t pe_rebuild_price_index(int i);
unsigned pe_rebuild_bought_slots(int i);
int pe_rebuild_index(int i, char *msg, size_t msg_size);

#endif
