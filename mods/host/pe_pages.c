#include "pe_mod_menu.h"
#include "pe_pages.h"

void pe_pages_describe(void) {
    pe_menu_top_page(PE_MENU_PAGE, "Version " PE_ENHANCEMENTS_VERSION ". Quality of life and rebalance, by topic.", 10);
    pe_menu_begin(PE_MENU_PAGE);
    pe_menu_page(PE_PAGE_MOVEMENT, "Always run and how fast Aya moves.", 10);
    pe_menu_page(PE_PAGE_COMBAT, "Damage rules and gun mods.", 20);
    pe_menu_page(PE_PAGE_MOD_EFFECTS, "How gun and armor mods behave in battle.", 10);
    pe_menu_page(PE_PAGE_INVENTORY, "Key-item pocket, remembered bag sorting and inventory counters.", 40);
    pe_menu_page(PE_PAGE_KEYS, "Browse pocketed keys and story items.", 10);
    pe_menu_page(PE_PAGE_DEBUG, "Tuning and test tools. Changes the balance.", 90);
    pe_menu_begin(0);
}
