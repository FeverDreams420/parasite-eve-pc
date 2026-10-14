#include "pe1/battle_saved_state.h"
#include "pe1/inventory_slots.h"
#include "pe1/inventory_state.h"
#include "pe1/battle_modifiers.h"
#include "pe1/stat_modifiers.h"

AyaBattleState g_AyaBattleState;
BattleStateTail g_SavedBattleStateTail;
BattleAttributes g_BattleEquipStateBlock;

void Battle_InitEquipLists(AyaBattleState **out) {
    *out = &g_AyaBattleState;
    g_AyaBattleState.action = &g_SavedBattleStateTail;
    g_AyaBattleState.attributes = &g_BattleEquipStateBlock;
    Inv_RecalcSlotStats();
    Inv_BuildWeaponList(0, &g_AyaBattleState.action->weapon);
    Inv_BuildArmorList(g_AyaBattleState.attributes);
}

/* USA templates at 0x80010928 and 0x80010998. Index native fields so the
 * widened FieldActorState pointer/padding cannot shift these defaults. */
#define STATE_WORD(field) (PE1_OFFSETOF(Combatant, field) / sizeof(s32))
AyaBattleState g_AyaBattleStateInitial = {
    .words = {
        [STATE_WORD(field04)] = 1,
        [STATE_WORD(exp_or_acc)] = 0x500000,
        [STATE_WORD(curHP)] = 0x002d002d,
        [STATE_WORD(maxHP)] = 0x0014002d,
        [STATE_WORD(stat20)] = 0x00190014,
        [STATE_WORD(maxAtk)] = 0x500000,
        [STATE_WORD(atbRate)] = 0x1b58,
        [STATE_WORD(atbGauge)] = -1,
    },
};
#undef STATE_WORD
BattleStateTail g_SavedBattleStateTailInitial;

void Battle_SaveAyaState(void) {
    AyaBattleState aya = g_AyaBattleStateInitial;
    BattleStateTail tail = g_SavedBattleStateTailInitial;
    BattleAttributes attributes;

    attributes.parameterWord.raw = 0;
    attributes.effectFlags = 0;
    g_AyaBattleState = aya;
    g_SavedBattleStateTail = tail;
    g_BattleEquipStateBlock = attributes;
}
