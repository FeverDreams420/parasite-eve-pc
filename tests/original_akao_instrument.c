#include "pe1/akao.h"
#include "pe1/akao/globals.h"
#include "pe1/akao/note_step.h"
#include "pe1/akao/voice_masks.h"
#include "pe1/psyq_spu_internal.h"
#include <stdio.h>
#include <stdlib.h>

/* Explicit input tables for isolated original procedures, not runtime audio. */
AkaoInstrument g_AkaoInstrumentTable[256];
int g_AkaoPitchPeriodTable[256 * 16];
AkaoSequencerState g_AkaoSequencerState;
#define bank g_AkaoSequencerState.primary
AkaoSequencerBank *g_AkaoCurTrack = &bank;
static unsigned short offsets[2048];
unsigned short *g_AkaoSampleOffsetTable = offsets;
static char sample_data[65536];
char *g_AkaoSampleDataBase = sample_data;
AkaoStagedCommand g_AkaoCmd;
AkaoSequenceCounter g_AkaoCmd24Sequence = {0x400};
AkaoQueueEntry g_AkaoMessageQueue[16];
int g_AkaoMessageQueueCount;
static AkaoQueueEntry received[16];
static unsigned received_count;
static void record_message(AkaoQueueEntry *entry)
{
    if (received_count >= 16) abort();
    received[received_count++] = *entry;
}
AkaoMessageHandler Akao_MessageHandlers[256];
volatile u32 g_AkaoMessageQueueLock;
static AkaoTickBank tick_bank;
AkaoTickBank *D_8009D2C8 = &tick_bank;
SpuReverbState g_SpuReverbState;
AkaoTrack g_AkaoVoiceChannelTable[12];
unsigned g_SpuActiveVoiceMask;
unsigned g_SpuPendingKeyOffMask;
unsigned g_AkaoTrack34Mask, g_AkaoTrack38Mask, g_AkaoTrack3CMask;
unsigned g_AkaoVoiceMaskScratch, g_AkaoVoiceUpdateFlags;
AkaoTrack g_AkaoVoiceStateTable[48], g_AkaoVoiceStateTable2[24];
AkaoSequencerBank g_AkaoTrackStateBackup;
AkaoVoiceBank g_AkaoVoiceStateBackup;
unsigned g_AkaoVoiceKeyOnState, g_AkaoSeqPendingFlags;
unsigned short g_AkaoSelectedBankId;
char g_AkaoDefaultVoiceProgram[] = {0};
/* This isolated command test must never request a hardware reverb change.
 * PsyZ has no SpuSetReverbModeType endpoint yet: fail instead of simulating it. */
int SpuSetReverbModeType(unsigned int mode)
{
    fprintf(stderr, "unexpected hardware reverb request %u\n", mode);
    abort();
}
void SeqOp_SetTempo(void *track);
void SeqOp_LoadInstrumentFromIndex(void *track);
void Seq_SetCurrentTrackField56(AkaoValueCommand *command);
void SeqOp_StreamPair(unsigned char **stream);
void SeqOp_ReadTrack64U16(unsigned char **stream);
void SeqOp_SetTrack34Mask(AkaoTrack *track, u32 mask);
void SeqOp_ClearTrack34Mask(AkaoTrack *track, u32 mask);
void Spu_StopActiveVoices(void);
void Akao_UpdateVoiceMask(AkaoU8 *new_base);
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"line %d: %s\n",__LINE__,#x); return 1; } } while(0)

int main(void)
{
    unsigned int sound_header[4] = {0x4f414b41u, 0x12345678u, 0, 0};
    for (unsigned i = 0; i < 256; ++i) Akao_MessageHandlers[i] = record_message;
    for (unsigned i = 0; i < 1025; ++i) {
        g_AkaoMessageQueueCount = 0;
        int sequence = Akao_Cmd_24(sound_header, 0x12345678, 0x180, 0xff);
        CHECK(sequence == (int)(0x400 + (i & 0x1ff)));
        CHECK(g_AkaoMessageQueueCount == 1);
        CHECK(g_AkaoMessageQueue[0].opcode.word == 0x24);
        CHECK(g_AkaoMessageQueue[0].arg0.sequence == (unsigned char *)(sound_header + 1));
        CHECK(g_AkaoMessageQueue[0].arg1 == 0x345678);
        CHECK(g_AkaoMessageQueue[0].arg2 == 0x80);
        CHECK(g_AkaoMessageQueue[0].arg3 == 0x7f);
        CHECK(g_AkaoMessageQueue[0].sequence == sequence);
        CHECK(g_AkaoMessageQueueLock == 0);
        received_count = 0;
        Akao_ProcessMessageQueue();
        CHECK(g_AkaoMessageQueueCount == 0);
        CHECK(received_count == 1);
        CHECK(received[0].arg0.sequence == (unsigned char *)(sound_header + 1));
        CHECK(received[0].sequence == sequence);
    }
    g_AkaoCmd.opcode = 0xda;
    g_AkaoCmd.arg0.value = 19;
    g_AkaoCmd.arg1 = 23;
    g_AkaoCmd.arg2 = 31;
    Akao_EnqueueStagedCommand();
    CHECK(g_AkaoMessageQueueCount == 2);
    received_count = 0;
    Akao_ProcessMessageQueue();
    CHECK(received_count == 2);
    CHECK(received[0].opcode.word == 0xd2);
    CHECK(received[1].opcode.word == 0xd6);
    for (unsigned i = 0; i < 2; ++i) {
        CHECK(received[i].arg0.value == 19);
        CHECK(received[i].arg1 == 23);
        CHECK(received[i].arg2 == 31);
    }
    g_AkaoCmd.opcode = 0x80;
    g_AkaoCmd.arg0.value = 0x12345678;
    g_AkaoCmd.arg1 = 37;
    g_AkaoCmd.arg2 = 0x44556677;
    g_AkaoCmd.arg3 = 53;
    Akao_EnqueueStagedCommand();
    CHECK(g_AkaoMessageQueueCount == 1);
    AkaoQueueEntry *message = &g_AkaoMessageQueue[0];
    Seq_SetCurrentTrackField56((AkaoValueCommand *)message);
    CHECK(bank.condition_value == 0x5678);
    CHECK(((AkaoBankPitchCommand *)message)->pitch == 0x12345678);
    CHECK(((AkaoBankPitchCommand *)message)->bank_selector == 53);
    CHECK(((AkaoBankPitchSlideCommand *)message)->target == 37);
    CHECK(((AkaoGlobalParamCommand *)message)->value == 0x78);
    CHECK(((AkaoGlobalSlideRangeCommand *)message)->target == 0x77);
    CHECK(((AkaoGlobalPitchCommand *)message)->pitch == 0x5678);
    CHECK(((AkaoGlobalPitchSlideCommand *)message)->target == 37);
    CHECK(((AkaoGlobalPitchRangeCommand *)message)->start == 37);
    CHECK(((AkaoGlobalPitchRangeCommand *)message)->target == 0x6677);
    g_AkaoMessageQueueCount = 0;
    for (unsigned mode = 0; mode < 2; ++mode) {
        g_SpuActiveVoiceMask = 0x555000;
        for (unsigned i = 0; i < 12; ++i) {
            g_AkaoVoiceChannelTable[i] = (AkaoTrack){0};
            g_AkaoVoiceChannelTable[i].key_on_mask = i % 3 == 0 ? 8 : 4;
            g_AkaoVoiceChannelTable[i].key_off_mask = i % 3 == 0 ? 73 : 72;
        }
        AkaoQueueEntry command = {0};
        command.arg0.value = 73;
        command.arg1 = mode ? 8 : 0;
        command.arg2 = 0x15a;
        Spu_SetVoiceVolumeImmediateMasked(&command);
        Spu_SetVoicePanImmediateMasked(&command);
        Spu_SetVoicePitchImmediateMasked(&command);
        for (unsigned i = 0; i < 12; ++i) {
            AkaoTrack *voice = &g_AkaoVoiceChannelTable[i];
            int selected = i % 2 == 0 && i % 3 == 0;
            CHECK(voice->pan_target == (selected ? 0x5a00 : 0));
            CHECK(voice->panpot == (selected ? 0x5a00 : 0));
            CHECK(voice->pitch_offset == (selected ? 0x5a00 : 0));
            CHECK(voice->update_flags == (selected ? (AKAO_VOICE_PARAM_VOLUME | AKAO_VOICE_PARAM_PITCH) : 0));
        }
    }
    unsigned char tempo[] = {0x34, 0x12};
    AkaoTrack track = {0};
    track.pc = tempo;
    bank.tempo_slide_duration = 37;
    SeqOp_SetTempo(&track);
    CHECK(bank.tempo == 0x12340000u);
    bank.key_off_dirty_mask = 4;
    track.parent_track_id = 0;
    SeqOp_SetTrack34Mask(&track, 8);
    CHECK(bank.key_off_dirty_mask == 12);
    SeqOp_ClearTrack34Mask(&track, 4);
    CHECK(bank.key_off_dirty_mask == 8);
    track.parent_track_id = 1;
    SeqOp_SetTrack34Mask(&track, 16);
    CHECK(g_AkaoTrack34Mask == 16);
    CHECK(bank.key_off_dirty_mask == 8);
    SeqOp_ClearTrack34Mask(&track, 16);
    CHECK(g_AkaoTrack34Mask == 0);
    for (unsigned i = 0; i < 12; ++i) {
        g_AkaoVoiceChannelTable[i] = (AkaoTrack){0};
        g_AkaoVoiceChannelTable[i].key_on_mask = i % 3 == 0 ? 0x02000000 : 1;
        g_AkaoVoiceChannelTable[i].flags = 0xffff;
    }
    g_SpuActiveVoiceMask = 0xfff000;
    Spu_StopActiveVoices();
    CHECK(g_SpuActiveVoiceMask == 0x249000);
    CHECK(g_SpuPendingKeyOffMask == 0xdb6000);
    for (unsigned i = 0; i < 12; ++i)
        CHECK(g_AkaoVoiceChannelTable[i].flags == (i % 3 == 0 ? 0xffff : 0));
    CHECK(bank.tempo_slide_duration == 0);
    CHECK(track.pc == tempo + 2);
    unsigned char timing[] = {48, 7, 0x34, 0x12};
    unsigned char *timing_cursor = timing;
    bank.sequence_base = timing;
    bank.tick = 19;
    bank.beat = 23;
    SeqOp_StreamPair(&timing_cursor);
    CHECK(timing_cursor == timing + 2);
    CHECK(bank.ticks_per_beat == 48);
    CHECK(bank.beats_per_measure == 7);
    CHECK(bank.tick == 0 && bank.beat == 0);
    SeqOp_ReadTrack64U16(&timing_cursor);
    CHECK(timing_cursor == timing + 4);
    CHECK(bank.measure == 0x1234);
    CHECK(bank.sequence_base == timing);
    CHECK(bank.tempo == 0x12340000u);
    for (unsigned i = 0; i < 1024; ++i) {
        offsets[i * 2] = i * 31;
        offsets[i * 2 + 1] = i % 2 ? 0xffff : i * 17;
        void *first = NULL, *second = NULL;
        Akao_LoadSamplePairFromIndex(&first, &second, i | 0x400);
        CHECK(first == sample_data + i * 31);
        CHECK(second == (i % 2 ? NULL : sample_data + i * 17));
    }
    offsets[0] = 0xffff;
    offsets[1] = 0;
    void *first = sample_data, *second = NULL;
    Akao_LoadSamplePairFromIndex(&first, &second, 0);
    CHECK(first == NULL);
    CHECK(second == sample_data);
    for (unsigned i = 0; i < 256; ++i) {
        AkaoInstrument *instrument = &g_AkaoInstrumentTable[i];
        instrument->adsr_attack_rate = i;
        instrument->adsr_decay_rate = i ^ 0x55;
        instrument->adsr_sustain_level = i ^ 0xaa;
        instrument->adsr_sustain_rate = i + 1;
        instrument->adsr_release_rate = i + 2;
        instrument->adsr_attack_mode = i & 1;
        instrument->adsr_sustain_mode = (i >> 1) & 1;
        instrument->adsr_release_mode = (i >> 2) & 1;
        track.note_pitch = i;
        track.update_flags = 3;
        SeqOp_LoadInstrumentFromIndex(&track);
        CHECK(track.adsr_attack_rate == instrument->adsr_attack_rate);
        CHECK(track.adsr_decay_rate == instrument->adsr_decay_rate);
        CHECK(track.adsr_sustain_level == instrument->adsr_sustain_level);
        CHECK(track.adsr_sustain_rate == instrument->adsr_sustain_rate);
        CHECK(track.adsr_release_rate == instrument->adsr_release_rate);
        CHECK(track.adsr_attack == instrument->adsr_attack_mode);
        CHECK(track.adsr_sustain == instrument->adsr_sustain_mode);
        CHECK(track.adsr_release == instrument->adsr_release_mode);
        CHECK(track.update_flags == (3 | AKAO_VOICE_PARAM_ADSR_ALL));
        CHECK(track.pc == tempo + 2);
        for (unsigned note = 0; note < 12; ++note)
            g_AkaoPitchPeriodTable[i * 16 + note] = 4096 + note * 32 + i;
        CHECK(Akao_LookupPitchPeriod(i, 72, 0) == 4096 + i);
        CHECK(Akao_LookupPitchPeriod(i, 60, 0) == (4096 + i) / 2);
        CHECK(Akao_LookupPitchPeriod(i, 84, 0) == (4096 + i) * 2);
        CHECK(Akao_LookupPitchPeriod(i, 73, 0) == 4128 + i);
        for (unsigned note = 0; note < 256; ++note) {
            for (int detune = -128; detune <= 127; detune += 17) {
                unsigned octave = note / 12;
                unsigned period = g_AkaoPitchPeriodTable[i * 16 + note % 12];
                period += (period * (unsigned)detune) >> 7;
                if (octave >= 7) period <<= octave - 6;
                else if (octave < 6) period >>= 6 - octave;
                CHECK(Akao_LookupPitchPeriod(i, note, detune) == (int)(period & 0xffff));
            }
        }
    }
    unsigned char old_sequence[256] = {0}, new_sequence[256] = {0};
    g_AkaoTrackStateBackup = (AkaoSequencerBank){0};
    g_AkaoTrackStateBackup.sequence_base = old_sequence;
    g_AkaoTrackStateBackup.active_voice_mask = 0x555555;
    g_AkaoTrackStateBackup.volume = 0x12345678;
    for (unsigned i = 0; i < 24; ++i) {
        AkaoTrack *voice = &g_AkaoVoiceStateBackup.tracks[i];
        *voice = (AkaoTrack){0};
        voice->pc = old_sequence + i;
        voice->branch_target = old_sequence + i + 24;
        for (unsigned j = 0; j < 4; ++j) voice->call_stack[j] = old_sequence + i + 48 + j;
        voice->note_length = 11;
        voice->pan_duration = 13;
    }
    Akao_UpdateVoiceMask(new_sequence);
    CHECK(bank.sequence_base == new_sequence);
    CHECK(bank.volume == 0x12345678);
    for (unsigned i = 0; i < 24; ++i) {
        AkaoTrack *voice = &g_AkaoVoiceStateTable[i];
        if (i % 2 == 0) {
            CHECK(voice->pc == new_sequence + i);
            CHECK(voice->branch_target == new_sequence + i + 24);
            for (unsigned j = 0; j < 4; ++j) CHECK(voice->call_stack[j] == new_sequence + i + 48 + j);
            CHECK(voice->note_length == 13 && voice->pan_duration == 15);
        } else {
            CHECK(voice->pc == (unsigned char *)g_AkaoDefaultVoiceProgram);
            CHECK(voice->note_length == 4 && voice->pan_duration == 2);
        }
    }
    puts("original AKAO: commands, native voice state and sequence rebasing pass");
    return 0;
}
