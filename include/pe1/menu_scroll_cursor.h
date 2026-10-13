#ifndef PE1_MENU_SCROLL_CURSOR_H
#define PE1_MENU_SCROLL_CURSOR_H

/* Grid list cursor stepping (Menu_StepScrollCursor). */

#include "pe1/menu_draw_list.h"
#include "pe1/menu_sound.h"

int Menu_StepListNavigate(MenuWidgetListNavigation *list, unsigned int flags);
int Menu_StepScrollCursor(MenuWidgetNode *node, unsigned int buttons);

#endif
