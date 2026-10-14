#include "pe1/akao.h"
#include "pe1/menu_sound.h"
#include "pe1/akao/init_state.h"
#include "pe1/akao/stream.h"
#include "pe1/akao/globals.h"
#include "pe1/akao/note_step.h"
#include "pe1/akao/voice_masks.h"
#include "pe1/akao/voice_state.h"
#include "pe1/akao/spu_registers.h"
#include "pe1/psyq_spu_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "akao_sequence.h"
#include "spu_bank.h"
#include <psyz/spu.h>

/* Explicit input tables for isolated original procedures, not runtime audio. */
#define fixture_bank g_AkaoSequencerState.primary
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
char g_AkaoDefaultVoiceProgram[4] = {0};
short g_AkaoPanVolumeTable[256];
AkaoU8 g_AkaoOpcodeArgLengths[96], g_AkaoExtOpcodeArgLengths[32];
u16 g_AkaoNoteLengthTable[11] = {48, 24, 16, 12, 8, 6, 4, 3, 2, 1, 96};
void *g_AkaoSoundEntryTable[256];
void *g_AkaoBgmHandle[2];
void Seq_GetGlobalPitch(unsigned int *out);
void Seq_ApplyGlobalPitch(void);
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

static int asset_audit(int execute)
{
    const char *path = getenv("PE_TEST_DISC");
    if (!path || !*path) return getenv("PE_REQUIRE_GAME_ASSETS") ? 1 : 77;
    PeDisc disc;
    PeRoomCatalog catalog;
    CHECK(pe_disc_open(&disc, path));
    CHECK(pe_room_catalog_open(&catalog, &disc));
    memcpy(g_AkaoOpcodeArgLengths, catalog.akao.lengths, 96);
    memcpy(g_AkaoExtOpcodeArgLengths, catalog.akao.extended_lengths, 32);
    memcpy(g_AkaoNoteLengthTable, catalog.akao.note_lengths, sizeof(g_AkaoNoteLengthTable));
    for (unsigned i = 0; i < 16; ++i)
        g_AkaoSoundEntryTable[i] = catalog.akao.lfo_words + catalog.akao.lfo_offsets[i];
    if (execute >= 2) {
        PeDiscFile executable;
        unsigned char tables[516];
        CHECK(pe_disc_find(&disc, "SLUS_006.62", &executable));
        CHECK(pe_disc_read(&disc, &executable, 0x9b8f4 - 0x10000 + 0x800, tables, sizeof(tables)));
        memcpy(g_AkaoDefaultVoiceProgram, tables, 4);
        for (unsigned j = 0; j < 256; ++j)
            g_AkaoPanVolumeTable[j] = (short)(tables[4 + j * 2] | (unsigned)tables[5 + j * 2] << 8);
        Psyz_SpuInit();
        SpuInit();
    }
    unsigned long long audible_samples = 0;
    uint64_t pcm_hash = UINT64_C(14695981039346656037);
    PeSpuBank *resident = calloc(1, sizeof(*resident));
    CHECK(resident != NULL);
    CHECK(pe_spu_bank_load_startup(&catalog, resident, NULL));
    memcpy(g_AkaoInstrumentTable, resident->instruments, sizeof(AkaoInstrument) * 256);
    if (execute == 3) {
        PeRoomBlob menu = {0};
        CHECK(pe_sector_bank_load(&catalog, 7, &menu));
        g_AkaoBgmHandle[0] = menu.bytes;
        Akao_MessageHandlers[0x24] = Seq_StartRelativeNestedStream;
        void (*sounds[])(void) = {Menu_PlayConfirmSound, Menu_PlayCancelSound,
                                 Menu_PlayMoveSound, Menu_PlayErrorSound};
        for (unsigned sound = 0; sound < 4; ++sound) {
            Psyz_SpuReset(0);
            Psyz_SpuMemWrite(0, resident->ram, sizeof(resident->ram));
            Spu_InitVoiceState();
            Psyz_SpuWrite(0x1aa, Psyz_SpuRead(0x1aa) | 0xc000);
            sounds[sound]();
            CHECK(g_AkaoMessageQueueCount == 1);
            Akao_ProcessMessageQueue();
            CHECK(g_SpuActiveVoiceMask != 0);
            unsigned nonzero = 0;
            for (unsigned tick = 0; tick < 240; ++tick) {
                short pcm[368];
                Akao_Tick();
                unsigned frames = ((tick + 1) * 44100 / 240) - (tick * 44100 / 240);
                Psyz_SpuPullSamples(pcm, frames);
                for (unsigned j = 0; j < frames * 2; ++j) nonzero += pcm[j] != 0;
            }
            CHECK(nonzero != 0);
            printf("original menu sound %x: %u nonzero samples\n", 0x44c + sound, nonzero);
        }
        g_AkaoBgmHandle[0] = NULL;
        pe_room_blob_close(&menu);
        free(resident);
        pe_disc_close(&disc);
        return 0;
    }
    PeSpuBank *record_samples = malloc(sizeof(*record_samples));
    CHECK(record_samples != NULL);
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
            if (execute) {
                *record_samples = *resident;
                uint8_t *sample_bytes = NULL;
                size_t sample_size = 0;
                CHECK(pe_audio_bank_read(&catalog, music.bank, &sample_bytes, &sample_size));
                PeSpuBankStats stats;
                CHECK(pe_spu_bank_upload_rebased(record_samples, sample_bytes, sample_size,
                    0x8000, 0x20, &stats, error, sizeof(error)));
                g_AkaoSequencerState = (AkaoSequencerState){0};
                g_AkaoCurTrack = &fixture_bank;
                memcpy(g_AkaoInstrumentTable, resident->instruments, sizeof(AkaoInstrument) * 256);
                if (execute == 2) {
                    Psyz_SpuReset(0);
                    Psyz_SpuMemWrite(0, resident->ram, sizeof(resident->ram));
                }
                Spu_SetStreamModeA();
                CHECK(Akao_StepNoteSequencer(sample_bytes, (unsigned)sample_size) == 0);
                CHECK(g_SpuTransferStatus == 0);
                CHECK(memcmp(g_AkaoInstrumentTable, record_samples->instruments,
                    sizeof(AkaoInstrument) * 256) == 0);
                uint8_t *ram = malloc(stats.bytes);
                CHECK(ram != NULL);
                Psyz_SpuMemRead(stats.address, ram, stats.bytes);
                CHECK(memcmp(ram, record_samples->ram + stats.address, stats.bytes) == 0);
                free(ram);
                free(sample_bytes);
            }
            unsigned char *snapshot = malloc(music.sequence.size);
            CHECK(snapshot != NULL);
            memcpy(snapshot, music.sequence.bytes, music.sequence.size);
            g_AkaoSequencerState = (AkaoSequencerState){0};
            g_AkaoSeqPendingFlags = 0;
            g_AkaoVoiceKeyOnState = 0;
            g_SpuActiveVoiceMask = 0;
            if (execute == 2) {
                Spu_InitVoiceState();
                Psyz_SpuWrite(0x1aa, Psyz_SpuRead(0x1aa) | 0xc000);
            }
            Akao_StepSequencerVoice((void *)(music.sequence.bytes + 16));
            CHECK(fixture_bank.active_voice_mask == music.sequence.active);
            CHECK(fixture_bank.pending_voice_mask == music.sequence.pending);
            CHECK(fixture_bank.yield_voice_mask == music.sequence.yield);
            for (unsigned voice = 0; voice < 24; ++voice) {
                if (music.sequence.channels[voice] == UINT32_MAX) {
                    CHECK(g_AkaoVoiceStateTable[voice].pc == (unsigned char *)g_AkaoDefaultVoiceProgram);
                } else {
                    CHECK(g_AkaoVoiceStateTable[voice].pc == music.sequence.bytes + music.sequence.channels[voice]);
                    CHECK(g_AkaoVoiceStateTable[voice].assigned_voice_index == 24);
                    if (execute == 1) {
                        if (getenv("PE_AKAO_PREFIX_TRACE"))
                            fprintf(stderr, "original prefix: map %u sequence %u voice %u\n", map, i, voice);
                        Akao_StepSampleLoader(&g_AkaoVoiceStateTable[voice], 1u << voice);
                        uintptr_t pc = (uintptr_t)g_AkaoVoiceStateTable[voice].pc;
                        CHECK(pc >= (uintptr_t)music.sequence.bytes);
                        CHECK(pc <= (uintptr_t)music.sequence.bytes + music.sequence.size);
                    }
                    ++channels;
                }
            }
            if (execute == 2) {
                if (getenv("PE_AKAO_PREFIX_TRACE")) fprintf(stderr, "tick map %u sequence %u\n", map, i);
                for (unsigned tick = 0; tick < 240; ++tick) {
                    short pcm[368];
                    Akao_Tick();
                    unsigned frames = ((tick + 1) * 44100 / 240) - (tick * 44100 / 240);
                    Psyz_SpuPullSamples(pcm, frames);
                    for (unsigned j = 0; j < frames * 2; ++j) {
                        audible_samples += pcm[j] != 0;
                        pcm_hash = (pcm_hash ^ ((uint16_t)pcm[j] & 255)) * UINT64_C(1099511628211);
                        pcm_hash = (pcm_hash ^ ((uint16_t)pcm[j] >> 8)) * UINT64_C(1099511628211);
                    }
                }
            }
            CHECK(memcmp(snapshot, music.sequence.bytes, music.sequence.size) == 0);
            free(snapshot);
            ++sequences;
        }
        pe_room_close(&room);
    }
    CHECK(rooms != 0 && sequences != 0 && channels != 0);
    if (execute == 2) { CHECK(audible_samples != 0); printf("nonzero PCM samples: %llu; PCM hash: %016llx\n", audible_samples, (unsigned long long)pcm_hash); }
    free(record_samples);
    free(resident);
    pe_disc_close(&disc);
    printf("original AKAO %s: %u rooms, %u sequences, %u channels; %u short records not executed\n",
           execute == 2 ? "240 ticks and PCM" : execute ? "start and first note/stop" : "start", rooms, sequences, channels, short_records);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc == 2 && strcmp(argv[1], "--menu-assets") == 0) return asset_audit(3);
    if (argc == 2 && strcmp(argv[1], "--tick-assets") == 0) return asset_audit(2);
    if (argc == 2 && strcmp(argv[1], "--assets") == 0) return asset_audit(0);
    if (argc == 2 && strcmp(argv[1], "--execute-assets") == 0) return asset_audit(1);
    g_AkaoGlobalPitchDepth = 0x12345678;
    CHECK(g_AkaoGlobalPitchScale == 0x34);
    g_AkaoGlobalPitchScale = 0xab;
    CHECK(g_AkaoGlobalPitchDepth == 0x12ab5678);
    g_AkaoGlobalPitchDepth = 0;
    g_AkaoTempoScaleAccum = 0x12345678;
    CHECK(g_AkaoTempoScale == 0x34);
    g_AkaoTempoScale = 0x80;
    CHECK(g_AkaoTempoScaleAccum == 0x12805678);
    g_AkaoTempoScaleAccum = 0;
    g_AkaoCdInputVolumeAccum = 0x12345678;
    CHECK(g_AkaoCdInputVolume == 0x1234);
    g_AkaoCdInputVolume = 0x7fff;
    CHECK(g_AkaoCdInputVolumeAccum == 0x7fff5678);
    g_AkaoCdInputVolumeAccum = 0;
    CHECK((void *)D_8009D2C8 == (void *)&fixture_bank);
    D_8009D2C8++;
    CHECK(g_AkaoCurTrack == &g_AkaoSequencerState.secondary);
    --g_AkaoCurTrack;
    CHECK((void *)D_8009D2C8 == (void *)&fixture_bank);
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
    CHECK(fixture_bank.condition_value == 0x5678);
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
    fixture_bank.tempo_slide_duration = 37;
    SeqOp_SetTempo(&track, 1);
    CHECK(fixture_bank.tempo == 0x12340000u);
    fixture_bank.key_off_dirty_mask = 4;
    track.parent_track_id = 0;
    SeqOp_SetTrack34Mask(&track, 8);
    CHECK(fixture_bank.key_off_dirty_mask == 12);
    SeqOp_ClearTrack34Mask(&track, 4);
    CHECK(fixture_bank.key_off_dirty_mask == 8);
    track.parent_track_id = 1;
    SeqOp_SetTrack34Mask(&track, 16);
    CHECK(g_AkaoTrack34Mask == 16);
    CHECK(fixture_bank.key_off_dirty_mask == 8);
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
    CHECK(fixture_bank.tempo_slide_duration == 0);
    CHECK(track.pc == tempo + 2);
    unsigned char timing[] = {48, 7, 0x34, 0x12};
    unsigned char *timing_cursor = timing;
    fixture_bank.sequence_base = timing;
    fixture_bank.tick = 19;
    fixture_bank.beat = 23;
    track.pc = timing_cursor;
    SeqOp_StreamPair(&track, 1);
    timing_cursor = track.pc;
    CHECK(timing_cursor == timing + 2);
    CHECK(fixture_bank.ticks_per_beat == 48);
    CHECK(fixture_bank.beats_per_measure == 7);
    CHECK(fixture_bank.tick == 0 && fixture_bank.beat == 0);
    SeqOp_ReadTrack64U16(&track, 1);
    timing_cursor = track.pc;
    CHECK(timing_cursor == timing + 4);
    CHECK(fixture_bank.measure == 0x1234);
    CHECK(fixture_bank.sequence_base == timing);
    CHECK(fixture_bank.tempo == 0x12340000u);
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
    CHECK(fixture_bank.sequence_base == new_sequence);
    CHECK(fixture_bank.volume == 0x12345678);
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
        fixture_bank.active_voice_mask = 1;
        fixture_bank.bank_id = 73;
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
        CHECK(g_AkaoCurTrack == &fixture_bank);
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
        fixture_bank = (AkaoSequencerBank){0};
        g_AkaoSequencerState.secondary = (AkaoSequencerBank){0};
        g_AkaoSeqPendingFlags = paused;
        g_SpuActiveVoiceMask = 0;
        g_AkaoVoiceKeyOnState = 0;
        for (unsigned i = 0; i < 48; ++i) g_AkaoVoiceStateTable[i] = (AkaoTrack){0};
        Akao_StepSequencerVoice(sequence_bytes);
        CHECK(fixture_bank.sequence_base == sequence_bytes);
        CHECK(fixture_bank.active_voice_mask == (paused ? 0 : 0x800005));
        CHECK(fixture_bank.pending_restore_mask == (paused ? 0x800005 : 0));
        CHECK(fixture_bank.pending_voice_mask == 0x123);
        CHECK(fixture_bank.yield_voice_mask == 0x456);
        CHECK(fixture_bank.tempo == 0xffff0000u);
        CHECK(fixture_bank.tick_accumulator == 1);
        CHECK(fixture_bank.allocated_voice_mask == 0xffffff);
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
    fixture_bank.pitch_current = 127 << 16;
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
    fixture_bank.active_voice_mask = 1;
    fixture_bank.allocated_voice_mask = 1;
    fixture_bank.key_off_request_mask = 0;
    Akao_StepSampleLoader(&interpreted, 1);
    CHECK(interpreted.pc == bytecode + 5);
    CHECK(interpreted.volume_base == 0x5a00);
    CHECK(interpreted.expression == -1);
    CHECK(interpreted.flags & 0x40);
    CHECK(fixture_bank.key_off_request_mask == 1);
    Akao_StepSampleLoader(&interpreted, 1);
    CHECK(interpreted.pc == bytecode + 6);
    CHECK(fixture_bank.active_voice_mask == 0);
    unsigned char phrase[] = {0x00, 0x84, 0x8f, 0xa0};
    g_AkaoInstrumentTable[0].pitch[0] = 4096;
    for (unsigned child = 0; child < 2; ++child) {
        u32 mask = child ? 0x1000 : 1;
        AkaoTrack note = {0};
        note.pc = phrase;
        note.parent_track_id = child;
        note.panpot_step = 6;
        note.assigned_voice_index = 2;
        fixture_bank = (AkaoSequencerBank){0};
        fixture_bank.active_voice_mask = 1;
        g_AkaoVoiceMaskScratch = 0;
        g_AkaoSfxAllocatedVoiceMask = 0;
        g_SpuActiveVoiceMask = mask;
        Akao_StepSampleLoader(&note, mask);
        CHECK(note.pc == phrase + 1);
        CHECK(note.pitch_base == 4096);
        CHECK(note.current_note == 72);
        CHECK(note.note_length == 48 && note.pan_duration == 48);
        CHECK(fixture_bank.key_on_request_mask == (child ? 0 : 1));
        CHECK(fixture_bank.allocated_voice_mask == (child ? 0 : 1));
        CHECK(g_AkaoVoiceMaskScratch == (child ? mask : 0));
        CHECK(g_AkaoSfxAllocatedVoiceMask == (child ? mask : 0));
        note.note_length = 0;
        fixture_bank.key_on_request_mask = 0;
        g_AkaoVoiceMaskScratch = 0;
        Akao_StepSampleLoader(&note, mask);
        CHECK(note.pc == phrase + 2);
        CHECK(note.pitch_base == 4096);
        CHECK(note.note_length == 48 && note.pan_duration == 46);
        CHECK(fixture_bank.key_on_request_mask == 0 && g_AkaoVoiceMaskScratch == 0);
        note.note_length = 0;
        Akao_StepSampleLoader(&note, mask);
        CHECK(note.pc == phrase + 3);
        CHECK(note.flags & 0x40);
        CHECK(note.note_length == 48 && note.pan_duration == 46);
        if (!child) CHECK(fixture_bank.key_off_request_mask == 1);
        Akao_StepSampleLoader(&note, mask);
        CHECK(note.pc == phrase + 4);
        CHECK(note.flags == 0);
        if (child) CHECK(g_SpuActiveVoiceMask == 0);
        else CHECK(fixture_bank.active_voice_mask == 0);
    }
    for (unsigned encoded_count = 0; encoded_count <= 3; encoded_count += 3) {
        unsigned char loop[] = {0xc8, 0x8f, 0xc9, encoded_count, 0xa0};
        unsigned repetitions = encoded_count ? encoded_count : 256;
        AkaoTrack repeated = {0};
        repeated.pc = loop;
        fixture_bank.active_voice_mask = 1;
        g_AkaoOpcodeArgLengths[0xc8 - 0xa0] = 1;
        for (unsigned i = 0; i < repetitions; ++i) {
            repeated.note_length = 0;
            Akao_StepSampleLoader(&repeated, 1);
            CHECK(repeated.pc == loop + 2);
            CHECK(repeated.call_stack_index == 1);
            CHECK(repeated.call_stack[1] == loop + 1);
            CHECK(repeated.repeat_counters[1] == i);
        }
        Akao_StepSampleLoader(&repeated, 1);
        CHECK(repeated.pc == loop + 5);
        CHECK(repeated.call_stack_index == 0);
        CHECK(repeated.repeat_counters[1] == repetitions);
        CHECK(fixture_bank.active_voice_mask == 0);
    }
    for (unsigned pop = 0; pop < 2; ++pop) {
        for (unsigned taken = 0; taken < 2; ++taken) {
            unsigned char branch[16] = {0};
            branch[8] = 2;
            branch[9] = 0xf8;
            branch[10] = 0xff;
            AkaoTrack conditional = {0};
            conditional.pc = branch + 8;
            conditional.call_stack_index = 1;
            conditional.repeat_counters[1] = taken;
            g_AkaoExtOpcodeHandlers[pop ? 9 : 8](&conditional, 1);
            CHECK(conditional.pc == branch + (taken ? 3 : 11));
            CHECK(conditional.call_stack_index == (pop && taken ? 0 : 1));
        }
    }
    void (*write_parameters)(int, AkaoVoiceParams *) = Akao_WriteVoiceParam;
    AkaoVoiceParams quiet = {0};
    quiet.pitch = 4096;
    quiet.volume_left = 17;
    quiet.volume_right = 23;
    write_parameters(2, &quiet);
    CHECK(quiet.flags == 0 && quiet.pitch == 4096);
    CHECK(quiet.volume_left == 17 && quiet.volume_right == 23);
    memset(_spu_RXX, 0, sizeof(*_spu_RXX));
    for (unsigned voice = 0; voice < 24; ++voice) {
        AkaoVoiceParams parameters = {0};
        parameters.flags = AKAO_VOICE_PARAM_INSTRUMENT_FULL | AKAO_VOICE_PARAM_VOLUME | AKAO_VOICE_PARAM_PITCH;
        parameters.volume_left = -1;
        parameters.volume_right = (short)0x8123;
        parameters.pitch = 0x3456;
        parameters.start_address = 0x12340;
        parameters.loop_address = 0x56780;
        parameters.adsr_attack_rate = 0x12;
        parameters.adsr_attack_mode = 4;
        parameters.adsr_decay_rate = 3;
        parameters.adsr_sustain_level = 5;
        parameters.adsr_sustain_rate = 0x34;
        parameters.adsr_sustain_mode = 6;
        parameters.adsr_release_rate = 0x11;
        parameters.adsr_release_mode = 4;
        write_parameters(voice, &parameters);
        CHECK(parameters.flags == 0);
        CHECK(_spu_RXX->voice[voice].volume_left == 0x7fff);
        CHECK(_spu_RXX->voice[voice].volume_right == 0x123);
        CHECK(_spu_RXX->voice[voice].pitch == 0x3456);
        CHECK(_spu_RXX->voice[voice].start_address == 0x2468);
        CHECK(_spu_RXX->voice[voice].repeat_address == 0xacf0);
        CHECK(_spu_RXX->voice[voice].adsr_low == 0x9235);
        CHECK(_spu_RXX->voice[voice].adsr_high == 0xcd31);
    }
    Spu_WriteKeyOn(0x12345678);
    Spu_WriteKeyOff(0x3456789a);
    Spu_WriteFmEnable(0x56789abc);
    Spu_WriteNoiseEnable(0x789abcde);
    Spu_WriteReverbEnable(0x9abcdef0);
    CHECK(_spu_RXX->key_on[0] == 0x5678 && _spu_RXX->key_on[1] == 0x1234);
    CHECK(_spu_RXX->key_off[0] == 0x789a && _spu_RXX->key_off[1] == 0x3456);
    CHECK(_spu_RXX->pitch_modulation[0] == 0x9abc && _spu_RXX->pitch_modulation[1] == 0x5678);
    CHECK(_spu_RXX->noise_enable[0] == 0xbcde && _spu_RXX->noise_enable[1] == 0x789a);
    CHECK(_spu_RXX->reverb_enable[0] == 0xdef0 && _spu_RXX->reverb_enable[1] == 0x9abc);
    for (int voice = 0; voice < 24; ++voice) {
        short level = -1;
        Psyz_SpuWrite(voice * 16 + 12, 0x1234 + voice);
        SpuGetVoiceEnvelope(voice, &level);
        CHECK(level == 0x1234 + voice);
    }
    SpuSetReverbModeDepth(-1234, 5678);
    CHECK(Psyz_SpuRead(0x184) == (u16)-1234);
    CHECK(Psyz_SpuRead(0x186) == 5678);
    SpuInit();
    for (unsigned mode = 0; mode < 10; ++mode) {
        unsigned actual;
        SpuReverbAttr attr;
        CHECK(SpuSetReverbModeType(mode | 0x100) == 0);
        Seq_GetGlobalPitch(&actual);
        CHECK(actual == mode);
        SpuGetReverbModeParam(&attr);
        CHECK(attr.depth.left == 0 && attr.depth.right == 0);
        CHECK(attr.delay == ((mode == 7 || mode == 8) ? 127 : 0));
        CHECK(attr.feedback == (mode == 7 ? 127 : 0));
    }
    CHECK(SpuSetReverbModeType(10) == -1);
    CHECK(SpuSetReverbModeType(0) == 0);
    g_AkaoGlobalSlideTick = 0;
    g_AkaoGlobalD2D0SlideCounter = 2;
    g_AkaoGlobalD2D0SlideStep = 0x10000;
    g_AkaoTempoScaleAccum = 0x400000;
    g_AkaoGlobalD2CCSlideCounter = 2;
    g_AkaoGlobalD2CCSlideStep = 0x10000;
    g_AkaoGlobalPitchDepth = 0;
    g_SpuActiveVoiceMask = 0;
    fixture_bank = (AkaoSequencerBank){0};
    g_AkaoSequencerState.secondary = (AkaoSequencerBank){0};
    fixture_bank.active_voice_mask = g_AkaoSequencerState.secondary.active_voice_mask = 1;
    fixture_bank.pitch_current = 0x200000;
    fixture_bank.pitch_delta = 0x10000;
    fixture_bank.pitch_slide_duration = 2;
    g_AkaoSequencerState.secondary.pitch_current = 0x300000;
    g_AkaoSequencerState.secondary.pitch_delta = -0x10000;
    g_AkaoSequencerState.secondary.pitch_slide_duration = 2;
    for (unsigned i = 0; i < 60; ++i) g_AkaoVoiceStateTable[i].update_flags = 0;
    for (unsigned tick = 1; tick <= 12; ++tick) {
        Spu_StepReverbLoad();
        unsigned steps = tick / 4;
        if (steps > 2) steps = 2;
        CHECK(g_AkaoTempoScaleAccum == (int)(0x400000 + steps * 0x10000));
        CHECK(g_AkaoGlobalPitchDepth == (int)(steps * 0x10000));
        CHECK(fixture_bank.pitch_current == (int)(0x200000 + steps * 0x10000));
        CHECK(g_AkaoSequencerState.secondary.pitch_current == (int)(0x300000 - steps * 0x10000));
        CHECK(g_AkaoCurTrack == &fixture_bank);
    }
    CHECK(g_AkaoVoiceStateTable[0].update_flags == (AKAO_VOICE_PARAM_PITCH | AKAO_VOICE_PARAM_VOLUME));
    CHECK(g_AkaoVoiceStateTable[1].update_flags == AKAO_VOICE_PARAM_PITCH);
    CHECK(g_AkaoVoiceStateTable[24].update_flags == AKAO_VOICE_PARAM_VOLUME);
    CHECK(g_AkaoVoiceStateTable[25].update_flags == 0);
    unsigned char end_note[] = {0xa0};
    g_AkaoSequencerState = (AkaoSequencerState){0};
    memset(g_AkaoSequencerTracks, 0, sizeof(g_AkaoSequencerTracks));
    g_AkaoVoiceStateTable[0].pc = end_note;
    g_AkaoVoiceStateTable[0].assigned_voice_index = 24;
    g_AkaoVoiceStateTable[0].note_length = 2;
    g_AkaoVoiceStateTable[0].pan_duration = 1;
    fixture_bank.active_voice_mask = fixture_bank.allocated_voice_mask = 1;
    fixture_bank.tempo = 1 << 16;
    g_AkaoSeqPendingFlags = 4;
    g_SpuActiveVoiceMask = g_SpuPendingKeyOffMask = g_SpuStoppedVoiceMask = 0;
    g_AkaoVoiceUpdateFlags = 0;
    g_AkaoGlobalSlideTick = 0;
    g_AkaoGlobalPitchDepth = g_AkaoTempoScaleAccum = 0;
    Akao_Tick();
    CHECK(g_AkaoVoiceStateTable[0].note_length == 1);
    CHECK(g_AkaoVoiceStateTable[0].pan_duration == 0);
    CHECK(fixture_bank.allocated_voice_mask == 0);
    CHECK(fixture_bank.active_voice_mask == 1);
    Akao_Tick();
    CHECK(fixture_bank.active_voice_mask == 0);
    CHECK(g_AkaoVoiceStateTable[0].pc == end_note + 1);
    CHECK(g_AkaoGlobalSlideTick == 2);
    CHECK(g_AkaoCurTrack == &fixture_bank);
    Spu_InitVoiceState();
    CHECK(Psyz_SpuRead(0x180) == 0x3fff && Psyz_SpuRead(0x182) == 0x3fff);
    CHECK(Psyz_SpuRead(0x1b0) == 0x7fff && Psyz_SpuRead(0x1b2) == 0x7fff);
    CHECK((Psyz_SpuRead(0x1aa) & 0x8f) == 0x81);
    unsigned initialized_mode;
    Seq_GetGlobalPitch(&initialized_mode);
    CHECK(initialized_mode == 4);
    CHECK(g_AkaoCurTrack == &fixture_bank);
    CHECK(g_AkaoSeqBusyFlags == 0 && g_AkaoMessageQueueCount == 0);
    for (unsigned i = 0; i < 60; ++i) {
        AkaoTrack *voice = &g_AkaoSequencerTracks[i].track;
        CHECK(voice->assigned_voice_index == (i < 48 ? 24 : i - 36));
        CHECK(voice->parent_track_id == (i < 48 ? 0 : 1));
        CHECK(voice->flags == 0);
    }
    /* Dispatch the original 0x24 handler with native pointers and both
     * relative streams, then the absent-stream sentinel. */
    unsigned relative_header[3] = {0x4f414b41, 0x00010000, 0x0000a0a0};
    unsigned char *relative_streams = (unsigned char *)(relative_header + 1);
    Akao_MessageHandlers[0x24] = Seq_StartRelativeNestedStream;
    AkaoQueueEntry *relative = &g_AkaoMessageQueue[0];
    int sound_id = Akao_Cmd_24(relative_header, 0x100, 0x80, 0x7f);
    CHECK(sound_id >= 0x400 && sound_id < 0x600);
    CHECK(g_AkaoMessageQueueCount == 1);
    Akao_ProcessMessageQueue();
    CHECK(g_AkaoMessageQueueCount == 0);
    CHECK(g_SpuActiveVoiceMask == 0xc00000);
    for (unsigned i = 10; i < 12; ++i) {
        AkaoTrack *voice = &g_AkaoVoiceChannelTable[i];
        CHECK(voice->pc == relative_streams + 4 + i - 10);
        CHECK(voice->key_off_mask == (unsigned)sound_id && voice->key_on_mask == 0x100);
        CHECK(voice->panpot == 0x8000 && voice->pan_target == 0x7f00);
    }
    Spu_InitVoiceState();
    unsigned char absent_streams[] = {255, 255, 255, 255};
    relative->arg0.sequence = absent_streams;
    Seq_StartRelativeNestedStream(relative);
    CHECK(g_SpuActiveVoiceMask == 0);
    unsigned sample_block[36] = {0};
    sample_block[0] = 0x4f414b41;
    sample_block[4] = 0x2000;
    sample_block[5] = 16;
    sample_block[6] = 3;
    sample_block[7] = 4;
    for (unsigned i = 16; i < 36; ++i) sample_block[i] = i * 0x01010101u;
    SpuSetTransferMode(0);
    CHECK(Spu_UploadSampleBlock((int *)sample_block, 0) == 0);
    CHECK(g_SpuTransferStatus == 0);
    CHECK(memcmp(&g_AkaoInstrumentTable[3], sample_block + 16, 64) == 0);
    unsigned sample_readback[4];
    Psyz_SpuMemRead(0x2000, sample_readback, 16);
    CHECK(memcmp(sample_readback, sample_block + 32, 16) == 0);
    CHECK(Spu_UploadSampleBlock((int *)sample_block, 1) == 0);
    CHECK(g_SpuTransferStatus == 0);
    short reference_pcm[1024];
    for (unsigned run = 0; run < 2; ++run) {
        Psyz_SpuInit();
        Psyz_SpuReset(0);
        unsigned char adpcm[16];
        memset(adpcm, 0x11, sizeof(adpcm));
        adpcm[0] = 0;
        adpcm[1] = 7;
        Psyz_SpuMemWrite(0x1000, adpcm, sizeof(adpcm));
        SpuCommonAttr common = {0};
        common.mask = 3;
        common.mvol.left = common.mvol.right = 0x3fff;
        SpuSetCommonAttr(&common);
        CHECK(Psyz_SpuRead(0x180) == 0x3fff);
        CHECK(Psyz_SpuRead(0x182) == 0x3fff);
        g_AkaoCdInputVolumeAccum = 0x12340000;
        Seq_ApplyGlobalPitch();
        CHECK(Psyz_SpuRead(0x1b0) == 0x1234);
        CHECK(Psyz_SpuRead(0x1b2) == 0x1234);
        CHECK(Psyz_SpuRead(0x180) == 0x3fff);
        CHECK(Psyz_SpuRead(0x182) == 0x3fff);
        Psyz_SpuWrite(0x1aa, 0xc000);
        AkaoVoiceParams audible = {0};
        audible.flags = AKAO_VOICE_PARAM_INSTRUMENT_FULL | AKAO_VOICE_PARAM_VOLUME | AKAO_VOICE_PARAM_PITCH;
        audible.start_address = audible.loop_address = 0x1000;
        audible.pitch = 0x1000;
        audible.volume_left = 0x3fff;
        audible.volume_right = 0x2000;
        audible.adsr_attack_rate = 0;
        audible.adsr_sustain_level = 15;
        audible.adsr_release_rate = 0;
        Akao_WriteVoiceParam(0, &audible);
        short pcm[1024];
        Psyz_SpuPullSamples(pcm, 512);
        for (unsigned i = 0; i < 1024; ++i) CHECK(pcm[i] == 0);
        Spu_WriteKeyOn(1);
        Psyz_SpuPullSamples(pcm, 512);
        int peak_left = 0, peak_right = 0;
        for (unsigned i = 0; i < 512; ++i) {
            if (abs(pcm[i * 2]) > peak_left) peak_left = abs(pcm[i * 2]);
            if (abs(pcm[i * 2 + 1]) > peak_right) peak_right = abs(pcm[i * 2 + 1]);
        }
        CHECK(peak_left > 100 && peak_right > 50);
        CHECK(abs(peak_left - peak_right * 2) <= 2);
        if (run == 0) memcpy(reference_pcm, pcm, sizeof(pcm));
        else CHECK(memcmp(reference_pcm, pcm, sizeof(pcm)) == 0);
        Spu_WriteKeyOff(1);
        Psyz_SpuPullSamples(pcm, 512);
        for (unsigned i = 768; i < 1024; ++i) CHECK(pcm[i] == 0);
    }
    puts("original AKAO: full tick and deterministic headless key-on/off PCM pass");
    return 0;
}
