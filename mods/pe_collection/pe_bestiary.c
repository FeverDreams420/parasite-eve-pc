/*
 * Bestiary: a registry of creatures (`bestiary` service, host/pe_services.h). Packs add creatures
 * and map their enemies to them; resolvers (a randomizer's shuffled enemies) are asked first. Per
 * creature:
 *   basics   (name, HP, defense, where)  first kill, or any purchase;
 *   weakness (weak / strong elements)    casting Scan on it, 5 kills (bosses: defeat), or Junk;
 *   loot     (drop and steal odds)       15 kills, or Junk (bosses: Junk only).
 * Kills: every battle frame (PE_HOOK_BATTLE_UPDATE) the enemy slots are watched; an enemy whose HP
 * goes from above 0 to 0 or less is a kill of the creature it resolves to. Scan: power 7 on an
 * enemy (PE_HOOK_SPELL). Unlocks are saved by the creature's key. F1 pages: area and progress
 * filters, a creature row browsed with left / right, its details, one purchase.
 */
#include <stdio.h>
#include <string.h>

#include "pe_collection.h"
#include "pe_game.h"
#include "pe_mod_host.h"
#include "pe_mod_menu.h"
#include "pe_pages.h"

#define PE_SCAN           7
#define PE_ENEMY_SLOTS    7
#define PE_MAX_CREATURES  512
#define PE_MAX_MAPPINGS   2048
#define PE_MAX_RESOLVERS  8
#define PE_MAX_SECTIONS   32
#define PE_KILLS_WEAK     5
#define PE_KILLS_LOOT     15

/* ---- the registry ---- */
static const PeCreatureInfo *s_creature[PE_MAX_CREATURES];
static int s_creatures;
static struct { uint16_t room, identity; int16_t handle; } s_map[PE_MAX_MAPPINGS];
static int s_maps;
static PeBestiaryResolver s_resolver[PE_MAX_RESOLVERS];
static int s_resolvers;
static int s_event = -1;

static int pe_valid(int h) { return h >= 0 && h < s_creatures; }

static int pe_find(const char *key) {
    int h;
    if (!key) return -1;
    for (h = 0; h < s_creatures; ++h) if (strcmp(s_creature[h]->key, key) == 0) return h;
    return -1;
}

static int pe_add(const PeCreatureInfo *info) {
    if (!info || !info->key || !*info->key || !info->name || s_creatures == PE_MAX_CREATURES || pe_find(info->key) >= 0) {
        pe_mod_log("pe_collection", "bestiary: creature %s refused", info && info->key ? info->key : "?");
        return -1;
    }
    s_creature[s_creatures] = info;
    return s_creatures++;
}

static const PeCreatureInfo *pe_info(int h) { return pe_valid(h) ? s_creature[h] : NULL; }

static int pe_replace(int h, const PeCreatureInfo *info) {
    if (!pe_valid(h) || !info || !info->key || strcmp(info->key, s_creature[h]->key) != 0) return 0;
    s_creature[h] = info;
    return 1;
}

static int pe_map(int room, unsigned identity, int h) {
    int i;
    if (!pe_valid(h) || room < 0) return 0;
    for (i = 0; i < s_maps; ++i)
        if (s_map[i].room == room && s_map[i].identity == identity) { s_map[i].handle = (int16_t)h; return 1; }
    if (s_maps == PE_MAX_MAPPINGS) return 0;
    s_map[s_maps].room = (uint16_t)room;
    s_map[s_maps].identity = (uint16_t)identity;
    s_map[s_maps++].handle = (int16_t)h;
    return 1;
}

static int pe_add_resolver(PeBestiaryResolver fn) {
    if (!fn || s_resolvers == PE_MAX_RESOLVERS) return 0;
    s_resolver[s_resolvers++] = fn;
    return 1;
}

/* Which creature an enemy is: resolvers (newest first), then the mapped table. */
static int pe_resolve(int room, unsigned identity, const EnemyCombatant *enemy) {
    int i;
    for (i = s_resolvers - 1; i >= 0; --i) {
        const int h = s_resolver[i](room, identity, enemy);
        if (pe_valid(h)) return h;
    }
    for (i = 0; i < s_maps; ++i)
        if (s_map[i].room == room && s_map[i].identity == identity) return s_map[i].handle;
    return -1;
}

static unsigned pe_state(int h) { return pe_valid(h) ? pe_profile_creature(s_creature[h]->key, NULL) : 0u; }
static unsigned pe_kills(int h) { unsigned k = 0; if (pe_valid(h)) (void)pe_profile_creature(s_creature[h]->key, &k); return k; }

static void pe_unlock_why(int h, unsigned bits, const char *why) {
    const unsigned old = pe_state(h), now = (old | bits | PE_BESTIARY_MET) & 7u, gained = now & ~old;
    const PeCreatureInfo *c = pe_info(h);
    char msg[96];
    if (!c || now == old) return;
    pe_profile_set_creature(c->key, now, pe_kills(h));
    snprintf(msg, sizeof msg, "Bestiary: %s%s%s%s", c->name, gained & PE_BESTIARY_WEAK ? " - weaknesses" : "",
             gained & PE_BESTIARY_LOOT ? " - loot" : "", gained == PE_BESTIARY_MET ? " - new entry" : "");
    pe_toast(msg);
    pe_log("bestiary", "creature=%s bits=0x%02X why=%s", c->key, now, why);
    {
        PeBestiaryEvent e = { h, gained, why };
        pe_event_emit(s_event, &e);
    }
}

static void pe_unlock(int h, unsigned bits) { pe_unlock_why(h, bits, "mod"); }

static void pe_kill(int h) {
    const PeCreatureInfo *c = pe_info(h);
    unsigned k = pe_kills(h) + 1u, bits = PE_BESTIARY_MET;
    if (!c) return;
    pe_profile_set_creature(c->key, pe_state(h), k);
    if (c->boss) bits |= PE_BESTIARY_WEAK;                 /* defeated: its weaknesses are known */
    else {
        if (k >= PE_KILLS_WEAK) bits |= PE_BESTIARY_WEAK;
        if (k >= PE_KILLS_LOOT) bits |= PE_BESTIARY_LOOT;
    }
    pe_unlock_why(h, bits, "kill");
}

const PeBestiaryApi g_pe_bestiary_api = {
    pe_add, pe_find, pe_info, pe_replace, pe_map, pe_add_resolver, pe_state, pe_kills, pe_unlock,
};

/* ---- kills and Scan ---- */
static struct { int tracking; uint8_t entity_id; uint16_t identity; int32_t hp; } s_slot[PE_ENEMY_SLOTS];

static int pe_on_battle(void *ctx, void *user) {
    const int room = pe_room_number();
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
        if (s_slot[s].hp > 0 && hp <= 0 && room > 0) {
            const int h = pe_resolve(room, s_slot[s].identity ? s_slot[s].identity : id, core);
            if (h >= 0) pe_kill(h);
        }
        s_slot[s].hp = hp;
        if (id) s_slot[s].identity = id;
    }
    return 0;
}

static int pe_on_spell(void *ctx, void *user) {
    const PeHookSpell *spell = (const PeHookSpell *)ctx;
    const EnemyCombatant *core;
    int h;
    (void)user;
    if (spell->spell != PE_SCAN || !spell->target || spell->target == pe_player() || !pe_game_loaded()) return 0;
    if (!(core = (const EnemyCombatant *)spell->target->core)) return 0;
    if ((h = pe_resolve(pe_room_number(), (uint16_t)core->field04.fieldId04, core)) >= 0) pe_unlock_why(h, PE_BESTIARY_WEAK, "scan");
    return 0;
}

static void pe_field_tick(void) { if (!pe_in_battle()) memset(s_slot, 0, sizeof s_slot); }   /* each battle starts fresh */

/* ---- F1 pages ---- */
static const char *s_sections[PE_MAX_SECTIONS + 1];
static unsigned s_nsections;
static int s_area, s_show, s_sel, s_list[PE_MAX_CREATURES], s_count;
static const char *const s_show_names[] = { "All", "Met", "Not complete" };
static char s_name[80];

/* Areas in the order packs registered them. */
static void pe_sections(void) {
    int h;
    s_sections[0] = "All areas";
    s_nsections = 1;
    for (h = 0; h < s_creatures; ++h) {
        const char *sec = s_creature[h]->section ? s_creature[h]->section : "Other";
        unsigned j = 1;
        while (j < s_nsections && strcmp(s_sections[j], sec)) ++j;
        if (j == s_nsections && s_nsections <= PE_MAX_SECTIONS) s_sections[s_nsections++] = sec;
    }
}
static unsigned pe_area_count(void) { pe_sections(); return s_nsections; }
static const char *pe_area_name(unsigned i) { pe_sections(); return i < s_nsections ? s_sections[i] : ""; }

static void pe_filter(void) {
    int h;
    pe_sections();
    s_count = 0;
    for (h = 0; h < s_creatures; ++h) {
        const unsigned s = pe_state(h);
        const char *sec = s_creature[h]->section ? s_creature[h]->section : "Other";
        if (s_area > 0 && (unsigned)s_area < s_nsections && strcmp(sec, s_sections[s_area])) continue;
        if (s_show == 1 && !(s & PE_BESTIARY_MET)) continue;
        if (s_show == 2 && (s & PE_BESTIARY_WEAK) && (s & PE_BESTIARY_LOOT)) continue;
        s_list[s_count++] = h;
    }
    if (s_sel >= s_count) s_sel = 0;
}

static int pe_current(void) { pe_filter(); return s_count ? s_list[s_sel] : -1; }
static unsigned pe_count(void) { pe_filter(); return s_count ? (unsigned)s_count : 1u; }

static const char *pe_name(unsigned i) {
    const PeCreatureInfo *c;
    unsigned s;
    pe_filter();
    if (!s_count) return "None";
    if (i >= (unsigned)s_count) i = 0;
    c = s_creature[s_list[i]];
    s = pe_state(s_list[i]);
    if (!(s & PE_BESTIARY_MET)) snprintf(s_name, sizeof s_name, "???  (%u/%d)", i + 1, s_count);
    else snprintf(s_name, sizeof s_name, "%s%s%s  (%u/%d)", c->ex ? "EX " : "", c->name,
                  (s & PE_BESTIARY_WEAK) && (s & PE_BESTIARY_LOOT) ? " *" : c->boss ? " (boss)" : "", i + 1, s_count);
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

static void pe_info_row(char *out, unsigned size) {
    const int h = pe_current();
    const PeCreatureInfo *c = pe_info(h);
    if (!c) { snprintf(out, size, "-"); return; }
    if (pe_state(h) & PE_BESTIARY_MET)
        snprintf(out, size, "HP %u  Def %u  %s  kills %u", c->hp, c->defense, c->section ? c->section : "", pe_kills(h));
    else snprintf(out, size, "Not met yet  (%s)", c->section ? c->section : "");
}

static void pe_elements_row(char *out, unsigned size) {
    const int h = pe_current();
    const PeCreatureInfo *c = pe_info(h);
    char weak[48], strong[48];
    if (!c) { snprintf(out, size, "-"); return; }
    if (pe_state(h) & PE_BESTIARY_WEAK)
        snprintf(out, size, "Weak: %s  Strong: %s", pe_elems(c->weak, weak, sizeof weak), pe_elems(c->strong, strong, sizeof strong));
    else snprintf(out, size, "Scan, %s or %u Junk", c->boss ? "defeat" : "5 kills", c->price_weak);
}

static void pe_drop_row(char *out, unsigned size) {
    const int h = pe_current();
    const PeCreatureInfo *c = pe_info(h);
    if (!c) snprintf(out, size, "-");
    else if (pe_state(h) & PE_BESTIARY_LOOT) snprintf(out, size, "%s", c->drop ? c->drop : "nothing");
    else snprintf(out, size, "%s%u Junk", c->boss ? "" : "15 kills or ", c->price_loot);
}

static void pe_steal_row(char *out, unsigned size) {
    const int h = pe_current();
    const PeCreatureInfo *c = pe_info(h);
    snprintf(out, size, "%s", c && (pe_state(h) & PE_BESTIARY_LOOT) ? (c->steal ? c->steal : "nothing") : "-");
}

static unsigned pe_next_price(int h, unsigned *bit) {
    const unsigned s = pe_state(h);
    const PeCreatureInfo *c = pe_info(h);
    *bit = 0;
    if (!c) return 0;
    if (!(s & PE_BESTIARY_WEAK)) { *bit = PE_BESTIARY_WEAK; return c->price_weak; }
    if (!(s & PE_BESTIARY_LOOT)) { *bit = PE_BESTIARY_LOOT; return c->price_loot; }
    return 0;
}

static void pe_buy(void) {
    const int h = pe_current();
    char msg[96];
    unsigned bit, price;
    if (h < 0) return;
    price = pe_next_price(h, &bit);
    if (!bit) { pe_col_say("Already complete"); return; }
    if (!pe_col_junk_ready(msg, sizeof msg)) { pe_col_say(msg); return; }
    if (!pe_col_spend(price)) {
        snprintf(msg, sizeof msg, "Need %u Junk (have %u)", price, pe_col_junk());
        pe_col_say(msg);
        return;
    }
    pe_unlock_why(h, bit, "junk");
    pe_profile_save();
}

static void pe_buy_label(char *out, unsigned size) {
    const int h = pe_current();
    unsigned bit = 0;
    const unsigned price = h >= 0 ? pe_next_price(h, &bit) : 0;
    if (!bit) snprintf(out, size, "Complete");
    else snprintf(out, size, "%s for %u Junk (have %u)", bit == PE_BESTIARY_WEAK ? "Weaknesses" : "Loot", price, pe_col_junk());
}

static void pe_progress(char *out, unsigned size) {
    int h, met = 0, done = 0;
    for (h = 0; h < s_creatures; ++h) {
        const unsigned s = pe_state(h);
        met += (s & PE_BESTIARY_MET) != 0;
        done += (s & (PE_BESTIARY_WEAK | PE_BESTIARY_LOOT)) == (PE_BESTIARY_WEAK | PE_BESTIARY_LOOT);
    }
    snprintf(out, size, "Met %d/%d  Complete %d", met, s_creatures, done);
}

static void pe_result(char *out, unsigned size) { snprintf(out, size, "%s", pe_col_last_said()); }

void pe_bestiary_reset(void) { memset(s_slot, 0, sizeof s_slot); s_sel = 0; }

/* Testing actions change only creatures registered by the loaded packs.
 * Keep real kill counts on unlock; reset clears both discovery and kills. */
static void pe_unlock_all(void) {
    int changed = 0;
    for (int h = 0; h < s_creatures; ++h) {
        unsigned gained = 7u & ~pe_state(h);
        if (!gained) continue;
        pe_profile_set_creature(s_creature[h]->key, 7u, pe_kills(h));
        PeBestiaryEvent e = { h, gained, "debug" };
        pe_event_emit(s_event, &e);
        ++changed;
    }
    pe_profile_save();
    char msg[80];
    snprintf(msg, sizeof msg, "Unlocked %d Bestiary entries", changed);
    pe_col_say(msg);
}
static void pe_reset_all(void) {
    for (int h = 0; h < s_creatures; ++h)
        if (pe_state(h) || pe_kills(h)) pe_profile_set_creature(s_creature[h]->key, 0, 0);
    pe_bestiary_reset(); /* discard pre-reset enemy HP tracking */
    pe_profile_save();
    pe_col_say("Bestiary reset: discoveries and kill counts cleared");
}
void pe_bestiary_register(const char *mod_id) {
    s_event = pe_event_id("bestiary.unlock");
    pe_menu_register_action(PE_PAGE_DEBUG_COLLECTION, "Unlock all Bestiary",
        "Testing: reveal every registered creature, weakness and loot entry. Keep kill counts.", pe_unlock_all);
    pe_menu_register_action(PE_PAGE_DEBUG_COLLECTION, "Reset Bestiary",
        "Testing: clear discoveries and kill counts only. Keep costumes and Junk.", pe_reset_all);
    pe_menu_register_dynamic_choice(PE_PAGE_BESTIARY, "Area", "Which creatures to browse.", &s_area, PE_MAX_SECTIONS + 1,
                                    pe_area_count, pe_area_name);
    pe_menu_register_choice(PE_PAGE_BESTIARY, "Show", "All, met only, or not complete.", &s_show, s_show_names, 3);
    pe_menu_register_dynamic_choice(PE_PAGE_BESTIARY, "Creature", "Left / right to browse. * = complete.", &s_sel,
                                    PE_MAX_CREATURES, pe_count, pe_name);
    pe_menu_register_info(PE_PAGE_BESTIARY, "Info", "HP, defense, where, kills.", pe_info_row);
    pe_menu_register_info(PE_PAGE_BESTIARY, "Elements", "Weak and strong elements.", pe_elements_row);
    pe_menu_register_info(PE_PAGE_BESTIARY, "Drop", "What it drops (odds).", pe_drop_row);
    pe_menu_register_info(PE_PAGE_BESTIARY, "Steal", "What can be stolen (odds).", pe_steal_row);
    pe_menu_register_info(PE_PAGE_BESTIARY, "Next tier", "What Buy unlocks: weaknesses, then loot.", pe_buy_label);
    pe_menu_register_action(PE_PAGE_BESTIARY, "Buy next tier", "Spend Junk on the next tier. Kept in every game.", pe_buy);
    pe_menu_register_info(PE_PAGE_BESTIARY, "Progress", "Bestiary totals.", pe_progress);
    pe_menu_register_info(PE_PAGE_BESTIARY, "Result", "What the last action did.", pe_result);
    pe_menu_register_tick(pe_field_tick);
    pe_hook_add(PE_HOOK_BATTLE_UPDATE, pe_on_battle, NULL, PE_PRIORITY_LATE, mod_id);
    pe_hook_add(PE_HOOK_SPELL, pe_on_spell, NULL, PE_PRIORITY_NORMAL, mod_id);
}
