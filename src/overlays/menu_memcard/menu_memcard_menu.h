#ifndef MENU_MEMCARD_MENU_H
#define MENU_MEMCARD_MENU_H

#include "pe1/display_env.h"
#include "pe1/gpu_image.h"
#include "pe1/psyq_gpu.h"
#include "pe1/render_packets.h"

#include "menu_memcard_image.h"
#include "menu_memcard_screen.h"

/* Matching views of the existing two pointer slots, not extra storage.
 * Separate names retain retail's reloads instead of caching their addresses. */
extern struct { MemcardScreenBuffer *value; } g_MemcardSecondScreen;
extern MemcardScreenBuffer *D_801D11C0;
extern MemcardScreenBuffer *g_MemcardFirstDrawScreen;

extern DISPENV D_801D1550, D_801D1564;
extern DRAWENV D_801D1498, D_801D14F4;
extern u8 D_800B0DCD;
extern s32 D_8009D1BC, g_ActiveDrawSlot;
extern u8 *g_CdLoadAreaBase, *D_800B0E3C, *D_800B0E50, *D_800B0E54;
extern RenderBufferPrefix D_800B0E38;
extern u8 D_8019319C[];

void func_80190660(void);
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
MemcardImageNode *func_8018FBC0(int);
void func_80190064(void);
int func_80192CE8(int);

#endif
