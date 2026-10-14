#ifndef PE1_AKAO_NOTE_STEP_H
#define PE1_AKAO_NOTE_STEP_H

/* Declarations used by the sequencer note step Akao_StepSampleLoader. */

#include "pe1/akao/tick.h"
#include "pe1/akao/track.h"

typedef void (*AkaoCommandHandler)(AkaoTrack *track, u32 voice_mask);
void SeqOp_JumpRelative(AkaoTrack *track, u32 voice_mask);
void SeqOp_StreamPair(AkaoTrack *track, u32 voice_mask);
void SeqOp_ReadTrack64U16(AkaoTrack *track, u32 voice_mask);
void SeqOp_Skip2(AkaoTrack *track, u32 voice_mask);
void SeqOp_Ignore(AkaoTrack *track, u32 voice_mask);
void SeqOp_Noop_904AC(AkaoTrack *track, u32 voice_mask);
void SeqOp_Noop_904B4(AkaoTrack *track, u32 voice_mask);
void SeqOp_Noop_904BC(AkaoTrack *track, u32 voice_mask);

void Akao_ResetReleaseRate(AkaoTrack *track, u32 voice_mask);
void Akao_SetExpressionSlide(AkaoTrack *track, u32 voice_mask);
void Akao_SetPanSlide(AkaoTrack *track, u32 voice_mask);
void Akao_SetPanTarget(AkaoTrack *track, u32 voice_mask);
void Akao_SetPanTargetSlide(AkaoTrack *track, u32 voice_mask);
void Akao_SetPanpot(AkaoTrack *track, u32 voice_mask);
void Akao_SetReleaseRate(AkaoTrack *track, u32 voice_mask);
void SeqOp_AdjustDetune(AkaoTrack *track, u32 voice_mask);
void SeqOp_AdjustExpression(AkaoTrack *track, u32 voice_mask);
void SeqOp_AdjustVolumeTarget(AkaoTrack *track, u32 voice_mask);
void SeqOp_BranchIfLoopLimitReached(AkaoTrack *track, u32 voice_mask);
void SeqOp_ClearFlag10(AkaoTrack *track, u32 voice_mask);
void SeqOp_ClearFlag20(AkaoTrack *track, u32 voice_mask);
void SeqOp_ClearFlag3(AkaoTrack *track, u32 voice_mask);
void SeqOp_DecPanpot(AkaoTrack *track, u32 voice_mask);
void SeqOp_EnableField84(AkaoTrack *track, u32 voice_mask);
void SeqOp_IncPanpot(AkaoTrack *track, u32 voice_mask);
void SeqOp_JumpIfLoopCount(AkaoTrack *track, u32 voice_mask);
void SeqOp_JumpIfLoopCountPop(AkaoTrack *track, u32 voice_mask);
void SeqOp_LoadNestedStreams(AkaoTrack *track, u32 voice_mask);
void SeqOp_LoadSoundEntry(AkaoTrack *track, u32 voice_mask);
void SeqOp_LoopCounter(AkaoTrack *track, u32 voice_mask);
void SeqOp_PushLoopPoint(AkaoTrack *track, u32 voice_mask);
void SeqOp_ReadAdsrAttack(AkaoTrack *track, u32 voice_mask);
void SeqOp_ReadAdsrAttackRate(AkaoTrack *track, u32 voice_mask);
void SeqOp_ReadAdsrRelease(AkaoTrack *track, u32 voice_mask);
void SeqOp_ReadAdsrReleaseRate(AkaoTrack *track, u32 voice_mask);
void SeqOp_ReadAdsrSustain(AkaoTrack *track, u32 voice_mask);
void SeqOp_ReadAdsrSustainRate(AkaoTrack *track, u32 voice_mask);
void SeqOp_ReadDetune(AkaoTrack *track, u32 voice_mask);
void SeqOp_ReadExpression(AkaoTrack *track, u32 voice_mask);
void SeqOp_ReadPanpot(AkaoTrack *track, u32 voice_mask);
void SeqOp_ReadVolume(AkaoTrack *track, u32 voice_mask);
void SeqOp_ResetFlag4State(AkaoTrack *track, u32 voice_mask);
void SeqOp_ResetTrack(AkaoTrack *track, u32 voice_mask);
void SeqOp_Return(AkaoTrack *track, u32 voice_mask);
void SeqOp_SetDuration(AkaoTrack *track, u32 voice_mask);
void SeqOp_SetDurationAlt(AkaoTrack *track, u32 voice_mask);
void SeqOp_SetFlag20(AkaoTrack *track, u32 voice_mask);
void SeqOp_SetFlag4(AkaoTrack *track, u32 voice_mask);
void SeqOp_SetPitchBase(AkaoTrack *track, u32 voice_mask);
void SeqOp_SetReturnPoint(AkaoTrack *track, u32 voice_mask);
void SeqOp_SetSustainFlag(AkaoTrack *track, u32 voice_mask);
void SeqOp_SetTremoloParams(AkaoTrack *track, u32 voice_mask);
void SeqOp_SetVibratoParams(AkaoTrack *track, u32 voice_mask);
void SeqOp_SetVolume(AkaoTrack *track, u32 voice_mask);
void SeqOp_SetVolumeLFO(AkaoTrack *track, u32 voice_mask);
void SeqOp_SetVolumeOrExpression(AkaoTrack *track, u32 voice_mask);
void SeqOp_SlideVolume(AkaoTrack *track, u32 voice_mask);
void SeqOp_StopPitchLFO(AkaoTrack *track, u32 voice_mask);
void SeqOp_StopTremolo(AkaoTrack *track, u32 voice_mask);
void SeqOp_UpdatePitchLFOTarget(AkaoTrack *track, u32 voice_mask);
void SeqOp_UpdateVolumeLFOTarget(AkaoTrack *track, u32 voice_mask);

/* Opcode handlers: 0xA0..0xFF, and the 0xFC extended page. */
extern AkaoCommandHandler g_AkaoOpcodeHandlers[];
extern AkaoCommandHandler g_AkaoExtOpcodeHandlers[];
/* Note lengths in ticks, indexed by opcode % 11. */
extern u16 g_AkaoNoteLengthTable[];
/* LFO waveform tables. */
extern void *g_AkaoSoundEntryTable[];
/* Key-on mask of the child (sound effect) tracks. */
extern u32 g_AkaoVoiceMaskScratch;
/* Instrument table, 0x40-byte records (also viewed as words by the
 * stream relocation code in akao.h). */
extern AkaoInstrument g_AkaoInstrumentTable[];

int Akao_LookupSampleBankByte(AkaoTrack *track);
int Akao_LookupPitchPeriod(int instrument, int note, int detune);
void Akao_SetNotePitchBounded(AkaoTrack *track, int note);
void Seq_MarkTrack38MaskDirty(void);
void SeqOp_SetVoiceInstrument(AkaoTrack *track, AkaoInstrument *instrument,
                              int sample_header);

#endif /* PE1_AKAO_NOTE_STEP_H */
