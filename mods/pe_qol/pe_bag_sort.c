/* Remembered bag sorting from recomp pe_bag_sort.c. The native game does the
 * sorting and keeps equipped slots attached to their items. Only pointer-free
 * setting IDs are saved, using the original SORT v1 layout and table IDs. */
#include "pe_inventory.h"
#include "pe_mod_host.h"
#include "pe_mod_menu.h"
#include "pe_pages.h"
#include "pe1/inventory_sort.h"
#include "pe1/inventory_state.h"

enum { PE_SORT_TAIL = 0x1C90 + 0x260 };
static struct {
    uint32_t mode, rank_ptr, type_ptr, weapon_stat, armor_stat, row_hidden;
} s_sort;
_Static_assert(sizeof s_sort == 24, "recomp SORT layout");
static s16 s_last[50];
static int s_seen, s_sorting;
static const char *const s_modes[] = { "Manual", "Auto" };

/* The PS1 values are disk identifiers only, never dereferenced. */
static u8 *pe_rank(uint32_t id) {
    switch (id) {
    case 0x800923F8u: return g_InvSortRankTableA;
    case 0x80092410u: return g_InvSortRankTableB;
    case 0x80092428u: return g_InvSortRankTableC;
    case 0x80092440u: return g_InvSortRankTableD;
    default: return NULL;
    }
}
static u8 *pe_type(uint32_t id) {
    if (id == 0x80092458u) return g_InvTypeRankTableA;
    if (id == 0x80092468u) return g_InvTypeRankTableB;
    return NULL;
}
static void pe_capture_settings(void) {
    static const uint32_t ranks[] = { 0x800923F8u, 0x80092410u, 0x80092428u, 0x80092440u };
    s_sort.rank_ptr = ranks[0]; s_sort.type_ptr = 0x80092458u;
    for (unsigned i = 0; i < 4; ++i)
        if (pe_rank(ranks[i]) == g_InvSortRankTable) s_sort.rank_ptr = ranks[i];
    if (g_InvSortTypeRankTable == g_InvTypeRankTableB) s_sort.type_ptr = 0x80092468u;
    s_sort.weapon_stat = g_InvWeaponSortStatIndex >= 0 && g_InvWeaponSortStatIndex <= 2 ? g_InvWeaponSortStatIndex : 0;
    s_sort.armor_stat = g_InvAmmoSortStatIndex >= 0 && g_InvAmmoSortStatIndex <= 2 ? g_InvAmmoSortStatIndex : 0;
}
static void pe_snapshot(void) { memcpy(s_last, g_AyaInventoryItems, sizeof s_last); s_seen = 1; }
static int pe_sort_chosen(void *ctx, void *user) {
    (void)ctx; (void)user;
    if (!s_sorting && pe_game_loaded()) {
        pe_capture_settings(); s_sort.mode = 1;
        pe_log("bag_sort", "auto rank=0x%08X type=0x%08X weapon=%u armor=%u",
            s_sort.rank_ptr, s_sort.type_ptr, s_sort.weapon_stat, s_sort.armor_stat);
        pe_snapshot();
    }
    return 0;
}
static int pe_same_items(void) {
    for (int i = 0; i < 50; ++i) {
        int before = 0, after = 0;
        for (int j = 0; j < 50; ++j) {
            before += s_last[j] == s_last[i];
            after += g_AyaInventoryItems[j] == s_last[i];
        }
        if (before != after) return 0;
    }
    return 1;
}
static int pe_sort_mode(void) { return s_sort.mode == 1; }
int pe_bag_sort_auto(void) { return pe_sort_mode(); }
static int pe_set_sort_mode(int mode) {
    if (!pe_game_loaded() || mode < 0 || mode > 1) return 0;
    if (!pe_rank(s_sort.rank_ptr) || !pe_type(s_sort.type_ptr)) pe_capture_settings();
    s_sort.mode = mode; s_seen = 0;
    return 1;
}
static void pe_sort_tick(void) {
    if (!pe_game_loaded()) { s_seen = 0; return; }
    int changed = s_seen && memcmp(s_last, g_AyaInventoryItems, sizeof s_last);
    if (g_GameStateFlags & 4u) {
        if (s_sort.mode && changed && pe_same_items()) {
            s_sort.mode = 0; pe_log("bag_sort", "manual (items moved)");
        }
        pe_snapshot(); return;
    }
    if (!pe_free_field()) return;
    if (!s_sort.mode || (s_seen && !changed)) { pe_snapshot(); return; }
    int slot = g_AyaSaveState.equipped_weapon_slot;
    /* The stock sorter dereferences this slot before sorting. */
    if (slot < 0 || slot >= pe_inventory_limit() || !g_AyaInventoryItems[slot]) return;
    if (!pe_rank(s_sort.rank_ptr) || !pe_type(s_sort.type_ptr)) pe_capture_settings();
    g_InvSortRankTable = pe_rank(s_sort.rank_ptr);
    g_InvSortTypeRankTable = pe_type(s_sort.type_ptr);
    g_InvWeaponSortStatIndex = s_sort.weapon_stat;
    g_InvAmmoSortStatIndex = s_sort.armor_stat;
    s_sorting = 1;
    Inv_SortInventoryByMode(-1, 0); /* retain all four remembered settings */
    s_sorting = 0;
    pe_snapshot();
}
static int pe_sort_save(void *ctx, void *user) {
    (void)user;
    s_sort.row_hidden = 0; /* Menu hiding is transient; never persist it in SORT. */
    pe_inventory_save(ctx, PE_SORT_TAIL, &s_sort, sizeof s_sort, 0x54524F53u, 0x736F7274u);
    return 0;
}
static int pe_sort_load(void *ctx, void *user) {
    (void)user;
    memset(&s_sort, 0, sizeof s_sort); s_seen = s_sorting = 0;
    if (!pe_inventory_load(ctx, PE_SORT_TAIL, &s_sort, sizeof s_sort, 0x54524F53u, 0x736F7274u)) return 0;
    if (s_sort.mode > 1 || s_sort.weapon_stat > 2 || s_sort.armor_stat > 2 ||
        !pe_rank(s_sort.rank_ptr) || !pe_type(s_sort.type_ptr)) memset(&s_sort, 0, sizeof s_sort);
    s_sort.row_hidden = 0;
    return 0;
}
static int pe_sort_command(void *ctx, void *user) {
    const PeHookInvCommand *c = ctx;
    (void)user;
    if (c->command == 1117) { memset(&s_sort, 0, sizeof s_sort); s_seen = s_sorting = 0; }
    return 0;
}
void pe_bag_sort_register(const char *mod_id) {
    pe_menu_register_live_choice(PE_PAGE_INVENTORY, "Bag sort", "Auto remembers the game's Sort. Moving items manually switches it off.",
        s_modes, 2, pe_sort_mode, pe_set_sort_mode);
    pe_menu_register_tick(pe_sort_tick);
    pe_hook_add(PE_HOOK_INVENTORY_SORTED, pe_sort_chosen, NULL, PE_PRIORITY_NORMAL, mod_id);
    pe_hook_add(PE_HOOK_INV_COMMAND, pe_sort_command, NULL, PE_PRIORITY_NORMAL, mod_id);
    pe_hook_add(PE_HOOK_SAVE_WRITE, pe_sort_save, NULL, PE_PRIORITY_NORMAL, mod_id);
    pe_hook_add(PE_HOOK_SAVE_READ, pe_sort_load, NULL, PE_PRIORITY_NORMAL, mod_id);
}
