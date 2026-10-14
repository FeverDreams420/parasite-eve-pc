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
