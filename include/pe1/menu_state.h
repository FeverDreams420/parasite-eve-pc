#ifndef PE1_MENU_STATE_H
#define PE1_MENU_STATE_H

#include "common.h"

/* 0: closed, 1: opening, 2+: save polling runs in the menu frame. */
extern int g_MemCardDialogState;
void MemCard_SetDialogActive(int active);

extern int g_MenuBattleEquipMode;
void Menu_SetBattleEquipMode(int mode);
int Menu_GetBattleEquipMode(void);

extern unsigned int g_MenuErrorSoundPending;
void Menu_RequestErrorSound(void);
int Menu_GetEquipMode(void);
/* Menu start-up, run while the scene data loads (item/active_list.c). */
void Menu_FullInit(void);

/* EXP review: the total counting up toward the target, the level reached
 * so far and the frames left on the LEVEL UP banner. */
extern int g_MenuPendingTotalExp;
extern int g_MenuExpAllocTarget;
extern int g_MenuExpReviewLevel;
extern int g_MenuLevelUpAnimTimer;

/* Selection state shared by the inventory, equipment and item-action screens. */
extern int g_MenuActionSubmenuOpen;
extern int g_InvSelectedItemIndex;
extern int g_InvSwapTargetIndex;
/* One-based packed cursor (bit 0 column, bits 1-7 row, bits 8+ scroll) of the
 * equipment slot a swap started from; zero when none. */
extern int g_MenuEquipSwapSource;
/* Inventory item picked to swap into the equipment slot. */
extern int g_MenuEquipSwapItem;
/* When set, reopening the inventory does not put the swap item back. */
extern int g_MenuEquipSwapSkipRestore;
/* 1 + equipment mode while the equip screen runs from the item menu. */
extern int g_MenuEquipMode;
/* Item-category mask the item menu lists. */
extern int g_MenuItemContextFlag;
extern int g_MenuBattleStatusOverlayActive;
extern int g_MenuSelectionLocked;
/* Parasite spell picked on the skill screen (its name is table 8 + 0xEB). */
extern int g_MenuSelectedSpell;
/* Row of the action submenu last picked (Menu_SetActionSubmenuSelection). */
extern unsigned int g_MenuActionSubmenuSel;
/* Scroll step chosen from D_8009CDAC: +4 below 300, -4 from 300 on. */
extern int D_8009CDAC;
extern int g_MenuScrollStep;
/* List target handed to Menu_RunFrameWithArg. */
extern u8 *g_MenuActiveListTarget;
/* Which list the equipment options act on: 0 sorts Aya's inventory, any
 * other value sorts Wayne's storage. */
extern int g_MenuInventoryViewMode;
extern int g_InvItemUsableFlag;
/* Item list pair compared and swapped by the ammo transfer panel. */
extern int g_InvAmmoSpendActiveList;
extern int g_InvSwapSourceList;
/* Arrow the ammo transfer panel points (0 or 1). */
extern int g_MenuSpendArrowDirection;
/* Set while the list column layout must not be restored from the saved
 * cursor table. */
extern int g_MenuLayoutLocked;
/* Item list modes: rename and discard dim the entries they cannot act on. */
extern int g_MenuItemRenameMode;
extern int g_MenuItemDiscardMode;

/* Window X and Y offset per menu window mode (Battle_SetWindowBoundsByMode). */
typedef struct {
    u16 values[8];
} SaveWindowModeBounds;
extern const SaveWindowModeBounds g_MenuWindowModeBounds;
void Battle_SetWindowBoundsByMode(int mode);
unsigned int Menu_GetActiveMode(void);

/* Visual settings: the blend colour and screen base Y in effect when the
 * colour and screen adjust panels opened, restored on cancel, and the
 * colour panel outline. */
extern int g_MenuBlendColorBackup;
extern int g_MenuScreenBaseYBackup;
extern unsigned char g_MenuBlendColorPanelPoints[];


/* Menu open/close state (the in-field item/status/equip menu drawn over the
 * paused field). Confirmed live via DuckStation GDB: snapshot-diffed the game
 * state block 0x8009C000..0x8009E000 across field <-> menu transitions; the
 * toggled words reverted cleanly on open then close.
 *
 * The menu is a LAYER over the field: opening it does NOT change the top game
 * mode; it (1) sets a modal flag, (2) allocates a MenuWidget node pool, and
 * (3) bumps a draw phase. The field data stays resident; field updates gate on
 * the modal flag.
 *
 * Confirmed toggling globals (field value -> menu value):
 *   g_GameStateFlags  global game-state flags
 *               bit 0x04 SET while a modal menu is open (clear in plain field).
 *               Also read by battle (Battle_IsActive / Battle_GetStateFlag1);
 *               Boot_RunFrame masks bits 0x30. This is the central mode bitfield.
 *   g_MenuWidgetActiveListHead  MenuWidget node-pool pointer : NULL in field -> 0x800a25b0 in menu
 *   g_MenuWidgetCurrentNode  MenuWidget node-pool pointer : NULL in field -> 0x800a2370 in menu
 *   g_MenuWidgetFreeListHead  active widget/draw pointer    : 0x800a22e0  -> 0x800a2640
 *   g_MenuDrawPhase  menu draw phase / step        : field 0x01  -> menu 0x08
 *   g_DrawPacketCursor  active draw buffer pointer     : switches with the layer
 *   (The g_DrawScratchBuffer..g_MenuWidgetSavedNode cluster is the MenuWidget pool head/free/tail,
 *    managed by MenuWidget_InitPool / MenuWidget_CreateNode; non-null only
 *    while a menu owns it -> a clean "is a menu open?" test.)
 *
 * OPEN  : MenuWidget_InitPool() allocates the node pool (g_MenuWidgetActiveListHead/15C != NULL),
 *         g_GameStateFlags |= 0x04, g_MenuDrawPhase set; the inventory root step begins
 *         (Menu_StepInventoryRoot via Menu_InitCategoryTabs).
 * CLOSE : the pool is torn down (pointers NULL), g_GameStateFlags &= ~0x04, phase
 *         restored; control returns to the field step.
 *
 * Per-frame: Boot_RunFrame() -> Field_HandleStateTransition() (field/state
 * machine, g_GameStateFlags) then Entity_FrameUpdate(); the menu widget tree is
 * driven by MenuWidget_UpdateAndDraw() while the pool is live.
 *
 * Quick live probe: a menu is open iff *(u32*)0x8009D154 != 0  (or g_GameStateFlags & 4).
 */

#endif /* PE1_MENU_STATE_H */
