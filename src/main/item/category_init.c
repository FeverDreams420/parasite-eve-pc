#include "pe1/inventory_slots.h"
#include "pe1/inventory_state.h"

int g_InvCategoryBaseItemId;

void Inv_BuildItemGridFromCategory(void)
{
    int i;
    ItemDataRecord *src;
    int category;
    int base;
    int lookup;

    Inv_SelectActiveList(0);

    for (i = 49; i >= 0; --i) g_AyaSaveState.inventory_items[i] = 0;

    i = 0;
    category = 0x13;
    do {
        lookup = i;
        i++;
        src = Item_LookupBaseData(lookup);
    } while (src != 0 && src->kind != category);

    g_InvCategoryBaseItemId = i;

    /* Nine grid cells cycle through the three records after the first
     * category match; each cell clears its third base stat and sets the
     * third bonus to the 999 placeholder. */
    i = 0;
    do {
        base = g_InvCategoryBaseItemId + i % 3;
        src = Item_LookupBaseData(base - 1);
        g_InvSpecialItemRecords[i] = *src;
        g_InvSpecialItemRecords[i].baseStats[2] = 0;
        g_InvSpecialItemRecords[i].bonusStats[2] = 999;
    } while (++i < 9);

    for (i = 99; i >= 0; --i) g_InventoryRuntime.storage[i] = 0;

    for (i = 81; i >= 0; --i) g_InventoryRuntime.specialStorage[i] = 0;

    g_InvActiveListOverride = 0;
}

