#ifndef PE1_BATTLE_ENTITY_ANIM_H
#define PE1_BATTLE_ENTITY_ANIM_H

#include "pe1/game_state_types.h"

#include "pe1/asset_find.h"
#include "pe1/entity_animation.h"

extern struct FieldActor *g_FieldActorListHead;
#include "pe1/player_entity.h"
extern s8 g_BattleLiveEnemyCount;
extern u16 g_BattlePeBonusReward;
extern u32 g_BattleExpReward;
extern BattleRewardSlot g_BattleRewardSlots[];

void Anim_SetInterpRate(RenderObjectEntity *anim, int rate);
void Akao_Cmd_21(int arg0, int arg1);
void Battle_SlotFree(void **combatantPtr);
void Battle_StartDeathAnim(void);
void Battle_StepEntityAnimState(BattleEntity *entity);
int Entity_TriggerAnimEvent(BattleEntity *entity, u8 slot);

#endif
