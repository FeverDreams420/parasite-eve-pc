/* Recomp God Mode, using Aya's native live Combatant rather than the save
 * mirror. Debug only, off by default; the host persists the toggle. */
#include "pe_game.h"
#include "pe_mod_host.h"
#include "pe_mod_menu.h"
#include "pe_pages.h"

static int s_enabled;

static void pe_god_mode_tick(void) {
    Combatant *core;
    if (!s_enabled || !pe_game_loaded() || !pe_in_battle()) return;
    core = pe_aya_core();
    if (core && core->maxHP) core->curHP = core->maxHP;
}

void pe_god_mode_register(const char *mod_id) {
    (void)mod_id;
    pe_menu_register_toggle(PE_PAGE_DEBUG, "God mode",
        "Keeps Aya's live HP full during battle.", &s_enabled, pe_god_mode_tick);
}
