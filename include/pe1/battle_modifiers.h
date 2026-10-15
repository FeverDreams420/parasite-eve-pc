#ifndef PE1_BATTLE_MODIFIERS_H
#define PE1_BATTLE_MODIFIERS_H

#include "pe1/inventory.h"
#include "pe1/stat_modifiers.h"
#include "pe1/weapon_list_output.h"

void Inv_SelectActiveList(int useOverride);
void Inv_RecalcSlotStats(void);
struct BattleAttributes;
extern struct BattleAttributes g_BattleEquipStateBlock;
void Inv_BuildArmorList(struct BattleAttributes *out);

extern int g_InvBonusSlotCount;

/* Historical name: retail selects the equipped armor slot at 0x800C0E22. */
void BattleCmd_LoadWeaponModifiers(void);

#endif
