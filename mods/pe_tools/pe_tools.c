/* PE Enhancements: tools, ported from the psxrecomp build. Development and testing aids, all under
 * F1 > PE Enhancements > Debug; they change the game directly and are off the player's path.
 * One file per tool:
 *   pe_stats_editor.c   Aya's level and stat levels (pe_stats_menu.c)
 *   pe_equip_editor.c   the equipped gun's and armor's bonuses, ammo and mods (pe_item_editor.c)
 *   pe_debug_tools.c    give item, unlock Parasite Energy, bonus points (pe_debug_menu.c)
 *   pe_play_logger.c    the detailed play log, pe_play_log.csv (pe_loot_logger.c) */
#include "pe_mod_host.h"

#define MOD_ID "pe_tools"

void pe_stats_editor_register(const char *mod_id);
void pe_equip_editor_register(const char *mod_id);
void pe_debug_tools_register(const char *mod_id);
void pe_play_logger_register(const char *mod_id);
void pe_play_logger_shutdown(void);

static int tools_init(void) {
    pe_stats_editor_register(MOD_ID);
    pe_equip_editor_register(MOD_ID);
    pe_debug_tools_register(MOD_ID);
    pe_play_logger_register(MOD_ID);
    return 0;
}

const PeMod pe_mod_pe_tools = {MOD_ID, "PE Enhancements: tools", "0.1", NULL, tools_init, pe_play_logger_shutdown};
