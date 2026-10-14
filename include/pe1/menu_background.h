#ifndef PE1_MENU_BACKGROUND_H
#define PE1_MENU_BACKGROUND_H

#include "common.h"
#include "pe1/display_env.h"
#include "pe1/game_state.h"
#include "pe1/psyq_gpu.h"

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

PE1_STATIC_ASSERT(sizeof(DRAWENV) == PE1_OFFSETOF(DRAWENV, dr_env) + sizeof(DR_ENV), menu_background_drawenv_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(DRAWENV, r0) == 0x19, menu_background_drawenv_rgb_offset);

void Menu_SaveBgApplyFadeStep(void);

void Menu_DrawSaveBg(void);

#endif /* PE1_MENU_BACKGROUND_H */
