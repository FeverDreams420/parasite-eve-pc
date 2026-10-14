#ifndef PE1_AKAO_VOICE_MASKS_H
#define PE1_AKAO_VOICE_MASKS_H

#include "pe1/akao/track.h"
#include "pe1/akao/queue.h"

void Spu_ManageVoicesCmd(AkaoQueueEntry *command);
void Spu_SetVoiceVolumeImmediateMasked(AkaoQueueEntry *command);
void Spu_ParameterSlide(AkaoQueueEntry *command);
void Spu_SetAllVoiceVolumeImmediate(AkaoQueueEntry *command);
void Spu_SlideAllVoiceVolume(AkaoQueueEntry *command);
void Spu_SetVoicePanImmediateMasked(AkaoQueueEntry *command);
void Spu_SlideVoicePanMasked(AkaoQueueEntry *command);
void Spu_SetAllVoicePanImmediate(AkaoQueueEntry *command);
void Spu_SlideAllVoicePan(AkaoQueueEntry *command);
void Spu_SetVoicePitchImmediateMasked(AkaoQueueEntry *command);
void Spu_SlideVoicePitchMasked(AkaoQueueEntry *command);
void Spu_SetAllVoicePitchImmediate(AkaoQueueEntry *command);
void Spu_SlideAllVoicePitch(AkaoQueueEntry *command);

extern unsigned g_SpuActiveVoiceMask, g_SpuStoppedVoiceMask;
extern unsigned g_SpuPendingKeyOffMask;
extern AkaoVoiceEnvelopeSlot g_AkaoVoiceEnvelopeTable[];
extern unsigned g_AkaoVoicePortamentoResetMask;
extern unsigned g_AkaoTrack34Mask, g_AkaoTrack38Mask, g_AkaoTrack3CMask;
extern unsigned g_AkaoVoiceUpdateFlags;
extern unsigned g_AkaoReverbEnableMask, g_AkaoNoiseEnableMask, g_AkaoFmEnableMask;
extern int g_AkaoPlaybackMode;
extern short g_AkaoPanVolumeTable[];

void Akao_SetVoiceKeyOn(AkaoTrack *voice, unsigned int voice_mask);

void Spu_ManageVoices(int id, int control);
void Seq_StartNestedTrack(AkaoTrack *track, AkaoNestedSource *source, unsigned mask, void *script);
void Seq_ClearTrackVoiceId(AkaoTrack *track, unsigned voice_id);

void Spu_VoiceMaskCompose(AkaoTrack *track, int *mask_out, int mask, int mask_keep);
void Seq_MarkTrack34MaskDirty(void);
void Seq_MarkTrack38MaskDirty(void);
void Seq_MarkTrack3CMaskDirty(void);
void Akao_SetVoiceKeyOff(AkaoTrack *track, unsigned mask, int index);
void Akao_StepVoiceNote(AkaoTrack *track, unsigned mask, unsigned direct, unsigned *output);
/* Retail callers supply differing numbers of trailing, unused arguments. */
void Akao_WriteVoiceParam();
void Spu_WriteKeyOff(unsigned value);
/* Mark every active track of a 24-track table for a full voice refresh. */
void Seq_MarkDirtyTracks(AkaoTrack *tracks);
void Spu_MarkActiveVoicesDirty(void);
void Akao_LoadSamplePairFromIndex(void **first, void **second, int sample_id);
void Akao_SetVoicePitch(void);
void Akao_SetVoiceVolume(void);
void Akao_SetVoiceAdsr(void);
void Akao_SetVoiceStartAddr(void);

#endif
