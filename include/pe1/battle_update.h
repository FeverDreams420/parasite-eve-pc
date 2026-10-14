#ifndef PE1_BATTLE_UPDATE_H
#define PE1_BATTLE_UPDATE_H

#include "pe1/asset_find.h"
#include "pe1/battle_turn.h"

int Akao_SendTableCommand(void *, int, int, int, int);
int Battle_BuildTargetList(void);
void Battle_ResolveHitOnTimer(void);
void Battle_RollEnemySpawn(void *);
void Battle_StepEscapeOrDeath(void);
void Battle_StepLevelUp(void);
void Battle_StepPostBattle(void);
int Battle_StepVictory(void);
void Draw_QueuePrimitive(void);
int Inv_CountByValue(int);
int Inv_FindItemById(int);
void Inv_OpenAyaItemList(unsigned int);
void Menu_MainUpdate(int);
int Menu_RunFrameWithArg(void *);
void Menu_SaveOverlayDraw(void);
void Menu_SetItemContext(int);
#include "pe1/pm.h"
void Save_DrawSlotMetadata(void);
void srand(unsigned int);

/* Only element zero is accessed in the wide matching views below. */
extern u8 g_BattleSuspendedModeState;
extern s32 g_BattleFrameCounter;
extern u32 g_FieldPadPressed;
extern u8 g_BattleSpritePrimCountdown;
extern u8 g_BattleActionActorCount;
extern int g_BattleAttackAnimFrame;
extern s32 g_BattleImpactPmSlot;
extern void *g_AkaoBgmHandle[16];
extern BattleEntity *g_BattleActionActors[7];

#endif
