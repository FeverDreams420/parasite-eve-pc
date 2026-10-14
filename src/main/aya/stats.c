#include "pe1/akao.h"
#include "pe1/aya.h"
#include "pe1/battle.h"
#include "pe1/battle_modifiers.h"
#include "pe1/battle_runtime.h"
#include "pe1/game_state.h"
#include "pe1/inventory_slots.h"
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
    if (D_8009D254 != 0) {
        actor = D_8009D254->core;
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
    return g_AyaStatAgility[0];
}

unsigned int Aya_GetActiveTimeRate(void)
{
    return g_AyaStatActiveTimeRate[0];
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


