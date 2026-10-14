#ifndef PE1_AKAO_TRACK_H
#define PE1_AKAO_TRACK_H

#include "common.h"

typedef unsigned char AkaoU8;
typedef unsigned short AkaoU16;
typedef unsigned int AkaoU32;
typedef signed short AkaoS16;

enum AkaoVoiceParamFlags {
    AKAO_VOICE_PARAM_VOLUME_L = 1 << 0,
    AKAO_VOICE_PARAM_VOLUME_R = 1 << 1,
    AKAO_VOICE_PARAM_VOLUME = AKAO_VOICE_PARAM_VOLUME_L | AKAO_VOICE_PARAM_VOLUME_R,
    AKAO_VOICE_PARAM_PITCH = 1 << 4,
    AKAO_VOICE_PARAM_START_ADDR = 1 << 7,
    AKAO_VOICE_PARAM_ADSR_ATTACK_MODE = 1 << 8,
    AKAO_VOICE_PARAM_ADSR_SUSTAIN_MODE = 1 << 9,
    AKAO_VOICE_PARAM_ADSR_RELEASE_MODE = 1 << 10,
    AKAO_VOICE_PARAM_ADSR_ATTACK_RATE = 1 << 11,
    AKAO_VOICE_PARAM_ADSR_DECAY_RATE = 1 << 12,
    AKAO_VOICE_PARAM_ADSR_SUSTAIN_RATE = 1 << 13,
    AKAO_VOICE_PARAM_ADSR_RELEASE_RATE = 1 << 14,
    AKAO_VOICE_PARAM_ADSR_SUSTAIN_LEVEL = 1 << 15,
    AKAO_VOICE_PARAM_LOOP_ADDR = 1 << 16,
    AKAO_VOICE_PARAM_ADSR_ATTACK =
        AKAO_VOICE_PARAM_ADSR_ATTACK_MODE | AKAO_VOICE_PARAM_ADSR_ATTACK_RATE,
    AKAO_VOICE_PARAM_ADSR_SUSTAIN =
        AKAO_VOICE_PARAM_ADSR_SUSTAIN_MODE | AKAO_VOICE_PARAM_ADSR_SUSTAIN_RATE,
    AKAO_VOICE_PARAM_ADSR_RELEASE =
        AKAO_VOICE_PARAM_ADSR_RELEASE_MODE | AKAO_VOICE_PARAM_ADSR_RELEASE_RATE,
    AKAO_VOICE_PARAM_ADSR_ALL =
        AKAO_VOICE_PARAM_ADSR_ATTACK | AKAO_VOICE_PARAM_ADSR_DECAY_RATE |
        AKAO_VOICE_PARAM_ADSR_SUSTAIN | AKAO_VOICE_PARAM_ADSR_RELEASE |
        AKAO_VOICE_PARAM_ADSR_SUSTAIN_LEVEL,
    AKAO_VOICE_PARAM_ADSR_DECAY_SUSTAIN =
        AKAO_VOICE_PARAM_ADSR_DECAY_RATE | AKAO_VOICE_PARAM_ADSR_SUSTAIN_LEVEL,
    AKAO_VOICE_PARAM_INSTRUMENT_NO_RELEASE =
        AKAO_VOICE_PARAM_START_ADDR | AKAO_VOICE_PARAM_LOOP_ADDR |
        AKAO_VOICE_PARAM_ADSR_ATTACK | AKAO_VOICE_PARAM_ADSR_DECAY_RATE |
        AKAO_VOICE_PARAM_ADSR_SUSTAIN | AKAO_VOICE_PARAM_ADSR_SUSTAIN_LEVEL,
    AKAO_VOICE_PARAM_INSTRUMENT_FULL =
        AKAO_VOICE_PARAM_INSTRUMENT_NO_RELEASE | AKAO_VOICE_PARAM_ADSR_RELEASE,
    AKAO_VOICE_PARAM_RESUME =
        AKAO_VOICE_PARAM_VOLUME | AKAO_VOICE_PARAM_PITCH |
        AKAO_VOICE_PARAM_ADSR_ATTACK | AKAO_VOICE_PARAM_ADSR_SUSTAIN
};

enum AkaoTrackFlags {
    AKAO_TRACK_FLAG_PITCH_LFO = 1 << 0,
    AKAO_TRACK_FLAG_VOLUME_LFO = 1 << 1,
    AKAO_TRACK_FLAG_AUX_LFO = 1 << 2,
    AKAO_TRACK_FLAG_BRANCH_ACTIVE = 1 << 3,
    AKAO_TRACK_FLAG_10 = 1 << 4,
    AKAO_TRACK_FLAG_20 = 1 << 5,
    AKAO_TRACK_FLAG_RELEASE_RATE_OVERRIDE = 1 << 9,
    AKAO_TRACK_FLAG_VOICE_ALLOCATED = 1 << 11,
    AKAO_TRACK_FLAG_PENDING_NOTE_PITCH = 1 << 12,
    /* Set by sequence op 0x9 (SeqOp_SetSustainFlag): a key-off request
     * does not silence the voice but sets KEY_OFF_PENDING instead. */
    AKAO_TRACK_FLAG_SUSTAIN = 1 << 20,
    /* Deferred key-off (Akao_VoiceSequence); opcode 0xCA ends the phrase
     * while it is set. */
    AKAO_TRACK_FLAG_KEY_OFF_PENDING = 1 << 21,
    AKAO_TRACK_FLAG_STOP_CLEAR_MASK =
        AKAO_TRACK_FLAG_PITCH_LFO | AKAO_TRACK_FLAG_VOLUME_LFO |
        AKAO_TRACK_FLAG_AUX_LFO | AKAO_TRACK_FLAG_10 | AKAO_TRACK_FLAG_20
};

enum AkaoGlobalUpdateFlags {
    AKAO_GLOBAL_UPDATE_TEMPO = 1 << 7,
    AKAO_GLOBAL_UPDATE_VOICE_MODES = 1 << 8
};

enum AkaoVoiceMask {
    AKAO_SPU_VOICE_SFX_START_INDEX = 12,
    AKAO_SPU_VOICE_SFX_START_MASK = 1 << AKAO_SPU_VOICE_SFX_START_INDEX
};

typedef struct AkaoInstrument {
    /* 0x00 */ AkaoU32 start_address;
    /* 0x04 */ AkaoU32 loop_address;
    /* 0x08 */ AkaoU8 adsr_attack_rate;
    /* 0x09 */ AkaoU8 adsr_decay_rate;
    /* 0x0A */ AkaoU8 adsr_sustain_level;
    /* 0x0B */ AkaoU8 adsr_sustain_rate;
    /* 0x0C */ AkaoU8 adsr_release_rate;
    /* 0x0D */ AkaoU8 adsr_attack_mode;
    /* 0x0E */ AkaoU8 adsr_sustain_mode;
    /* 0x0F */ AkaoU8 adsr_release_mode;
    /* 0x10 */ AkaoU32 pitch[12];
} AkaoInstrument;
typedef char AkaoInstrumentSizeCheck[
    (sizeof(AkaoInstrument) == 0x40) ? 1 : -1];

typedef struct AkaoVoiceLoopRange {
    /* 0x00 */ AkaoU8 note;
    /* 0x01 */ AkaoU8 min_value;
    /* 0x02 */ AkaoU8 max_value;
    /* 0x03 */ AkaoU8 adsr_attack_rate;
    /* 0x04 */ AkaoU8 adsr_sustain_rate;
    /* 0x05 */ AkaoU8 adsr_sustain_mode;
    /* 0x06 */ AkaoU8 adsr_release_rate;
    /* 0x07 */ AkaoU8 field_07;
} AkaoVoiceLoopRange;

typedef struct AkaoVoiceParams {
    /* 0x00 */ int assigned_voice_index;
    /* 0x04 */ AkaoU32 flags;
    /* 0x08 */ AkaoU32 start_address;
    /* 0x0C */ AkaoU32 loop_address;
    /* 0x10 */ int adsr_attack_mode;
    /* 0x14 */ int adsr_sustain_mode;
    /* 0x18 */ int adsr_release_mode;
    /* 0x1C */ AkaoU16 pitch;
    /* 0x1E */ AkaoU16 adsr_attack_rate;
    /* 0x20 */ AkaoU16 adsr_decay_rate;
    /* 0x22 */ AkaoU16 adsr_sustain_level;
    /* 0x24 */ AkaoU16 adsr_sustain_rate;
    /* 0x26 */ AkaoU16 adsr_release_rate;
    /* 0x28 */ AkaoS16 volume_left;
    /* 0x2A */ AkaoS16 volume_right;
} AkaoVoiceParams;

/* Eight-byte envelope slot; level zero denotes an available SPU voice. */
typedef struct AkaoVoiceEnvelopeSlot {
    AkaoS16 level;
    unsigned char reserved02[6];
} AkaoVoiceEnvelopeSlot;
typedef char AkaoVoiceEnvelopeSlotSizeCheck[
    (sizeof(AkaoVoiceEnvelopeSlot) == 8) ? 1 : -1];

/* Source descriptor passed when a nested AKAO stream allocates a voice. */
/* The pan is read as a byte and written as a whole word. */
typedef union AkaoNestedPan {
    AkaoU8 value;
    AkaoU32 word;
} AkaoNestedPan;

typedef struct AkaoNestedSource {
    /* 0x00 */ unsigned char pad_00[4];
    /* 0x04 */ AkaoU32 key_off_mask;
    /* 0x08 */ AkaoU32 key_on_mask;
    /* 0x0C */ AkaoNestedPan pan;
    /* 0x10 */ AkaoU32 pan_target;
} AkaoNestedSource;

/* One of the twelve nested-stream voice slots at g_AkaoVoiceChannelTable. */
typedef struct AkaoNestedVoiceSlot {
    /* 0x00 */ unsigned char pad_00[0x2C];
    /* 0x2C */ AkaoU32 flags;
    /* 0x30 */ unsigned char pad_30[0xEC];
} AkaoNestedVoiceSlot;

/* One 0x68-byte bank in the sequencer control state. Primary and secondary
 * banks are adjacent; code switches the active bank by adding 0x68. */
typedef struct AkaoSequencerBank {
    /* 0x00 */ AkaoU32 status_flags;
    /* 0x04 */ AkaoU32 active_voice_mask;
    /* 0x08 */ AkaoU32 pending_voice_mask;
    /* 0x0C */ AkaoU32 yield_voice_mask; /* sequence header word 2; these channels release voices that sound effects claim */
    /* 0x10 */ AkaoU32 key_on_request_mask;
    /* 0x14 */ AkaoU32 allocated_voice_mask;
    /* 0x18 */ AkaoU32 key_off_request_mask;
    /* 0x1C */ AkaoU32 pending_restore_mask;
    /* 0x20 */ AkaoU32 tempo;       /* 16.16; SeqOp_SetTempo */
    /* 0x24 */ AkaoU32 tempo_delta; /* per-tick step while tempo_slide_duration runs */
    /* 0x28 */ AkaoU32 tick_accumulator; /* beat timer accumulator, see AkaoTickBank */
    /* 0x2C */ AkaoU8 *sequence_base; /* rebased along with voice program pointers */
    /* 0x30 */ AkaoU32 reserved_voice_mask; /* voices taken by parent-track notes */
    /* 0x34 */ AkaoU32 key_off_dirty_mask;
    /* 0x38 */ AkaoU32 volume_dirty_mask;
    /* 0x3C */ AkaoU32 adsr_dirty_mask;
    /* 0x40 */ AkaoU32 volume;       /* 16.16 master volume */
    /* 0x44 */ AkaoU32 volume_delta; /* per-tick step while volume_slide_duration runs */
    /* 0x48 */ int pitch_current; /* 16.16 bank pitch, set from a 7-bit command value */
    /* 0x4C */ int pitch_delta;   /* signed per-tick increment for rising or falling slides */
    /* 0x50 */ AkaoS16 pitch_slide_duration; /* signed countdown in Spu_StepReverbLoad */
    /* 0x52 */ AkaoU16 tempo_slide_duration;
    /* 0x54 */ AkaoU16 bank_id;
    /* 0x56 */ AkaoU16 condition_value; /* compared by the conditional-jump opcode */
    /* 0x58 */ AkaoU16 volume_slide_duration;
    /* 0x5A */ AkaoU16 noise_clock; /* SpuSetNoiseClock value */
    /* 0x5C */ AkaoU16 beats_per_measure;
    /* 0x5E */ AkaoU16 beat;
    /* 0x60 */ AkaoU16 ticks_per_beat;
    /* 0x62 */ AkaoU16 tick;
    /* 0x64 */ AkaoU16 measure;
    /* 0x66 */ AkaoU16 pad_66;
} AkaoSequencerBank;

typedef struct AkaoSequencerState {
    AkaoSequencerBank primary;
    AkaoSequencerBank secondary;
} AkaoSequencerState;

/* Both original pointer names refer to the word at 0x8009D2C8. */
typedef union AkaoCurrentBankPointer {
    AkaoSequencerBank *pointer;
    union AkaoTickBank *tick;
} AkaoCurrentBankPointer;
extern AkaoCurrentBankPointer g_AkaoCurrentBankPointer;
#define g_AkaoCurTrack (g_AkaoCurrentBankPointer.pointer)
#define D_8009D2C8 (g_AkaoCurrentBankPointer.tick)

typedef struct AkaoTrack {
    /* 0x000 */ AkaoU8 *pc;
    /* 0x004 */ AkaoU8 *call_stack[4];
    /* 0x014 */ AkaoU8 *branch_target;
    /* 0x018 */ AkaoU8 *repeat_target;
    /* 0x01C */ void *pitch_lfo_table;
    /* 0x020 */ void *volume_lfo_table;
    /* 0x024 */ void *aux_lfo_table;
    /* 0x028 */ AkaoU32 key_off_mask;
    /* 0x02C */ AkaoU32 key_on_mask;
    /* 0x030 */ AkaoU32 pitch_base;
    /* 0x034 */ AkaoU32 voice_mask_a;
    /* 0x038 */ AkaoU32 flags;
    /* 0x03C */ AkaoU32 pitch_offset; /* 8.8 voice pitch command value */
    /* 0x040 */ AkaoU32 pitch_offset_step; /* signed increment accessed as a word by slides */
    /* 0x044 */ int expression_value;
    /* 0x048 */ int expression_delta;
    /* 0x04C */ int pitch_slide_step;
    /* 0x050 */ AkaoU16 field_50_duration;
    /* 0x052 */ AkaoU16 field_52_duration;
    /* 0x054 */ AkaoU16 parent_track_id;
    /* 0x056 */ AkaoU16 note_length;
    /* 0x058 */ AkaoU16 pan_duration;
    /* 0x05A */ AkaoU16 note_pitch;
    /* 0x05C */ AkaoU16 voice_index;
    /* 0x05E */ AkaoU16 pitch_slide_current;
    /* 0x060 */ AkaoU16 pitch_slide_duration;
    /* 0x062 */ AkaoU16 repeat_counters[4];
    /* 0x06A */ AkaoS16 volume;
    /* 0x06C */ AkaoU16 volume_base;
    /* 0x06E */ AkaoU16 volume_duration;
    /* 0x070 */ AkaoU16 pitch_offset_duration; /* countdown in akao/sequencer.c */
    /* 0x072 */ AkaoU16 expression_duration;
    /* 0x074 */ AkaoU16 panpot_duration;
    /* 0x076 */ AkaoU16 panpot; /* 8.8 stereo position, set by the voice pan commands */
    /* 0x078 */ AkaoU16 panpot_slide_duration;
    /* 0x07A */ AkaoU16 pitch_slide_steps;
    /* 0x07C */ AkaoU16 panpot_step;
    /* 0x07E */ AkaoU16 vibrato_duration;
    /* 0x080 */ AkaoU16 tremolo_counter;
    /* 0x082 */ AkaoU16 tremolo_duration;
    /* 0x084 */ AkaoU16 tremolo_phase;
    /* 0x086 */ unsigned char pad_086[0x02];
    /* 0x088 */ AkaoU16 pitch_lfo_delay;
    /* 0x08A */ AkaoU16 pitch_lfo_counter;
    /* 0x08C */ AkaoU16 pitch_lfo_duration;
    /* 0x08E */ AkaoU16 pitch_lfo_phase;
    /* 0x090 */ AkaoU16 pitch_lfo_selector;
    /* 0x092 */ AkaoU16 pitch_lfo_depth;
    /* 0x094 */ AkaoU16 pitch_lfo_target;
    /* 0x096 */ AkaoU16 pitch_lfo_slide_duration;
    /* 0x098 */ AkaoS16 pitch_lfo_delta;
    /* 0x09A */ AkaoS16 field_9A;
    /* 0x09C */ AkaoU16 volume_lfo_delay;
    /* 0x09E */ AkaoU16 volume_lfo_counter;
    /* 0x0A0 */ AkaoU16 volume_lfo_duration;
    /* 0x0A2 */ AkaoU16 volume_lfo_phase;
    /* 0x0A4 */ AkaoU16 volume_lfo_selector;
    /* 0x0A6 */ AkaoU16 volume_lfo_target;
    /* 0x0A8 */ AkaoU16 volume_lfo_slide_duration;
    /* 0x0AA */ AkaoS16 volume_lfo_delta;
    /* 0x0AC */ AkaoS16 field_AC;
    /* 0x0AE */ AkaoU16 aux_lfo_duration;
    /* 0x0B0 */ AkaoU16 aux_lfo_phase;
    /* 0x0B2 */ AkaoU16 aux_lfo_selector;
    /* 0x0B4 */ AkaoU16 aux_lfo_target;
    /* 0x0B6 */ AkaoU16 aux_lfo_slide_duration;
    /* 0x0B8 */ AkaoS16 aux_lfo_delta;
    /* 0x0BA */ AkaoU16 key_on_delay;
    /* 0x0BC */ AkaoU16 key_off_delay;
    /* 0x0BE */ unsigned char pad_0BE[0x10];
    /* 0x0CE */ AkaoU16 call_stack_index;
    /* 0x0D0 */ AkaoS16 default_note_length;
    /* 0x0D2 */ AkaoS16 fixed_note_length;
    /* 0x0D4 */ AkaoS16 volume_delta;
    /* 0x0D6 */ AkaoS16 pitch_slide_delta;
    /* 0x0D8 */ AkaoS16 pan_target; /* 8.8 split between the source and stereo-copy SPU slots
                                   * (Spu_CopyVoiceToStereoSlot); separate from panpot */
    /* 0x0DA */ AkaoS16 pan_delta;
    /* 0x0DC */ AkaoS16 panpot_delta;
    /* 0x0DE */ AkaoS16 expression;
    /* 0x0E0 */ AkaoS16 detune;
    /* 0x0E2 */ AkaoS16 current_note;
    /* 0x0E4 */ AkaoS16 vibrato_delta;
    /* 0x0E6 */ AkaoS16 tremolo_delta;
    /* 0x0E8 */ AkaoS16 pitch_lfo_value;
    /* 0x0EA */ AkaoS16 volume_lfo_value;
    /* 0x0EC */ AkaoS16 aux_lfo_value;
    /* 0x0EE */ unsigned char pad_0EE[0x02];
    /* 0x0F0 */ int assigned_voice_index;
    /* 0x0F4 */ AkaoU32 update_flags;
    /* 0x0F8 */ AkaoU32 sample_header;
    /* 0x0FC */ AkaoU32 sample_data;
    /* 0x100 */ int adsr_attack;
    /* 0x104 */ int adsr_sustain;
    /* 0x108 */ int adsr_release;
    /* 0x10C */ AkaoU16 pitch;
    /* 0x10E */ AkaoU16 adsr_attack_rate;
    /* 0x110 */ AkaoU16 adsr_decay_rate;
    /* 0x112 */ AkaoU16 adsr_sustain_level;
    /* 0x114 */ AkaoU16 adsr_sustain_rate;
    /* 0x116 */ AkaoU16 adsr_release_rate;
    /* 0x118 */ AkaoS16 volume_left;
    /* 0x11A */ AkaoS16 volume_right;
} AkaoTrack;

/* The voice record that contains a member the voice loops walk: retail biases
 * those loop cursors to one field and reaches the record back from it. */
#define AKAO_TRACK_OF(cursor, member) \
    ((AkaoTrack *)((unsigned char *)(cursor) - PE1_OFFSETOF(AkaoTrack, member)))

/* View at AkaoTrack::update_flags. Tables using this view retain the full
 * AkaoTrack stride while exposing the field the sequencer updates. */
typedef struct AkaoTrackUpdateSlot {
    /* 0x000 */ AkaoU32 update_flags;
    /* 0x004 */ unsigned char pad_004[sizeof(AkaoTrack) - sizeof(AkaoU32)];
} AkaoTrackUpdateSlot;

#define AKAO_VOICE_COUNT 24

/* Original contiguous storage: primary 24, secondary 24, nested SFX 12. */
#define g_AkaoVoiceStateTable ((AkaoTrack *)g_AkaoSequencerTracks)
#define g_AkaoVoiceStateTable2 (g_AkaoVoiceStateTable + AKAO_VOICE_COUNT)
#define g_AkaoVoiceChannelTable (g_AkaoVoiceStateTable + 2 * AKAO_VOICE_COUNT)

typedef struct AkaoVoiceBank {
    AkaoTrack tracks[AKAO_VOICE_COUNT];
} AkaoVoiceBank;

#define AKAO_TRACK_VOICE(track) (*(AkaoVoiceParams *)&(track)->assigned_voice_index)

typedef void (*AkaoTrackHandler)(AkaoTrack *track);

/* Track record as the timer tick and the voice driver see it: the whole
 * record, its words, and the timing fields AkaoTrack leaves anonymous. */
typedef union AkaoTickTrack {
    AkaoTrack track;
    AkaoU32 words[sizeof(AkaoTrack) / 4];
    struct {
        unsigned char pad_000[PE1_OFFSETOF(AkaoTrack, key_on_mask)];
        /* 0x02C */ AkaoU32 key_on_mask;
        unsigned char pad_030[PE1_OFFSETOF(AkaoTrack, field_50_duration) -
                             PE1_OFFSETOF(AkaoTrack, key_on_mask) - sizeof(AkaoU32)];
        /* 0x050 */ AkaoU32 tick_count;
        /* 0x054 */ AkaoU16 parent_track_id;
        /* 0x056 */ AkaoU16 note_length;
        /* 0x058 */ AkaoU16 gate_length;
    } timing;
} AkaoTickTrack;
PE1_STATIC_ASSERT(sizeof(AkaoTickTrack) == sizeof(AkaoTrack), akao_tick_track_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(AkaoTickTrack, timing.key_on_mask) ==
                  PE1_OFFSETOF(AkaoTrack, key_on_mask), akao_tick_track_key_on);
PE1_STATIC_ASSERT(PE1_OFFSETOF(AkaoTickTrack, timing.tick_count) ==
                  PE1_OFFSETOF(AkaoTrack, field_50_duration), akao_tick_track_count);
PE1_STATIC_ASSERT(PE1_OFFSETOF(AkaoTickTrack, timing.parent_track_id) ==
                  PE1_OFFSETOF(AkaoTrack, parent_track_id), akao_tick_track_parent);
PE1_STATIC_ASSERT(PE1_OFFSETOF(AkaoTickTrack, timing.gate_length) ==
                  PE1_OFFSETOF(AkaoTrack, pan_duration), akao_tick_track_gate);

/* The 48 sequencer tracks: 24 for music, 24 saved or for sound effects. */
extern AkaoTickTrack g_AkaoSequencerTracks[60];
#define g_AkaoTrackStateArray ((AkaoTrackUpdateSlot *)&g_AkaoSequencerTracks[0].track.update_flags)

void Seq_StartNestedStreams(AkaoNestedSource *source, void *first, void *second);

#endif
