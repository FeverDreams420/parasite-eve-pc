#ifndef PE1_AKAO_H
#define PE1_AKAO_H

#include "pe1/akao/commands.h"
#include "pe1/akao/queue.h"
#include "pe1/akao/spu_voice.h"
#include "pe1/akao/stream.h"
#include "pe1/akao/track.h"
#include "pe1/akao_send.h"

/*
 * Track structs are still intentionally partial in C files. The FF7/FF8 AKAO
 * cross-reference has stable field roles, but PE1 offsets do not fully match
 * either game yet.
 */

/* SPU sample transfers (akao/spu_transfer.c). The transfer status is
 * 1 while a DMA transfer runs; the completion callback clears it. */
extern volatile int g_SpuTransferStatus;

/* Stream instrument relocation. */
/* Stream instrument scratch table; bank selection advances by 0x400 bytes. */
extern AkaoInstrument g_AkaoStreamInstrumentCacheB[];
/* An instrument record address as the word retail adds a byte offset to
 * (table slots, cache slots copied word by word). */
typedef union AkaoInstrumentAddress {
    AkaoInstrument *instrument;
    unsigned *words;
    uintptr_t word;
} AkaoInstrumentAddress;
/* Instrument records replaced by Spu_UploadSampleBlock. */
extern AkaoInstrument g_AkaoInstrumentTable[];
extern unsigned g_AkaoStreamUploadMode, g_AkaoStreamAAddress, g_AkaoStreamARemaining, g_AkaoInstrumentUploadAddress, g_AkaoInstrumentUploadRemaining;

extern void *g_AkaoBgmHandle[];

int Akao_SendPositionalCmdStereo(int arg0, int arg1, short x, short y, short z);

#endif
