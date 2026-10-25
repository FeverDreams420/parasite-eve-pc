/*
 * Parasite Energy rebalance, ported from the psxrecomp build (src/mods/pe_pe_balance.c; bridge
 * findings pe-parasite-energy-rebalance-2026-10-15.md). F1 > PE Enhancements > Items & Parasite Energy.
 *
 * Rules: Vanilla / Rebalanced (default) / Custom (any row changed).
 *
 *   - PE heals scale with Aya's max HP (Heal 1 / 2 / 3: 15 / 30 / 65%, never below the vanilla
 *     30 / 60 / 280), and Detox / Scan cost 60 / 30 PE (vanilla 100 / 50). Both at the entry of
 *     Battle_ApplySpellEffect (PE_HOOK_SPELL, in battle and from the menu): the extra HP is added
 *     before the game's own heal (it clamps to max HP after), and the PE difference is refunded
 *     before the game subtracts its fixed cost (PE Light armor, effect flag 0x200, takes a third
 *     off both, as the game does).
 *   - Scan summary: vanilla Scan only flashes one "Susceptible to ..." line per weakness and never
 *     shows resistances; Rebalanced adds one on-screen line with both.
 *
 * Not ported yet: the half-bar turn rows and Quick powers (they drive the half-bar ATB mod, a
 * QoL port) and Panacea (a custom item, economy port).
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "pe_game.h"
#include "pe_mod_host.h"
#include "pe_mod_menu.h"
#include "pe_pages.h"

enum { RULES_VANILLA, RULES_REBALANCED, RULES_CUSTOM };
static const char *const s_rules_names[] = { "Vanilla", "Rebalanced", "Custom" };
static int s_rules = RULES_REBALANCED, s_rules_shown = -1;
static int s_heal1, s_heal2, s_heal3, s_cost_detox, s_cost_scan, s_scan_summary;

typedef struct { const char *label, *desc; int vanilla, rebalanced, min, max, step; int *value; int toggle; } PeRow;
static PeRow s_rows[] = {
    { "Heal 1 % of max HP", "0 = vanilla (+30 HP).", 0, 15, 0, 100, 5, &s_heal1, 0 },
    { "Heal 2 % of max HP", "0 = vanilla (+60 HP).", 0, 30, 0, 100, 5, &s_heal2, 0 },
    { "Heal 3 % of max HP", "0 = vanilla (+280 HP).", 0, 65, 0, 100, 5, &s_heal3, 0 },
    { "Detox PE cost", "Vanilla 100.", 100, 60, 0, 300, 10, &s_cost_detox, 0 },
    { "Scan PE cost", "Vanilla 50.", 50, 30, 0, 300, 10, &s_cost_scan, 0 },
    { "Scan: weak & strong", "Scan lists weaknesses and resists.", 0, 1, 0, 1, 1, &s_scan_summary, 1 },
};
#define PE_ROWS ((int)(sizeof s_rows / sizeof s_rows[0]))

static void pe_apply(int rules) {
    int i;
    if (rules == RULES_CUSTOM) return;
    for (i = 0; i < PE_ROWS; ++i) *s_rows[i].value = rules == RULES_VANILLA ? s_rows[i].vanilla : s_rows[i].rebalanced;
}

static int pe_matches(int rules) {
    int i;
    for (i = 0; i < PE_ROWS; ++i)
        if (*s_rows[i].value != (rules == RULES_VANILLA ? s_rows[i].vanilla : s_rows[i].rebalanced)) return 0;
    return 1;
}

static void pe_balance_tick(void) {
    if (s_rules != s_rules_shown) { pe_apply(s_rules); s_rules_shown = s_rules; }
    else if (s_rules != RULES_CUSTOM && !pe_matches(s_rules)) s_rules = s_rules_shown = RULES_CUSTOM;
    else if (s_rules == RULES_CUSTOM && pe_matches(RULES_REBALANCED)) s_rules = s_rules_shown = RULES_REBALANCED;
    else if (s_rules == RULES_CUSTOM && pe_matches(RULES_VANILLA)) s_rules = s_rules_shown = RULES_VANILLA;
}

/* Scan summary, from the target's statusFlags2 pairs (0 average, 1 resists, 2 weak). */
static void pe_scan_summary(BattleEntity *target) {
    static const struct { int shift; const char *name; } pairs[] = {
        { 14, "Heat" }, { 16, "Cold" }, { 0, "Poison" }, { 2, "Acid" }, { 4, "Tranq" },
        { 10, "Confuse" }, { 8, "Slow" }, { 12, "Steal" },
    };
    const EnemyCombatant *core;
    char weak[96] = "", strong[96] = "", text[200];
    uint32_t s2;
    unsigned i;
    if (!target || target == pe_player() || !(core = (const EnemyCombatant *)target->core)) return;
    s2 = core->statusFlags2;
    for (i = 0; i < sizeof pairs / sizeof pairs[0]; ++i) {
        const unsigned v = (s2 >> pairs[i].shift) & 3u;
        char *dst = v == 2u ? weak : v == 1u ? strong : NULL;
        if (!dst) continue;
        if (dst[0]) strncat(dst, ", ", 95 - strlen(dst));
        strncat(dst, pairs[i].name, 95 - strlen(dst));
    }
    snprintf(text, sizeof text, "Scan  Weak: %s   Strong: %s", weak[0] ? weak : "none", strong[0] ? strong : "none");
    pe_toast(text);
    pe_log("scan", "status2=0x%05X weak=%s strong=%s", (unsigned)(s2 & 0x3FFFFu), weak, strong);
}

static int pe_on_spell(void *ctx, void *user) {
    static const int vanilla_heal[3] = { 30, 60, 280 };
    const PeHookSpell *spell = (const PeHookSpell *)ctx;
    Combatant *core = pe_aya_core();
    int vanilla_cost = 0, cost = 0;
    (void)user;
    if (spell->spell == 7u && s_scan_summary) pe_scan_summary(spell->target);
    if (!core) return 0;
    if (spell->spell <= 2u) {                          /* heals: a share of max HP, never below vanilla */
        const int pct = spell->spell == 0 ? s_heal1 : spell->spell == 1 ? s_heal2 : s_heal3;
        if (pct > 0) {
            const int want = core->maxHP * pct / 100;
            if (want > vanilla_heal[spell->spell]) core->curHP = (u16)(core->curHP + (want - vanilla_heal[spell->spell]));
        }
    }
    if (spell->spell == 3u) { vanilla_cost = 100; cost = s_cost_detox; }
    if (spell->spell == 7u) { vanilla_cost = 50; cost = s_cost_scan; }
    if (vanilla_cost && cost != vanilla_cost) {        /* refund the difference before the game charges */
        int32_t delta = (int32_t)(vanilla_cost - cost) * 0x10000;
        if (core->attributes && (core->attributes->effectFlags & 0x200u)) delta = delta * 2 / 3;
        core->exp_or_acc += delta;
    }
    return 0;
}

#define PE_PAGE PE_PAGE_PE_BALANCE
void pe_pe_balance_register(const char *mod_id) {
    int i;
    pe_apply(RULES_REBALANCED);
    pe_menu_register_choice(PE_PAGE, "Rules", "Vanilla, Rebalanced, or Custom.", &s_rules, s_rules_names, 3);
    for (i = 0; i < PE_ROWS; ++i) {
        if (s_rows[i].toggle)
            pe_menu_register_toggle(PE_PAGE, s_rows[i].label, s_rows[i].desc, s_rows[i].value, NULL);
        else
            pe_menu_register_int(PE_PAGE, s_rows[i].label, s_rows[i].desc, s_rows[i].value,
                                 s_rows[i].min, s_rows[i].max, s_rows[i].step);
    }
    s_rules = pe_matches(RULES_REBALANCED) ? RULES_REBALANCED : pe_matches(RULES_VANILLA) ? RULES_VANILLA : RULES_CUSTOM;
    s_rules_shown = s_rules;
    pe_menu_register_tick(pe_balance_tick);
    pe_hook_add(PE_HOOK_SPELL, pe_on_spell, NULL, PE_PRIORITY_NORMAL, mod_id);
}
