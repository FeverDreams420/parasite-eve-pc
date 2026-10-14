#ifndef PE1_PSYQ_CD_AUDIO_H
#define PE1_PSYQ_CD_AUDIO_H

#include "pe1/psyq_types.h"

/* LIBCD input mixing needs no drive, GPU or DMA layout declarations. */
typedef struct CdlATV {
    u_char val0;
    u_char val1;
    u_char val2;
    u_char val3;
} CdlATV;

int CdMix(CdlATV *volume);

#endif
