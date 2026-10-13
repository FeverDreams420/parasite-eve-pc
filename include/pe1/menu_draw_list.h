#ifndef PE1_MENU_DRAW_LIST_H
#define PE1_MENU_DRAW_LIST_H

#include "pe1/menu_widget.h"

extern int g_MenuInputActive;
extern int g_DrawTextPosX, D_8009D168;

/* The list renderer passes the row index in a0 to its drawing callback. */
void MenuWidget_DrawList(MenuWidgetNode *node, void (*draw_callback)(int));
void MenuWidget_DrawListRow(MenuWidgetNode *node, void (*draw_callback)(int),
                            int row, int draw_cursor);
void MenuWidget_EaseNodePosition(MenuWidgetNode *node);
int Draw_RemapStatusFlags(void);
void Draw_AllocColorTriGradient(int width, int height, int mode, int focused);

#endif
