#include "pe1/battle_runtime.h"

/* Original USA 800109B0 defaults, using the shared native event records. */
const EnemyCombatant g_EnemyCombatantInitial = {
    .lowAndEntityId.field06 = 2,
    .records = {
        {.enterStep=0x10000,.exitStep=0x10000},
        {.enterStep=0x10000,.exitStep=0x10000},
        {.enterStep=0x10000,.exitStep=0x10000},
        {.enterStep=0x10000,.exitStep=0x10000},
        {.enterStep=0x10000,.exitStep=0x10000},
        {.enterStep=0x10000,.exitStep=0x10000},
    },
    .contactActionIds = {-1,-1,-1,-1,-1,-1,-1,-1},
    .lootItemAux = -1, .deathAssetEnabled = 1, .deathPersist = 1,
    .resumePhase = 1, .savedAnimStep = 0x10000, .statusFlags2 = 0x500000,
};

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
                    &((EnemyCombatant *)entity->core)->records[0];
                Entity_SetActionMode(entity, 2);
                g_BattleLiveEnemyCount++;
            }
            return;
        }
    }
}

void Battle_SlotFree(BattleEntity *entity) {
    u8 index;

    index = 0;
    do {
        if (&g_BattleEnemySlots[index].combatant == entity->core) {
            g_BattleEnemySlots[index].active = 0;
        }
        index++;
    } while (index < 7);
    entity->core = 0;
}
void Battle_ClearMotionTable(void) {
    u8 i;
    i = 0;
    do {
        g_BattleEnemySlots[i].active = 0;
        i++;
    } while (i < 7);
}
