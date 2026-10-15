#ifndef PE1_MENU_BONUS_STATS_H
#define PE1_MENU_BONUS_STATS_H

#include "common.h"

/* Nine-word arrays at 0x800A18B4, 0x800A18D8 and 0x800A18FC.
 * Seven growth categories are normally initialized; the parasite-screen
 * commit loop transfers nine entries, matching AyaSaveState.stats.transfer. */
typedef struct MenuBonusStatWorkspace {
    s32 queryResults[9];
    s32 deltas[9];
    s32 multipliers[9];
} MenuBonusStatWorkspace;

PE1_STATIC_ASSERT(PE1_OFFSETOF(MenuBonusStatWorkspace, deltas) == 0x24,
                  menu_bonus_stat_deltas_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(MenuBonusStatWorkspace, multipliers) == 0x48,
                  menu_bonus_stat_multipliers_offset);
PE1_STATIC_ASSERT(sizeof(MenuBonusStatWorkspace) == 0x6C,
                  menu_bonus_stat_workspace_size);

extern MenuBonusStatWorkspace g_MenuBonusStatWorkspace;
#define g_BonusPointStatQueryResults (g_MenuBonusStatWorkspace.queryResults)
#define g_BonusPointStatDeltas (g_MenuBonusStatWorkspace.deltas)
#define g_BonusPointStatMultipliers (g_MenuBonusStatWorkspace.multipliers)

/* Level-up and bonus point panels: shown values step toward their targets. */
extern int g_MenuLevelDisplayValue, g_MenuLevelDisplayTarget;
extern int g_MenuHpMaxDisplayValue, g_MenuHpMaxDisplayTarget;
extern int g_BonusPointDisplayValue, g_MenuBonusPointDisplayTarget;
extern int g_BonusPointBarAnimStep, g_BonusPointBarAnimProgress;
extern int g_MenuBonusPointBarAnimActive;
/* Frame counter pacing the maximum HP count-up. */
extern int g_MenuHpMaxAnimTimer;
/* Bonus point spend dialog: the stat being raised, the points left while
 * spending and the stat value being edited. */
extern int g_BonusPointSpendStat;
extern int g_BonusPointSpendWorkingValue;
extern int g_BonusPointSpendCurrentValue;
extern int g_MenuStatBarWidths[8];
extern int g_MenuStatBarSettleTimers[8];
/* Panel outlines of the EXP review and the level-up result. */
extern unsigned char g_MenuExpReviewPanelPoints[];
extern unsigned char g_MenuLevelUpPanelPoints[];
/* Outlines of the status panel and the stat list beside it. */
extern unsigned char g_MenuStatusPanelPoints[];
extern unsigned char g_MenuStatsListPanelPoints[];

#endif
