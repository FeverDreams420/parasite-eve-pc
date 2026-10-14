#include "pe1/inventory_slots.h"
#include "pe1/resident_tables.h"

/* Nine native key records replace the PS1 address biased by -0x4000. */
ItemDataRecord g_KeyItemRecords[9];

/* Inventory item-ID and active-slot lookup helpers. */

ItemDataRecord *Inv_LookupData(int arg0) {
    if ((arg0 - 0x100U) < 0x80U) {
        return &g_InventoryRuntime.equipment[arg0 - 0x100];
    }

    if ((arg0 - 1U) < 0xFFU) {
        return Item_LookupBaseData(arg0 - 1);
    }

    if ((arg0 - 0x200U) >= 9U) {
        return 0;
    }

    return &g_KeyItemRecords[arg0 - 0x200];
}


ItemDataRecord *Inv_LookupActiveListData(int index) {
    int value;
    int saved_value;
    ItemDataRecord *entry;

    if (index >= 0 && index < g_InvSlotLimit) {
        value = g_InvItemPtr[index];
        saved_value = value;
        if (value - 0x100U < 0x80U) {
            entry = &g_InventoryRuntime.equipment[value - 0x100];
        } else if (value - 1U < 0xFFU) {
            entry = Item_LookupBaseData(value - 1);
        } else if (saved_value - 0x200U < 9U) {
            entry = &g_KeyItemRecords[saved_value - 0x200];
        } else {
            entry = 0;
        }
    } else {
        entry = 0;
    }

    return entry;
}

