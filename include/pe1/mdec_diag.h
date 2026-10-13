#ifndef PE1_MDEC_DIAG_H
#define PE1_MDEC_DIAG_H

#include "common.h"

extern s32 g_MdecQuantTableCommand[];
extern s32 g_MdecQuantTableLuma[16];
extern s32 g_MdecQuantTableChroma[16];
extern s32 g_MdecScaleTableCommand[];
extern char g_MdecInSyncName[];
extern char g_MdecOutSyncName[];
extern char g_MdecResetBadOptionFormat[];
extern char g_MdecTimeoutFormat[];
extern s32 *volatile *g_MdecDma0MadrPtr;
extern volatile u32 *g_MdecDma0BcrPtr;
extern volatile u32 *g_MdecDma0ChcrPtr;
extern s32 *volatile *g_MdecDma1MadrPtr;
extern volatile u32 *g_MdecDma1BcrPtr;
extern volatile u32 *g_MdecDma1ChcrPtr;
extern volatile u32 *g_MdecCommandRegPtr;
extern volatile u32 *g_MdecStatusRegPtr;
extern volatile u32 *g_MdecDmaControlRegPtr;

void MdecDiag_SendInput(s32 *data, u32 size);
int MdecDiag_WaitInputSync(void);
int MdecDiag_WaitOutputSync(void);
int MdecDiag_ReadStatus(void);
int MdecDiag_HandleTimeout(char *message);
/* printf. */
int printf(const char *format, ...);

#endif
