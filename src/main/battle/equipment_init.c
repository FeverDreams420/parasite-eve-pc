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
