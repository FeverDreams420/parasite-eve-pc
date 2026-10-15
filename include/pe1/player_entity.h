#ifndef PE1_PLAYER_ENTITY_H
#define PE1_PLAYER_ENTITY_H

/* One player pointer, with field and battle names for the same native actor. */
struct FieldActor;
struct BattleEntity;
extern struct FieldActor *g_PlayerEntity;
#define D_8009D254 ((struct BattleEntity *)g_PlayerEntity)

#include "pe1/field_actor.h"

/* Points the player actor's state at Aya's battle record and rebuilds her
 * equipment lists (battle/encounter.c). */
struct AyaBattleState;
void Battle_InitEquipLists(struct AyaBattleState **out);

#endif /* PE1_PLAYER_ENTITY_H */
