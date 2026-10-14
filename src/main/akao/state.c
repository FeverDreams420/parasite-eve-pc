#include "pe1/akao/globals.h"
#include "pe1/akao/voice_state.h"
#include "pe1/akao/voice_masks.h"
#include "pe1/akao/tick.h"
#include "pe1/akao/spu_common.h"

/* Native storage for the original driver. Asset tables and platform SPU
 * registers are initialized by their own loading/platform endpoints. */
AkaoInstrument g_AkaoInstrumentTable[256];
AkaoSequencerState g_AkaoSequencerState;
AkaoCurrentBankPointer g_AkaoCurrentBankPointer = {.pointer = &g_AkaoSequencerState.primary};
unsigned g_SpuActiveVoiceMask;
unsigned g_SpuPendingKeyOffMask;
unsigned g_AkaoTrack34Mask, g_AkaoTrack38Mask, g_AkaoTrack3CMask;
unsigned g_AkaoVoiceMaskScratch, g_AkaoVoiceUpdateFlags;
AkaoSequencerBank g_AkaoTrackStateBackup;
AkaoVoiceBank g_AkaoVoiceStateBackup;
unsigned g_AkaoVoiceKeyOnState, g_AkaoSeqPendingFlags;
unsigned short g_AkaoSelectedBankId;
int g_AkaoPlaybackMode;
AkaoTickTrack g_AkaoSequencerTracks[60];
AkaoNestedSource g_AkaoNestedSource;
int g_AkaoSeqLoopCounter;
unsigned g_AkaoSfxAllocatedVoiceMask, g_SpuStoppedVoiceMask;
u16 g_AkaoTrack5ATransposeValue;
int g_AkaoGlobalPitchDepth;
int g_AkaoTempoScaleAccum;
AkaoCdInputAccumulator g_AkaoCdInputAccumulator;
u16 g_AkaoGlobalSlideTick;
short g_AkaoGlobalPitchSlideCounter, g_AkaoGlobalD2D0SlideCounter, g_AkaoGlobalD2CCSlideCounter;
int g_AkaoGlobalPitchSlideStep, g_AkaoGlobalD2D0SlideStep, g_AkaoGlobalD2CCSlideStep;
unsigned g_AkaoFmEnableMask, g_AkaoNoiseEnableMask, g_AkaoReverbEnableMask;
unsigned g_AkaoVoicePortamentoResetMask;
AkaoTickTempo g_AkaoSfxTempo;
u32 g_AkaoSfxTickAccumulator;
AkaoVoiceEnvelopeSlot g_AkaoVoiceEnvelopeTable[24];
SpuCommonAttr g_AkaoSpuCommonAttr;

volatile int g_AkaoSeqBusyFlags;
