#ifndef PE1_MENU_FLAGS_H
#define PE1_MENU_FLAGS_H

/* Small menu-state flags and counters shared by the menu, memory-card and
 * field units. */

/* Frames left on the menu effect timer (set to 0x5A by the main update). */
extern unsigned char g_AyaStatusEffectTimer;
/* Set by Menu_SetEquipSlotIndex (1 with game-state flag 0x40000000, see
 * Menu_FullInit): textbox glyph 0x0F then draws 7 pixels further left. */
extern unsigned char g_TextboxKernGlyph0F;
/* Memory-card dialog mode and the pending confirm flag. */
extern int g_McDialogMode;
extern int g_MenuMemCardConfirmPending;
/* Queued battle actions; the menu is blocked while it is nonzero. */
extern unsigned char g_BattleActionQueueCount;

#endif
