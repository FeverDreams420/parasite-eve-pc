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



PE1_STATIC_ASSERT(sizeof(DRAWENV) == 0x5C, menu_background_drawenv_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(DRAWENV, r0) == 0x19, menu_background_drawenv_rgb_offset);

void Menu_SaveBgApplyFadeStep(void);


int Menu_SaveBgIsFadeActive(void);
void Menu_SaveBgAdvanceFade(void);

/* Geometry OT depth byte (0x800BD024, outside the gp window); the
 * incomplete array keeps -G8 units from treating it as small data. */

void Menu_DrawSaveBg(void);

#endif /* PE1_MENU_BACKGROUND_H */
