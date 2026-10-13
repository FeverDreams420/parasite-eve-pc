#ifndef PE1_BATTLE_TURN_H
#define PE1_BATTLE_TURN_H

#include "pe1/battle_palette.h"
#include "pe1/battle_reset_stats.h"
#include "pe1/battle_runtime.h"
#include "pe1/battle_status.h"
#include "pe1/entity_animation.h"
#include "pe1/entity_floor.h"
#include "pe1/render_setup.h"

extern s32 g_BattleModeState;


int Battle_ProcessActionSlot(BattleEntity *);
void Battle_ResetEnemyStats(int);
int CdRom_StepReadState(int);

void Akao_Cmd_21(int, int);
int Asset_LoadTimTextures(int);
void Battle_DrawStatusPanel(int, BattleStatusPanel *);
void Battle_UpdateEnemy(BattleEntity *);
void Entity_TickAnimSequences(BattleEntity *);
void Battle_PhaseInitEnemyTurn(void);
void Battle_PhaseEndTurn(void);
void Battle_PhaseHitReaction(void);

#endif
