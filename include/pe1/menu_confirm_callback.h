#ifndef PE1_MENU_CONFIRM_CALLBACK_H
#define PE1_MENU_CONFIRM_CALLBACK_H

struct MenuWidgetNode;
typedef void (*MenuConfirmCallback)(struct MenuWidgetNode *node, int confirmed);

/* Callback invoked by the yes/no dialog handler. */
extern MenuConfirmCallback g_MenuConfirmCallback;

void Menu_ItemUseAction(struct MenuWidgetNode *node, int confirmed);
void Menu_HandleMemCardWriteOrError(struct MenuWidgetNode *node, int confirmed);
void Menu_OnBattleCommandConfirm(struct MenuWidgetNode *node, int confirmed);
void Menu_TriggerSaveWrite(struct MenuWidgetNode *node, int confirmed);

#endif
