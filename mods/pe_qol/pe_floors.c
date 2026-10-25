/*
 * Debug floors, ported from the psxrecomp build (pe_inventory_floor.c, pe_bonus_points_floor.c).
 * Both are cheats and off by default (the recomp turned them off after they skewed a balance
 * playthrough). F1 > PE Enhancements > Debug.
 *
 *   Bag size floor      Aya's bag holds at least this many slots. The game sets the size from a
 *                       per-level table on New Game, level-ups and loads (10 at level 1 to 46 at
 *                       99), so the floor is reapplied every frame; past the floor the natural
 *                       size is left alone. When Aya's bag is the active list, its cached length
 *                       (g_InvSlotLimit) is raised with it, or new slots stay invisible until the
 *                       game re-selects the bag.
 *   Bonus points floor  Aya always has at least this many unspent bonus points. Bonus points are
 *                       saved state, so spending is never fought: the value is only raised back
 *                       up when it drops below the floor (the game clamps them at 99999).
 *
 * A game counts as loaded by max HP (pe_game_loaded), not by these values: 0 bonus points is the
 * genuine New Game value.
 */
#include "pe_game.h"
#include "pe_mod_host.h"
#include "pe_mod_menu.h"
#include "pe_pages.h"
#include "pe1/inventory_state.h"

#define PE_BAG_MAX         50
#define PE_BONUS_POINTS_MAX 99999

static int s_bag_floor;     /* 0 = off */
static int s_bp_floor;      /* 0 = off */

static void pe_floors_tick(void) {
    int limit;
    if (!pe_game_loaded()) return;
    if (s_bag_floor && g_AyaSaveState.inventory_slot_count && g_AyaSaveState.inventory_slot_count < s_bag_floor) {
        g_AyaSaveState.inventory_slot_count = (u8)s_bag_floor;
        if (g_InvItemPtr == g_AyaSaveState.inventory_items) {
            limit = g_AyaSaveState.inventory_slot_count + g_InvBonusSlotCount;
            if (limit > PE_BAG_MAX) limit = PE_BAG_MAX;
            if (g_InvSlotLimit < limit) g_InvSlotLimit = limit;
        }
    }
    if (s_bp_floor && g_AyaSaveState.bonus_points < (u32)s_bp_floor) g_AyaSaveState.bonus_points = (u32)s_bp_floor;
}

void pe_floors_register(const char *mod_id) {
    (void)mod_id;
    pe_menu_register_int(PE_PAGE_DEBUG, "Bag size floor", "Aya's bag holds at least this many slots (0 = off).",
                         &s_bag_floor, 0, PE_BAG_MAX, 5);
    pe_menu_register_int(PE_PAGE_DEBUG, "Bonus points floor", "Unspent bonus points never drop below this (0 = off).",
                         &s_bp_floor, 0, PE_BONUS_POINTS_MAX, 100);
    pe_menu_register_tick(pe_floors_tick);
}
