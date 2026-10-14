#include "pe1/aya.h"
#include "pe1/inventory_slots.h"
#include "pe1/inventory_queries.h"

s16 *g_InvItemPtr;
s32 g_InvSlotLimit;
int g_InvBonusSlotCount;

int Inv_GetBonusSlotCount(void) {
    return g_InvBonusSlotCount;
}

void Inv_SetAyaSlotCount(int count) {
    if (count >= 0x33) {
        count = 0x32;
    }

    g_AyaSaveState.inventory_slot_count = count;
    if (g_InvItemPtr == g_AyaSaveState.inventory_items) {
        g_InvSlotLimit = Inv_GetAyaSlotLimit();
    }
}

int Inv_GetAyaSlotLimit(void) {
    if (g_AyaSaveState.inventory_slot_count + Inv_GetBonusSlotCount() >= 0x33) {
        return 0x32;
    }

    return g_AyaSaveState.inventory_slot_count + Inv_GetBonusSlotCount();
}

