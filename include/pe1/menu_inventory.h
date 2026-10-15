#ifndef PE1_MENU_INVENTORY_H
#define PE1_MENU_INVENTORY_H

#include "pe1/bounds_check.h"
#include "pe1/menu_sound.h"
#include "pe1/menu_state.h"
#include "pe1/menu_widget.h"

void Inv_OpenAyaItemList(unsigned int mode);

void MenuWidget_RestoreSavedCurrentNode(void);
int Menu_InventoryNavigate(MenuWidgetNode *current, MenuWidgetNode *node,
                           u32 flags);
void Menu_CreateBonusPointAllocationView(void);
void Menu_SetSwapReturnFlag(void);
void Menu_ProcessSwapReturnIfPending(void);
int Menu_HandleAyaInventorySelectionInput(MenuWidgetNode *node, unsigned int flags);

extern int g_InvItemUsableFlag, g_MenuInputPausePending;
void Menu_OpenItemUsePanelAtIndex(int index);
void Menu_OpenSkillSelectionView(void);
void Menu_OpenInventoryOrSwapView(int showList);
void Menu_ConfigureScreen(void);
void Menu_InitCategoryTabs(void);
void Menu_CreateAmmoSpendPanel(MenuWidgetNode *parent);
void Menu_CreateEquipScreen(MenuWidgetNode *parent);
void Menu_CreateSkillActionScreen(MenuWidgetNode *parent);
void Menu_CreateInventoryListView(MenuWidgetNode *parent);
void Menu_CreateEquipItemSelectionView(MenuWidgetNode *parent);
void Menu_CreateMainMenuView(MenuWidgetNode *parent);
void Menu_CreateInvSwapView(MenuWidgetNode *parent, int mode);
void Menu_InitScrollDirection(void);
void Menu_OpenItemList(void);
void Menu_CreateActionSubmenu(MenuWidgetNode *parent);
void Menu_OpenBlendColorScreen(MenuWidgetNode *parent);
void Menu_OpenScreenAdjustView(MenuWidgetNode *parent);
int Menu_StepItemSelectScreen(MenuWidgetNode *node, unsigned int buttons);
void Menu_DrawItemList(MenuWidgetNode *node);
void Menu_StepInventoryRoot(int mode, int index, int arg);
void Menu_CreateNotificationDialog(int message, int arg);
void MenuInput_SetPollingPaused(int paused);
void Menu_OnEquipConfirm(MenuWidgetNode *unused, int confirmed);

int Menu_ClampRange(int value);
void Menu_SaveBgInitFade(void);

/* Retail script command numbers. Command 1120 falls through and returns 0. */
enum InvCommand {
    INV_CMD_COUNT_OCCUPIED = 1100,
    INV_CMD_GET_CAPACITY = 1101,
    INV_CMD_COUNT_ITEM = 1102,
    INV_CMD_SET_REBUILD_FILTER = 1103,
    INV_CMD_REBUILD_SLOTS = 1104,
    INV_CMD_SET_CAPACITY = 1105,
    INV_CMD_GET_CURRENT_HP = 1106,
    INV_CMD_GET_MAX_HP = 1107,
    INV_CMD_SET_CURRENT_HP = 1108,
    INV_CMD_GET_REMAINING_AMMO = 1109,
    INV_CMD_GET_AMMO_VALUE = 1110,
    INV_CMD_SET_CURRENT_MP = 1111,
    INV_CMD_REMOVE_ITEM = 1112,
    INV_CMD_OPEN_STORAGE = 1113,
    INV_CMD_COMPUTE_GAMMA = 1114,
    INV_CMD_INIT_MEMCARD = 1115,
    INV_CMD_START_SAVE_FADE = 1116,
    INV_CMD_START_NEW_GAME = 1117,
    INV_CMD_INIT_BONUS_POINTS = 1118,
    INV_CMD_CLEAR_STORAGE_ITEM = 1119,
    INV_CMD_MERGE_STORAGE = 1120
};

int Inv_DispatchCommand(int command, int value, int other, int *unused);
void Menu_CreateContextHelpPanel(void);
void Menu_ComputeGammaLut(int initial, int threshold);
void Menu_SaveBgStartFadeOut(void);
void Menu_SetMemCardConfirmPending(void);
void Menu_InitBonusPointAllocState(int gainedPoints);

void Menu_CopyPromptCodes(u8 *source);
void Menu_SetActionSubmenuSelection(int value);
unsigned int Menu_GetActionSubmenuSelection(void);
void MenuWidget_SetColumnLayoutMode(int mode);

extern int g_MenuItemUseMode;
void MenuWidget_ClampScroll(MenuWidgetNode *node);
void MenuWidget_OffsetPosition(MenuWidgetNode *node, int dx, int dy);
void MenuWidget_ClearCursorY(MenuWidgetNode *node);
void Menu_OpenBonusPointSpendDialog(MenuWidgetNode *node, int stat);
void Menu_ExitBonusPointAllocation(void);
int Menu_StatSlotInputHandler(MenuWidgetNode *node, unsigned int flags);
int Menu_StepListNavigate(MenuWidgetListNavigation *node, unsigned int flags);

int Menu_CheckItemAffordable(int actionId);
extern int g_MenuItemSelectFlag; /* Restricts selected parasite actions when nonzero. */
/* Incomplete arrays retain the retail full-address HP accesses with -G8. */
extern u16 g_AyaHpCurrent[]; /* Current HP. */
extern u16 g_AyaHpMax[]; /* Maximum HP. */

int Menu_StepNameEntry(MenuWidgetNode *parent, unsigned int flags);
int Menu_StepNameEntryAlt(MenuWidgetNode *parent, unsigned int input);
/* Name entry panel outlines (item and save names) and the save preview. */
extern u8 g_MenuRenamePanelPointsWindow0[];
extern u8 g_MenuRenamePanelPointsWindow1[];
extern u8 g_MenuRenamePreviewPanelPoints[];
extern u8 *g_MenuItemIconGlyphTable;

#endif
