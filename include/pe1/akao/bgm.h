#ifndef PE1_AKAO_BGM_H
#define PE1_AKAO_BGM_H

#include "pe1/game_state.h"

/* USA 800B0E08 is the resident sound pointer in the canonical game state. */
#define g_AkaoBgmHandle (g_GameState.sound_table_handles)

#endif
