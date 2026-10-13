#ifndef PSYQ_LIBAPI_PAD_EXTERNS_H
#define PSYQ_LIBAPI_PAD_EXTERNS_H

#include "common.h"

typedef struct PadWork {
    char pad[0xA];
    s16 unk_a;
} PadWork;

extern void DisablePAD(void);
extern void StopPAD2(void);
extern int (*D_800A34B4[2])(void);
extern int D_800A34BC;
extern PadWork *g_PadSioBasePtr;
extern int *g_PadIntrRegsPtr;

#endif /* PSYQ_LIBAPI_PAD_EXTERNS_H */
