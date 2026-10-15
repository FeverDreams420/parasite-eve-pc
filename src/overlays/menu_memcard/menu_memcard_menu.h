#ifndef MENU_MEMCARD_MENU_H
#define MENU_MEMCARD_MENU_H

#include "pe1/display_env.h"
#include "pe1/gpu_image.h"
#include "pe1/psyq_gpu.h"
#include "pe1/render_buffers.h"
#include "pe1/render_packets.h"

#include "menu_memcard_image.h"
#include "menu_memcard_screen.h"

/* Original names refer to the same native pointer slots. */
#define g_MemcardFirstDrawScreen (g_MemcardScreenBuffers[0])
#define g_MemcardSecondScreenView (g_MemcardScreenBuffers[1])

extern DISPENV g_MemcardListSavedDispEnv[2];
extern DRAWENV g_MemcardListSavedDrawEnv[2];
#include "pe1/pe_image.h"
extern s32 g_LogoFadePlayed, g_ActiveDrawSlot;
extern u8 *g_CdLoadAreaBase;

void MenuCard_ShowLogoFade(void);
void Draw_InitBuffers(void);
void GameTime_InitCounter0(void);
void GameTime_InitCounter2(void);
void Field_HandleStateTransition(void);
void Save_SetTitleStyleFlag(int);
void MemCard_InitState(void);
void MemCard_UpdateSavePolling(void);
void Menu_OpenMemCardSelectDialog(int);
void MemCard_SetDialogActive(int);
int Menu_RunFrameWithArg(int);
void Draw_SetPresentEnabled(int);
void Draw_SetPresentImage(void *);
void Draw_SelectBuffer(void);
void Draw_PresentFrame(int);
void func_8018F2F4(void);
void MenuCard_RedrawImageNodes(void);
MemcardImageNode *MenuCard_CreateImageNode(int);
void MenuCard_HandleListInput(void);
int MenuCard_PlayVideo(int);

#endif
