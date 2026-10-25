/*
 * Wardrobe: Aya's outfits are the game's own player bundles, picked by g_SceneAreaType
 * (g_GameState.requested_entity_bank): the room sets it when it loads, and her model, textures and
 * per-day animation rows all follow it. Wearing an outfit rewrites it from 10 / 12 / 13 (the
 * gameplay outfits) at the start of the room load's model step (PE_HOOK_PLAYER_BANK, phase 0) and
 * asks for a reload when the loaded bundle differs; taking it off needs nothing, the game reloads
 * its own when the room's bank differs from the loaded one. 11 (Day 1 long dress) and 14
 * (transformed) have no full player moveset and are not offered. Wearing from F1 asks for the
 * game's same-room reload (g_GameStateFlags 0x2000 and flags 0x800 end the frame loop and the room
 * loads again), so it shows as soon as the panel closes.
 *
 * Character skins (pe_skins/<folder>/, built from the user's disc by the bridge's build_skin.py)
 * are listed as "soon" until their loader is ported.
 *
 * Bought once with Junk, kept in every game (pe_collection.dat). Publishes the `wardrobe` service:
 * Wayne's Workshop shows a Wardrobe row with the costumes that can be worn now.
 */
#include <stdio.h>
#include <string.h>

#include "pe_collection.h"
#include "pe_collection_data.h"
#include "pe_game.h"
#include "pe_mod_host.h"
#include "pe_mod_menu.h"
#include "pe_pages.h"
#include "pe_services.h"
#include "pe1/game_state.h"

#define PE_RELOAD_ROOM   0x2000u   /* g_GameStateFlags: end the frame loop */
#define PE_RELOAD_SAME   0x800u    /* g_GameState.flags: the same room again */
#define PE_NEW_BUNDLE    0x200000u /* g_GameState.flags: load the player bundle */

static int pe_gameplay_area(unsigned a) { return a == 10u || a == 12u || a == 13u; }

static int pe_wearable(int i) {
    if (i < 0 || i >= PE_SKINS) return 0;
    return g_pe_skins[i].kind == 0 && pe_gameplay_area(g_pe_skins[i].area);   /* skins: loader not ported */
}

static int pe_owned(int i) { return (int)((pe_col_profile()->skins >> i) & 1u); }
static int pe_worn(void) { return pe_col_profile()->worn < PE_SKINS ? pe_col_profile()->worn : 0; }

/* ---- the outfit at room load ---- */
static int pe_on_player_bank(void *ctx, void *user) {
    const int w = pe_worn();
    const PeSkin *k = &g_pe_skins[w];
    (void)ctx;
    (void)user;
    if (!w || !pe_game_loaded() || g_GameState.entity_texture_phase != 0 || !pe_wearable(w)) return 0;
    if (!pe_gameplay_area(g_SceneAreaType) || g_SceneAreaType == k->area) return 0;
    g_SceneAreaType = k->area;                                /* an outfit: the game's own bundle */
    if (k->area != g_GameState.loaded_entity_bank) g_GameState.flags |= PE_NEW_BUNDLE;
    g_GameState.story_day_flags |= 1u;                        /* and its day rows */
    pe_log("skin", "outfit=%s area=%u", k->name, (unsigned)k->area);
    return 0;
}

/* The game's same-room reload, so a new outfit shows when F1 closes. */
static int pe_request_reload(void) {
    if (!pe_game_loaded() || pe_in_battle() || (unsigned)(g_SceneAreaType - 10u) >= 5u) return 0;
    g_GameStateFlags |= PE_RELOAD_ROOM;
    g_GameState.flags |= PE_RELOAD_SAME;
    pe_log("skin", "reload worn=%s", g_pe_skins[pe_worn()].name);
    return 1;
}

static void pe_wear(int i, int reload) {
    char msg[96];
    pe_col_profile()->worn = (uint8_t)i;
    pe_col_save_now();
    pe_log("wardrobe", "wear=%s", g_pe_skins[i].name);
    snprintf(msg, sizeof msg, reload && pe_request_reload() ? "Wearing %s" : "Wearing %s from the next room", g_pe_skins[i].name);
    pe_col_say(msg);
}

/* Buy (if needed); 0 with a reason in msg if it can't. */
static int pe_buy(int i, char *msg, size_t size) {
    const PeSkin *k = &g_pe_skins[i];
    if (pe_owned(i)) return 1;
    if (!pe_col_junk_ready(msg, (unsigned)size)) return 0;
    if (!pe_col_spend(k->price)) { snprintf(msg, size, "Need %u Junk (have %u)", (unsigned)k->price, pe_col_junk()); return 0; }
    pe_col_profile()->skins |= 1ull << i;
    pe_col_changed();
    pe_log("wardrobe", "bought=%s price=%u", k->name, (unsigned)k->price);
    return 1;
}

/* ---- F1 pages: a group row, a costume row browsed with left / right, buy / wear ---- */
static const char *const s_group_names[] = { "Aya's outfits", "Main cast", "Supporting", "Extras" };
static int s_group, s_sel, s_list[PE_SKINS], s_count;
static char s_name[64];

static int pe_group_of(int i) {
    const PeSkin *k = &g_pe_skins[i];
    if (k->kind == 0) return 0;
    return k->price >= 250 ? 1 : k->price >= 100 ? 2 : 3;   /* main 250+, supporting 100, extras 25 */
}

static void pe_filter(void) {
    int i;
    s_count = 0;
    for (i = 0; i < PE_SKINS; ++i) if (pe_group_of(i) == s_group) s_list[s_count++] = i;
    if (s_sel >= s_count) s_sel = 0;
}

static unsigned pe_count(void) { pe_filter(); return s_count ? (unsigned)s_count : 1u; }

static const char *pe_state_name(int i) {
    if (!pe_wearable(i)) return "soon";
    if (pe_worn() == i) return "worn";
    return pe_owned(i) ? "owned" : NULL;
}

static const char *pe_name(unsigned n) {
    int i;
    const char *state;
    pe_filter();
    if (!s_count) return "None";
    if (n >= (unsigned)s_count) n = 0;
    i = s_list[n];
    if ((state = pe_state_name(i))) snprintf(s_name, sizeof s_name, "%s  [%s]  (%u/%d)", g_pe_skins[i].name, state, n + 1, s_count);
    else snprintf(s_name, sizeof s_name, "%s  %u Junk  (%u/%d)", g_pe_skins[i].name, (unsigned)g_pe_skins[i].price, n + 1, s_count);
    return s_name;
}

static void pe_accept(void) {
    char msg[96];
    int i;
    pe_filter();
    if (!s_count) return;
    i = s_list[s_sel];
    if (!pe_wearable(i)) { pe_col_say(g_pe_skins[i].kind ? "Character skins aren't ported yet" : "This outfit has no full moveset"); return; }
    if (pe_worn() == i) { pe_col_say("Already wearing it"); return; }
    if (!pe_buy(i, msg, sizeof msg)) { pe_col_say(msg); return; }
    pe_wear(i, 1);
}

static void pe_default(void) {
    if (pe_worn() == 0) { pe_col_say("Already wearing Aya's own clothes"); return; }
    pe_wear(0, 1);
}

static void pe_status(char *out, unsigned size) {
    snprintf(out, size, "Wearing %s   Junk %u", g_pe_skins[pe_worn()].name, pe_col_junk());
}

static void pe_result(char *out, unsigned size) { snprintf(out, size, "%s", pe_col_last_said()); }

/* ---- the `wardrobe` service (Wayne's Workshop): what can be worn now, by list index ---- */
static int pe_ward_skin(int k) {
    int i;
    for (i = 0; i < PE_SKINS; ++i)
        if ((i == 0 || pe_wearable(i)) && k-- == 0) return i;
    return -1;
}
static int pe_ward_count(void) { int n = 0; while (pe_ward_skin(n) >= 0) ++n; return n; }
static const char *pe_ward_name(int k) { const int i = pe_ward_skin(k); return i >= 0 ? g_pe_skins[i].name : ""; }
static unsigned pe_ward_price(int k) { const int i = pe_ward_skin(k); return i >= 0 ? g_pe_skins[i].price : 0u; }
static int pe_ward_state(int k) {
    const int i = pe_ward_skin(k);
    if (i < 0) return 0;
    return pe_worn() == i ? 2 : pe_owned(i);
}
static int pe_ward_pick(int k, char *msg, size_t size) {   /* buy if needed, then wear (from the next room) */
    const int i = pe_ward_skin(k);
    if (i < 0) return 0;
    if (!pe_buy(i, msg, size)) return 0;
    pe_col_profile()->worn = (uint8_t)i;
    pe_col_save_now();
    pe_log("wardrobe", "wear=%s at=wayne", g_pe_skins[i].name);
    snprintf(msg, size, "Wearing %s when you leave", g_pe_skins[i].name);
    return 1;
}
static const PeWardrobeApi s_api = { pe_ward_count, pe_ward_name, pe_ward_price, pe_ward_state, pe_ward_pick };

void pe_wardrobe_reset(void) { s_sel = 0; }

void pe_wardrobe_register(const char *mod_id) {
    pe_menu_register_choice(PE_PAGE_WARDROBE, "Group", "Aya's outfits or characters.", &s_group, s_group_names, 4);
    pe_menu_register_dynamic_choice(PE_PAGE_WARDROBE, "Costume", "Left / right to browse.", &s_sel, PE_SKINS, pe_count, pe_name);
    pe_menu_register_action(PE_PAGE_WARDROBE, "Buy / wear", "Buy once (every game), wear now: the room reloads.", pe_accept);
    pe_menu_register_action(PE_PAGE_WARDROBE, "Wear Aya's own", "Back to the clothes the story gives her.", pe_default);
    pe_menu_register_info(PE_PAGE_WARDROBE, "Status", "What Aya wears, and your Junk.", pe_status);
    pe_menu_register_info(PE_PAGE_WARDROBE, "Result", "What the last action did.", pe_result);
    pe_service_publish("wardrobe", 1, &s_api, mod_id);
    pe_hook_add(PE_HOOK_PLAYER_BANK, pe_on_player_bank, NULL, PE_PRIORITY_NORMAL, mod_id);
}
