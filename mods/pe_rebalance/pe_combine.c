/* Recomp medicine recipes (pe_native_menu.c), using the shared F1 panel.
 * Bag-only and outside battle. The optional Junk service gates this economy
 * feature; there is no link-time dependency on pe_economy. Confirmation is
 * press-twice, with an unchanged bag required between presses. */
#include <stdio.h>
#include <string.h>
#include "pe_game.h"
#include "pe_mod_host.h"
#include "pe_mod_menu.h"
#include "pe_pages.h"
#include "pe_panacea.h"
#include "../pe_economy/pe_economy.h"
#include "pe1/item_ids.h"

static const struct { int first, second, result; const char *name; } s_recipes[] = {
    {ITEM_MEDICINE_1, ITEM_MEDICINE_1, ITEM_MEDICINE_2, "2 Medicine 1 into Medicine 2"},
    {ITEM_MEDICINE_2, ITEM_MEDICINE_2, ITEM_MEDICINE_3, "2 Medicine 2 into Medicine 3"},
    {ITEM_MEDICINE_3, ITEM_MEDICINE_3, ITEM_MEDICINE_4, "2 Medicine 3 into Medicine 4"},
    {ITEM_MEDICINE_4, ITEM_FULL_CURE, ITEM_ITEM25, "Medicine 4 + Full Cure into Panacea"},
};
static int s_recipe, s_armed = -1;
static s16 s_bag[50];
static char s_result[96];

static unsigned pe_recipe_count(void) { return g_pe_panacea ? 4 : 3; }
static const char *pe_recipe_name(unsigned i) { return i < pe_recipe_count() ? s_recipes[i].name : "None"; }

static int pe_pair(int *a, int *b) {
    int limit = Inv_GetAyaSlotLimit();
    *a = *b = -1;
    if (s_recipe < 0 || (unsigned)s_recipe >= pe_recipe_count()) return 0;
    if (limit > 50) limit = 50;
    for (int i = 0; i < limit; ++i) {
        int item = g_AyaInventoryItems[i];
        if (*a < 0 && item == s_recipes[s_recipe].first) *a = i;
        else if (*b < 0 && item == s_recipes[s_recipe].second) *b = i;
    }
    return *a >= 0 && *b >= 0;
}

static void pe_result(const char *text) {
    snprintf(s_result, sizeof s_result, "%s", text);
    pe_toast(text);
}

static void pe_combine(void) {
    const PeJunkApi *junk = pe_service_get("junk", 1);
    int a, b;
    if (!pe_game_loaded() || pe_in_battle() || (g_GameStateFlags & 0x2004u)) {
        s_armed = -1; pe_result("Combine in the field, outside battle and game menus"); return;
    }
    if (!junk || !junk->enabled()) { s_armed = -1; pe_result("Turn on Junk economy to combine items"); return; }
    if (!pe_pair(&a, &b)) { s_armed = -1; pe_result("The ingredients must be in Aya's bag"); return; }
    if (s_armed != s_recipe || memcmp(s_bag, g_AyaInventoryItems, sizeof s_bag)) {
        s_armed = s_recipe;
        memcpy(s_bag, g_AyaInventoryItems, sizeof s_bag);
        pe_result("Press Combine again to consume the two ingredients");
        return;
    }
    s_armed = -1;
    g_AyaInventoryItems[a] = s_recipes[s_recipe].result;
    g_AyaInventoryItems[b] = 0;
    pe_log("combine", "from=0x%02X into=0x%02X", s_recipes[s_recipe].first, s_recipes[s_recipe].result);
    pe_result(s_recipe == 3 ? "Combined into Panacea" : s_recipe == 2 ? "Combined into Medicine 4" :
              s_recipe == 1 ? "Combined into Medicine 3" : "Combined into Medicine 2");
}

static void pe_ingredients(char *out, unsigned size) {
    int a, b;
    snprintf(out, size, "%s", pe_game_loaded() && pe_pair(&a, &b) ? "Ready" : "Missing ingredients");
}
static void pe_last_result(char *out, unsigned size) { snprintf(out, size, "%s", s_result[0] ? s_result : "Choose a recipe"); }
static void pe_disarm_tick(void) { s_armed = -1; }

void pe_combine_register(const char *mod_id) {
    (void)mod_id;
    pe_menu_register_dynamic_choice(PE_PAGE_COMBINE, "Recipe", "Combines two carried medicines, outside battle.",
        &s_recipe, 4, pe_recipe_count, pe_recipe_name);
    pe_menu_register_info(PE_PAGE_COMBINE, "Ingredients", "Both ingredients must be in Aya's bag.", pe_ingredients);
    pe_menu_register_info(PE_PAGE_COMBINE, "Result", "The last combine result or confirmation.", pe_last_result);
    pe_menu_register_action(PE_PAGE_COMBINE, "Combine", "Press twice: consumes two items and gives the result.", pe_combine);
    pe_menu_register_tick(pe_disarm_tick);
}
