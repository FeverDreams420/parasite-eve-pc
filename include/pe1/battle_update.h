#ifndef PE1_BATTLE_UPDATE_H
#define PE1_BATTLE_UPDATE_H

#include "pe1/asset_find.h"
#include "pe1/battle_turn.h"

int Akao_SendTableCommand(void *, int, int, int, int);
int Battle_BuildTargetList(void);
void Battle_ResolveHitOnTimer(void);
void Battle_RollEnemySpawn(int);
void Battle_StepEscapeOrDeath(void);
void Battle_StepLevelUp(void);
void Battle_StepPostBattle(void);
int Battle_StepVictory(void);
void Draw_QueuePrimitive(void);
int Inv_CountByValue(int);
int Inv_FindItemById(int);
void Inv_OpenAyaItemList(unsigned int);
void Menu_MainUpdate(int);
int Menu_RunFrameWithArg(int);
void Menu_SaveOverlayDraw(void);
void Menu_SetItemContext(int);
int Pm_SendCmd(int, int, int, int, int, int);
void Save_DrawSlotMetadata(void);
void srand(unsigned int);

/* Only element zero is accessed in the wide matching views below. */
extern u8 D_8009CE7C;
extern s32 D_8009D1D0;
extern s32 g_BattleFrameCounter;
extern s8 D_8009D1F0[16];
extern u32 D_8009D1F4;
extern u8 D_8009D235[16];
extern u8 g_BattleActionActorCount;
extern s32 D_8009D290[16];
extern s32 D_8009D2FC;
extern void *g_AkaoBgmHandle[16];
extern BattleEntity *g_BattleActionActors[7];

#endif
