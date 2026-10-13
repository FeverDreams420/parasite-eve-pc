#ifndef PE1_AKAO_GLOBALS_H
#define PE1_AKAO_GLOBALS_H

/* Driver-wide AKAO state shared by the sequencer, voice driver and
 * voice-bank units (src/main/akao). One declaration per word; the layout
 * headers (track.h, tick.h, queue.h) own the record types. */

#include "common.h"
#include "pe1/akao/track.h"

/* Primary (music) and secondary (sound effect) sequencer banks, and the
 * one the commands act on. */
extern AkaoSequencerState g_AkaoSequencerState;
extern AkaoSequencerBank *g_AkaoCurTrack;

/* Set while the voice driver is busy; the sequencer polls it. */
extern volatile int D_8009D2E0;

/* CD audio mix volume passed to CdMix (a CdlATV). */
extern struct CdlATV g_AkaoCdVolume;
/* High byte of the global pitch depth word g_AkaoGlobalPitchDepth. */
extern unsigned char D_8009D2CE;

/* Global slides stepped every fourth tick: counters, 16.16 steps and the
 * integer half of the pitch accumulator D_8009D2B4. */
extern u16 D_8009CDEC; /* tick divider */
extern short g_AkaoGlobalPitchSlideCounter;
extern short g_AkaoGlobalD2D0SlideCounter;
extern short g_AkaoGlobalD2CCSlideCounter;
extern int g_AkaoGlobalPitchSlideStep;
extern int g_AkaoGlobalD2D0SlideStep;
extern int g_AkaoGlobalD2CCSlideStep;
extern unsigned short D_8009D2B6;

/* Rolling timer-callback durations, newest first, and their sum. */
extern s32 g_AkaoTimerDeltaHist0;
extern s32 g_AkaoTimerDeltaHist1;
extern s32 g_AkaoTimerDeltaHist2;
extern s32 g_AkaoTimerDeltaHist3;
extern s32 g_AkaoTimerDeltaSum;

/* Argument byte counts of opcodes 0xA0-0xFF and of the 0xFC page. */
extern AkaoU8 g_AkaoOpcodeArgLengths[];
extern AkaoU8 g_AkaoExtOpcodeArgLengths[];

/* Pitch table of the instrument records, viewed as words. */
extern int g_AkaoPitchPeriodTable[];
extern void *g_AkaoSoundEntryTable[];

/* Voice masks. */
extern unsigned int g_SpuActiveVoiceMask;
extern unsigned int g_SpuPendingKeyOffMask;
extern unsigned int g_SpuStoppedVoiceMask;
extern unsigned int g_AkaoSeqPendingFlags;
extern int g_AkaoPlaybackMode;
extern unsigned int g_AkaoTrack34Mask;
extern unsigned int g_AkaoTrack38Mask;
extern unsigned int g_AkaoTrack3CMask;
extern unsigned int g_AkaoVoiceUpdateFlags;
extern unsigned int g_AkaoVoiceMaskScratch;
extern u16 g_AkaoTrack5ATransposeValue;
extern int g_AkaoSeqLoopCounter;

/* Sound-effect sequencer masks next to the 34/38/3C dirty masks. */
extern unsigned int g_AkaoSfxAllocatedVoiceMask;

/* update_flags of the sequencer tracks. */
extern AkaoTrackUpdateSlot g_AkaoTrackStateArray[];

/* Nested-stream source filled by SeqOp_LoadNestedStreams and passed to
 * Seq_StartNestedStreams. */
extern AkaoNestedSource g_AkaoNestedSource;
/* Its key_off_mask, key_on_mask, pan word and pan_target as words of their
 * own: retail stores each one absolutely, where the record keeps the field
 * address in a and derives the record address from it. */
extern u32 D_800B89D4;
extern u32 D_800B89D8;
extern u32 D_800B89DC;
extern u32 D_800B89E0;

#endif
