#include "pe1/inventory_slots.h"
#include "pe1/inventory_state.h"
#include "pe1/menu_fade_state.h"

static inline void SelectAya(void) {
    g_InvItemPtr = g_AyaSaveState.inventory_items;
    g_InvSlotLimit = Inv_GetAyaSlotLimit();
    g_InvSelectionBits = g_InvSelectionBitStorage;
    g_InvSelectionBitWords = 2;
}

/* Carry a tagged record forward, removing its first storage-list reference. */
static inline int TakeTagged(int armor) {
    ItemDataRecord *p;
    int id = 0;
    for (p = g_InventoryRuntime.equipment; p < g_InventoryRuntime.equipment + 128; p++) {
        if (p->iconId && (armor ? p->kind == 9 : p->kind != 9) && (p->flags & 16)) break;
    }
    if (p < g_InventoryRuntime.equipment + 128) {
        s16 *slot;
        id = (p - g_InventoryRuntime.equipment) + 256;
        for (slot = g_InventoryRuntime.specialStorage;
             slot < g_InventoryRuntime.specialStorage + 82; slot++)
            if (*slot == id) break;
        if (slot < g_InventoryRuntime.specialStorage + 82) *slot = 0;
    }
    return id;
}

void Inv_InitNewGameInventory(void) {
    int i, capacity;
    AyaLevelStats *stats;
    u16 *growth = g_AyaSaveState.stats.levels;
    for (i = 0; i < 7; i++) *growth++ = *(u16 *)Aya_GetStatGrowthTable(i);
    stats = Aya_LookupLevelStats(0);
    g_AyaSaveState.max_hp = g_AyaSaveState.current_hp = stats->hp;
    g_AyaSaveState.inventory_slot_count = stats->inventoryCapacity;
    g_AyaSaveState.parasite_spell_flags = 1;
    g_InvSpecialItemRecords[0].ammo = g_InvSpecialItemRecords[1].ammo = g_InvSpecialItemRecords[2].ammo = 0;
    SelectAya();
    capacity = stats->inventoryCapacity;
    if (capacity > 50) capacity = 50;
    g_AyaSaveState.inventory_slot_count = capacity;
    if (g_InvItemPtr == g_AyaSaveState.inventory_items)
        g_InvSlotLimit = Inv_GetAyaSlotLimit();
    SelectAya();
    for (i = 49; i >= 0; i--) g_InvItemPtr[i] = 0;
    Inv_AddItem(0x44);
    Inv_AddItem(0x96);
    Inv_AddItem(0x3F);
    Inv_AddItem(1);
    Inv_AddItem(6);
    i = TakeTagged(0);
    if (i) Inv_AddItem(i);
    i = TakeTagged(1);
    if (i) Inv_AddItem(i);
    g_InventoryRuntime.tracked[2] = 1;
    g_InventoryRuntime.tracked[0] = 0;
    g_AyaSaveState.menu_clamp_value = 61;
    Menu_ClampRange(61);
    g_AyaSaveState.total_exp = 0;
    g_AyaSaveState.level = 0;
    Menu_SaveBgInitFade();
}

