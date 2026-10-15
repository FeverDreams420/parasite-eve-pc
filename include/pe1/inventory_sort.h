#ifndef PE1_INVENTORY_SORT_H
#define PE1_INVENTORY_SORT_H

#include "pe1/inventory_slots.h"

extern int g_InvSortStatIndex;
extern ItemDataRecord *(*g_InvSortLookup)(int index);
extern unsigned char *g_InvSortRankTable;
extern unsigned char *g_InvSortTypeRankTable;

int Inv_CompareItemsByStatForSort(const void *leftElement, const void *rightElement);
int Inv_CompareItemsForSort(const void *leftElement, const void *rightElement);

void Inv_SortInventoryByMode(int mode, int key);
void Inv_SortContainerItems(int key);
void Inv_SortInventoryItems(int mode, int key);


#endif
