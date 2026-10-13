/* Psy-Q LIBPAD PADCMD.OBJ: _padSetAct, _padSetCmd, _padSendAtLoadInfo, _padRecvAtLoadInfo, _padGetActSize, _padLoadActInfo, _padSetActAlign, _padSetMainMode, _padCmdParaMode and the command emitters. */

#include "common.h"
#include "pe1/card_obj.h"
#include "pe1/psyq_pad_main.h"

#include "padcmd_externs.h"

void CardObj_EmitCommand45(CardObj *arg0);
void CardObj_EmitCommand4C(CardObj *arg0, unsigned char arg1);
void CardObj_EmitCommand47(CardObj *arg0, unsigned char arg1);

void _padSetAct(CardObj *arg0, u8 *arg1, int arg2)
{
    arg0->payload_28 = arg1;
    arg0->payload_28_len = arg2;
}

void _padSetCmd(CardObj *arg0, unsigned char arg1, u8 *arg2, unsigned char arg3)
{
    arg0->command = arg1;
    arg0->payload_2c = arg2;
    arg0->payload_2c_len = arg3;
}

void _padSendAtLoadInfo(CardObj *arg0)
{
    int state = arg0->sequenceStep;

    switch (state) {
    case 2:
        CardObj_EmitCommand45(arg0);
        break;
    case 3:
        CardObj_EmitCommand4C(arg0, arg0->currentModeIndex);
        break;
    case 4:
        CardObj_EmitCommand47(arg0, arg0->infoRecordIndex);
        break;
    }
}

int _padGetActSize(CardObj *obj);
int _padLoadActInfo(CardObj *obj, unsigned char *dst);

#define CARD_RESPONSE(obj) \
    (*(unsigned char * volatile *)&(obj)->response_3c)

int _padRecvAtLoadInfo(CardObj *obj) {
    unsigned int chunk;
    unsigned int next;
    unsigned int responseValue;
    switch (obj->sequenceStep) {
    case 2:
        obj->modeCount = CARD_RESPONSE(obj)[3];
        obj->currentModeIndex = CARD_RESPONSE(obj)[4];
        obj->currentExtendedId = 0;
        obj->actuatorCount = CARD_RESPONSE(obj)[5];
        obj->combinationCount = CARD_RESPONSE(obj)[6];
        obj->combinationStorageBytes = 0;
        break;

    case 3:
        responseValue = obj->response_3c[4];
        next = obj->response_3c[5];
        obj->infoRecordIndex = 0;
        obj->currentExtendedId = (responseValue << 8) + next;
        break;

    case 4:
        chunk = obj->combinationStorageBytes;
        next = obj->infoRecordIndex;
        responseValue = obj->response_3c[4];
        next++;
        obj->infoRecordIndex = next;
        chunk += 8;
        chunk += (responseValue + 3) & 0x1FC;
        obj->combinationStorageBytes = chunk;
        if ((next & 0xFF) < obj->combinationCount) {
return_zero:
            return 0;
        }

        if (_padGetActSize(obj) >= 0x81) {
            obj->sequenceStep = 0xFE;
            obj->communicationState = 2;
            goto return_zero;
        }

        obj->sequenceStep = 0xFF;
        _padLoadActInfo(obj, (unsigned char *)obj + 0x63);
        obj->sequenceStep = 2;
        goto return_zero;
    }

    return 1;
}

int _padGetActSize(CardObj *arg0) {
    int first;
    int second;
    int responseBytes;
    int base;
    int raw_first;
    int raw_second;

    raw_first = arg0->modeCount;
    raw_second = arg0->actuatorCount;
    base = arg0->combinationStorageBytes;

    first = raw_first + 1;
    first >>= 1;
    first <<= 2;

    responseBytes = (raw_second << 2) + raw_second;
    second = (responseBytes + 3) & 0xFFC;
    second += 4;

    first += second;
    return first + base;
}

void CardObj_EmitReadTransferCommand(CardObj *obj);
int LIBPAD_PADCMD_text_3A0(CardObj *obj);

int _padLoadActInfo(CardObj *obj, unsigned char *buffer) {
    int cursor;
    int result;
    int state;
    int (*processFn)(CardObj *);
    int rowCount;
    int columnCount;

    if (buffer == 0) {
        goto return_zero;
    }
    if (obj->capabilities != 0) {
        return 0;
    }
    if (g_MemCardIsTransferActiveFn() != 0) {

return_zero:
    return 0;

    }
initialize:
    result = 1;
    state = 4;
    cursor = ((int)buffer + 3) >> 2;
    obj->communicationState = state;
    state = 1;
    obj->sequenceStep = state;
    obj->fn_14 = (void (*)(void *))CardObj_EmitReadTransferCommand;
    rowCount = obj->modeCount;
        processFn = LIBPAD_PADCMD_text_3A0;
    obj->fn_18 = processFn;
    columnCount = obj->actuatorCount;

    cursor <<= 2;
    obj->modeTable = (u16 *)cursor;
    obj->infoRecordIndex = 0;
    cursor += ((rowCount + 1) >> 1) * 4;
    obj->capabilities = (PadCapabilityRecord *)cursor;
    cursor += (columnCount * 5 + 3) & 0xFFC;
    obj->combinations = (PadCombinationRecord *)cursor;
    return result;
}

void CardObj_EmitCommand46(CardObj *arg0, unsigned char arg1);
void CardObj_EmitCommand4B(CardObj *arg0);

void CardObj_EmitReadTransferCommand(CardObj *arg0) {
    int state = arg0->sequenceStep;

    switch (state) {
    case 2:
        CardObj_EmitCommand4C(arg0, arg0->infoRecordIndex);
        break;
    case 3:
        CardObj_EmitCommand46(arg0, arg0->infoRecordIndex);
        break;
    case 4:
        if (arg0->combinationBytesRemaining == 0) {
            CardObj_EmitCommand47(arg0, arg0->infoRecordIndex);
        } else {
            CardObj_EmitCommand4B(arg0);
        }
        break;
    }
}

/* Psy-Q LIBPAD/PADCMD.OBJ private text_3A0. */
int LIBPAD_PADCMD_text_3A0(CardObj *inPort) {
    CardObj *port = inPort;
    switch (port->sequenceStep) {
    case 2:
        port->modeTable[port->infoRecordIndex] =
            port->response_3c[5] + (port->response_3c[4] << 8);
        port->infoRecordIndex++;
        if (port->infoRecordIndex >= port->modeCount) {
            port->infoRecordIndex = 0;
            break;
        }
        return 0;
    case 3: {
        PadCapabilityRecord *record =
            port->capabilities + port->infoRecordIndex;
        record->protocol[0] = port->response_3c[4];
        record->protocol[1] = port->response_3c[5] & 127;
        record->payloadBytes = port->response_3c[6];
        record->activationCost = port->response_3c[7];
        {
            int high = port->response_3c[5];
            record->high_bit = high >> 7;
        }
        port->infoRecordIndex++;
        if (port->infoRecordIndex >= port->actuatorCount) {
            port->infoRecordIndex = 0;
            port->combinationBytesRemaining = 0;
            break;
        }
        return 0;
    }
    case 4: {
        PadCombinationRecord *record = port->combinations + port->infoRecordIndex;
        u8 *source, *base;
        int bytes;
        unsigned offset;
        if (port->combinationBytesRemaining == 0) {
            {
                int length = port->response_3c[4];
                bytes = 3;
                port->combinationBytesRemaining = length;
            }
            record->length = port->combinationBytesRemaining;
            {
                u8 *response = port->response_3c;
                int index = port->infoRecordIndex;
                source = response + 5;
                if (index == 0) {
                    base = (u8 *)port->combinations;
                    offset = port->combinationCount * 8;
                } else {
                    base = record[-1].data;
                    offset = (record[-1].length + 3) & 0x1fc;
                }
            }
            base = base + offset;
            record->data = base;
            g_PadInfoWriteCursor = base;
        } else {
            u8 *response = port->response_3c;
            bytes = 6;
            source = response + 2;
        }
        bytes--;
        if (bytes != -1) {
            u8 **destination = &g_PadInfoWriteCursor;
            do {
                int remaining = port->combinationBytesRemaining;
                bytes--;
                if (!remaining)
                    goto exhausted;
                {
                    u8 *dst = *destination;
                    int value = *source++;
                    *dst = value;
                    *destination = dst + 1;
                }
                port->combinationBytesRemaining--;
            } while (bytes != -1);
        }

        if (port->combinationBytesRemaining != 0) {
    zero:
        return 0;
        }
    exhausted:
        {
            port->infoRecordIndex++;
            if (port->infoRecordIndex >= port->combinationCount) {
                port->communicationState = 6;
                port->sequenceStep = 254;
                return 0;
            }
            port->combinationBytesRemaining = 0;
        }
        goto zero;
    }
    default:
        return 1;
    }
complete:
    return 1;
}

void CardObj_EmitCommand4D(CardObj *arg0);
s32 Render_CheckParticleBounds(CardObj *arg0);

int _padSetActAlign(CardObj *obj, u8 *command) {
    int result;
    int active;

    result = g_MemCardIsTransferActiveFn(obj);
    if (result != 0) {
        return 0;
    } else {
        active = 1;
        obj->sequenceStep = active;
        obj->fn_14 = (void (*)(void *))CardObj_EmitCommand4D;
        obj->requestedActuatorMap = command;
        obj->fn_18 = Render_CheckParticleBounds;
        result = 1;
    }
    return result;
}

void CardObj_EmitCommand4D(CardObj *arg0) {
    u8 *value = arg0->requestedActuatorMap;

    arg0->command = 0x4D;
    arg0->payload_2c_len = 6;
    arg0->payload_2c = value;
}

#define NULL ((void *)0)
s32 Render_CheckParticleBounds(CardObj *arg0) {
    s32 i;
    s32 ff;
    u8 *cursor;
    s32 matched;
    s32 remaining;
    s32 written;
    s32 needed;
    u8 *out;
    s32 limit;

    i = 0;
    if (arg0->actuatorCount != 0) {
        ff = 0xFF;
        do {
            cursor = arg0->requestedActuatorMap;
            matched = 0;
            remaining = 5;
            do {
                if (*cursor++ == i) {
                    matched += 1;
                }
                remaining -= 1;
            } while (remaining >= 0);
            needed = arg0->capabilities[i].payloadBytes;
            cursor = arg0->requestedActuatorMap;
            written = 0;
            if (needed == 0) {
                needed = 1;
            }
            out = (u8 *)arg0;
            do {
                if (*cursor++ == i) {
                    if (matched < needed) {
                        out[PE1_OFFSETOF(CardObj, actuatorMap)] = ff;
                        matched -= 1;
                    } else {
                        out[PE1_OFFSETOF(CardObj, actuatorMap)] = i;
                    }
                }
                written += 1;
                out += 1;
            } while (written < 6);
            limit = arg0->actuatorCount;
            i += 1;
        } while (i < limit);
    }
    arg0->sequenceStep = 0xFE;
    return 0;
}

void CardObj_EmitReadIdCommand(CardObj *obj);
int CardObj_CheckAbortOrDispatch(CardObj *obj);

int _padSetMainMode(CardObj *obj, int byte1, int byte2) {
    int compareByte;
    int flag;
    int result;

    compareByte = byte1;
    result = g_MemCardIsTransferActiveFn(obj);
    if (result != 0) {
        return 0;
    }

    result = 1;
    flag = 1;
    obj->sequenceStep = flag;
    obj->fn_14 = (void (*)(void *))CardObj_EmitReadIdCommand;
    obj->fn_18 = CardObj_CheckAbortOrDispatch;
    obj->requestedModeIndex = byte1;
    obj->requestedModeLock = byte2;
    flag = (compareByte & 0xFF) ^ obj->currentModeIndex;
    flag = (unsigned int)flag < 1;
    obj->modeAlreadySelected = flag;
    return result;
}

void CardObj_EmitReadIdCommand(CardObj *arg0) {
    int state = arg0->sequenceStep;

    switch (state) {
    case 2:
        arg0->command = 0x44;
        arg0->payload_2c = (unsigned char *)arg0 + 0x51;
        arg0->payload_2c_len = state;
        break;
    case 3:
        arg0->command = 0x4D;
        arg0->payload_2c = arg0->actuatorMap;
        arg0->payload_2c_len = 6;
        break;
    }
}

int CardObj_CheckAbortOrDispatch(CardObj *arg0) {
    if (arg0->modeAlreadySelected != 0) {
        if (arg0->sequenceStep == 2) {
            return 1;
        }

        arg0->sequenceStep = 0xFE;
        return 0;
    }

    g_MemCardObjResetFn(arg0);
    return 0;
}
void _padCmdParaMode(CardObj *obj, unsigned char value) {
    obj->command = 0x43;
    obj->payload_2c = (unsigned char *)obj + 0x24;
    obj->pad_24[0] = value;
    obj->payload_2c_len = 1;
}
void CardObj_EmitCommand45(CardObj *obj) {
    obj->command = 0x45;
    obj->payload_2c = 0;
    obj->payload_2c_len = 0;
}
void CardObj_EmitCommand4C(CardObj *obj, unsigned char value) {
    obj->command = 0x4C;
    obj->payload_2c = (unsigned char *)obj + 0x24;
    obj->pad_24[0] = value;
    obj->payload_2c_len = 1;
}
void CardObj_EmitCommand46(CardObj *obj, unsigned char value) {
    obj->command = 0x46;
    obj->payload_2c = (unsigned char *)obj + 0x24;
    obj->pad_24[0] = value;
    obj->payload_2c_len = 1;
}
void CardObj_EmitCommand47(CardObj *obj, unsigned char value) {
    obj->command = 0x47;
    obj->payload_2c = (unsigned char *)obj + 0x24;
    obj->pad_24[0] = value;
    obj->payload_2c_len = 1;
}
void CardObj_EmitCommand4B(CardObj *obj) {
    obj->command = 0x4B;
    obj->payload_2c = 0;
    obj->payload_2c_len = 0;
}
