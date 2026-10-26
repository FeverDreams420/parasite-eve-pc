/* Tools fixture: the stats and equipment editors' rules on a stand-in save (production files,
 * the game's tables and the F1 panel stubbed). */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "pe_game.h"
#include "pe_mod_host.h"
#include "pe_mod_menu.h"
#include "pe1/aya.h"
#include "pe1/inventory_state.h"

SaveRuntimeState g_SaveRuntimeState;
int g_InvBonusSlotCount;
static int s_growth[7][128];
static u32 s_exp[99];
static ParasiteSpellEntry s_spells[20];
static int s_recalcs, s_loaded = 1;
static ItemDataRecord s_gun, s_armor;

void *Aya_GetStatGrowthTable(int k) { return s_growth[k]; }
void *Aya_GetLevelExpTable(void) { return s_exp; }
void *Aya_GetParasiteSpellUnlockTable(void) { return s_spells; }
void Inv_RecalcSlotStats(void) { ++s_recalcs; }
int pe_game_loaded(void) { return s_loaded; }
int pe_in_battle(void) { return 0; }
ItemDataRecord *pe_equipped(int armor) { return armor ? &s_armor : &s_gun; }
void pe_log(const char *t, const char *f, ...) { (void)t; (void)f; }

/* The F1 rows, kept so the test can drive them. */
static PeMenuPerFrame s_tick;
static int *s_ints[8], s_nint;
static PeMenuSet s_set[16];
static const char *s_set_label[16];
static int s_nset;
int pe_menu_register_int(const char *s, const char *l, const char *d, int *v, int lo, int hi, int st) { (void)s; (void)l; (void)d; (void)lo; (void)hi; (void)st; s_ints[s_nint++] = v; return 0; }
int pe_menu_register_tick(PeMenuPerFrame f) { s_tick = f; return 0; }
int pe_menu_register_live_int(const char *s, const char *l, const char *d, int lo, int hi, int st, PeMenuGet g, PeMenuSet set) { (void)s; (void)d; (void)lo; (void)hi; (void)st; (void)g; s_set_label[s_nset] = l; s_set[s_nset++] = set; return 0; }
int pe_menu_register_live_choice(const char *s, const char *l, const char *d, const char *const *n, unsigned c, PeMenuGet g, PeMenuSet set) { (void)s; (void)d; (void)n; (void)c; (void)g; s_set_label[s_nset] = l; s_set[s_nset++] = set; return 0; }
static int *s_slot_choice[2], s_nslot;
int pe_menu_register_dynamic_choice(const char *s, const char *l, const char *d, int *i, unsigned m, PeMenuChoiceCount c, PeMenuChoiceName n) {
    (void)s; (void)d; (void)m; (void)c; (void)n; if (strstr(l, "mod slot")) s_slot_choice[s_nslot++] = i; return 0;
}
static PeMenuSet setter(const char *label) { int i; for (i = 0; i < s_nset; ++i) if (!strcmp(s_set_label[i], label)) return s_set[i]; return NULL; }

void pe_stats_editor_register(const char *);
void pe_equip_editor_register(const char *);

int main(void) {
    int k, l;
    for (k = 0; k < 7; ++k) for (l = 0; l < 128; ++l) s_growth[k][l] = l * 10;   /* 10 EXP a level */
    for (l = 0; l < 99; ++l) s_exp[l] = (u32)l * 100u;
    s_spells[0] = (ParasiteSpellEntry){0, 5};  s_spells[1] = (ParasiteSpellEntry){4, 5};
    s_spells[2] = (ParasiteSpellEntry){20, 5}; s_spells[3] = (ParasiteSpellEntry){2, 0};   /* cost 0: a dummy */
    g_AyaSaveState.max_hp = 50;

    /* Stats: rows mirror the game; a level-up is a real one. */
    pe_stats_editor_register("test");
    s_tick();
    assert(*s_ints[0] == 1 && *s_ints[1] == 1);
    g_AyaSaveState.pe_bonus_pool = 300; g_AyaSaveState.bonus_points = 99900;
    *s_ints[0] = 6; s_tick();                        /* level 1 -> 6 */
    assert(g_AyaSaveState.level == 5 && g_AyaSaveState.total_exp == 500);
    for (k = 0; k < 7; ++k) assert(g_AyaSaveState.stats.levels[k] == 50);
    assert(g_AyaSaveState.bonus_points == 99999 && g_AyaSaveState.pe_bonus_pool == 0);   /* capped */
    assert(g_AyaSaveState.parasite_spell_flags == 0x3);  /* levels 0 and 4, not 20, not the dummy */
    assert(*s_ints[1] == 6 && s_recalcs == 1);        /* HP level follows its stat EXP */
    *s_ints[0] = 2; s_tick();                         /* down: only level and EXP rewind */
    assert(g_AyaSaveState.level == 1 && g_AyaSaveState.total_exp == 100 && g_AyaSaveState.stats.levels[0] == 50);
    *s_ints[2] = 12; s_tick();                        /* offense level 12: its threshold, then the game derives */
    assert(g_AyaSaveState.stats.levels[1] == 110 && s_recalcs == 2 && *s_ints[2] == 12);
    s_loaded = 0; *s_ints[3] = 40; s_tick();          /* no game: the edit is dropped */
    s_loaded = 1; s_tick(); assert(g_AyaSaveState.stats.levels[2] == 50 && *s_ints[3] == 6);

    /* Equipment: bonuses, ammo cap, one mod per group, the bag guard. */
    pe_equip_editor_register("test");
    s_gun.baseStats[2] = 15; s_gun.ammo = 15; s_gun.tailCount = 3;
    assert(setter("Gun: bullets +")(-5) && s_gun.ammo == 10);          /* capacity 10 now */
    assert(setter("Gun: ammo loaded")(50) && s_gun.ammo == 10);
    *s_slot_choice[0] = 0; assert(setter("Gun: mod")(3));               /* ROF 5 in slot 1 */
    *s_slot_choice[0] = 1; assert(setter("Gun: mod")(5));               /* ROF 10 replaces it */
    assert(s_gun.tailData[0] == 0 && s_gun.tailData[1] == ((1 << 5) | 5));
    g_AyaSaveState.inventory_slot_count = 10; s_armor.tailCount = 2;
    *s_slot_choice[1] = 0; assert(setter("Armor: mod")(10) && g_InvBonusSlotCount == 4);   /* bag +4 */
    g_AyaSaveState.inventory_items[12] = 6;                              /* an item in a +4 slot */
    assert(!setter("Armor: mod")(8) && g_InvBonusSlotCount == 4);        /* +1 would strand it */
    g_AyaSaveState.inventory_items[12] = 0;
    assert(setter("Armor: mod")(8) && g_InvBonusSlotCount == 1);
    puts("Tools checks passed: level-up stats/BP/abilities, level down, stat thresholds, no-game guard, bonuses, ammo, mod groups, bag guard.");
    return 0;
}
