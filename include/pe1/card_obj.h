#ifndef PE1_CARD_OBJ_H
#define PE1_CARD_OBJ_H

#include "common.h"

/* PADCMD.OBJ capability response tables addressed by CardObj capabilities/combinations.
 * payloadBytes controls alignment width (zero selects a single control bit);
 * activationCost is summed against the shared activation allowance. */
typedef struct PadCapabilityRecord {
    u8 protocol[2];
    u8 payloadBytes;
    u8 activationCost;
    u8 high_bit;
} PadCapabilityRecord;
typedef struct PadCombinationRecord {
    u8 length;
    u8 reserved[3];
    u8 *data;
} PadCombinationRecord;
PE1_STATIC_ASSERT(sizeof(PadCapabilityRecord) == 5, pad_capability_record_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(PadCapabilityRecord, high_bit) == 4,
                  pad_capability_high_bit_offset);
PE1_STATIC_ASSERT(sizeof(PadCombinationRecord) == 8, pad_data_record_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(PadCombinationRecord, data) == 4, pad_data_record_pointer);

/* LIBPAD per-port command object (legacy CardObj_* names). sequenceStep is unsigned char: stored as 1
 * by BeginCommand4D and compared ==0xFF (lbu) by IsTransferActive. */
typedef struct CardObj {
    u16 *modeTable;                   /* 0x00 */
    PadCapabilityRecord *capabilities; /* 0x04 */
    PadCombinationRecord *combinations; /* 0x08 */
    struct CardObj *subPorts;         /* 0x0C: four multitap port objects */
    struct CardObj *ownerPort;         /* 0x10: owning port object, self for a direct port */
    void (*fn_14)(void *);            /* 0x14 */
    int (*fn_18)(struct CardObj *);            /* 0x18 */
    unsigned char pad_1C[0x04];
    u8 *requestedActuatorMap;          /* 0x20: six-byte map passed to _padSetActAlign */
    unsigned char pad_24[0x04];
    unsigned char *payload_28;         /* 0x28 */
    unsigned char *payload_2c;         /* 0x2C */
    unsigned char *output_30;          /* 0x30 */
    unsigned char payload_28_len;      /* 0x34 */
    unsigned char payload_2c_len;      /* 0x35 */
    unsigned char command;             /* 0x36 */
    unsigned char saved_command;       /* 0x37 */
    unsigned char pad_38[0x04];
    unsigned char *response_3c;        /* 0x3C */
    unsigned char *statusBuffer;       /* 0x40: receives the inverted first exchanged byte */
    unsigned char response_index;     /* 0x44: received-byte cursor */
    unsigned char payload_index;       /* 0x45 */
    unsigned char sequenceStep;        /* 0x46: command sequence step, 0xFE/0xFF terminal */
    unsigned char infoRecordIndex;           /* 0x47: cursor shared by mode, capability and combination reads */
    unsigned char combinationBytesRemaining;           /* 0x48: bytes still to copy for the current combination */
    unsigned char communicationState; /* 0x49: internal PadGetState status */
    unsigned char retryCount;          /* 0x4A: retries spent in the current step */
    unsigned char pad_4B[0x01];
    int frameCounter;                  /* 0x4C: sequence ticks, PadInfoMode term 100 */
    unsigned char pad_50[0x01];
    unsigned char requestedModeIndex;  /* 0x51: PadSetMainMode offset */
    unsigned char requestedModeLock;   /* 0x52: PadSetMainMode lock */
    unsigned char modeAlreadySelected;           /* 0x53: requested mode equals the current mode */
    unsigned char pad_54[0x03];
    unsigned char actuatorEnabled[0x06]; /* 0x57: payload-byte activation flags */
    unsigned char actuatorMap[0x06]; /* 0x5D: capability index per payload byte, 0xFF = unused */
    unsigned char pad_63[0x80];
    unsigned char modeCount;           /* 0xE3 */
    unsigned char currentModeIndex;    /* 0xE4: PadInfoMode InfoModeCurExOffs */
    unsigned char pad_e5[0x01];
    unsigned short currentExtendedId;  /* 0xE6: PadInfoMode InfoModeCurExID */
    unsigned char currentId;           /* 0xE8: PadInfoMode InfoModeCurID, 8 = multitap */
    unsigned char actuatorCount;           /* 0xE9 */
    unsigned char combinationCount;           /* 0xEA */
    unsigned char pad_eb[0x01];
    int combinationStorageBytes;                     /* 0xEC: combination headers and aligned data size */
} CardObj;

/* LIBPAD addresses ports, sub-ports, capability records and the multitap
 * count words by byte offset, as integer sums in retail's operand order. */
typedef union CardObjAddress {
    struct CardObj *obj;
    PadCapabilityRecord *capability;
    int *count;
    u8 *bytes;
    u32 word;
} CardObjAddress;

/* PadInitDirect walks the objects by their statusBuffer member. */
#define CARD_OBJ_FROM_STATUS_BUFFER(pointer) \
    ((CardObj *)((u8 *)(pointer) - PE1_OFFSETOF(CardObj, statusBuffer)))

/* Direct-port operations shared by PADPORTD and PADMAIN. */
void PadSetAct(int channel, u8 *payload, int size);
void _padSetAct(CardObj *obj, u8 *payload, int size);
void PadSetActAlign(int channel, u8 *alignment);
int _padSetActAlign(CardObj *obj, u8 *alignment);
void CardObj_ResetFields(CardObj *obj);
void CardObj_SwapByteField(CardObj *obj);
int CardObj_GetChannelId(CardObj *obj);
CardObj *CardObj_LookupByChannelId(int channel);
extern int g_MemCardServiceReady;
extern void (*g_MemCardObjResetFn)(CardObj *obj);

/* Byte-provider callback. The direct-port reader ignores needsAck. */
extern int (*g_MemCardObjReadPayloadByteFn)(CardObj *obj, int needsAck);
int CardObj_ReadPayloadByte(CardObj *obj, int needsAck);

PE1_STATIC_ASSERT(PE1_OFFSETOF(CardObj, response_index) == 0x44, card_response_index);
PE1_STATIC_ASSERT(PE1_OFFSETOF(CardObj, payload_index) == 0x45, card_payload_index);
PE1_STATIC_ASSERT(sizeof(CardObj) == 0xF0, card_obj_size);

/* LIBPAD port state: the per-port objects, the multitap command byte and
 * the lookup that maps a channel number to its object. */
extern int (*g_MemCardStateDispatchFn)(CardObj *obj);
extern CardObj *g_MemCardObjArray;
extern int g_MemCardCommandByte;
extern CardObj *(*g_MemCardObjLookupFn)(int channel);
/* Transfer-busy predicate that _padInitDirSeq installs. Unprototyped:
 * _padSetMainMode passes the port object, _padLoadActInfo calls it
 * without setting a0. */
extern int (*g_MemCardIsTransferActiveFn)();
/* Response hook that _padInitDirSeq installs; the interrupt handler calls
 * it with the port object. */
extern void (*g_MemCardResponseHandler)(CardObj *obj);

#endif /* PE1_CARD_OBJ_H */
