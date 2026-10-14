#include "pe1/aya.h"
#include "pe1/battle.h"
#include "pe1/player_entity.h"
#include "pe1/inventory.h"
#include "pe1/stat_modifiers.h"

AyaSaveState g_AyaSaveState;
int g_StatScaleBase[7];

void Inv_RecalcSlotStats(void) {
    u16 *p = g_AyaSaveState.stats.levels;
    int level, hp;
    Combatant *actor;
    AyaLevelStats *row;

    Aya_StatQueryLevelAndSubLevel(0, (s16)*p++, &level, 0);
    row = Aya_LookupLevelStats(level);
    hp = ((g_StatScaleBase[0] + 20) * row->hp) / 20;
    g_AyaSaveState.max_hp = hp;
    if (((BattleEntity *)g_PlayerEntity) && (actor = ((BattleEntity *)g_PlayerEntity)->core)) {
        if ((s16)(actor->maxHP = hp) < (s16)actor->curHP)
            actor->curHP = hp;
        if ((s16)actor->maxHP < (s16)actor->hpMirror)
            actor->hpMirror = actor->maxHP;
        if (g_AyaSaveState.max_hp < g_AyaSaveState.current_hp)
            g_AyaSaveState.current_hp = g_AyaSaveState.max_hp;

        Aya_StatQueryLevelAndSubLevel(1, (s16)*p++, &level, 0);
        actor->stat1E = Aya_LookupLevelStats(level + g_StatScaleBase[1])->offense;
        Aya_StatQueryLevelAndSubLevel(2, (s16)*p++, &level, 0);
        actor->stat20 = Aya_LookupLevelStats(level + g_StatScaleBase[2])->defense;
        Aya_StatQueryLevelAndSubLevel(3, (s16)*p++, &level, 0);
        row = Aya_LookupLevelStats(level + g_StatScaleBase[3]);
        actor->maxAtk = row->battleMaxAtk;
        actor->atbRate = row->battleAtbRate;
        actor->atbStep = row->battleAtbStep;
        Aya_StatQueryLevelAndSubLevel(4, (s16)*p++, &level, 0);
        row = Aya_LookupLevelStats(level + g_StatScaleBase[4]);
        actor->statusStep3C = row->statusStep3C;
        actor->statusStep3E = row->statusStep3E;
        Aya_StatQueryLevelAndSubLevel(5, (s16)*p++, &level, 0);
        actor->stat22 = Aya_LookupLevelStats(level + g_StatScaleBase[5])->battleStat22;
        Aya_StatQueryLevelAndSubLevel(6, (s16)*p, &level, 0);
        actor->stat26 = Aya_LookupLevelStats(level)->inventoryCapacity;
        Inv_SetAyaSlotCount(actor->stat26);
        actor->field04.fieldId04 = g_AyaSaveState.level + 1;
    }
}

