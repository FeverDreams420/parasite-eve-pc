#ifndef PE1_TEXTBOX_OPEN_H
#define PE1_TEXTBOX_OPEN_H

#include "pe1/textbox.h"

/* Globals read by Render_SetupColorTable when it opens a textbox. */

/* Box rectangle set by Menu_SetTextCursorRect, copied into styled boxes. */
typedef struct TextboxRect {
    s16 x, y, width, height;
} TextboxRect;

extern TextboxRect g_TextboxPendingRect;
extern u8 g_DialogueChoiceCursor;
extern s8 g_MenuEquipSlotIndex;
extern u8 g_PlayerMovementEnabled;

#endif /* PE1_TEXTBOX_OPEN_H */
