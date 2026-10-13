#include "pe1/battle_cmd.h"
#include "pe1/inventory.h"
#include "pe1/inventory_slots.h"
#include "pe1/menu_inventory.h"

s16 *g_InvItemPtr;
s32 g_InvPackedListCount;
s32 g_InvSlotLimit;

static inline ItemDataRecord *LookupTrackedItem(int index) {
    int value;
    int saved;
    if (index < 0 || index >= g_InvSlotLimit) return 0;
    value = g_InvItemPtr[index];
    saved = (s16)value;
    if ((unsigned)(value - 0x100) < 0x80)
        return &g_InventoryRuntime.equipment[value - 0x100];
    if ((unsigned)(value - 1) < 0xFF)
        return Item_LookupBaseData(value - 1);
    if ((unsigned)(saved - 0x200) < 9) {
        int shifted = saved << 5;
        return (ItemDataRecord *)(g_KeyItemDataTable + shifted);
    }
    return 0;
}
static inline int GetFilteredSlot(int index) {
    int result;
    if (index >= 0 && index < g_InvPackedListCount)
        result = g_InvPackedList[index];
    else result = 0;
    return result;
}
void Menu_OpenItemUsePanelAtIndex(int index) {
    BattleCmdEntry *command;
    int slot;
    command = BattleCmd_AllocSlot();
    command->header.word = 2;
    command->payload.equip_restore.item_data = LookupTrackedItem(g_InventoryRuntime.tracked[0]);
    slot = GetFilteredSlot(index);
    g_InventoryRuntime.tracked[0] = GetFilteredSlot(index);
    Inv_SetActiveList(2, &slot);
}
