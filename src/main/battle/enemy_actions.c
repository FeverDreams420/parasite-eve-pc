#include "common.h"
#include "pe1/battle.h"
#include "pe1/battle_entity_anim.h"
#include "pe1/battle_runtime.h"
#include "pe1/game_state.h"
#include "pe1/inventory.h"
#include "pe1/player_entity.h"

void Battle_SubActionStep(void) {
    Combatant *state;
    BattleEntity *actor;

    state = g_ActiveActor;
    if (state->subActionCounter >= state->subActionPeriod) {
        state->curHP -= state->subActionStep;
        state->hpMirror -= state->subActionStep;
        state->subActionCounter = 0;
        state->panelAux_val = state->subActionStep;
        actor = (BattleEntity *)g_PlayerEntity;
        g_ActiveActor->panelAux_x = actor->renderObject.projected_x;
        g_ActiveActor->panelAux_y = actor->renderObject.projected_y;
        g_ActiveActor->panelAux_timer = 0x1E;
        g_ActiveActor->panelAux_mode = 3;
    }

    g_ActiveActor->subActionCounter++;
}

#define NULL ((void *)0)

void Battle_ApplyEnemyAttack(EnemyCombatant *ent) {
    s32 *ps = &g_ActiveActor->stateFlags;

    switch (ent->effect->effectType) {
    case ENEMY_EFFECT_POISON:
        if ((*ps & AYA_POISON_MASK) == AYA_POISON) {
            break;
        }
        if (g_ActiveActor->attributes->effectFlags & EFFECT_RESIST_POISON) {
            if ((rand() % 100) < AYA_POISON_RESIST_CHANCE) {
                break;
            }
        }
        {
            s32 f = *ps;
            if ((f & AYA_POISON_MASK) == AYA_POISON_IMMUNE) {
                *ps = f & ~AYA_POISON_MASK;
                break;
            }
            {
                Combatant *pD = g_ActiveActor;
                u8 lvl;
                *ps = (f & ~AYA_POISON_MASK) | AYA_POISON;
                pD->statusTimer40 = 0x2328;
                lvl = ent->effectLevel;
                pD->subActionCounter = 0;
                pD->subActionStep = lvl;
            }
        }
        {
            Combatant *pD = g_ActiveActor;
            pD->subActionPeriod = ent->effectDuration;
        }
        break;
    case ENEMY_EFFECT_CLEAR_BUFFS:
        {
            Combatant *pD = g_ActiveActor;
            s32 flags;
            flags = pD->stateFlags;
            g_ActorEffectFlag100Timer = 0;
            pD->stateFlags = flags & ~AYA_HASTE;
            {
                s32 remainingFlags = ((volatile Combatant *)pD)->stateFlags;
                remainingFlags &= ~AYA_BARRIER;
                remainingFlags &= ~AYA_GENE_HEAL;
                remainingFlags &= ~AYA_PRERAISE;
                pD->stateFlags = remainingFlags;
            }
        }
        break;
    case ENEMY_EFFECT_CONFUSION:
        if ((*ps & AYA_CONFUSION_MASK) == AYA_CONFUSION) {
            break;
        }
        if (g_ActiveActor->attributes->effectFlags & EFFECT_RESIST_CONFUSION) {
            if ((rand() % 100) < AYA_CONFUSION_RESIST_CHANCE) {
                break;
            }
        }
        {
            s32 f = *ps;
            s32 r;
            s32 a3v;
            s32 d;
            Combatant *pD;
            if ((f & AYA_CONFUSION_MASK) == AYA_CONFUSION_IMMUNE) {
                *ps = f & ~AYA_CONFUSION_MASK;
                break;
            }
            d = g_FieldMoveLock;
            *ps = (f & ~AYA_CONFUSION_MASK) | AYA_CONFUSION;
            g_FieldMoveLock = d | FIELD_MOVE_CONFUSED;
            r = rand();
            pD = g_ActiveActor;
            a3v = r;
            if (r < 0) {
                a3v = r + 3;
            }
            pD->statusTimer42 = 0x2328;
            pD->stateFlags = (pD->stateFlags & ~AYA_CONFUSION_ANGLE_MASK) | (((r - ((a3v >> 2) << 2)) & 3) << 17);
        }
        break;
    case ENEMY_EFFECT_DARKNESS:
        if ((*ps & AYA_DARKNESS_MASK) == AYA_DARKNESS) {
            break;
        }
        if (g_ActiveActor->attributes->effectFlags & EFFECT_RESIST_DARKNESS) {
            if ((rand() % 100) < AYA_DARKNESS_RESIST_CHANCE) {
                break;
            }
        }
        {
            s32 f = *ps;
            Combatant *pD;
            if ((f & AYA_DARKNESS_MASK) == AYA_DARKNESS_IMMUNE) {
                *ps = f & ~AYA_DARKNESS_MASK;
                break;
            }
            pD = g_ActiveActor;
            *ps = (f & ~AYA_DARKNESS_MASK) | AYA_DARKNESS;
            pD->statusTimer44 = 0x2328;
        }
        break;
    case ENEMY_EFFECT_STIFFNESS_LOCK:
        if ((*ps & AYA_STIFFNESS_MASK) == AYA_STIFFNESS_LOCK) {
            break;
        }
        if (g_ActiveActor->attributes->effectFlags & EFFECT_RESIST_STIFFNESS) {
            if ((rand() % 100) < AYA_STIFFNESS_LOCK_RESIST_CHANCE) {
                break;
            }
        }
        {
            s32 f = *ps;
            if ((f & AYA_STIFFNESS_MASK) == AYA_STIFFNESS_IMMUNE) {
                *ps = f & ~AYA_STIFFNESS_MASK;
                break;
            }
            if (f & AYA_HASTE) {
                *ps = f & ~AYA_HASTE;
            }
            {
                s32 nf = (*ps & ~AYA_STIFFNESS_MASK) | AYA_STIFFNESS_LOCK;
                Combatant *pD = g_ActiveActor;
                *ps = nf;
                pD->statusTimer46 = 0x2328;
            }
        }
        break;
    case ENEMY_EFFECT_STIFFNESS_SLOW:
        {
            s32 m = *ps & AYA_STIFFNESS_MASK;
            if (m == AYA_STIFFNESS_LOCK) {
                break;
            }
            if (m == AYA_STIFFNESS_SLOW) {
                break;
            }
        }
        if (g_ActiveActor->attributes->effectFlags & EFFECT_RESIST_STIFFNESS) {
            if ((rand() % 100) < AYA_STIFFNESS_SLOW_RESIST_CHANCE) {
                break;
            }
        }
        {
            s32 f = *ps;
            if ((f & AYA_STIFFNESS_MASK) == AYA_STIFFNESS_IMMUNE) {
                *ps = f & ~AYA_STIFFNESS_MASK;
                break;
            }
            if (f & AYA_HASTE) {
                *ps = f & ~AYA_HASTE;
            }
            {
                s32 nf = (*ps & ~AYA_STIFFNESS_MASK) | AYA_STIFFNESS_SLOW;
                Combatant *pD = g_ActiveActor;
                *ps = nf;
                pD->statusTimer46 = 0x2328;
            }
        }
        break;
    case 7:
        *ps = *ps | 0x1000;
        break;
    case ENEMY_EFFECT_HALVE_HP:
        if (!(*ps & AYA_BARRIER)) {
            Combatant *pD = g_ActiveActor;
            s32 tmp = pD->curHP << 16;
            if ((tmp >> 16) >= 2) {
                *(s16 *)&pD->curHP = tmp >> 17;
            }
        }
        break;
    case ENEMY_EFFECT_REDUCE_HP_TO_1:
        {
            Combatant *pD = g_ActiveActor;
            s32 hp;
            s32 f;
            g_ActorEffectFlag100Timer = 0;
            hp = *(s16 *)&pD->curHP;
            pD->stateFlags = pD->stateFlags & ~AYA_HASTE;
            f = ((volatile Combatant *)pD)->stateFlags;
            f &= ~AYA_BARRIER;
            f &= ~AYA_GENE_HEAL;
            f &= ~AYA_PRERAISE;
            pD->stateFlags = f;
            if (hp >= 2) {
                *(s16 *)&pD->curHP = 1;
            } else if (hp == 1) {
                *(s16 *)&pD->curHP = -1;
            }
        }
        break;
    case ENEMY_EFFECT_STEAL_ITEM:
    case ENEMY_EFFECT_STEAL_ITEM_ALT:
        if (g_ActiveActor->attributes->effectFlags & EFFECT_RESIST_STEAL) {
            s32 r = rand();
            if ((r % 100) >= ENEMY_STEAL_RESIST_CHANCE) {
                ent->lootItemId = Inv_PickRandomItem(r / 100);
                ent->effect->effectType = 0;
                Inv_FindItemById(ent->lootItemId);
                {
                    s32 it = ent->lootItemId;
                    g_BattleSaveOverlayActive = 1;
                    g_CurItemEffectData = Inv_GetItemEffectData(it, 0);
                }
            }
        }
        break;
    case ENEMY_EFFECT_STEAL_AMMO:
    case ENEMY_EFFECT_STEAL_AMMO_ALT:
        if (g_ActiveActor->attributes->effectFlags & EFFECT_RESIST_STEAL) {
            if ((rand() % 100) >= ENEMY_STEAL_RESIST_CHANCE) {
                Inv_RollRandomItemType(&ent->lootItemId,
                                       &ent->lootItemAux);
                ent->effect->effectType = 0;
                {
                    s32 it = ent->lootItemId;
                    g_BattleSaveOverlayActive = 1;
                    g_CurItemEffectData = Inv_GetItemEffectData(it, 0);
                }
            }
        }
        break;
    case ENEMY_EFFECT_HP_TO_THREE_QUARTERS:
        if (!(*ps & AYA_BARRIER)) {
            Combatant *pD = g_ActiveActor;
            s16 t = *(s16 *)&pD->curHP;
            if (t >= 2) {
                *(s16 *)&pD->curHP = (t * 3) / 4;
            }
        }
        break;
    }
}

void Battle_SetupEnemyAnims(void) {
    EnemyAnimTable animTable;
    EnemyOffenseScaleTable scaleTable;
    BattleAction *action;
    Combatant *combatant;
    u8 *animRow;
    unsigned int actionId;

    animTable = g_BattleActionAnimRows;
    scaleTable = g_BattleActionOffenseScale;

    combatant = g_ActiveActor;
    action = combatant->action;
    actionId = action->actionCode.bytes.actionIdByte;
    combatant->scaledOffense =
        (combatant->stat22 * scaleTable.values[actionId]) / 10;
    combatant->actionMode12 = 4;
    g_ActiveActor->actionMode13 = 5;
    g_ActiveActor->targetAngleActionModes[0] = 6;
    g_ActiveActor->followUpAngleActionModes[0] = 7;
    g_ActiveActor->targetAngleActionModes[1] = 8;
    g_ActiveActor->followUpAngleActionModes[1] = 9;
    g_ActiveActor->targetAngleActionModes[2] = 10;
    g_ActiveActor->followUpAngleActionModes[2] = 11;

    animRow = animTable.rows[actionId];
    g_ActiveActor->action->animMode[0] = animRow[0];
    g_ActiveActor->action->animMode[1] = animRow[1];
    g_ActiveActor->action->animMode[2] = animRow[2];
    g_ActiveActor->action->animMode[3] = animRow[3];
    g_ActiveActor->action->hitTrigger = animRow[4];
    Scene_SetStoryDay(g_ActiveActor->action->actionCode.actionId);
}
