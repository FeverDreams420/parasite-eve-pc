#ifndef PE1_AKAO_QUEUE_H
#define PE1_AKAO_QUEUE_H

/* Runtime AKAO messages: native pointers enlarge the historical 0x24-byte
 * record. Dispatch uses the opcode's low byte; this is not a disc layout. */
typedef struct AkaoQueueEntry {
    /* 0x00 */ union {
        unsigned char id;
        int word;
    } opcode;
    /* 0x04 */ union {
        int value;
        /* sample load messages: sample data past the header */
        unsigned short *sample_data;
        /* sequence messages: the sequence data to play */
        unsigned char *sequence;
    } arg0;
    union {
        struct { int arg1; int arg2; };
        /* Message 0x34 carries two native sequence pointers and replaces pan. */
        void *second_sequence;
    };
    /* 0x10 */ int arg3;
    /* 0x14 */ int sequence;
    /* 0x18 */ int unk_18;
    /* 0x1C */ int unk_1C;
    /* 0x20 */ int unk_20;
} AkaoQueueEntry;

typedef void (*AkaoMessageHandler)(void *entry);

extern AkaoQueueEntry g_AkaoMessageQueue[16];
extern int g_AkaoMessageQueueCount;
extern AkaoMessageHandler Akao_MessageHandlers[];

void Akao_MessageNoop(void *message);
void Akao_AllocMessageSlot(AkaoQueueEntry **out_msg);
void Akao_ProcessMessageQueue(void);

#endif
