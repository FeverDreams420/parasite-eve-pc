#include "pe1/inventory_slots.h"

u32 *g_InvSelectionBits;
s32 g_InvSelectionBitWords;
s16 *g_InvActiveListOverride;
s32 g_InvOverrideSlotLimit;
u32 g_InvSelectionBitStorage[2];
u32 g_InvStorageSelectionBits[4];

void Inv_SelectActiveList(int useOverride) {
    if (useOverride != 0 && g_InvActiveListOverride != 0) {
        g_InvItemPtr = g_InvActiveListOverride;
        g_InvSelectionBits = g_InvStorageSelectionBits;
        g_InvSelectionBitWords = 4;
        g_InvSlotLimit = g_InvOverrideSlotLimit;
    } else {
        g_InvItemPtr = g_AyaInventoryItems;
        g_InvSlotLimit = Inv_GetAyaSlotLimit();
        g_InvSelectionBits = g_InvSelectionBitStorage;
        g_InvSelectionBitWords = 2;
    }
}

void Inv_SetActiveListOverride(s16 *items, int count) {
    g_InvActiveListOverride = items;
    g_InvOverrideSlotLimit = count;
}

void Inv_ResetActiveList(void) {
    g_InvActiveListOverride = 0;
    g_InvOverrideSlotLimit = 0;
    g_InvItemPtr = g_AyaInventoryItems;
    g_InvSlotLimit = Inv_GetAyaSlotLimit();
    g_InvSelectionBits = g_InvSelectionBitStorage;
    g_InvSelectionBitWords = 2;
}

int Inv_IsActiveListOverrideSelected(void) {
    return g_InvItemPtr != g_AyaInventoryItems;
}

