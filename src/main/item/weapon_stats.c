#include "pe1/inventory_slots.h"
#include "pe1/weapon_list_output.h"

/* Original USA effect-count data, formerly at 0x800923D0. */
u8 g_WeaponEffectCodes[6] = {1,2,3,5,7,10};

static inline int ClampStat(ItemDataRecord *item, int which) {
    int value = item->baseStats[which] + item->bonusStats[which];
    int result = 999;
    if (value < 1000) result = value;
    return result;
}
void Inv_BuildWeaponList(int unused, WeaponListOutput *out) {
    ItemDataRecord *item;
    int i, kind, code, root, shift, trial, radicand;
    int product;
    Inv_SelectActiveList(0);
    item = Inv_LookupActiveListData(g_InventoryRuntime.tracked[0]);
    if (item->baseStats[0] + item->bonusStats[0] >= 1000) out->attack = 999;
    else out->attack = item->baseStats[0] + item->bonusStats[0];
    {
        int base2 = item->baseStats[1];
        int bonus2 = item->bonusStats[1];
        if (base2 + bonus2 < 1000) product = (base2 + bonus2) * 22500;
        else product = 0x156fabc;
        radicand = product;
    }
    root = 0;
    for (shift = 30; shift >= 0; shift -= 2) {
        trial = ((root << 2) + 1) << shift;
        root <<= 1;
        if (radicand >= trial) { radicand -= trial; root |= 1; }
    }
    out->range = root;
    out->kind = item->kind;
    out->packed.bits.count = item->ammo;
    out->packed.bits.power = ClampStat(item, 2);
    kind = item->kind;
    if (kind != 0 && (unsigned)kind < 8) {
        code = kind - 4;
        if (code <= 0) {
            code = 1;
        }
    } else {
        code = 0;
        if ((unsigned)kind >= 19) code = kind - 18;
    }
    out->effects.word = 0x11;
    out->packed.bits.category = code;
    for (i = 0; i < item->tailCount; i++) {
        int effect = item->tailData[i] & 31;
        switch (effect) {
        case 1: case 2: case 3: case 4: case 5:
            out->effects.bits.actions = g_WeaponEffectCodes[effect];
            break;
        case 6: case 7: case 8:
            out->effects.bits.mode = effect - 5;
            break;
        case 9: out->effects.bits.element8 = 1; break;
        case 10: out->effects.bits.element9 = 1; break;
        case 11: out->effects.bits.element10 = 1; break;
        case 12: out->effects.bits.element11 = 1; break;
        case 13: out->effects.bits.element12 = 1; break;
        case 14: out->effects.bits.flag15 = 1; break;
        case 15: out->effects.bits.flag16 = 1; break;
        case 16: case 17:
            out->effects.bits.hitState = effect - 15;
            break;
        case 18: out->effects.bits.flag17 = 1; break;
        case 19: case 20:
            out->effects.bits.slot = effect - 17;
            break;
        }
    }
    if (out->effects.bits.hitState == 1) out->attack = (s16)out->attack >> 1;
}

