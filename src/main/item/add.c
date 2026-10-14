#include "pe1/inventory_slots.h"
#include "pe1/inventory_state.h"

static inline int FindFreeEquipmentRecord(void) {
    int result;
    ItemDataRecord *p = g_InventoryRuntime.equipment;
    ItemDataRecord *end = p + 128;

    if (p < end) {
        do {
            if (!p->iconId)
                break;
            p++;
        } while (p < end);
        if (p < g_InventoryRuntime.equipment + 128) {
            int index = p - g_InventoryRuntime.equipment;
            result = index;
        } else {
            result = -1;
        }
    } else {
        result = -1;
    }
    return result;
}

static inline int FindFreeActiveSlot(void) {
    s16 *p = g_InvItemPtr, *end = p + g_InvSlotLimit;

    if (p < end) {
        while (p < end && *p != 0)
            p++;
        if (p < g_InvItemPtr + g_InvSlotLimit)
            return p - g_InvItemPtr;
    }
    return -1;
}

ItemDataRecord *Inv_FindSlotByIndex(int id) {
    ItemDataRecord *result = 0;
    int record = FindFreeEquipmentRecord();
    int slot = FindFreeActiveSlot();

    if (record >= 0 && slot >= 0) {
        result = &g_InventoryRuntime.equipment[record];
        *result = *Item_LookupBaseData(id - 1);
        g_InvItemPtr = g_InventoryRuntime.slots;
        g_InvSlotLimit = Inv_GetAyaSlotLimit();
        g_InvSelectionBits = g_InvSelectionBitStorage;
        g_InvSelectionBitWords = 2;
        g_InvItemPtr[slot] = record + 256;
    }
    return result;
}

static inline int FindActiveSlot(int value) {
    s16 *p = g_InvItemPtr, *end = p + g_InvSlotLimit;
    if (p < end) {
        while (p < end && *p != value)
            p++;
        if (p < g_InvItemPtr + g_InvSlotLimit)
            return p - g_InvItemPtr;
    }
    return -1;
}
static inline void ClampAmmo(ItemDataRecord *item) {
    int current = item->ammo;
    if (item->baseStats[2] + item->bonusStats[2] < 1000 ?
        item->baseStats[2] + item->bonusStats[2] < current : 999 < current) {
        item->ammo = item->baseStats[2] + item->bonusStats[2] < 1000 ?
            item->baseStats[2] + (u16)item->bonusStats[2] : 999;
    }
}

/* Add loaded ammunition even when no empty inventory slot is available. */
int Inv_WriteSlotById(ItemDataRecord *item) {
    unsigned kind = item->kind;
    int failed = 0;
    int category;

    if (kind < 16) {
        if (kind && kind < 8)
            category = (int)(kind - 4) > 0 ? kind - 5 : 0;
        else
            category = kind >= 19 ? kind - 19 : -1;
    } else {
        category = kind - 16;
    }
    if ((unsigned)category < 3) {
        ItemDataRecord *pool;
        if (FindActiveSlot(category + 512) < 0) {
            int slot = FindFreeActiveSlot();
            if (slot >= 0)
                g_InvItemPtr[slot] = category + 512;
            else
                failed = 1;
        }
        pool = &g_InvSpecialItemRecords[category];
        pool->ammo += item->ammo;
        ClampAmmo(pool);
    } else {
        failed = 1;
    }
    return failed;
}

/* Insert an item into a free active slot. Non-zero means the insert failed. */
s32 Inv_AddItem(s32 arg0) {
    s32 ret;
    s32 slot;
    s32 t;
    s16 *cur;
    s16 *end;
    ItemDataRecord *ent;

    ret = 0;
    cur = g_InvItemPtr;
    end = g_InvItemPtr + g_InvSlotLimit;
    if (cur < end) {
        do {
            if (*cur == 0) {
                break;
            }
            cur++;
        } while (cur < end);
        if (cur < g_InvItemPtr + g_InvSlotLimit) {
            t = cur - g_InvItemPtr;
        } else {
            t = -1;
        }
    } else {
        t = -1;
    }
    slot = t;
    if (arg0 < 0x100) {
        ent = Item_LookupBaseData(arg0 - 1);
        if (ent == NULL) {
            return ret;
        }
        switch (ent->kind) {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
            ret = Inv_FindSlotByIndex(arg0) == 0;
            break;
        case 10:
        case 12:
        case 13:
        case 14:
        case 15:
            if (slot >= 0) {
                g_InvItemPtr[slot] = arg0;
            } else {
                ret = 1;
            }
            break;
        case 16:
        case 17:
        case 18:
            ret = Inv_WriteSlotById(ent);
            break;
        }
    } else {
        if (slot >= 0) {
            g_InvItemPtr[slot] = arg0;
        } else {
            ret = 1;
        }
    }
    return ret;
}
