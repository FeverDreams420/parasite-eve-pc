/*
 * Bestiary: 75 creatures, story and Chrysler (pe_collection_data.h, generated from the bridge's
 * Bestiary draft). Per creature:
 *   basics   (name, HP, defense, where)  first kill, or any purchase;
 *   weakness (weak / strong elements)    casting Scan on it, 5 kills (bosses: defeat), or Junk;
 *   loot     (drop and steal odds)       15 kills, or Junk (bosses: Junk only).
 * Kills: every battle frame (PE_HOOK_BATTLE_UPDATE) the enemy slots are watched; an enemy whose HP
 * goes from above 0 to 0 or less is a kill of the creature found by (room, identity). Scan: power 7
 * on an enemy (PE_HOOK_SPELL). F1 pages: area and progress filters, a creature row browsed with
 * left / right, its details, one purchase.
 */
#include <stdio.h>
#include <string.h>

#include "pe_collection.h"
#include "pe_collection_data.h"
#include "pe_game.h"
#include "pe_mod_host.h"
#include "pe_mod_menu.h"
#include "pe_pages.h"

#define PE_SCAN        7
#define PE_ENEMY_SLOTS 7
#define PE_MAX_SECTIONS 12

/* ---- creatures ---- */
static int pe_creature_of(int map, unsigned identity) {
    int lo = 0, hi = PE_CREATURE_KEYS - 1;      /* keys are sorted by (map, id) */
    while (lo <= hi) {
        const int mid = (lo + hi) / 2;
        const PeCreatureKey *k = &g_pe_creature_keys[mid];
        if (k->map == map && k->id == identity) return k->creature;
        if (k->map < map || (k->map == map && k->id < identity)) lo = mid + 1;
        else hi = mid - 1;
    }
    return -1;
}

static uint8_t *pe_state(int c) { return &pe_col_profile()->creature[c]; }
static unsigned pe_kills(int c) { return *pe_state(c) >> 3; }

static void pe_unlock(int c, uint8_t bits, const char *why) {
    const uint8_t old = *pe_state(c), now = (uint8_t)(old | bits | C_MET), gained = (uint8_t)(now & ~old);
    char msg[96];
    if (now == old) return;
    *pe_state(c) = now;
    pe_col_changed();
    snprintf(msg, sizeof msg, "Bestiary: %s%s%s%s", g_pe_creatures[c].name, gained & C_WEAK ? " - weaknesses" : "",
             gained & C_LOOT ? " - loot" : "", gained == C_MET ? " - new entry" : "");
    pe_toast(msg);
    pe_log("bestiary", "creature=%s bits=0x%02X why=%s", g_pe_creatures[c].name, now & 7u, why);
}

static void pe_kill(int c) {
    unsigned k = pe_kills(c);
    uint8_t bits = C_MET;
    if (k < 31u) {
        ++k;
        *pe_state(c) = (uint8_t)((*pe_state(c) & 7u) | (k << 3));
        pe_col_changed();
    }
    if (g_pe_creatures[c].boss) bits |= C_WEAK;            /* defeated: its weaknesses are known */
    else {
        if (k >= 5u) bits |= C_WEAK;
        if (k >= 15u) bits |= C_LOOT;
    }
    pe_unlock(c, bits, "kill");
}

/* Kills: watch the enemy slots during battle. */
static struct { int tracking; uint8_t entity_id; uint16_t identity; int32_t hp; } s_slot[PE_ENEMY_SLOTS];

static int pe_on_battle(void *ctx, void *user) {
    const int map = pe_room_number();
    int s;
    (void)ctx;
    (void)user;
    if (!pe_game_loaded()) return 0;
    for (s = 0; s < PE_ENEMY_SLOTS; ++s) {
        const EnemyCombatant *core = &g_BattleEnemySlots[s].combatant;
        const uint8_t eid = core->lowAndEntityId.bytes.entityId;
        const int32_t hp = core->hpAlive;
        const uint16_t id = (uint16_t)core->field04.fieldId04;
        if (!g_BattleEnemySlots[s].active) { s_slot[s].tracking = 0; continue; }
        if (!s_slot[s].tracking || s_slot[s].entity_id != eid) {
            s_slot[s].tracking = 1;
            s_slot[s].entity_id = eid;
            s_slot[s].hp = hp;
            s_slot[s].identity = id;
            continue;
        }
        if (s_slot[s].hp > 0 && hp <= 0 && map > 0) {
            const int c = pe_creature_of(map, s_slot[s].identity ? s_slot[s].identity : id);
            if (c >= 0) pe_kill(c);
        }
        s_slot[s].hp = hp;
        if (id) s_slot[s].identity = id;
    }
    return 0;
}

static int pe_on_spell(void *ctx, void *user) {
    const PeHookSpell *spell = (const PeHookSpell *)ctx;
    const EnemyCombatant *core;
    int c;
    (void)user;
    if (spell->spell != PE_SCAN || !spell->target || spell->target == pe_player() || !pe_game_loaded()) return 0;
    if (!(core = (const EnemyCombatant *)spell->target->core)) return 0;
    if ((c = pe_creature_of(pe_room_number(), (uint16_t)core->field04.fieldId04)) >= 0) pe_unlock(c, C_WEAK, "scan");
    return 0;
}

static void pe_field_tick(void) { if (!pe_in_battle()) memset(s_slot, 0, sizeof s_slot); }   /* each battle starts fresh */

/* ---- F1 pages ---- */
static const char *s_sections[PE_MAX_SECTIONS + 1] = { "All areas" };
static unsigned s_nsections = 1;
static int s_area, s_show, s_sel, s_list[PE_CREATURES], s_count;
static const char *const s_show_names[] = { "All", "Met", "Not complete" };
static char s_name[64];

static void pe_build_sections(void) {
    int c;
    for (c = 0; c < PE_CREATURES; ++c) {
        unsigned j = 1;
        while (j < s_nsections && strcmp(s_sections[j], g_pe_creatures[c].section)) ++j;
        if (j == s_nsections && s_nsections <= PE_MAX_SECTIONS) s_sections[s_nsections++] = g_pe_creatures[c].section;
    }
}

static void pe_filter(void) {
    int c;
    s_count = 0;
    for (c = 0; c < PE_CREATURES; ++c) {
        const uint8_t s = *pe_state(c);
        if (s_area > 0 && (unsigned)s_area < s_nsections && strcmp(g_pe_creatures[c].section, s_sections[s_area])) continue;
        if (s_show == 1 && !(s & C_MET)) continue;
        if (s_show == 2 && (s & C_WEAK) && (s & C_LOOT)) continue;
        s_list[s_count++] = c;
    }
    if (s_sel >= s_count) s_sel = 0;
}

static int pe_current(void) { pe_filter(); return s_count ? s_list[s_sel] : -1; }

static unsigned pe_count(void) { pe_filter(); return s_count ? (unsigned)s_count : 1u; }

static const char *pe_name(unsigned i) {
    int c;
    uint8_t s;
    pe_filter();
    if (!s_count) return "None";
    if (i >= (unsigned)s_count) i = 0;
    c = s_list[i];
    s = *pe_state(c);
    if (!(s & C_MET)) snprintf(s_name, sizeof s_name, "???  (%u/%d)", i + 1, s_count);
    else snprintf(s_name, sizeof s_name, "%s%s%s  (%u/%d)", g_pe_creatures[c].ex ? "EX " : "", g_pe_creatures[c].name,
                  (s & C_WEAK) && (s & C_LOOT) ? " *" : g_pe_creatures[c].boss ? " (boss)" : "", i + 1, s_count);
    return s_name;
}

static const char *pe_elems(uint8_t bits, char *buf, unsigned size) {
    static const char *const n[5] = { "Heat", "Frost", "Cyanide", "Acid", "Tranq" };
    int i;
    buf[0] = 0;
    for (i = 0; i < 5; ++i)
        if (bits & (1u << i)) snprintf(buf + strlen(buf), size - strlen(buf), "%s%s", buf[0] ? ", " : "", n[i]);
    return buf[0] ? buf : "none";
}

static void pe_info(char *out, unsigned size) {
    const int c = pe_current();
    const PeCreature *k;
    if (c < 0) { snprintf(out, size, "-"); return; }
    k = &g_pe_creatures[c];
    if (*pe_state(c) & C_MET)
        snprintf(out, size, "HP %u  Def %u  %s  kills %u", k->hp, k->defense, k->section, pe_kills(c));
    else snprintf(out, size, "Not met yet  (%s)", k->section);
}

static void pe_elements(char *out, unsigned size) {
    const int c = pe_current();
    char weak[48], strong[48];
    if (c < 0) { snprintf(out, size, "-"); return; }
    if (*pe_state(c) & C_WEAK)
        snprintf(out, size, "Weak: %s  Strong: %s", pe_elems(g_pe_creatures[c].weak, weak, sizeof weak),
                 pe_elems(g_pe_creatures[c].strong, strong, sizeof strong));
    else snprintf(out, size, "Scan, %s or %u Junk", g_pe_creatures[c].boss ? "defeat" : "5 kills", g_pe_creatures[c].price_weak);
}

static void pe_drop(char *out, unsigned size) {
    const int c = pe_current();
    if (c < 0) snprintf(out, size, "-");
    else if (*pe_state(c) & C_LOOT) snprintf(out, size, "%s", g_pe_creatures[c].drop);
    else snprintf(out, size, "%s%u Junk", g_pe_creatures[c].boss ? "" : "15 kills or ", g_pe_creatures[c].price_loot);
}

static void pe_steal(char *out, unsigned size) {
    const int c = pe_current();
    snprintf(out, size, "%s", c >= 0 && (*pe_state(c) & C_LOOT) ? g_pe_creatures[c].steal : "-");
}

static unsigned pe_next_price(int c, uint8_t *bit) {
    const uint8_t s = *pe_state(c);
    if (!(s & C_WEAK)) { *bit = C_WEAK; return g_pe_creatures[c].price_weak; }
    if (!(s & C_LOOT)) { *bit = C_LOOT; return g_pe_creatures[c].price_loot; }
    *bit = 0;
    return 0;
}

static void pe_buy(void) {
    const int c = pe_current();
    char msg[96];
    uint8_t bit;
    unsigned price;
    if (c < 0) return;
    price = pe_next_price(c, &bit);
    if (!bit) { pe_col_say("Already complete"); return; }
    if (!pe_col_junk_ready(msg, sizeof msg)) { pe_col_say(msg); return; }
    if (!pe_col_spend(price)) {
        snprintf(msg, sizeof msg, "Need %u Junk (have %u)", price, pe_col_junk());
        pe_col_say(msg);
        return;
    }
    pe_unlock(c, bit, "junk");
    pe_col_save_now();
}

static void pe_buy_label(char *out, unsigned size) {
    const int c = pe_current();
    uint8_t bit = 0;
    const unsigned price = c >= 0 ? pe_next_price(c, &bit) : 0;
    if (!bit) snprintf(out, size, "Complete");
    else snprintf(out, size, "%s for %u Junk (have %u)", bit == C_WEAK ? "Weaknesses" : "Loot", price, pe_col_junk());
}

static void pe_progress(char *out, unsigned size) {
    int c, met = 0, done = 0;
    for (c = 0; c < PE_CREATURES; ++c) {
        met += (*pe_state(c) & C_MET) != 0;
        done += (*pe_state(c) & (C_WEAK | C_LOOT)) == (C_WEAK | C_LOOT);
    }
    snprintf(out, size, "Met %d/%d  Complete %d", met, PE_CREATURES, done);
}

static void pe_result(char *out, unsigned size) { snprintf(out, size, "%s", pe_col_last_said()); }

void pe_bestiary_reset(void) { memset(s_slot, 0, sizeof s_slot); s_sel = 0; }

void pe_bestiary_register(const char *mod_id) {
    pe_build_sections();
    pe_menu_register_choice(PE_PAGE_BESTIARY, "Area", "Which creatures to browse.", &s_area, s_sections, s_nsections);
    pe_menu_register_choice(PE_PAGE_BESTIARY, "Show", "All, met only, or not complete.", &s_show, s_show_names, 3);
    pe_menu_register_dynamic_choice(PE_PAGE_BESTIARY, "Creature", "Left / right to browse. * = complete.", &s_sel,
                                    PE_CREATURES, pe_count, pe_name);
    pe_menu_register_info(PE_PAGE_BESTIARY, "Info", "HP, defense, where, kills.", pe_info);
    pe_menu_register_info(PE_PAGE_BESTIARY, "Elements", "Weak and strong elements.", pe_elements);
    pe_menu_register_info(PE_PAGE_BESTIARY, "Drop", "What it drops (odds).", pe_drop);
    pe_menu_register_info(PE_PAGE_BESTIARY, "Steal", "What can be stolen (odds).", pe_steal);
    pe_menu_register_info(PE_PAGE_BESTIARY, "Next tier", "What Buy unlocks: weaknesses, then loot.", pe_buy_label);
    pe_menu_register_action(PE_PAGE_BESTIARY, "Buy next tier", "Spend Junk on the next tier. Kept in every game.", pe_buy);
    pe_menu_register_info(PE_PAGE_BESTIARY, "Progress", "Bestiary totals.", pe_progress);
    pe_menu_register_info(PE_PAGE_BESTIARY, "Result", "What the last action did.", pe_result);
    pe_menu_register_tick(pe_field_tick);
    pe_hook_add(PE_HOOK_BATTLE_UPDATE, pe_on_battle, NULL, PE_PRIORITY_LATE, mod_id);
    pe_hook_add(PE_HOOK_SPELL, pe_on_spell, NULL, PE_PRIORITY_NORMAL, mod_id);
}
