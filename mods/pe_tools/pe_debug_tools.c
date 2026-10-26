/*
 * Items and abilities: F1 > Debug > Items and abilities (the psxrecomp build's pe_debug_menu.c).
 *
 *   - Give item: pick a group, then an item (pe_item_table.h, the game's own item ids), and the
 *     game's own Inv_AddItem puts it in Aya's bag -- gear gets a free equipment record, ammo goes
 *     to its pool -- so other mods see it as any pickup (key items go to the key pocket).
 *   - Abilities: bits of g_AyaSaveState.parasite_spell_flags, bit = id - 0xEC; one or all.
 *   - Add 1000 bonus points, up to the game's own 99999.
 */
#include <stdio.h>

#include "pe_game.h"
#include "pe_item_table.h"
#include "pe_mod_host.h"
#include "pe_mod_menu.h"
#include "pe_pages.h"
#include "pe1/inventory.h"

void Inv_SelectActiveList(int list);

#define PE_BP_GRANT 1000u
#define PE_BP_CAP   99999u
#define PE_TECH_BASE 0xEC

static int s_type, s_item, s_tech;
static char s_result[64];

static int pe_ready(void) {
    if (pe_game_loaded()) return 1;
    snprintf(s_result, sizeof s_result, "No game loaded");
    pe_toast(s_result);
    return 0;
}

static void pe_say(const char *text) {
    snprintf(s_result, sizeof s_result, "%s", text);
    pe_toast(text);
}

/* ---- bonus points ---- */
static void pe_add_bonus_points(void) {
    u32 points;
    if (!pe_ready()) return;
    points = g_AyaSaveState.bonus_points + PE_BP_GRANT;
    g_AyaSaveState.bonus_points = points > PE_BP_CAP ? PE_BP_CAP : points;
    pe_log("tweak", "Debug/Add bonus points total=%u", g_AyaSaveState.bonus_points);
    pe_say("Added 1000 bonus points");
}

/* ---- give item ---- */
static unsigned pe_item_count(void) { return s_type >= 0 && (unsigned)s_type < PE_TYPE_COUNT ? pe_type_count[s_type] : 0u; }
static const char *pe_item_name(unsigned i) { return i < pe_item_count() ? pe_item_names[pe_type_first[s_type] + i] : ""; }

static void pe_give_item(void) {
    char msg[64];
    int id;
    if (!pe_ready()) return;
    if (pe_in_battle()) { pe_say("Give items outside battle"); return; }
    if (s_item < 0 || (unsigned)s_item >= pe_item_count()) return;
    id = pe_item_ids[pe_type_first[s_type] + s_item];
    Inv_SelectActiveList(0);                          /* Aya's bag */
    if (Inv_AddItem(id) != 0) { pe_say("Bag full"); return; }
    pe_log("tweak", "Debug/Give item=0x%02X", (unsigned)id);
    snprintf(msg, sizeof msg, "Added %s", pe_item_names[pe_type_first[s_type] + s_item]);
    pe_say(msg);
}

/* ---- abilities ---- */
static void pe_unlock_ability(void) {
    if (!pe_ready() || s_tech < 0 || (unsigned)s_tech >= PE_TECH_COUNT) return;
    g_AyaSaveState.parasite_spell_flags |= 1u << (pe_tech_ids[s_tech] - PE_TECH_BASE);
    pe_log("tweak", "Debug/Unlock ability=0x%02X", (unsigned)pe_tech_ids[s_tech]);
    pe_say("Ability unlocked");
}

static void pe_unlock_all(void) {
    unsigned i;
    if (!pe_ready()) return;
    for (i = 0; i < PE_TECH_COUNT; ++i) g_AyaSaveState.parasite_spell_flags |= 1u << (pe_tech_ids[i] - PE_TECH_BASE);
    pe_log("tweak", "Debug/Unlock all abilities");
    pe_say("All abilities unlocked");
}

static void pe_result(char *out, unsigned size) { snprintf(out, size, "%s", s_result[0] ? s_result : "-"); }

void pe_debug_tools_register(const char *mod_id) {
    const char *page = PE_PAGE_DEBUG_ITEMS;
    (void)mod_id;
    pe_menu_register_choice(page, "Item type", "Left / right picks the item group.", &s_type, pe_type_names, PE_TYPE_COUNT);
    pe_menu_register_dynamic_choice(page, "Item", "Left / right picks, then Give item.", &s_item, PE_TYPE_MAX_ITEMS, pe_item_count, pe_item_name);
    pe_menu_register_action(page, "Give item", "Adds the item above to Aya's bag.", pe_give_item);
    pe_menu_register_choice(page, "Ability", "Left / right picks a Parasite Energy.", &s_tech, pe_tech_names, PE_TECH_COUNT);
    pe_menu_register_action(page, "Unlock ability", "Unlocks the ability chosen above.", pe_unlock_ability);
    pe_menu_register_action(page, "Unlock all abilities", "Unlocks every Parasite Energy.", pe_unlock_all);
    pe_menu_register_action(page, "Add 1000 bonus points", "Spend them on the status screen (max 99999).", pe_add_bonus_points);
    pe_menu_register_info(page, "Result", "What the last action did.", pe_result);
}
