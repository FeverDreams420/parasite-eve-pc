#include "pe1/inventory_slots.h"
#include "pe1/inventory_state.h"
#include "pe1/battle_modifiers.h"
#include "pe1/stat_modifiers.h"

static inline int ClampStat(ItemDataRecord *item, int which) {
    int value = item->baseStats[which] + item->bonusStats[which];
    int result = 999;
    if (value < 1000) result = value;
    return result;
}

/* Restoring integer square root; negative inputs yield zero. */
static inline int Root(int value) {
    int root = 0, shift;
    for (shift = 30; shift >= 0; shift -= 2) {
        int trial = ((root << 2) + 1) << shift;
        root <<= 1;
        if (value >= trial) {
            value -= trial;
            root |= 1;
        }
    }
    return root;
}

void Inv_BuildArmorList(BattleAttributes *out) {
    ItemDataRecord *item;
    int value, i;
    out->effectFlags = 0;
    Inv_SelectActiveList(0);
    item = Inv_LookupActiveListData(g_InventoryRuntime.tracked[2]);
    if (item) {
        value = ClampStat(item, 0);
        out->parameterWord.fields.first = value;
        value = ClampStat(item, 1);
        out->parameterWord.fields.second = value;
        value = item->baseStats[2] + item->bonusStats[2];
        if (value >= 1000) value = 999;
        if (value < 85) value = Root(value * 3000) / 10;
        else value = (value * 249 / 208 + 402) / 10;
        out->parameterWord.fields.third = value;
        for (i = 0; i < item->tailCount; i++) {
            int effect = item->tailData[i] & 31;
            switch (effect) {
            case 1: out->effectFlags |= 1; break;
            case 2: out->effectFlags |= 2; break;
            case 3: out->effectFlags |= 4; break;
            case 4: out->effectFlags |= 8; break;
            case 5: out->effectFlags |= 16; break;
            case 6: out->effectFlags |= 0x4000; break;
            case 7: out->effectFlags |= 0x8000; break;
            case 8: case 9: case 10:
                out->effectFlags = (out->effectFlags & ~0x1E0u) |
                    (((1 << (effect - 8)) & 15) << 5);
                break;
            case 11: out->effectFlags |= 0x800; break;
            case 12: out->effectFlags |= 0x2000; break;
            case 13: out->effectFlags |= 0x10000; break;
            case 14: out->effectFlags |= 0x200; break;
            case 15: out->effectFlags |= 0x20000; break;
            case 16: out->effectFlags |= 0x400; break;
            case 17: out->effectFlags |= 0x1000; break;
            }
        }
    } else {
        out->parameterWord.fields.first = 0;
        out->parameterWord.fields.second = 0;
        out->parameterWord.fields.third = 0;
    }
    BattleCmd_LoadWeaponModifiers();
}

void BattleCmd_LoadWeaponModifiers(void)
{
    int saved = Inv_IsActiveListOverrideSelected();
    int i;
    ItemDataRecord *item;
    Inv_SelectActiveList(0);
    g_InvBonusSlotCount = 0;
    for (i = 6; i >= 0; --i) g_StatScaleBase[i] = 0;
    /* Historical function name notwithstanding, this is the armor index. */
    item = Inv_LookupActiveListData(g_AyaSaveState.equipped_armor_slot);
    if (item) {
        for (i = 0; i < item->tailCount; ++i) {
            unsigned mod = item->tailData[i] & 31;
            /* These are decoded low-five-bit values, not full ItemMod IDs. */
            switch (mod) {
            case 8: case 9: case 10: g_InvBonusSlotCount = 1 << (mod - 8); break;
            case 11: g_StatScaleBase[0] = 3; break;
            case 12: g_StatScaleBase[1] = 2; break;
            case 13: g_StatScaleBase[5] = -2; break;
            case 15: g_StatScaleBase[1] = -2; break;
            }
        }
    }
    Inv_RecalcSlotStats();
    Inv_SelectActiveList(0);
    Inv_SelectActiveList(saved);
}

