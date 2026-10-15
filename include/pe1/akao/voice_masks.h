#ifndef PE1_AKAO_VOICE_MASKS_H
#define PE1_AKAO_VOICE_MASKS_H

#include "pe1/akao/track.h"
#include "pe1/akao/queue.h"

void Spu_ManageVoicesCmd(void *message);
void Spu_SetVoiceVolumeImmediateMasked(void *message);
void Spu_ParameterSlide(void *message);
void Spu_SetAllVoiceVolumeImmediate(void *message);
void Spu_SlideAllVoiceVolume(void *message);
void Spu_SetVoicePanImmediateMasked(void *message);
void Spu_SlideVoicePanMasked(void *message);
void Spu_SetAllVoicePanImmediate(void *message);
void Spu_SlideAllVoicePan(void *message);
void Spu_SetVoicePitchImmediateMasked(void *message);
void Spu_SlideVoicePitchMasked(void *message);
void Spu_SetAllVoicePitchImmediate(void *message);
void Spu_SlideAllVoicePitch(void *message);

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
void Akao_RemoveVoice(AkaoTrack *tracks, int voice_id);

void Spu_VoiceMaskCompose(AkaoTrack *track, int *mask_out, int mask, int mask_keep);
void Seq_MarkTrack34MaskDirty(void);
void Seq_MarkTrack38MaskDirty(void);
void Seq_MarkTrack3CMaskDirty(void);
void Akao_SetVoiceKeyOff(AkaoTrack *track, unsigned mask, int index);
void Akao_StepVoiceNote(AkaoTrack *track, unsigned mask, unsigned direct, unsigned *output);
/* Only the two consumed arguments are retained in the native C11 ABI. */
void Akao_WriteVoiceParam(int voice_index, AkaoVoiceParams *params);
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
