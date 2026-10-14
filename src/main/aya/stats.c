#include "pe1/akao.h"
#include "pe1/aya.h"
#include "pe1/battle.h"
#include "pe1/battle_modifiers.h"
#include "pe1/battle_runtime.h"
#include "pe1/game_state.h"
#include "pe1/inventory_slots.h"
#include "pe1/inventory.h"
#include "pe1/inventory_state.h"
#include "pe1/menu_sound.h"
#include "pe1/player_entity.h"
#include "pe1/stat_modifiers.h"
#include "pe1/textbox.h"


int *Battle_GetModifierTable(void) {
    return g_StatScaleBase;
}

/* Reports the active-time step and rate of the current PE energy level. */
void Aya_DeriveStats(int *out_max, int *out_current) {
    int base;
    AyaLevelStats *stats;

    Aya_StatQueryLevelAndSubLevel(3, g_AyaSaveState.stats.levels[3], &base, 0);
    stats = Aya_LookupLevelStats(base + g_StatScaleBase[3]);

    if (out_max != 0) {
        *out_max = stats->battleAtbStep;
    }
    if (out_current != 0) {
        *out_current = stats->battleAtbRate;
    }
}

/* Recomputes the battle stats, then refills the player's current HP. */
void Battle_SyncEquipSlots(void) {
    Combatant *actor;
    u16 maxHP;

    Inv_RecalcSlotStats();
    if (g_PlayerEntity != 0) {
        actor = ((BattleEntity *)g_PlayerEntity)->core;
        if (actor != 0) {
            maxHP = actor->maxHP;
            actor->hpMirror = maxHP;
            actor->curHP = maxHP;
        }
    }
}

int Battle_GetEnemyEscapeFlag(void)
{
    Combatant *state;

    if (g_PlayerEntity != 0) {
        state = (Combatant *)g_PlayerEntity->state;
        if (state != 0) {
            return ((unsigned int)state->stateFlags >> 9) & 1;
        }
    }

    return 0;
}

int Battle_GetAgilityBonus(void)
{
    return g_AyaSaveState.stats.levels[0];
}

unsigned int Aya_GetActiveTimeRate(void)
{
    return g_AyaSaveState.stats.levels[5];
}

int Battle_IsActiveWrapped(void)
{
    return (signed char)Battle_IsActive();
}

int Battle_IsInputAllowedWrapped(void)
{
    return (signed char)Pad_IsMenuConfirmAvailable();
}

int Battle_GetStateFlag1(void)
{
    return (g_GameStateFlags >> 1) & 1;
}

void Menu_CopyPromptCodes(unsigned char *src) {
    unsigned char *dst;
    unsigned char *end;
    int value;

    dst = g_TextboxNameGlyphs.glyphs;
    end = dst + 8;
    while (dst < end) {
        value = *src;
        if (value == 0xFF) {
            break;
        }
        *dst++ = value;
        src++;
    }

    g_TextboxNameGlyphs.count = dst - g_TextboxNameGlyphs.glyphs;
}


PlayerInventoryState g_PlayerInventoryState;
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

