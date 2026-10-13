#ifndef PSYQ_LIBCD_STCDINTERRUPT_EXTERNS_H
#define PSYQ_LIBCD_STCDINTERRUPT_EXTERNS_H

#include "common.h"
#include "pe1/psyq_cd.h"

extern volatile u8 *g_CdStIntrDataFifoPtr;
extern volatile u8 *g_CdStIntrRequestRegPtr;
extern volatile u32 *g_CdStIntrDelayRegPtr;
extern volatile u32 *g_CdStIntrComDelayRegPtr;
extern volatile u32 *g_CdStIntrMdecDmaChcrPtr;
extern volatile u32 *g_CdStIntrDmaChcrPtr;
extern s32 g_CdStIntrState;
extern StHEADER *g_StCurrentHeader;
extern s32 g_CdStreamDataReadyFlag, g_CdStreamSectorSlot, g_CdStreamRingReadSlot;
extern StSectorPayload *g_CdStreamSectorPayloadPtr;

#endif /* PSYQ_LIBCD_STCDINTERRUPT_EXTERNS_H */
