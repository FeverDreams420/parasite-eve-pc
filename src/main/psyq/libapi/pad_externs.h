#ifndef PSYQ_LIBAPI_PAD_EXTERNS_H
#define PSYQ_LIBAPI_PAD_EXTERNS_H

#include "common.h"

typedef struct PadWork {
    char pad[0xA];
    s16 unk_a;
} PadWork;

extern void DisablePAD(void);
extern void StopPAD2(void);
/* The handler pair and spare word of the pad interrupt queue node (link
 * word g_PadIntQueueNode, layout PadIntQueueNode) as their own symbols:
 * Sys_SetInterruptHandler addresses each word separately, which the record
 * form folds into one base register. */
extern int (*g_PadIntQueueHandlers[2])(void);
extern int g_PadIntQueueSpare;
extern PadWork *g_PadSioBasePtr;
extern int *g_PadIntrRegsPtr;

#endif /* PSYQ_LIBAPI_PAD_EXTERNS_H */
