#ifndef PE1_MENU_ITEM_LIST_INPUT_H
#define PE1_MENU_ITEM_LIST_INPUT_H

/* Item list, skill screen and inventory page input declarations. */

#include "pe1/inventory.h"
#include "pe1/menu_confirm_callback.h"
#include "pe1/menu_equipment.h"
#include "pe1/menu_inventory.h"

int Menu_ItemListInputHandler(MenuWidgetNode *node, unsigned int flags);
int Menu_InventoryPageInputHandler(MenuWidgetNode *root, unsigned int flags);
int Menu_StepSkillScreen(MenuWidgetNode *node, int flags);
void Menu_DrawUsableItemActionList(MenuWidgetNode *node);
void Menu_DrawSoundTestList(MenuWidgetNode *node);
void Menu_DrawItemListInvPanel(MenuWidgetNode *node);
void Menu_SetupSkillSubmenu(MenuWidgetNode *node);
void Menu_StepSkillList(MenuWidgetNode *panel, int refresh);
void Menu_OpenRenameScreen(int item);
void Menu_ReopenEquipScreen(void);
int Menu_SkillActionHandler(MenuWidgetNode *node, unsigned int flags);
int Menu_GetItemSlotAvailability(int index);

/* Bonus point allocation view is open. */
extern int g_MenuBattleStatusOverlayActive;
/* Pending stat bonus: item index (negative when none), stat and amount. */
extern int g_MenuSourceItemList;
/* Copy of the item record being modified. */
extern ItemDataRecord g_MenuItemUseSelectedItemSnapshot;

#endif
