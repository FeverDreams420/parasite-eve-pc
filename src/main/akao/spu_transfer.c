#include "common.h"
#include "pe1/akao.h"
#include "pe1/akao/file_header.h"
#include "pe1/akao/init_state.h"
#include "pe1/akao/tick.h"
#include "pe1/akao/voice_masks.h"
#include "pe1/psyq_bios.h"
#include "pe1/psyq_spu_internal.h"

int Spu_ValidateSampleHeader(void *header) {
    return ((AkaoFilePrefix *)header)->magic + 0xB0BEB4BF;
}

void Spu_ClearTransferCallback(void) {
    SpuSetTransferCallback(0);
    g_SpuTransferStatus = 0;
}

void Spu_PrepareTransfer(void) {
    g_SpuTransferStatus = 1;
    SpuSetTransferCallback(Spu_ClearTransferCallback);
}

void Spu_UploadWithPrepare(void *source, unsigned int size) {
    Spu_PrepareTransfer();
    SpuWrite(source, size);
}

void Spu_ReadWithPrepare(void *destination, unsigned int size) {
    Spu_PrepareTransfer();
    SpuRead(destination, size);
}

void Spu_WaitTransferDone(void) {
    while (g_SpuTransferStatus == 1) {
    }
}

s32 Spu_UploadSampleBlock(s32 *header, s32 wait) {
    u32 *src;
    u32 *dst;
    s32 size; /* ADPCM byte count, then the instrument word count */
    s32 first;
    s32 end;
    s32 count;

    Spu_WaitTransferDone();
    if (Spu_ValidateSampleHeader(header) == 0) {
        /* Walk the header words after the file prefix: SPU address,
         * ADPCM size, first and end instrument, then the records. */
        header += 4;
        SpuSetTransferStartAddr(*header++);
        size = *header++;
        first = *header++;
        end = *header;
        src = (u32 *)(header + 9);
        if (end == 0) {
            end = 0x100;
        }

        count = end - first;
        /* The ADPCM data follows the instrument records. */
        Spu_UploadWithPrepare((AkaoInstrument *)src + count, size);

        dst = (u32 *)&g_AkaoInstrumentTable[first];
        size = count * (sizeof(AkaoInstrument) / sizeof(u32));
        if (size != 0) {
            do {
                size--;
                *dst++ = *src++;
            } while (size != 0);
        }

        if (wait != 0) {
            Spu_WaitTransferDone();
        }
        return 0;
    }

    g_SpuTransferStatus = -1;
    return -1;
}

void Spu_InitVoiceState(void) {
    u16 i;
    AkaoTickTrack *voice;
    AkaoSequencerBank *bank;
    int reset;

    g_AkaoCurTrack = &g_AkaoSequencerState.primary;
    g_AkaoPlaybackMode = 1;
    g_AkaoSequencerState.secondary.pitch_current = 0x7F0000;
    g_AkaoSequencerState.primary.pitch_current = 0x7F0000;
    g_AkaoCdInputVolumeAccum = 0x7FFF0000;
    g_SpuActiveVoiceMask = 0;

    g_AkaoSequencerState.primary.active_voice_mask = 0;
    g_AkaoSequencerState.primary.pending_voice_mask = 0;
    g_AkaoSequencerState.primary.bank_id = 0;
    g_SpuStoppedVoiceMask = 0;
    g_AkaoSequencerState.primary.pending_restore_mask = 0;
    g_AkaoSequencerState.secondary.pending_restore_mask = 0;
    g_AkaoSequencerState.secondary.active_voice_mask = 0;
    g_AkaoSequencerState.secondary.pending_voice_mask = 0;
    g_AkaoSequencerState.secondary.bank_id = 0;
    g_AkaoSequencerState.secondary.pitch_slide_duration = 0;
    g_AkaoSequencerState.primary.pitch_slide_duration = 0;

    g_AkaoGlobalD2CCSlideCounter = 0;
    g_AkaoGlobalPitchDepth = 0;
    g_AkaoGlobalD2D0SlideCounter = 0;
    g_AkaoTempoScaleAccum = 0;
    g_AkaoGlobalPitchSlideCounter = 0;
    g_AkaoTrack34Mask = 0;
    g_AkaoSequencerState.primary.key_off_dirty_mask = 0;
    g_AkaoTrack38Mask = 0;
    g_AkaoSequencerState.primary.volume_dirty_mask = 0;
    g_AkaoTrack3CMask = 0;
    g_AkaoSequencerState.primary.adsr_dirty_mask = 0;
    g_AkaoSequencerState.secondary.adsr_dirty_mask = 0;
    g_AkaoSequencerState.secondary.volume_dirty_mask = 0;
    g_AkaoSequencerState.secondary.key_off_dirty_mask = 0;
    g_AkaoMessageQueue[0].opcode.word = 0;
    g_AkaoSequencerState.primary.ticks_per_beat = 0;
    g_AkaoSequencerState.primary.beat = 0;
    g_AkaoSequencerState.primary.beats_per_measure = 0;
    g_AkaoSequencerState.primary.measure = 0;

    g_AkaoSpuCommonAttr.mask = 0x3FCF;
    g_AkaoSpuCommonAttr.mvol.right = 0x3FFF;
    g_AkaoSpuCommonAttr.mvol.left = 0x3FFF;
    g_AkaoSpuCommonAttr.mvolmode.left = 0;
    g_AkaoSpuCommonAttr.mvolmode.right = 0;
    g_AkaoSpuCommonAttr.cd.volume.right = 0x7FFF;
    g_AkaoSpuCommonAttr.cd.volume.left = 0x7FFF;
    g_AkaoSpuCommonAttr.cd.reverb = 0;
    g_AkaoSpuCommonAttr.cd.mix = 1;
    g_AkaoSpuCommonAttr.ext.volume.right = 0;
    g_AkaoSpuCommonAttr.ext.volume.left = 0;
    g_AkaoSpuCommonAttr.ext.reverb = 0;
    g_AkaoSpuCommonAttr.ext.mix = 0;
    SpuSetCommonAttr(&g_AkaoSpuCommonAttr);

    g_AkaoMessageQueueLock = 0;
    g_AkaoSeqLoopCounter = 0;
    g_AkaoVoicePortamentoResetMask = 0;
    g_AkaoFmEnableMask = 0;
    g_AkaoNoiseEnableMask = 0;
    g_AkaoReverbEnableMask = 0;
    /* The message-queue lock is shared with the timer interrupt, so the
     * original reloads it before copying it to the three queue words. */
    reset = g_AkaoMessageQueueLock;
    g_AkaoMessageQueueCount = reset;
    g_AkaoSeqPendingFlags = reset;
    g_AkaoSeqBusyFlags = reset;

    voice = g_AkaoSequencerTracks;
    for (i = 0; i < AKAO_VOICE_COUNT; i++, voice++) {
        voice->track.flags = 0;
        voice->track.assigned_voice_index = AKAO_VOICE_COUNT;
        voice->track.parent_track_id = 0;
        voice->timing.tick_count = 0;
        SpuSetVoiceVolumeAttr(i, 0, 0, 0, 0);
    }
    for (i = 0; i < AKAO_VOICE_COUNT; i++, voice++) {
        voice->track.flags = 0;
        voice->track.assigned_voice_index = AKAO_VOICE_COUNT;
        voice->track.parent_track_id = 0;
        voice->timing.tick_count = 0;
        SpuSetVoiceVolumeAttr(i, 0, 0, 0, 0);
    }

    voice = (AkaoTickTrack *)g_AkaoVoiceChannelTable;
    for (i = AKAO_SPU_VOICE_SFX_START_INDEX; i < AKAO_VOICE_COUNT; i++, voice++) {
        voice->track.flags = 0;
        voice->track.assigned_voice_index = i;
        voice->track.parent_track_id = 1;
        voice->timing.tick_count = 0;
        voice->track.pan_target = 0x7F00;
        voice->track.panpot_duration = 0;
        voice->track.pitch_offset_duration = 0;
        voice->track.pitch_offset = 0;
    }

    bank = g_AkaoCurTrack;
    bank[0].key_off_request_mask = 0;
    bank[0].allocated_voice_mask = 0;
    bank[0].key_on_request_mask = 0;
    bank[1].key_off_request_mask = 0;
    bank[1].allocated_voice_mask = 0;
    bank[1].key_on_request_mask = 0;
    g_AkaoSfxTickAccumulator = 1;
    g_AkaoSfxTempo.value = 0x66A80000;
    g_SpuPendingKeyOffMask = 0;
    g_AkaoSfxAllocatedVoiceMask = 0;
    g_AkaoVoiceMaskScratch = 0;
    bank[1].volume = 0x3FFF0000;
    bank[0].volume = 0x3FFF0000;
    bank[1].volume_delta = 0;
    bank[0].volume_delta = 0;
    bank[1].volume_slide_duration = 0;
    bank[0].volume_slide_duration = 0;

    g_AkaoVoiceUpdateFlags |= AKAO_GLOBAL_UPDATE_TEMPO;
    Seq_SetParamWithReset(4);
    SpuSetReverb(1);
}

void Spu_InitHardware(void) {
    long event;

    SpuStart();
    SpuInitMalloc(4, g_AkaoSpuMallocTable);
    SpuSetTransferMode(0);
    SpuSetTransferStartAddr(0x1010);
    Spu_UploadWithPrepare(g_AkaoSilentLoopSample, sizeof(g_AkaoSilentLoopSample));
    Spu_WaitTransferDone();
    Spu_InitVoiceState();
    SpuSetIRQ(0);
    SpuSetIRQCallback(0);
    do {
    } while (SetRCnt(RCntCNT2, 0x44E8, RCntMdINTR) == 0);
    do {
    } while (StartRCnt(RCntCNT2) == 0);
    do {
        event = OpenEvent(RCntCNT2, EvSpINT, EvMdINTR, Akao_TimerCallback);
        g_AkaoTimerEventDesc = event;
    } while (event == -1);
    do {
    } while (EnableEvent(g_AkaoTimerEventDesc) == 0);
}

void Spu_SetSampleTableBase(unsigned short *table) {
    g_AkaoSampleOffsetTable = table;
    g_AkaoSampleDataBase = (char *)&table[0x400];
}

void Spu_Shutdown(void) {
    unsigned int counter;

    if (g_SpuTransferStatus == 1) {
        Spu_UploadWithPrepare(g_AkaoSilentEndSample, sizeof(g_AkaoSilentEndSample));
        Spu_WaitTransferDone();
    }

    do {
        counter = RCntCNT2;
    } while (StopRCnt(counter) == 0);

    UnDeliverEvent(RCntCNT2, EvSpINT);

    while (DisableEvent(g_AkaoTimerEventDesc) == 0) {
    }

    while (CloseEvent(g_AkaoTimerEventDesc) == 0) {
    }

    Spu_WriteKeyOff(0xFFFFFF);
    SpuQuit();
}

/* Original silence blocks and native SPU allocator/event storage. */
unsigned char g_AkaoSilentLoopSample[0x20] = {
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x0c,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};
unsigned char g_AkaoSilentEndSample[0x40] = {
    0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};
_Alignas(8) char g_AkaoSpuMallocTable[40];
long g_AkaoTimerEventDesc;
