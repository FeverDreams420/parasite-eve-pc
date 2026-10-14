#ifndef PE1_MENU_FADE_STATE_H
#define PE1_MENU_FADE_STATE_H
#include "common.h"
extern int g_MenuSaveBgFadeState; /* Fade state. */
extern int g_MenuSaveBgFadeHeight; /* Number of 256-pixel blocks. */
extern int g_MenuSaveBgFadeLutLen; /* Fade curve length. */
extern int g_MenuSaveBgFadeStep; /* Signed curve step. */
extern int g_MenuSaveBgFadeIndex; /* Curve position. */
extern int g_MenuSaveBgFadeTint; /* Fade parameter. */
extern u8 g_MenuSaveBgFadeLut[]; /* Fade curve. */



extern int g_MenuItemContextFlag;
void Menu_SaveBgInitFade(void);
void Menu_ComputeGammaLut(int initial, int threshold);
int Menu_ClampRange(int value);
int Menu_SaveBgIsFadeActive(void);
void Menu_SaveBgAdvanceFade(void);
#endif
