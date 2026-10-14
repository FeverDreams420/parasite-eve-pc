#ifndef PE1_AKAO_COUNTERS_H
#define PE1_AKAO_COUNTERS_H

#include "pe1/akao/globals.h"

/* Sequencer-wide slide values and SPU sample-table pointers shared by the
 * AKAO command handlers and the four-tick slide step. */

/* 16.16 slide accumulators stepped by Spu_StepReverbLoad. */
extern int g_AkaoGlobalPitchDepth;
extern int g_AkaoTempoScaleAccum;

/* Sample offset table and sample data base set by the voice-bank loader. */
extern unsigned short *g_AkaoSampleOffsetTable;
extern char *g_AkaoSampleDataBase;

/* BGM volume fade queued by the menu and sent with Akao_Cmd_21. */
extern int g_AkaoPendingBgmVolumeFade;

/* Duck the BGM while a menu panel counts up; flushed when it closes. */
void Akao_SetBgmVolumeFade(void);
void Akao_FlushBgmVolumeFade(void);

#endif
