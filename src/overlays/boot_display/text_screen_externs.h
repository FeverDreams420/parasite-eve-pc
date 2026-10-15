#ifndef PE1_EXT_TEXT_SCREEN
#define PE1_EXT_TEXT_SCREEN

#include "common.h"
#include "pe1/boot_display.h"
#include "pe1/game_state.h"

extern BootDisplayCallbacks g_BootDisplaySpriteCallbacks, g_BootDisplayImageCallbacks;
extern u32 g_FieldPadPressed;
extern u32 g_FieldPadBits;
extern s32 g_BootDisplayDone;
extern u32 g_BootDisplayFrameCounter;
s32 BootDisplay_DrawTextBlock(void);
extern void Field_HandleStateTransition(void);
#endif /* PE1_EXT_TEXT_SCREEN */
