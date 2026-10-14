#ifndef PE1_BATTLE_PALETTE_H
#define PE1_BATTLE_PALETTE_H

#include "pe1/game_state_types.h"

#include "common.h"
#include "pe1/game_state.h"

extern struct BattleEntity *D_8009D254;
extern unsigned int D_8009D2E8;
extern u8 g_EntityInstanceCounter;
extern s8 g_BattleLiveEnemyCount;

void Battle_FlushScriptSounds(void);
void BattleCmd_SyncActiveAmmo(void);
void Battle_SetupPlayerPalette(void);

#endif
