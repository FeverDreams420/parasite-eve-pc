#ifndef PE_GAME_H
#define PE_GAME_H

/* Game access shared by mods: the recomp build's address helpers, in terms
 * of the native game's own structs and globals. */

#include "common.h"
#include "pe1/aya.h"
#include "pe1/battle_runtime.h"
#include "pe1/inventory_slots.h"

BattleEntity *pe_player(void);       /* Aya's actor, or NULL */
Combatant *pe_aya_core(void);        /* her Combatant (stats, gauge, action), or NULL */
int pe_game_loaded(void);            /* a game is running (not the title screens) */
int pe_in_battle(void);
int pe_room_number(void);            /* the room's map number ("M374..." -> 374), or -1 */

/* Equipped weapon (armor = 0) or armor (armor = 1) record, or NULL. */
ItemDataRecord *pe_equipped(int armor);
/* Base item data for item id (1-based), or NULL. */
ItemDataRecord *pe_base_item(unsigned id);
/* The record carries mod effect `effect` (low 5 bits of a mod byte). */
int pe_rec_has_effect(const ItemDataRecord *rec, int effect);

unsigned pe_isqrt(unsigned v);

#endif
