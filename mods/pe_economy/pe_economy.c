/* PE Enhancements: economy, ported from the psxrecomp build. F1 > PE Enhancements > Economy.
 * One file per feature; each registers its own rows, hooks and services. */
#include "pe_mod_host.h"

#define MOD_ID "pe_economy"

void pe_loot_register(const char *mod_id);    /* pe_loot.c: Junk, battle rewards, rare drops */
void pe_catalog_register(const char *mod_id); /* pe_loot_catalog.c: Mod Catalog, Dismantle, Rebuild */
void pe_catalog_menu_register(const char *mod_id); /* pe_catalog_menu.c: the Catalog in Aya's menu */

static int economy_init(void) {
    pe_loot_register(MOD_ID);
    pe_catalog_register(MOD_ID);
    pe_catalog_menu_register(MOD_ID);
    return 0;
}

const PeMod pe_mod_pe_economy = {MOD_ID, "PE Enhancements: economy", "0.1", NULL, economy_init, NULL};
