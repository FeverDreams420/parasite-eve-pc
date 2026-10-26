/*
 * Equipment editor: F1 > Debug > Equipment editor (the psxrecomp build's pe_item_editor.c).
 *
 * Edits the EQUIPPED gun and armor in place, with live rows (applied at once; the game is paused
 * while F1 is open):
 *   - the three "+N" bonuses (the numbers bonus points and Tools change; base stats are left
 *     alone), -999..999;
 *   - the gun's loaded ammo, capped at its bullet capacity (base + bonus, at most 999);
 *   - mods: pick a slot, then what is in it. A mod byte is effect (low 5 bits) and exclusivity group
 *     (high 3 bits); one mod per non-zero group on an item, a same-group mod is replaced, as the
 *     game's own transfer does.
 * Bag +1/+2/+4 armor mods change Aya's bag size (g_InvBonusSlotCount); lowering it is refused
 * while a slot it would cut off holds an item. With the economy loaded, its equipment sync reruns
 * the game's equipment rebuild, so edits reach the next battle.
 */
#include <stdio.h>

#include "pe_game.h"
#include "pe_item_table.h"
#include "pe_mod_host.h"
#include "pe_mod_menu.h"
#include "pe_pages.h"
#include "pe1/inventory_state.h"

#define PE_MAX_MODS 10
#define PE_BONUS_MAX 999

enum { PE_GUN = 0, PE_ARMOR = 1 };

/* The game's own effects, named by the host; filled at registration. */
static const char *s_gun_mods[PE_GUN_EFFECTS_GAME + 1];
static const char *s_armor_mods[PE_ARMOR_EFFECTS_GAME + 1];

static const char *pe_item_name(int id) {
    unsigned i;
    for (i = 0; i < PE_ITEM_COUNT; ++i) if (pe_item_ids[i] == id) return pe_item_names[i];
    return "UNKNOWN";
}

static ItemDataRecord *pe_record(int which) { return pe_game_loaded() ? pe_equipped(which) : NULL; }
static int pe_mod_count(const ItemDataRecord *rec) { return rec ? (rec->tailCount > PE_MAX_MODS ? PE_MAX_MODS : rec->tailCount) : 0; }

/* ---- the equipped items' names ---- */
static int s_gun_header, s_armor_header;   /* one-entry lists */
static unsigned pe_one(void) { return 1; }
static const char *pe_gun_name(unsigned i) { const ItemDataRecord *r = pe_record(PE_GUN); (void)i; return r ? pe_item_name(r->itemId) : "NONE EQUIPPED"; }
static const char *pe_armor_name(unsigned i) { const ItemDataRecord *r = pe_record(PE_ARMOR); (void)i; return r ? pe_item_name(r->itemId) : "NONE EQUIPPED"; }

/* ---- + bonuses and ammo ---- */
static void pe_clamp_ammo(ItemDataRecord *rec) {
    int cap = rec->baseStats[2] + rec->bonusStats[2];
    if (cap > PE_BONUS_MAX) cap = PE_BONUS_MAX;
    if (cap < 0) cap = 0;
    if (rec->ammo > cap) rec->ammo = (u16)cap;
}

static int pe_get_bonus(int which, int stat) { const ItemDataRecord *r = pe_record(which); return r ? r->bonusStats[stat] : 0; }

static int pe_set_bonus(int which, int stat, int value) {
    ItemDataRecord *rec = pe_record(which);
    if (!rec) return 0;
    rec->bonusStats[stat] = (s16)value;
    if (which == PE_GUN && stat == 2) pe_clamp_ammo(rec);
    pe_log("tweak", "Equipment/%s bonus %d = %d", which ? "armor" : "gun", stat, value);
    return 1;
}

static int g0(void) { return pe_get_bonus(PE_GUN, 0); }
static int g1(void) { return pe_get_bonus(PE_GUN, 1); }
static int g2(void) { return pe_get_bonus(PE_GUN, 2); }
static int a0(void) { return pe_get_bonus(PE_ARMOR, 0); }
static int a1(void) { return pe_get_bonus(PE_ARMOR, 1); }
static int a2(void) { return pe_get_bonus(PE_ARMOR, 2); }
static int sg0(int v) { return pe_set_bonus(PE_GUN, 0, v); }
static int sg1(int v) { return pe_set_bonus(PE_GUN, 1, v); }
static int sg2(int v) { return pe_set_bonus(PE_GUN, 2, v); }
static int sa0(int v) { return pe_set_bonus(PE_ARMOR, 0, v); }
static int sa1(int v) { return pe_set_bonus(PE_ARMOR, 1, v); }
static int sa2(int v) { return pe_set_bonus(PE_ARMOR, 2, v); }

static int pe_get_ammo(void) { const ItemDataRecord *r = pe_record(PE_GUN); return r ? r->ammo : 0; }
static int pe_set_ammo(int value) {
    ItemDataRecord *rec = pe_record(PE_GUN);
    if (!rec) return 0;
    rec->ammo = (u16)value;
    pe_clamp_ammo(rec);
    return 1;
}

/* ---- mods ---- */
static int s_slot[2];                       /* the chosen slot, per item */
static char s_slot_names[2][PE_MAX_MODS][12];

static unsigned pe_slot_count(int which) { const int n = pe_mod_count(pe_record(which)); return n > 0 ? (unsigned)n : 1u; }
static const char *pe_slot_name(int which, unsigned i) {
    const int n = pe_mod_count(pe_record(which));
    if (n == 0) return "NO SLOTS";
    if (i >= PE_MAX_MODS) return "";
    snprintf(s_slot_names[which][i], sizeof s_slot_names[0][0], "%u OF %d", i + 1, n);
    return s_slot_names[which][i];
}
static unsigned gsc(void) { return pe_slot_count(PE_GUN); }
static unsigned asc(void) { return pe_slot_count(PE_ARMOR); }
static const char *gsn(unsigned i) { return pe_slot_name(PE_GUN, i); }
static const char *asn(unsigned i) { return pe_slot_name(PE_ARMOR, i); }

static int pe_get_mod(int which) {
    const ItemDataRecord *rec = pe_record(which);
    int effect;
    if (!rec || s_slot[which] >= pe_mod_count(rec)) return 0;
    effect = rec->tailData[s_slot[which]] & 0x1F;
    return effect <= (which == PE_GUN ? PE_GUN_EFFECTS_GAME : PE_ARMOR_EFFECTS_GAME) ? effect : 0;
}

/* The bag slots +1/+2/+4 armor mods give (effects 8/9/10). */
static int pe_bag_bonus(const u8 *mods, int n) {
    int i;
    for (i = 0; i < n; ++i) {
        const int e = mods[i] & 0x1F;
        if (e >= 8 && e <= 10) return 1 << (e - 8);
    }
    return 0;
}

static int pe_set_mod(int which, int effect) {
    ItemDataRecord *rec = pe_record(which);
    const int n = pe_mod_count(rec);
    u8 next[PE_MAX_MODS];
    int group, i;
    if (!rec || n == 0) return 0;
    if (s_slot[which] >= n) s_slot[which] = 0;
    for (i = 0; i < n; ++i) next[i] = rec->tailData[i];
    group = effect ? pe_mod_effect_group(which == PE_ARMOR, effect) : 0;
    if (group)                                     /* one mod per group */
        for (i = 0; i < n; ++i)
            if (i != s_slot[which] && (next[i] & 0x1F) && (next[i] >> 5) == group) next[i] = 0;
    next[s_slot[which]] = effect ? (u8)((group << 5) | effect) : 0;
    if (which == PE_ARMOR) {
        const int before = pe_bag_bonus(rec->tailData, n), after = pe_bag_bonus(next, n);
        if (after < before) {                       /* a smaller bag must not strand an item */
            int s, from = g_AyaSaveState.inventory_slot_count + after, to = g_AyaSaveState.inventory_slot_count + before;
            if (from > 50) from = 50;
            if (to > 50) to = 50;
            for (s = from; s < to; ++s) if (g_AyaSaveState.inventory_items[s]) return 0;
        }
        if (after != before) g_InvBonusSlotCount = after;
    }
    for (i = 0; i < n; ++i) rec->tailData[i] = next[i];
    pe_log("tweak", "Equipment/%s mod slot %d = %d", which ? "armor" : "gun", s_slot[which] + 1, effect);
    return 1;
}
static int ggm(void) { return pe_get_mod(PE_GUN); }
static int agm(void) { return pe_get_mod(PE_ARMOR); }
static int sgm(int v) { return pe_set_mod(PE_GUN, v); }
static int sam(int v) { return pe_set_mod(PE_ARMOR, v); }

void pe_equip_editor_register(const char *mod_id) {
    const char *page = PE_PAGE_DEBUG_EQUIP;
    int e;
    (void)mod_id;
    for (e = 0; e <= PE_GUN_EFFECTS_GAME; ++e) s_gun_mods[e] = pe_mod_effect_name(0, e);
    for (e = 0; e <= PE_ARMOR_EFFECTS_GAME; ++e) s_armor_mods[e] = pe_mod_effect_name(1, e);
    pe_menu_register_dynamic_choice(page, "Gun", "The equipped weapon.", &s_gun_header, 1, pe_one, pe_gun_name);
    pe_menu_register_live_int(page, "Gun: attack +", "Attack bonus. Applies now.", -PE_BONUS_MAX, PE_BONUS_MAX, 1, g0, sg0);
    pe_menu_register_live_int(page, "Gun: range +", "Range bonus. Applies now.", -PE_BONUS_MAX, PE_BONUS_MAX, 1, g1, sg1);
    pe_menu_register_live_int(page, "Gun: bullets +", "Bullet capacity bonus. Applies now.", -PE_BONUS_MAX, PE_BONUS_MAX, 1, g2, sg2);
    pe_menu_register_live_int(page, "Gun: ammo loaded", "Capped at bullet capacity.", 0, PE_BONUS_MAX, 1, pe_get_ammo, pe_set_ammo);
    pe_menu_register_dynamic_choice(page, "Gun: mod slot", "Pick a slot, then set its mod below.", &s_slot[PE_GUN], PE_MAX_MODS, gsc, gsn);
    pe_menu_register_live_choice(page, "Gun: mod", "One per kind; replaces a match.", s_gun_mods, PE_GUN_EFFECTS_GAME + 1, ggm, sgm);
    pe_menu_register_dynamic_choice(page, "Armor", "The equipped armor.", &s_armor_header, 1, pe_one, pe_armor_name);
    pe_menu_register_live_int(page, "Armor: defense +", "Defense bonus. Applies now.", -PE_BONUS_MAX, PE_BONUS_MAX, 1, a0, sa0);
    pe_menu_register_live_int(page, "Armor: PE defense +", "PE defense bonus. Applies now.", -PE_BONUS_MAX, PE_BONUS_MAX, 1, a1, sa1);
    pe_menu_register_live_int(page, "Armor: crit evasion +", "Critical evasion bonus. Applies now.", -PE_BONUS_MAX, PE_BONUS_MAX, 1, a2, sa2);
    pe_menu_register_dynamic_choice(page, "Armor: mod slot", "Pick a slot, then set its mod below.", &s_slot[PE_ARMOR], PE_MAX_MODS, asc, asn);
    pe_menu_register_live_choice(page, "Armor: mod", "Bag mods can't shrink a full bag.", s_armor_mods, PE_ARMOR_EFFECTS_GAME + 1, agm, sam);
}
