/* PE Enhancements: economy (Junk, loot; Mod Catalog, Requisition and Wayne's Workshop as they
 * are ported), from the psxrecomp build. Rows under F1 > PE Enhancements > Workshop. */
#include "pe_mod_host.h"

void pe_loot_register(void);

static int economy_init(void) {
    pe_loot_register();
    return 0;
}

const PeMod pe_mod_pe_economy = {"pe_economy", "PE Enhancements: economy", "0.1", NULL, economy_init, NULL};
