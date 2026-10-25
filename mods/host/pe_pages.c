#include "pe_mod_menu.h"
#include "pe_pages.h"

void pe_pages_describe(void) {
    pe_menu_begin(PE_MENU_PAGE);
    pe_menu_page(PE_PAGE_MOVEMENT, "Always run and how fast Aya moves.", 10);
    pe_menu_page(PE_PAGE_COMBAT, "Damage rules, gun mods and Parasite Energy.", 20);
    pe_menu_page(PE_PAGE_MOD_EFFECTS, "How gun and armor mods behave in battle.", 10);
    pe_menu_page(PE_PAGE_PE_BALANCE, "Heals and Parasite Energy costs.", 20);
    pe_menu_page(PE_PAGE_ECONOMY, "Junk, difficulty, and Wayne's Workshop services.", 30);
    pe_menu_page(PE_PAGE_CATALOG, "Install unlocked effects on your equipped gun or armor.", 10);
    pe_menu_page(PE_PAGE_DISMANTLE, "Break down spare gear for Junk and new effects.", 20);
    pe_menu_page(PE_PAGE_REBUILD, "Buy dismantled gear back.", 30);
    pe_menu_page(PE_PAGE_DEBUG, "Tuning and test tools. Changes the balance.", 90);
    pe_menu_page(PE_PAGE_DEBUG_LOOT, "Junk amounts, rare drop chances, debug Junk.", 10);
    pe_menu_page(PE_PAGE_DEBUG_CATALOG, "Prices, unlock-all and test gear.", 20);
    pe_menu_begin(0);
}
