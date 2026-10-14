#include "pe1/akao.h"
#include "pe1/akao/globals.h"
#include "pe1/akao/note_step.h"
#include "pe1/akao/voice_masks.h"
#include "pe1/psyq_spu_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "akao_sequence.h"

/* Explicit input tables for isolated original procedures, not runtime audio. */
AkaoInstrument g_AkaoInstrumentTable[256];
AkaoSequencerState g_AkaoSequencerState;
#define bank g_AkaoSequencerState.primary
AkaoCurrentBankPointer g_AkaoCurrentBankPointer = {.pointer = &bank};
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
SpuReverbState g_SpuReverbState;
unsigned g_SpuActiveVoiceMask;
unsigned g_SpuPendingKeyOffMask;
unsigned g_AkaoTrack34Mask, g_AkaoTrack38Mask, g_AkaoTrack3CMask;
unsigned g_AkaoVoiceMaskScratch, g_AkaoVoiceUpdateFlags;
AkaoSequencerBank g_AkaoTrackStateBackup;
AkaoVoiceBank g_AkaoVoiceStateBackup;
unsigned g_AkaoVoiceKeyOnState, g_AkaoSeqPendingFlags;
unsigned short g_AkaoSelectedBankId;
char g_AkaoDefaultVoiceProgram[] = {0};
short g_AkaoPanVolumeTable[256];
int g_AkaoPlaybackMode;
unsigned char g_AkaoGlobalPitchScale;
AkaoTickTrack g_AkaoSequencerTracks[60];
AkaoU8 g_AkaoOpcodeArgLengths[96], g_AkaoExtOpcodeArgLengths[32];
u16 g_AkaoNoteLengthTable[11] = {48, 24, 16, 12, 8, 6, 4, 3, 2, 1, 96};
AkaoNestedSource g_AkaoNestedSource;
int g_AkaoSeqLoopCounter;
unsigned g_AkaoSfxAllocatedVoiceMask, g_SpuStoppedVoiceMask;
u16 g_AkaoTrack5ATransposeValue;
void *g_AkaoSoundEntryTable[256];
/* This isolated command test must never request a hardware reverb change.
 * PsyZ has no SpuSetReverbModeType endpoint yet: fail instead of simulating it. */
int SpuSetReverbModeType(unsigned int mode)
{
    fprintf(stderr, "unexpected hardware reverb request %u\n", mode);
    abort();
}
void Seq_SetCurrentTrackField56(AkaoValueCommand *command);
void SeqOp_StreamPair(AkaoTrack *track, u32 voice_mask);
void SeqOp_ReadTrack64U16(AkaoTrack *track, u32 voice_mask);
void SeqOp_SetTrack34Mask(AkaoTrack *track, u32 mask);
void SeqOp_ClearTrack34Mask(AkaoTrack *track, u32 mask);
void Spu_StopActiveVoices(void);
void Akao_UpdateVoiceMask(AkaoU8 *new_base);
void Akao_InitVoiceState(AkaoTrack *track, void *script);
void Akao_InitPrimarySecondaryVoices(void);
void Akao_InitPrimarySecondaryVoicesWithMode(AkaoValueCommand *command);
void Akao_StepSequencerVoice(void *sequence);
void Akao_SetVoiceKeyOff(AkaoTrack *track, u32 voice_mask, int index);
void Akao_SetVoiceKeyOn(AkaoTrack *track, u32 voice_mask);
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"line %d: %s\n",__LINE__,#x); return 1; } } while(0)

static int asset_audit(void)
{
    const char *path = getenv("PE_TEST_DISC");
    if (!path || !*path) return getenv("PE_REQUIRE_GAME_ASSETS") ? 1 : 77;
    PeDisc disc;
    PeRoomCatalog catalog;
    CHECK(pe_disc_open(&disc, path));
    CHECK(pe_room_catalog_open(&catalog, &disc));
    unsigned rooms = 0, sequences = 0, short_records = 0, channels = 0;
    for (unsigned map = 1; map <= PE_USA_ROOM_SLOTS; ++map) {
        if (!catalog.ranges[map - 1].room_sectors) continue;
        PeRoom room = {0};
        CHECK(pe_room_load(&catalog, map, &room));
        ++rooms;
        const uint8_t *records;
        unsigned count;
        CHECK(pe_room_blob_table(&room.data, 12, 12, &records, &count));
        for (unsigned i = 0; i < count; ++i) {
            PeRoomMusic music;
            char error[256];
            CHECK(pe_room_music(&room, i, &music, error, sizeof(error)));
            if (music.unresolved_short_record) { ++short_records; continue; }
            unsigned char *snapshot = malloc(music.sequence.size);
            CHECK(snapshot != NULL);
            memcpy(snapshot, music.sequence.bytes, music.sequence.size);
            g_AkaoSequencerState = (AkaoSequencerState){0};
            g_AkaoSeqPendingFlags = 0;
            g_AkaoVoiceKeyOnState = 0;
            g_SpuActiveVoiceMask = 0;
            Akao_StepSequencerVoice((void *)(music.sequence.bytes + 16));
            CHECK(bank.active_voice_mask == music.sequence.active);
            CHECK(bank.pending_voice_mask == music.sequence.pending);
            CHECK(bank.yield_voice_mask == music.sequence.yield);
            for (unsigned voice = 0; voice < 24; ++voice) {
                if (music.sequence.channels[voice] == UINT32_MAX) {
                    CHECK(g_AkaoVoiceStateTable[voice].pc == (unsigned char *)g_AkaoDefaultVoiceProgram);
                } else {
                    CHECK(g_AkaoVoiceStateTable[voice].pc == music.sequence.bytes + music.sequence.channels[voice]);
                    CHECK(g_AkaoVoiceStateTable[voice].assigned_voice_index == 24);
                    ++channels;
                }
            }
            CHECK(memcmp(snapshot, music.sequence.bytes, music.sequence.size) == 0);
            free(snapshot);
            ++sequences;
        }
        pe_room_close(&room);
    }
    CHECK(rooms != 0 && sequences != 0 && channels != 0);
    pe_disc_close(&disc);
    printf("original AKAO start: %u rooms, %u sequences, %u channels; %u short records not executed\n",
           rooms, sequences, channels, short_records);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc == 2 && strcmp(argv[1], "--assets") == 0) return asset_audit();
    CHECK((void *)D_8009D2C8 == (void *)&bank);
    D_8009D2C8++;
    CHECK(g_AkaoCurTrack == &g_AkaoSequencerState.secondary);
    --g_AkaoCurTrack;
    CHECK((void *)D_8009D2C8 == (void *)&bank);
    for (unsigned i = 0; i < 60; ++i) {
        g_AkaoVoiceStateTable[i].flags = i + 1;
        CHECK(g_AkaoSequencerTracks[i].track.flags == i + 1);
        g_AkaoTrackStateArray[i].update_flags = i + 73;
        CHECK(g_AkaoVoiceStateTable[i].update_flags == i + 73);
    }
    CHECK(g_AkaoVoiceStateTable2 == &g_AkaoSequencerTracks[24].track);
    CHECK(g_AkaoVoiceChannelTable == &g_AkaoSequencerTracks[48].track);
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
    SeqOp_SetTempo(&track, 1);
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
    track.pc = timing_cursor;
    SeqOp_StreamPair(&track, 1);
    timing_cursor = track.pc;
    CHECK(timing_cursor == timing + 2);
    CHECK(bank.ticks_per_beat == 48);
    CHECK(bank.beats_per_measure == 7);
    CHECK(bank.tick == 0 && bank.beat == 0);
    SeqOp_ReadTrack64U16(&track, 1);
    timing_cursor = track.pc;
    CHECK(timing_cursor == timing + 4);
    CHECK(bank.measure == 0x1234);
    CHECK(bank.sequence_base == timing);
    CHECK(bank.tempo == 0x12340000u);
    track.pc = tempo + 2;
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
        SeqOp_LoadInstrumentFromIndex(&track, 1);
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
            g_AkaoInstrumentTable[i].pitch[note] = 4096 + note * 32 + i;
        CHECK(Akao_LookupPitchPeriod(i, 72, 0) == 4096 + i);
        CHECK(Akao_LookupPitchPeriod(i, 60, 0) == (4096 + i) / 2);
        CHECK(Akao_LookupPitchPeriod(i, 84, 0) == (4096 + i) * 2);
        CHECK(Akao_LookupPitchPeriod(i, 73, 0) == 4128 + i);
        for (unsigned note = 0; note < 256; ++note) {
            for (int detune = -128; detune <= 127; detune += 17) {
                unsigned octave = note / 12;
                unsigned period = g_AkaoInstrumentTable[i].pitch[note % 12];
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
    AkaoTrack initialized = {0};
    initialized.flags = 0xffffffff;
    initialized.assigned_voice_index = 19;
    initialized.update_flags = 3;
    Akao_InitVoiceState(&initialized, new_sequence);
    CHECK(initialized.pc == new_sequence);
    CHECK(initialized.flags == 0);
    CHECK(initialized.volume_base == 0x6e00);
    CHECK(initialized.expression_value == 0x32000000);
    CHECK(initialized.sample_header == g_AkaoInstrumentTable[0].start_address);
    CHECK(initialized.adsr_release_rate == g_AkaoInstrumentTable[0].adsr_release_rate);
    CHECK(initialized.update_flags == (3 | AKAO_VOICE_PARAM_INSTRUMENT_FULL));
    CHECK(initialized.assigned_voice_index == 19);
    for (unsigned mode = 0; mode < 3; ++mode) {
        bank.active_voice_mask = 1;
        bank.bank_id = 73;
        g_AkaoSequencerState.secondary.active_voice_mask = 1;
        g_AkaoSequencerState.secondary.bank_id = 74;
        for (unsigned i = 0; i < 48; ++i) {
            g_AkaoVoiceStateTable[i] = (AkaoTrack){0};
            g_AkaoVoiceStateTable[i].pc = new_sequence;
            g_AkaoVoiceStateTable[i].note_length = 11;
            g_AkaoVoiceStateTable[i].pan_duration = 13;
            g_AkaoVoiceStateTable[i].adsr_release_rate = 17;
        }
        if (mode == 0) Akao_InitPrimarySecondaryVoices();
        else {
            AkaoValueCommand command = {0};
            command.value = mode == 1 ? 73 : 75;
            Akao_InitPrimarySecondaryVoicesWithMode(&command);
        }
        CHECK(g_AkaoCurTrack == &bank);
        for (unsigned i = 0; i < 48; ++i) {
            int reset = mode == 0 || (mode == 1 && i < 24);
            AkaoTrack *voice = &g_AkaoVoiceStateTable[i];
            CHECK(voice->pc == (reset ? (unsigned char *)g_AkaoDefaultVoiceProgram : new_sequence));
            CHECK(voice->note_length == (reset ? 3 : 11));
            CHECK(voice->pan_duration == (reset ? 1 : 13));
            CHECK(voice->adsr_release_rate == (reset ? 5 : 17));
        }
    }
    unsigned int sequence_words[64] = {0x800005, 0x123, 0x456, 0};
    unsigned char *sequence_bytes = (unsigned char *)sequence_words;
    sequence_bytes[16] = 30;
    sequence_bytes[18] = 32;
    sequence_bytes[20] = 34;
    for (unsigned paused = 0; paused < 2; ++paused) {
        bank = (AkaoSequencerBank){0};
        g_AkaoSequencerState.secondary = (AkaoSequencerBank){0};
        g_AkaoSeqPendingFlags = paused;
        g_SpuActiveVoiceMask = 0;
        g_AkaoVoiceKeyOnState = 0;
        for (unsigned i = 0; i < 48; ++i) g_AkaoVoiceStateTable[i] = (AkaoTrack){0};
        Akao_StepSequencerVoice(sequence_bytes);
        CHECK(bank.sequence_base == sequence_bytes);
        CHECK(bank.active_voice_mask == (paused ? 0 : 0x800005));
        CHECK(bank.pending_restore_mask == (paused ? 0x800005 : 0));
        CHECK(bank.pending_voice_mask == 0x123);
        CHECK(bank.yield_voice_mask == 0x456);
        CHECK(bank.tempo == 0xffff0000u);
        CHECK(bank.tick_accumulator == 1);
        CHECK(bank.allocated_voice_mask == 0xffffff);
        for (unsigned i = 0; i < 24; ++i) {
            AkaoTrack *voice = &g_AkaoVoiceStateTable[i];
            if (i == 0 || i == 2 || i == 23) {
                unsigned index = i == 0 ? 0 : i == 2 ? 1 : 2;
                CHECK(voice->pc == sequence_bytes + 18 + index * 2 + 30 + index * 2);
                CHECK(voice->assigned_voice_index == 24);
                CHECK(voice->note_length == 4 && voice->pan_duration == 2);
                CHECK(voice->panpot == 0x8000);
            } else {
                CHECK(voice->pc == (unsigned char *)g_AkaoDefaultVoiceProgram);
                CHECK(voice->note_length == 3 && voice->pan_duration == 1);
            }
        }
    }
    unsigned char slide[] = {0, 64};
    AkaoTrack lfo = {0};
    lfo.pc = slide;
    lfo.pitch_lfo_target = 0x2000;
    SeqOp_SetPanSlide(&lfo, 1);
    CHECK(lfo.pc == slide + 2);
    CHECK(lfo.pitch_lfo_slide_duration == 256);
    CHECK(lfo.pitch_lfo_delta == 32);
    lfo.pc = slide;
    lfo.volume_lfo_target = 0x6000;
    SeqOp_SetExpressionSlide(&lfo, 1);
    CHECK(lfo.volume_lfo_slide_duration == 256);
    CHECK(lfo.volume_lfo_delta == -32);
    lfo.pc = slide;
    lfo.aux_lfo_target = 0x1000;
    SeqOp_PitchSlide(&lfo, 1);
    CHECK(lfo.aux_lfo_slide_duration == 256);
    CHECK(lfo.aux_lfo_delta == 16);
    unsigned char branches[16] = {0};
    branches[8] = 0xf8;
    branches[9] = 0xff;
    lfo.pc = branches + 8;
    lfo.expression_value = (int)0xffed1234u;
    SeqOp_SetBranchTarget(&lfo, 1);
    CHECK(lfo.pc == branches + 10);
    CHECK(lfo.branch_target == branches + 2);
    CHECK(lfo.volume == -19);
    CHECK(lfo.flags & AKAO_TRACK_FLAG_BRANCH_ACTIVE);
    CHECK(lfo.pitch_lfo_target == 0x2000 && lfo.volume_lfo_target == 0x6000);
    short wave[] = {32767, -32768, 0, 0, -2};
    AkaoTrack modulated = {0};
    modulated.flags = AKAO_TRACK_FLAG_PITCH_LFO | AKAO_TRACK_FLAG_VOLUME_LFO | AKAO_TRACK_FLAG_AUX_LFO;
    modulated.pitch_lfo_table = wave;
    modulated.volume_lfo_table = wave;
    modulated.aux_lfo_table = wave;
    modulated.pitch_lfo_phase = modulated.volume_lfo_phase = modulated.aux_lfo_phase = 1;
    modulated.pitch_lfo_duration = modulated.volume_lfo_duration = modulated.aux_lfo_duration = 1;
    modulated.pitch_lfo_depth = 256;
    modulated.volume_lfo_target = modulated.aux_lfo_target = 0x4000;
    modulated.expression_value = 128 << 16;
    modulated.volume_base = 0x4000;
    modulated.pitch_base = 4096;
    bank.pitch_current = 127 << 16;
    AkaoTrack modulation_start = modulated;
    for (unsigned path = 0; path < 3; ++path) {
    modulated = modulation_start;
    if (path == 2) modulated.expression_value = (int)0xff800000u;
    for (unsigned i = 0; i < 16; ++i) {
        modulated.update_flags = 0;
        if (path == 0) Akao_SetVoiceKeyOff(&modulated, 1, 0);
        else Akao_SetVoiceKeyOn(&modulated, 1);
        CHECK(modulated.pitch_lfo_value == (i % 2 ? -128 : 254));
        CHECK(modulated.volume_lfo_value == (path == 2 ? (i % 2 ? 32 : -32) : (i % 2 ? -32 : 31)));
        CHECK(modulated.aux_lfo_value == (i % 2 ? -64 : 63));
        CHECK(modulated.pitch == (i % 2 ? 3968 : 4350));
        CHECK(modulated.update_flags == (AKAO_VOICE_PARAM_VOLUME | AKAO_VOICE_PARAM_PITCH));
        CHECK(modulated.pitch_lfo_table == wave + (i % 2 ? 2 : 1));
        CHECK(modulated.volume_lfo_table == wave + (i % 2 ? 2 : 1));
        CHECK(modulated.aux_lfo_table == wave + (i % 2 ? 2 : 1));
    }
    }
    AkaoCommandHandler handlers[] = {SeqOp_Ignore, SeqOp_Noop_904AC,
        SeqOp_Noop_904B4, SeqOp_Noop_904BC, SeqOp_Skip2, SeqOp_JumpRelative};
    AkaoTrack dispatch_track = {0};
    dispatch_track.pc = branches + 8;
    for (unsigned i = 0; i < 4; ++i) {
        handlers[i](&dispatch_track, 1);
        CHECK(dispatch_track.pc == branches + 8);
    }
    handlers[4](&dispatch_track, 1);
    CHECK(dispatch_track.pc == branches + 10);
    dispatch_track.pc = branches + 8;
    handlers[5](&dispatch_track, 1);
    CHECK(dispatch_track.pc == branches + 2);
    unsigned char parameters[] = {0xff, 0x80, 0xe0, 0x5a, 0x37};
    AkaoCommandHandler parameter_handlers[] = {SeqOp_ReadExpression,
        SeqOp_ReadDetune, Akao_SetPanpot, SeqOp_ReadVolume, Akao_SetReleaseRate,
        SeqOp_ClearFlag3};
    dispatch_track.pc = parameters;
    dispatch_track.flags = AKAO_TRACK_FLAG_BRANCH_ACTIVE;
    for (unsigned i = 0; i < 6; ++i) parameter_handlers[i](&dispatch_track, 0x800000);
    CHECK(dispatch_track.pc == parameters + 5);
    CHECK(dispatch_track.expression == -1);
    CHECK(dispatch_track.detune == -128);
    CHECK(dispatch_track.panpot == 0x2000);
    CHECK(dispatch_track.volume_base == 0x5a00);
    CHECK(dispatch_track.adsr_release_rate == 0x37);
    CHECK(dispatch_track.flags == AKAO_TRACK_FLAG_RELEASE_RATE_OVERRIDE);
    CHECK(dispatch_track.update_flags == (AKAO_VOICE_PARAM_VOLUME | AKAO_VOICE_PARAM_ADSR_RELEASE));
    unsigned char bytecode[] = {0xa3, 0x5a, 0xc0, 0xff, 0x8f, 0xa0};
    AkaoTrack interpreted = {0};
    interpreted.pc = bytecode;
    interpreted.assigned_voice_index = 2;
    bank.active_voice_mask = 1;
    bank.allocated_voice_mask = 1;
    bank.key_off_request_mask = 0;
    Akao_StepSampleLoader(&interpreted, 1);
    CHECK(interpreted.pc == bytecode + 5);
    CHECK(interpreted.volume_base == 0x5a00);
    CHECK(interpreted.expression == -1);
    CHECK(interpreted.flags & 0x40);
    CHECK(bank.key_off_request_mask == 1);
    Akao_StepSampleLoader(&interpreted, 1);
    CHECK(interpreted.pc == bytecode + 6);
    CHECK(bank.active_voice_mask == 0);
    unsigned char phrase[] = {0x00, 0x84, 0x8f, 0xa0};
    g_AkaoInstrumentTable[0].pitch[0] = 4096;
    for (unsigned child = 0; child < 2; ++child) {
        u32 mask = child ? 0x1000 : 1;
        AkaoTrack note = {0};
        note.pc = phrase;
        note.parent_track_id = child;
        note.panpot_step = 6;
        note.assigned_voice_index = 2;
        bank = (AkaoSequencerBank){0};
        bank.active_voice_mask = 1;
        g_AkaoVoiceMaskScratch = 0;
        g_AkaoSfxAllocatedVoiceMask = 0;
        g_SpuActiveVoiceMask = mask;
        Akao_StepSampleLoader(&note, mask);
        CHECK(note.pc == phrase + 1);
        CHECK(note.pitch_base == 4096);
        CHECK(note.current_note == 72);
        CHECK(note.note_length == 48 && note.pan_duration == 48);
        CHECK(bank.key_on_request_mask == (child ? 0 : 1));
        CHECK(bank.allocated_voice_mask == (child ? 0 : 1));
        CHECK(g_AkaoVoiceMaskScratch == (child ? mask : 0));
        CHECK(g_AkaoSfxAllocatedVoiceMask == (child ? mask : 0));
        note.note_length = 0;
        bank.key_on_request_mask = 0;
        g_AkaoVoiceMaskScratch = 0;
        Akao_StepSampleLoader(&note, mask);
        CHECK(note.pc == phrase + 2);
        CHECK(note.pitch_base == 4096);
        CHECK(note.note_length == 48 && note.pan_duration == 46);
        CHECK(bank.key_on_request_mask == 0 && g_AkaoVoiceMaskScratch == 0);
        note.note_length = 0;
        Akao_StepSampleLoader(&note, mask);
        CHECK(note.pc == phrase + 3);
        CHECK(note.flags & 0x40);
        CHECK(note.note_length == 48 && note.pan_duration == 46);
        if (!child) CHECK(bank.key_off_request_mask == 1);
        Akao_StepSampleLoader(&note, mask);
        CHECK(note.pc == phrase + 4);
        CHECK(note.flags == 0);
        if (child) CHECK(g_SpuActiveVoiceMask == 0);
        else CHECK(bank.active_voice_mask == 0);
    }
    puts("original AKAO: full dispatch executes notes, ties, rests and stop for BGM/SFX");
    return 0;
}
