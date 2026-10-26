/*
 * Stats editor: F1 > Debug > Stats editor (the psxrecomp build's pe_stats_menu.c).
 *
 * Every row shows the same 1-based number as the in-game status screen and is changed with
 * left / right; edits apply when F1 closes. In between, a tick mirrors the game's values into the
 * rows, so they always open showing what the game has.
 *
 *   - Level: g_AyaSaveState.level (0-based) and total_exp from the game's EXP table, so "next
 *     level" stays consistent. Raising it does what the game's level-up does (Aya_SetTotalExp and
 *     the level-up screen): every stat gains 10 EXP a level, the banked bonus-point pool is paid
 *     out (capped at 99999) and cleared, and every Parasite Energy whose unlock level is reached is
 *     unlocked. Lowering it has no in-game equivalent: it only rewinds level and EXP.
 *   - HP, offense, defense, PE, status recovery, active time: each stat's EXP total
 *     (g_AyaSaveState.stats.levels[k]); its level is the highest one in its growth table
 *     (Aya_GetStatGrowthTable) whose threshold is reached, max 99. Setting a level writes that
 *     level's threshold, then the game's own Inv_RecalcSlotStats derives max HP (current HP
 *     clamped) and the battle values.
 *   - Item capacity is deliberately not editable: the game never moves items when capacity drops,
 *     so anything past the new limit (the equipped gear included) would be stranded.
 */
#include "pe_game.h"
#include "pe_mod_host.h"
#include "pe_mod_menu.h"
#include "pe_pages.h"
#include "pe1/aya.h"

void Inv_RecalcSlotStats(void);

#define PE_MAX_LEVEL      99        /* status-screen numbering */
#define PE_EDITABLE_STATS 6         /* item capacity (6) stays the game's */
#define PE_STATS          7
#define PE_EXP_PER_LEVEL  10
#define PE_BP_CAP         99999u
#define PE_SPELLS         20

typedef struct {
    int value;      /* the row, 1-based level */
    int shadow;     /* last level mirrored from / written to the game */
    int raw;        /* last stat EXP seen, -1 = unknown */
} PeStatRow;

static PeStatRow s_stats[PE_EDITABLE_STATS];
static int s_level = 1, s_level_shadow = 1;

/* The highest 0-based level <= 98 whose threshold is <= value (Stat_QueryLevelAndSubLevel). */
static int pe_stat_level0(int kind, int value) {
    const int *table = Aya_GetStatGrowthTable(kind);
    int level = 0, l;
    for (l = 1; l < PE_MAX_LEVEL; ++l) {
        if (table[l] > value) break;
        level = l;
    }
    return level;
}

static void pe_apply_stat(int kind, int level1) {
    const int *table = Aya_GetStatGrowthTable(kind);
    g_AyaSaveState.stats.levels[kind] = (u16)table[level1 - 1];
    Inv_RecalcSlotStats();                             /* max HP and battle values, as the game derives them */
}

static void pe_apply_level(int level1) {
    const int new0 = level1 - 1, old0 = g_AyaSaveState.level;
    const u32 *exp = Aya_GetLevelExpTable();
    const ParasiteSpellEntry *spells = Aya_GetParasiteSpellUnlockTable();
    u32 bp;
    int k;
    g_AyaSaveState.total_exp = exp[new0];
    g_AyaSaveState.level = (u8)new0;
    if (new0 <= old0) return;
    for (k = 0; k < PE_STATS; ++k) {                   /* a real level-up's stat EXP */
        const u32 v = g_AyaSaveState.stats.levels[k] + (u32)(new0 - old0) * PE_EXP_PER_LEVEL;
        g_AyaSaveState.stats.levels[k] = (u16)(v > 0xFFFFu ? 0xFFFFu : v);
    }
    Inv_RecalcSlotStats();
    bp = g_AyaSaveState.bonus_points + g_AyaSaveState.pe_bonus_pool;
    g_AyaSaveState.bonus_points = bp > PE_BP_CAP ? PE_BP_CAP : bp;
    g_AyaSaveState.pe_bonus_pool = 0;
    for (k = 0; k < PE_SPELLS; ++k)
        if (spells[k].cost != 0 && spells[k].unlockKey <= (u16)new0) g_AyaSaveState.parasite_spell_flags |= 1u << k;
}

/* Apply rows the player changed in F1, then mirror the game back into them. */
static void pe_stats_tick(void) {
    static const char *const names[PE_EDITABLE_STATS] = { "HP", "Offense", "Defense", "PE", "Status recovery", "Active time" };
    int k;
    if (!pe_game_loaded() || pe_in_battle()) {
        /* Edits made with no game (or mid battle) don't land on the next one. */
        s_level = s_level_shadow;
        for (k = 0; k < PE_EDITABLE_STATS; ++k) s_stats[k].value = s_stats[k].shadow;
        return;
    }
    if (s_level != s_level_shadow) {
        pe_log("tweak", "Stats/Level %d->%d", s_level_shadow, s_level);
        pe_apply_level(s_level);
    }
    s_level = s_level_shadow = g_AyaSaveState.level + 1;
    for (k = 0; k < PE_EDITABLE_STATS; ++k) {
        PeStatRow *row = &s_stats[k];
        int raw;
        if (row->value != row->shadow) {
            pe_log("tweak", "Stats/%s level %d->%d", names[k], row->shadow, row->value);
            pe_apply_stat(k, row->value);
            row->raw = -1;
        }
        raw = g_AyaSaveState.stats.levels[k];
        if (raw != row->raw) {                         /* only search the table on change */
            row->raw = raw;
            row->shadow = pe_stat_level0(k, raw) + 1;
        }
        row->value = row->shadow;
    }
}

void pe_stats_editor_register(const char *mod_id) {
    static const char *const labels[PE_EDITABLE_STATS] = {
        "HP level", "Offense level", "Defense level", "PE level", "Status recovery level", "Active time level",
    };
    static const char *const descriptions[PE_EDITABLE_STATS] = {
        "Max HP. Applies when F1 closes.", "Attack power. Applies when F1 closes.", "Defense. Applies when F1 closes.",
        "PE and recharge. Applies when F1 closes.", "Status recovery. Applies when F1 closes.",
        "Turn gauge speed. Applies when F1 closes.",
    };
    int k;
    (void)mod_id;
    for (k = 0; k < PE_EDITABLE_STATS; ++k) {
        s_stats[k].value = s_stats[k].shadow = 1;
        s_stats[k].raw = -1;
    }
    pe_menu_register_int(PE_PAGE_DEBUG_STATS, "Level", "Up is a real level-up. Applies when F1 closes.", &s_level, 1, PE_MAX_LEVEL, 1);
    for (k = 0; k < PE_EDITABLE_STATS; ++k)
        pe_menu_register_int(PE_PAGE_DEBUG_STATS, labels[k], descriptions[k], &s_stats[k].value, 1, PE_MAX_LEVEL, 1);
    pe_menu_register_tick(pe_stats_tick);
}
