#ifndef PE1_ENTITY_FLOOR_H
#define PE1_ENTITY_FLOOR_H

#include "pe1/game_state.h"

/* Declarations used by the per-actor floor tracking driver
 * Entity_UpdateAndRender. */

#include "pe1/battle.h"
#include "pe1/field_collision.h"

/* The floor driver reads the player and the field flags word (8: the player
 * stands on a ramp edge) absolutely; its unit runs under MASPSX_FORCE_G0. */
#include "pe1/player_entity.h"

int abs(int value);
int Geo_ClipToFloorBoundary(s16 x, s16 z, void *face);

#endif /* PE1_ENTITY_FLOOR_H */
