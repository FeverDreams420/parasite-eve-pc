#ifndef PE1_INVENTORY_STATE_H
#define PE1_INVENTORY_STATE_H

#include "common.h"
#include "pe1/save_blob.h"

/* Inventory list, sort and selection state shared by src/main/item. */

typedef struct InventorySlotRange {
    s16 start;
    s16 stop;
} InventorySlotRange;

extern signed char g_AyaEquippedArmorSlot[];
extern s16 D_800C0EAA[];
extern s16 D_800C1F7E[];
extern s16 D_800C2022[];
extern u8 D_800A1E6D[][32];
extern s16 D_800A1E76[][16];
extern int g_InvBonusSlotCount;
extern s32 D_8009D0A4;
extern u16 *g_InvSortListBase;
extern s32 g_InvSortListCount;
extern s32 g_InvAmmoSortStatIndex;
extern unsigned char g_InvSortRankTableA[];
extern unsigned char g_InvSortRankTableC[];
extern unsigned char g_InvSortRankTableD[];
extern unsigned char g_InvTypeRankTableA[];
extern unsigned char g_InvTypeRankTableB[];
extern unsigned char g_InvSortRankTableB[];
extern u16 g_InvItemArray[];
extern u8 g_ItemCodeRemapTable[];
extern unsigned char g_ItemDropRngXorKeyHigh[];
extern unsigned char g_ItemDropRngTable[];
extern unsigned char g_ItemDropRngXorKeyLow[];
extern u8 g_CursorRenderDataBlock[];
extern s16 g_InvActiveListItems[];
extern int g_InvActiveListCount;
/* Two 16-byte text windows the comparison panel and the save screen draw. */
extern SaveMetadataWindow g_CursorRenderMetadataWindows[2];
extern s16 g_BattleCountTable[];
extern int g_MenuBattleCount;
extern s32 g_InvCategoryBaseItemId;
extern InventorySlotRange g_InvItemTableRanges[];
extern short g_InvPackedList[];

#endif /* PE1_INVENTORY_STATE_H */
