#ifndef PE1_ENTITY_ANIMATION_H
#define PE1_ENTITY_ANIMATION_H

#include "pe1/battle.h"

/* Each model selects a row of 48 host action-record pointers. */
typedef struct EntityActionTable {
    void *actions[48];
} EntityActionTable;

PE1_STATIC_ASSERT(sizeof(EntityActionTable) == 48 * sizeof(void *),
                  entity_action_table_size);
extern EntityActionTable D_800B0E98[];

/* Battle state the entity hit and drop handlers read and write. */
extern Combatant *g_ActiveActor;
extern void *g_BattlePendingEnemySpawn;
extern u8 g_BattleHitActionMode[1];
extern u8 g_BattleHitActionSubmode[];
extern s32 g_BattleHitActionParam[1];
extern int g_BattleModeState;

void Entity_AdvanceAnim(BattleEntity *entity);
void Entity_SetAction(BattleEntity *entity, int mode);
void Entity_SetActionMode(BattleEntity *entity, int mode);

#endif
