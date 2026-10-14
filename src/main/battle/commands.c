#include "common.h"
#include "pe1/aya.h"
#include "pe1/battle.h"
#include "pe1/battle_cmd.h"
#include "pe1/battle_entity_anim.h"
#include "pe1/battle_modifiers.h"
#include "pe1/battle_runtime.h"
#include "pe1/inventory.h"
#include "pe1/inventory_queries.h"
#include "pe1/inventory_slots.h"
#include "pe1/menu_state.h"
#include "pe1/menu_widget.h"
#include "pe1/player_entity.h"
#include "pe1/weapon_list_output.h"

/* COMMON supplies small-data metadata to MASPSX: these words are reached
 * through $gp, every other extern stays absolute. */
BattleCmdEntry *g_BattleCmdStackTop;
u8 *g_MenuActiveListTarget;
int g_MenuCommandResult;

void BattleCmd_InitTableCursor(void) {
    g_BattleCmdStackTop = g_BattleCmdStack;
}

BattleCmdEntry *BattleCmd_AllocSlot(void) {
    BattleCmdEntry *top;
    BattleCmdEntry *end;
    BattleCmdEntry *limit;
    BattleCmdEntry *next;

    top = g_BattleCmdStackTop;
    end = (BattleCmdEntry *)g_StatScaleBase;
    if (top < end) {
        next = top + 1;
        g_BattleCmdStackTop = next;
    } else {
        top = end - 4;
        end = end - 1;
        if (top < end) {
            limit = end;
            do {
                BattleCmdEntry *src = top + 1;
                *top = *src;
                top += 1;
            } while (top < limit);
        }
    }
    return top;
}

void BattleCmd_UndoPending(void) {
    BattleCmdEntry *entry;
    BattleCmdEntry *entry_tmp;
    BattleCmdEntry *top;
    int opcode;
    s16 *slot;
    ItemDataRecord *data;
    int index;

    top = g_BattleCmdStackTop;
    if (g_BattleCmdStack < top) {
        entry_tmp = top - 1;
        opcode = (top - 1)->header.word;
        g_BattleCmdStackTop = entry_tmp;
        entry = entry_tmp;

        switch (opcode) {
        case 0:
            slot = &g_AyaInventoryItems[entry->payload.inventory_restore.slot_index];
            if (*slot == 0) {
                *slot = entry->payload.inventory_restore.item_id;
            } else {
                Inv_AddItem(entry->payload.inventory_restore.item_id);
            }
            break;

        case 1:
            break;

        case 2:
            g_InvTrackedSlots[0] =
                Inv_FindIndexByData(entry->payload.equip_restore.item_data);
            break;

        case 3:
            data = entry->payload.equip_restore.item_data;
            index = -1;
            if (data != 0) {
                index = Inv_FindIndexByData(data);
            }
            g_AyaEquippedArmorSlot[0] = index;
            break;

        case 4:
            ((ItemDataRecord *)entry->payload.ammo_restore.item_data0)->ammo =
                entry->payload.ammo_restore.ammo0;
            ((ItemDataRecord *)entry->payload.ammo_restore.item_data1)->ammo =
                entry->payload.ammo_restore.ammo1;
            ((ItemDataRecord *)entry->payload.ammo_restore.item_data2)->ammo =
                entry->payload.ammo_restore.ammo2;
            break;
        }
    }
}

void BattleCmd_ResetTableCursor(void) {
    g_BattleCmdStackTop = g_BattleCmdStack;
}

void BattleCmd_UndoAllPending(void) {
    if (g_BattleCmdStack < g_BattleCmdStackTop) {
        BattleCmdEntry *base = g_BattleCmdStack;

        do {
            BattleCmd_UndoPending();
        } while (base < g_BattleCmdStackTop);
    }
}

/* Active-list commands from the item menus and the pending command result
 * (g_MenuCommandResult) that the menu loop polls and clears. */

void Battle_DispatchSpecialAction(int);

void Inv_SetActiveList(int mode, int *slot) {
    Combatant *active;
    int *selection;
    BattleEntity *entity;
    int value;
    int present;
    selection = slot;
    entity = D_8009D254;
    active = 0;
    if (entity) {
        value = (int)entity->core;
        present = value != 0;
            active = (Combatant *)(value & -present);
    }
    switch (mode) {
    case 0:
        g_MenuCommandResult = *selection + 3;
        break;
    case 1:
        g_MenuCommandResult = *selection + 0x183;
        break;
    case 2:
        if (active) {
            u8 *out = g_MenuActiveListTarget;
            if (out) {
                *(BattleAction *)out = *(BattleAction *)(active->action);
                Inv_BuildWeaponList(0, (WeaponListOutput *)g_MenuActiveListTarget);
            }
            if (Menu_GetBattleEquipMode()) g_MenuCommandResult = 0x197;
            else Battle_DispatchSpecialAction(0x197);
        }
        break;
    case 3:
        if (active) {
            u8 *out = g_MenuActiveListTarget;
            if (out) {
                *(BattleAttributes *)out = *active->attributes;
                Inv_BuildArmorList((BattleAttributes *)g_MenuActiveListTarget);
            }
            if (Menu_GetBattleEquipMode()) g_MenuCommandResult = 0x198;
            else Battle_DispatchSpecialAction(0x198);
        }
        break;
    case 5:
        if (active) {
            BattleAction *data = active->action;
            ItemDataRecord *item = *(ItemDataRecord **)selection;
            u32 word = data->attackWord;
            int kind;
            int encoded;
            u32 flags2;
            encoded = -0x400;
            word = (word & encoded) | (item->ammo & 0x3FF);
            data->attackWord = word;
                    item = *(ItemDataRecord **)selection;
            kind = item->kind;
            data = active->action;
            if (kind != 0 && (unsigned)kind < 8) {
                encoded = kind - 4;
                if (encoded <= 0) encoded = 1;
            } else {
                            item = *(ItemDataRecord **)selection;
                kind = item->kind;
                encoded = 0;
                if ((unsigned)kind >= 19) encoded = kind - 18;
            }
            flags2 = data->attackWord;
            flags2 &= 0xFFCFFFFF;
            flags2 |= (u32)(encoded & 3) << 20;
            data->attackWord = flags2;
        }
        break;
    case 8: g_MenuCommandResult = 0x199; break;
    case 9: g_MenuCommandResult = -1; break;
    case 10: g_MenuCommandResult = 1000; break;
    case 11: g_MenuCommandResult = 1; break;
    case 12:
        g_InvBonusSlotCount = 4;
        Inv_SelectActiveList(0);
        BattleCmd_LoadWeaponModifiers();
        g_MenuCommandResult = 2;
        break;
    default: break;
    }
    if (g_MenuCommandResult) MenuWidget_InitPool();
}

int Menu_GetCommandResult(void) {
    return g_MenuCommandResult;
}

void Menu_ClearCommandResult(void) {
    g_MenuCommandResult = 0;
}

/* g_PlayerEntity points at the player-side field actor; its core is the
 * Combatant record used by the command paths below. */

void BattleCmd_SyncActiveAmmo(void) {
    BattleEntity *entity;
    Combatant *current;
    ItemDataRecord *entry;
    int saved;

    entity = (BattleEntity *)g_PlayerEntity;
    if (entity != 0) {
        current = entity->core;
        if (current != 0) {
            g_AyaHpCurrent[0] = current->curHP;
            if (current->action != 0) {
                saved = Inv_IsActiveListOverrideSelected();
                Inv_SelectActiveList(0);
                entry = Inv_LookupActiveListData(g_AyaEquippedWeaponSlot[0]);
                if (entry != 0) {
                    entry->ammo = current->action->attackWord & 0x3FF;
                }
                Inv_SelectActiveList(saved);
            }
        }
    }
}

void BattleCmd_SetCurrentHP(int arg0) {
    Combatant *current;

    if (g_PlayerEntity != 0) {
        current = ((BattleEntity *)g_PlayerEntity)->core;
        if (current != 0) {
            current->curHP = arg0;
        }
    }

    g_AyaHpCurrent[0] = arg0;
}

/* MASPSX_FORCE_G0 keeps the player pointer absolute, as retail; the COMMON
 * stack top supplies the small-data metadata of the one $gp-relative word. */
BattleCmdEntry *g_BattleCmdStackTop;

int BattleCmd_GetRemainingAmmo(int *out) {
    BattleCmdEntry *cursor;
    BattleEntity *entity;
    Combatant *current;
    int remaining = 0;

    entity = (BattleEntity *)g_PlayerEntity;
    if (entity != 0) {
        if (entity->core != 0) {
            current = entity->core;
            remaining = (s16)(current->exp_or_acc >> 16);
            cursor = g_BattleCmdStack;

            while (cursor < g_BattleCmdStackTop) {
                /* retail compares the whole first word, not the opcode byte */
                if (cursor->header.word == 1) {
                    remaining -= cursor->payload.ammo_spend.amount;
                }
                cursor++;
            }

            if (out != 0) {
                *out = (s16)(current->maxAtk >> 16);
            }
        }
    }

    return remaining;
}

void BattleCmd_SetCurrentMP(int arg0) {
    Combatant *current;

    if (g_PlayerEntity != 0) {
        current = ((BattleEntity *)g_PlayerEntity)->core;
        if (current != 0) {
            current->exp_or_acc = arg0 << 16;
        }
    }
}

void BattleCmd_CommitAndSyncAmmo(int item) {
    BattleEntity *entity;
    Combatant *current;
    ItemDataRecord *entry;
    int saved;

    Battle_ApplyDamage(item);
    entity = (BattleEntity *)g_PlayerEntity;
    if (entity != 0) {
        current = entity->core;
        if (current != 0) {
            g_AyaHpCurrent[0] = current->curHP;
            if (current->action != 0) {
                saved = Inv_IsActiveListOverrideSelected();
                Inv_SelectActiveList(0);
                entry = Inv_LookupActiveListData(g_AyaEquippedWeaponSlot[0]);
                if (entry != 0) {
                    entry->ammo = current->action->attackWord & 0x3FF;
                }
                Inv_SelectActiveList(saved);
            }
        }
    }
}

/* Applies a weapon change, then writes the loaded ammo back to the item. */
void BattleCmd_ChangeWeaponAndSync(int arg0) {
    Combatant *current;
    ItemDataRecord *entry;
    int saved;

    Battle_ApplySpellEffect(arg0, D_8009D254);
    if (D_8009D254 != 0) {
        current = D_8009D254->core;
        if (current != 0) {
            g_AyaHpCurrent[0] = current->curHP;
            if (current->action != 0) {
                saved = Inv_IsActiveListOverrideSelected();
                Inv_SelectActiveList(0);
                entry = Inv_LookupActiveListData(g_AyaEquippedWeaponSlot[0]);
                if (entry != 0) {
                    entry->ammo = current->action->attackWord & 0x3FF;
                }
                Inv_SelectActiveList(saved);
            }
        }
    }
}

int Aya_HasParasiteSpell(int spell)
{
    return (g_AyaParasiteSpellFlags[0] >> spell) & 1;
}

void Aya_UnlockParasiteSpell(int spell) {
    g_AyaSaveState.parasite_spell_flags |= 1 << spell;
}

int Math_IntSqrt(int value) {
    int result = 0;
    int shift = 30;

    do {
        int step = ((result << 2) + 1) << shift;

        result <<= 1;
        if (value >= step) {
            value -= step;
            result |= 1;
        }

        shift -= 2;
    } while (shift >= 0);

    return result;
}

int BattleCmd_CommitAmmoAndUpdate(void *out) {
    Combatant *current;
    ItemDataRecord *entry;
    int saved;
    int result;

    if (D_8009D254 != 0) {
        current = D_8009D254->core;
        if (current != 0) {
            g_AyaSaveState.current_hp = current->curHP;
            if (current->action != 0) {
                saved = Inv_IsActiveListOverrideSelected();
                Inv_SelectActiveList(0);
                entry = Inv_LookupActiveListData(g_InvTrackedSlots[0]);
                if (entry != 0) {
                    entry->ammo = current->action->attackWord & 0x3FF;
                }
                Inv_SelectActiveList(saved);
            }
        }
    }

    result = Inv_DrawSlotItemIcon();
    Inv_BuildWeaponList(0, out);
    return result;
}

int Battle_GetScaledMaxHP(int level) {
    unsigned short maxHp;

    maxHp = Aya_LookupLevelStats(level)->hp;
    return ((g_StatScaleBase[0] + 0x14) * maxHp) / 20;
}
