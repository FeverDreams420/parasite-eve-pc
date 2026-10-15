#ifndef PE1_BATTLE_MAIN_H
#define PE1_BATTLE_MAIN_H

#include "common.h"
#include "pe1/battle.h"

/* Battle turn, status and map-selection flow (src/main/battle, script). */

/* 20-byte record copied as one unit by the enemy effect flow. */
typedef struct { u8 bytes[0x14]; } Tbl20;
/* 18-byte status symbol sprite template. */
typedef struct StatusSymbolTemplate {
    u8 bytes[18];
} StatusSymbolTemplate;

extern const BattleActionSoundTable g_BattleActionSoundIds;
extern u8 g_BattleHitTimer;
extern s16 g_BattlePlayerAimHeight;
extern const Tbl20 g_BattleActionEffectIds;
extern const StatusSymbolTemplate g_BattleStatusSymbolUv;
extern const StatusSymbolTemplate g_BattleStatusSymbolSize;
extern const StatusSymbolTemplate g_BattleStatusSymbolOffset;

#endif /* PE1_BATTLE_MAIN_H */
