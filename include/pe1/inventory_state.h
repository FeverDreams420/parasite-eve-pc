#ifndef PE1_INVENTORY_STATE_H
#define PE1_INVENTORY_STATE_H

#include "common.h"
#include "pe1/save_blob.h"

/* Inventory list, sort and selection state shared by src/main/item. */

typedef struct InventorySlotRange {
    s16 start;
    s16 stop;
} InventorySlotRange;

extern int g_InvBonusSlotCount;
extern s32 g_InvWeaponSortStatIndex;
extern u16 *g_InvSortListBase;
extern s32 g_InvSortListCount;
extern s32 g_InvAmmoSortStatIndex;
extern unsigned char g_InvSortRankTableA[];
extern unsigned char g_InvSortRankTableC[];
extern unsigned char g_InvSortRankTableD[];
extern unsigned char g_InvTypeRankTableA[];
extern unsigned char g_InvTypeRankTableB[];
extern unsigned char g_InvSortRankTableB[];
#define g_InvItemArray (g_InventoryRuntime.specialStorage)
extern u8 g_ItemCodeRemapTable[];
extern unsigned char g_ItemDropRngTable[];
extern s16 g_InvActiveListItems[10];
extern int g_InvActiveListCount;
extern s16 g_BattleCountTable[];
extern int g_MenuBattleCount;
extern s32 g_InvCategoryBaseItemId;
extern InventorySlotRange g_InvItemTableRanges[];

#endif /* PE1_INVENTORY_STATE_H */
