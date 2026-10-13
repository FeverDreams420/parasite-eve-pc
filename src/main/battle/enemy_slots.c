#include "pe1/battle_runtime.h"

void Entity_AllocSlot(BattleEntity *entity) {
    EnemyCombatant initial = g_EnemyCombatantInitial;
    u8 index;
    EnemyCombatant *allocated;

    for (index = 0; index < 7; index++) {
        if (g_BattleEnemySlots[index].active == 0) {
            g_BattleEnemySlots[index].active = 1;
            g_BattleEnemySlots[index].combatant = initial;
            allocated = &g_BattleEnemySlots[index].combatant;
            entity->core = allocated;
            g_EntityInstanceCounter++;
            allocated->lowAndEntityId.bytes.entityId = g_EntityInstanceCounter;
            ((EnemyCombatant *)entity->core)->slotBit = 1 << index;

            if (!(entity->entityFlags & 0x2000)) {
                ((EnemyCombatant *)entity->core)->effect =
                    (EnemyActionEffect *)&((EnemyCombatant *)entity->core)->pad_1C;
                Entity_SetActionMode(entity, 2);
                D_8009D2A0++;
            }
            return;
        }
    }
}

void Battle_SlotFree(void **combatantPtr) {
    u8 index;

    index = 0;
    do {
        if (&g_BattleEnemySlots[index].combatant == *combatantPtr) {
            g_BattleEnemySlots[index].active = 0;
        }
        index++;
    } while (index < 7);
    *combatantPtr = 0;
}
void Battle_ClearMotionTable(void) {
    u8 i;
    i = 0;
    do {
        g_BattleEnemySlots[i].active = 0;
        i++;
    } while (i < 7);
}
