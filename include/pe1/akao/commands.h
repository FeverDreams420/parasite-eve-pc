#ifndef PE1_AKAO_COMMANDS_H
#define PE1_AKAO_COMMANDS_H

#include "pe1/akao/file_header.h"
#include "pe1/akao/queue.h"
#include "pe1/akao/track.h"

/* The relative-stream command initially holds an offset-table address in
 * source.key_off_mask; the handler replaces it with the saved mask before
 * passing the source descriptor to Seq_StartNestedStreams. */
typedef struct AkaoRelativeNestedCommand {
    AkaoNestedSource source;
    unsigned int key_off_mask;
} AkaoRelativeNestedCommand;
typedef char AkaoRelativeNestedCommandSizeCheck[
    (sizeof(AkaoRelativeNestedCommand) == 0x18) ? 1 : -1];

void Seq_StartDefaultNestedStream(AkaoNestedSource *source);
void Seq_StartIndexedNestedStreamWithDefaults(AkaoNestedSource *source);
void Seq_StartIndexedNestedStream(AkaoNestedSource *source);
void Seq_StartRelativeNestedStream(AkaoRelativeNestedCommand *command);

typedef signed char AkaoCommandS8;

enum AkaoSequenceOpcode {
    AKAO_SEQ_OP_FINISH_CHANNEL = 0xA0,
    AKAO_SEQ_OP_LOAD_INSTRUMENT = 0xA1,
    AKAO_SEQ_OP_SET_NEXT_NOTE_LENGTH = 0xA2,
    AKAO_SEQ_OP_CHANNEL_MASTER_VOLUME = 0xA3,
    AKAO_SEQ_OP_CHANNEL_VOLUME = 0xA8,
    AKAO_SEQ_OP_CHANNEL_VOLUME_SLIDE = 0xA9,
    AKAO_SEQ_OP_CHANNEL_PAN = 0xAA,
    AKAO_SEQ_OP_CHANNEL_PAN_SLIDE = 0xAB,
    AKAO_SEQ_OP_NOISE_CLOCK = 0xAC,
    AKAO_SEQ_OP_ADSR_ATTACK_RATE = 0xAD,
    AKAO_SEQ_OP_PITCH_BEND_SLIDE = 0xA4,
    AKAO_SEQ_OP_SET_OCTAVE = 0xA5,
    AKAO_SEQ_OP_TRANSPOSE_ABSOLUTE = 0xC0,
    AKAO_SEQ_OP_TRANSPOSE_RELATIVE = 0xC1,
    AKAO_SEQ_OP_REVERB_ON = 0xC2,
    AKAO_SEQ_OP_LOOP_POINT = 0xC8,
    AKAO_SEQ_OP_RETURN_LOOP_N_TIMES = 0xC9,
    AKAO_SEQ_OP_RETURN_LOOP = 0xCA,
    AKAO_SEQ_OP_FULL_LENGTH_NOTE_ON = 0xD0,
    AKAO_SEQ_OP_FULL_LENGTH_NOTE_OFF = 0xD1,
    AKAO_SEQ_OP_FM_TOGGLE_DELAY_ON = 0xD2,
    AKAO_SEQ_OP_PLAYBACK_RATE_SIDE_CHAIN_ON = 0xD4,
    AKAO_SEQ_OP_PLAYBACK_RATE_SIDE_CHAIN_OFF = 0xD5,
    AKAO_SEQ_OP_PITCH_VOLUME_SIDE_CHAIN_ON = 0xD6,
    AKAO_SEQ_OP_FINE_TUNE_ABSOLUTE = 0xD8,
    AKAO_SEQ_OP_FINE_TUNE_RELATIVE = 0xD9,
    AKAO_SEQ_OP_PORTAMENTO_ON = 0xDA,
    AKAO_SEQ_OP_JUMP_ON_REPEAT = 0xF0,
    AKAO_SEQ_OP_BREAK_LOOP_ON_REPEAT = 0xF1
};

/* Bank pitch messages use full signed words before masking pitch to 7 bits.
 * The bank selector is at +0x10 in all three message layouts. */
typedef struct AkaoBankPitchCommand {
    int opcode;
    int pitch;
    int reserved[2];
    int bank_selector;
} AkaoBankPitchCommand;

typedef struct AkaoBankPitchSlideCommand {
    int opcode;
    int duration;
    int target;
    int reserved;
    int bank_selector;
} AkaoBankPitchSlideCommand;

typedef struct AkaoBankPitchRangeCommand {
    int opcode;
    int duration;
    int start;
    int target;
    int bank_selector;
} AkaoBankPitchRangeCommand;

void Seq_SetTrackPitchImmediate(AkaoBankPitchCommand *command);
void Seq_SlideTrackPitch(AkaoBankPitchSlideCommand *command);
void Seq_TrackPitchSetup(AkaoBankPitchRangeCommand *command);

typedef struct AkaoValueCommand {
    int pad_0;
    int value;
} AkaoValueCommand;

typedef struct AkaoGlobalSlideCommand {
    int pad_0;
    int duration;
    AkaoCommandS8 target;
} AkaoGlobalSlideCommand;

typedef struct AkaoGlobalSlideRangeCommand {
    int pad_0;
    int duration;
    int start;
    AkaoCommandS8 target;
} AkaoGlobalSlideRangeCommand;

typedef struct AkaoGlobalParamCommand {
    int pad_0;
    AkaoCommandS8 value;
} AkaoGlobalParamCommand;

/* Global pitch messages carry the pitch in the low half of each word. */
typedef struct AkaoGlobalPitchCommand {
    int opcode;
    unsigned short pitch;
} AkaoGlobalPitchCommand;

typedef struct AkaoGlobalPitchSlideCommand {
    int opcode;
    int duration;
    unsigned short target;
} AkaoGlobalPitchSlideCommand;

typedef struct AkaoGlobalPitchRangeCommand {
    int opcode;
    int duration;
    unsigned short start;
    unsigned short start_high;
    unsigned short target;
} AkaoGlobalPitchRangeCommand;

/*
 * Staging area for the public AKAO command wrappers at 0x800864xx..0x800870xx
 * (0x800BCD80). The wrappers fill it, then Akao_EnqueueStagedCommand turns it
 * into one or two AkaoQueueEntry messages and returns a status value.
 */
typedef struct AkaoStagedCommand {
    /* 0x00 */ int opcode;
    /* 0x04 */ union {
        int value;
        /* sample load opcodes 0x10/0x12/0x19: address of the sample header */
        unsigned short *sample_header;
        /* opcode 0x24: buffer contents following the shared magic word */
        unsigned char *command_data;
    } arg0;
    /* 0x08 */ int arg1;
    /* 0x0C */ int arg2;
    /* 0x10 */ int arg3;
} AkaoStagedCommand;

extern AkaoStagedCommand g_AkaoCmd;

/* Rolling 0x400..0x5FF sequence number stamped into opcode 0x24 messages. */
typedef struct AkaoSequenceCounter {
    int value;
} AkaoSequenceCounter;

extern AkaoSequenceCounter g_AkaoCmd24Sequence;

int Akao_EnqueueStagedCommand(void);
/* Queues a voice-bank load; returns the queue result. */
int Akao_Cmd_10(void *bank);

/* Returns the header validation difference, or the queued sequence number. */
int Akao_Cmd_24(void *header, int arg1, int arg2, int arg3);
void Akao_Cmd_98_9A_9C(int command);
void Akao_Cmd_99_9B_9D(int command);
void Akao_Cmd_D8(int command);
void Akao_Cmd_F0(void);
void Akao_Cmd_F1(void);

void Akao_Cmd_21(int arg0, int arg1);
void Akao_Cmd_40(void);
void Akao_Cmd_C0_WithSlot(int slot, int volume);
void Akao_Cmd_C1_WithSlot(int slot, int duration, int volume);

/* Opcode 0x80, 0x81 or 0x82 chosen by the argument. */
void Akao_Cmd_80_81_82(int arg0);

void Akao_Cmd_11(int slot);
void Akao_Cmd_90(int command);
void Akao_Cmd_92(int command);
void Akao_Cmd_A0(int command, int arg1, int arg2);
void Akao_Cmd_A1(int command, int arg1, int arg2, int arg3);
void Akao_Cmd_A2(int command, int arg1, int arg2);
void Akao_Cmd_A3(int command, int arg1, int arg2, int arg3);
void Akao_Cmd_A4(int command, int arg1, int arg2);
void Akao_Cmd_A5(int command, int arg1, int arg2, int arg3);
void Akao_Cmd_A8(int command);
void Akao_Cmd_A9(int command, int arg1);
void Akao_Cmd_AA(int command);
void Akao_Cmd_AB(int command, int arg1);
void Akao_Cmd_AC(int command);
void Akao_Cmd_AD(int command, int arg1);
void Akao_Cmd_C2_WithSlot(int slot, int value, int arg2, int arg3);
void Akao_Cmd_D0(int command);
void Akao_Cmd_D1(int command, int arg1);
void Akao_Cmd_D2(int command, int arg1, int arg2);
void Akao_Cmd_D4(int command);
void Akao_Cmd_D5(int command, int arg1);
void Akao_Cmd_D6(int command, int arg1, int arg2);
void Akao_Cmd_D9(int command, int arg1);
void Akao_Cmd_DA(int command, int arg1, int arg2);

#endif
