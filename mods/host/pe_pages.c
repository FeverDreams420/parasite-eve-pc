#include "pe_mod_menu.h"
#include "pe_pages.h"

void pe_pages_describe(void) {
    pe_menu_top_page(PE_MENU_PAGE, "Version " PE_ENHANCEMENTS_VERSION ". Quality of life, by topic.", 10);
    pe_menu_begin(PE_MENU_PAGE);
    pe_menu_page(PE_PAGE_MOVEMENT, "Always run and how fast Aya moves.", 10);
    pe_menu_page(PE_PAGE_COMBAT, "Random battles and how often they happen.", 20);
    pe_menu_begin(0);
}
