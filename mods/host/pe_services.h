#ifndef PE_SERVICES_H
#define PE_SERVICES_H

/* The services mods offer each other (pe_service_publish / pe_service_get), in one neutral
 * place so no mod includes another mod's headers. A service is a versioned table of functions;
 * a mod that doesn't find one carries on without it.
 *
 *   "junk"      v1  pe_economy  the Junk counter
 *   "catalog"   v1  pe_economy  armor effects unlocked in the Mod Catalog
 *   "keys"      v1  pe_qol      the key item pocket
 *   "wardrobe"  v1  pe_collection  outfits bought with Junk
 */

#include <stddef.h>
#include <stdint.h>

/* Economy armor effects, in the game's unused armor effect numbers 17-20. */
#define PE_FX_BARGAIN    17 /* prices -15% */
#define PE_FX_SCAVENGER  18 /* battle Junk +50% */
#define PE_FX_LUCKY_FIND 19 /* pity builds twice as fast */
#define PE_FX_SALVAGER   20 /* dismantle Junk +25% */

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

typedef struct PeCatalogApi {
    int (*armor_unlocked)(int effect);
    int (*unlock_armor)(int effect);   /* 1 if it was new */
} PeCatalogApi;

typedef struct PeKeysApi {
    int (*take)(uint16_t item);        /* pocket a key; 1 means handled */
} PeKeysApi;

/* state: 0 for sale, 1 owned, 2 worn. Wayne's Workshop shows a Wardrobe row while published. */
typedef struct PeWardrobeApi {
    int (*count)(void);
    const char *(*name)(int k);
    unsigned (*price)(int k);
    int (*state)(int k);
    int (*pick)(int k, char *msg, size_t size);   /* buy or wear; msg says what happened */
} PeWardrobeApi;

#endif
