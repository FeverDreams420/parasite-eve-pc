#ifndef PE1_INVENTORY_SLOTS_H
#define PE1_INVENTORY_SLOTS_H

#include "common.h"
#include "pe1/inventory.h"
#include "pe1/inventory_queries.h"

#include "pe1/aya.h"
#define g_AyaInventoryItems (g_AyaSaveState.inventory_items)
/* Count of filtered slot indices in g_InvPackedList; distinct from the
 * pending item-ID list at g_InvActiveListItems (count g_InvActiveListCount). */
extern s32 g_InvPackedListCount;
extern s32 g_InvTransferItemCount;
extern s16 g_InvTransferItemList[];
extern s16 *g_InvActiveItemList;
extern s32 g_InvActiveListSlotCount;
extern s16 g_InvPackedList[];
/* Saved by inventory commands as a word; the frame loop reads its low byte. */
typedef union SavedMenuMode {
    u32 value;
    struct { u8 mode; u8 reserved[3]; } bytes;
} SavedMenuMode;
extern SavedMenuMode g_SavedMenuMode;

void Inv_SetActiveList(int mode, int *slot);
void Inv_ResetActiveList(void);
void Battle_UseItem(int item);
/* Moves ammo between the compared weapons; nonzero when it cannot. */
int Inv_SpendAmmo(int amount);
void Inv_InitMaxLevelInventory(int variant);
int Inv_CountTotal(void);
void Item_PackParasiteSpells(void);
int Inv_LoadWayneItemsAsOverride(short *items);
extern u8 g_InvBaseCapacity;

extern u32 g_InvSelectionBitStorage[];
extern s16 *g_InvItemPtr;
extern s32 g_InvSlotLimit;
extern u32 *g_InvSelectionBits;
extern s32 g_InvSelectionBitWords;
extern s32 g_InvPendingCompactCount;
extern s32 g_InvPackedListCursor;
/* Equipment comparison filter: nonzero selects kinds 1..8, zero kind 9; first-stat floor. */
extern s32 g_InvRebuildFilterMode, g_InvRebuildMinFirstStat;
/* Pending item IDs and the current number to transfer. */
extern s32 g_InvActiveListCount;
extern s16 g_InvActiveListItems[];
#define g_InvTrackedSlots g_InventoryRuntime.tracked
/* Two remembered selections. The retail indexed accesses use full addresses;
 * fixed-slot accesses use the individual small-data symbols below. Keeping
 * incomplete arrays avoids the old, overlapping three-element declarations. */
extern s32 g_InvSavedSelectionIndex[];
extern s32 g_InvSavedSelectionFromStorage[];
extern s32 g_InvSavedSelectionIndex0, g_InvSavedSelectionIndex1; /* indices 0, 1 */
extern s32 g_InvSavedSelectionFromStorage0, g_InvSavedSelectionFromStorage1; /* storage flags 0, 1 */
extern s16 *g_InvActiveListOverride;
extern s32 g_InvOverrideSlotLimit;
extern u32 g_InvStorageSelectionBits[];
/* Scratch state and full record snapshots used by the item-detail operation. */
extern u8 D_800A1FE8[];
extern u8 *D_8009D084;
extern s32 D_8009D088, D_8009D08C;
extern ItemDataRecord g_InvSavedSelectionRecord0, g_InvSavedSelectionRecord1;
/* Global fallback ammunition record, then the records of item values
 * 0x200..0x208 (the first three are the category ammunition pools);
 * storage tool IDs. */
extern ItemDataRecord g_InvFallbackAmmoRecord;
extern ItemDataRecord g_InvSpecialItemRecords[];
extern s16 g_WayneStorageItems[];

void Inv_InitWayneStorage(void);
void Inv_RememberSelection(unsigned int index, int value);
int Inv_RestoreSelection(unsigned int index);
void Inv_BuildStorageDisplay(void);

void Inv_SetActiveListOverride(s16 *items, int count);
int Inv_GetBonusSlotCount(void);
int Inv_FindFirstEmptySlot(int neededSlots);
void Inv_CompactActiveListSlots(void);
int Inv_CheckFreeSlotCapacity(int requestedSlots);

extern int g_InvCategoryBaseItemId; /* First base-item ID represented by the three ammo pools. */

int Inv_GetPackedListItem(int index);

int Menu_GetCommandResult(void);
void Menu_ClearCommandResult(void);

/* Item record of an inventory value: 0x100..0x17F equipment records,
 * 1..0xFF base item data, 0x200..0x208 the special item records at g_InvSpecialItemRecords
 * (32 bytes each). Retail units spell the key item address two ways: as a
 * record index, or scaled from the key item table base g_KeyItemDataTable. */
static inline ItemDataRecord *Inv_LookupItemRecord(int value) {
    int saved = value;
    ItemDataRecord *result;
    if (value - 0x100U < 0x80U) {
        result = &g_InventoryRuntime.equipment[value - 0x100];
    } else {
        if (value - 1U < 0xFFU) {
            result = Item_LookupBaseData(value - 1);
        } else if (saved - 0x200U < 9U) {
            result = &g_InvSpecialItemRecords[saved - 0x200];
        } else {
            result = 0;
        }
    }
    return result;
}

static inline ItemDataRecord *Inv_LookupItemRecordScaled(int value) {
    int saved = value;
    ItemDataRecord *result;
    if (value - 0x100U < 0x80U) {
        result = &g_InventoryRuntime.equipment[value - 0x100];
    } else {
        if (value - 1U < 0xFFU) {
            result = Item_LookupBaseData(value - 1);
        } else if (saved - 0x200U < 9U) {
            int shifted = saved << 5;
            result = (ItemDataRecord *)(g_KeyItemDataTable + shifted);
        } else {
            result = 0;
        }
    }
    return result;
}

#endif
