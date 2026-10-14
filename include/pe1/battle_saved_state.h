#ifndef PE1_BATTLE_SAVED_STATE_H
#define PE1_BATTLE_SAVED_STATE_H

#include "common.h"
#include "pe1/battle.h"
#include "pe1/weapon_list_output.h"

/* Word-aligned battle views over opaque save records serialized as bytes. */
typedef union BattleStateTail {
    s32 words[6];
    WeaponListOutput weapon;
} BattleStateTail;
PE1_STATIC_ASSERT(sizeof(BattleStateTail) == 0x18, battle_state_tail_size);

/* Native prefix of Combatant through its equipment pointers. Serialized PS1
 * records are still 0x70 bytes and require explicit conversion on save/load. */
typedef struct AyaBattleState {
    s32 words[PE1_OFFSETOF(Combatant, action) / sizeof(s32)];
    BattleStateTail *action;
    BattleAttributes *attributes;
} AyaBattleState;
PE1_STATIC_ASSERT(PE1_OFFSETOF(AyaBattleState, action) ==
                  PE1_OFFSETOF(Combatant, action), aya_battle_state_action_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(AyaBattleState, attributes) ==
                  PE1_OFFSETOF(Combatant, attributes), aya_battle_state_attributes_offset);
PE1_STATIC_ASSERT(sizeof(AyaBattleState) == PE1_OFFSETOF(Combatant, field70),
                  aya_battle_state_size);

/* Rodata defaults Battle_SaveAyaState loads into the saved battle records. */
extern AyaBattleState g_AyaBattleStateInitial;
extern BattleStateTail g_SavedBattleStateTailInitial;

extern AyaBattleState g_AyaBattleState;
extern BattleStateTail g_SavedBattleStateTail;

void Battle_SaveAyaState(void);

#endif /* PE1_BATTLE_SAVED_STATE_H */
