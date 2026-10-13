#ifndef PE1_BATTLE_PALETTE_H
#define PE1_BATTLE_PALETTE_H

#include "common.h"
#include "pe1/battle_hud_colors.h"
#include "pe1/game_state.h"

extern struct BattleEntity *D_8009D254;
/* Word copied into the player entity's action check callback. */
extern u32 D_800915E0[];
extern unsigned int g_GameStateFlags;
extern unsigned int D_8009D2E8;
extern u8 g_EntityInstanceCounter;
extern s8 D_8009D2A0;

void Battle_FlushScriptSounds(void);
void BattleCmd_SyncActiveAmmo(void);
void Battle_SetupPlayerPalette(void);

#endif
