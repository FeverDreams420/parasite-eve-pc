/* PSY-Q LIBPAD PADPORTD: PadInitDirect, CardObj_ResetFields,
 * MemCard_DmaCompleteCallback, CardObj_SwapByteField, CardObj_ReadPayloadByte,
 * LIBPAD_PADPORTD_text_388, CardObj_GetChannelId, CardObj_LookupByChannelId. */

#include "common.h"
#include "pe1/card_obj.h"
#include "pe1/memcard_state.h"
#include "pe1/psyq_pad_main.h"

#include "padportd_externs.h"

void _padInitDirSeq(void);
int MemCard_DmaCompleteCallback(int result);
void LIBPAD_PADPORTD_text_388(CardObj *);
void bzero(void *ptr, int size);
void _padSetVsyncParam(void);

int PadInitDirect(u8 *output0, u8 *output1) {
    u8 *savedOutput0 = output0;
    u8 *savedOutput1 = output1;
    CardObj *obj;
    u8 *table3c;
    u8 *table40;
    int i;
    int fill;
    u8 **field40;
    u8 *cursor;
    void (*swapCommand)(CardObj *);
    CardObj *clearObj;
    int count;

    g_MemCardServiceReady = 0;
    g_MemCardCommandByte = 0;
    _padInitDirSeq();

    obj = &D_800A5B70[0];
    g_PadDmaCompleteFn = MemCard_DmaCompleteCallback;
    g_MemCardObjResetFn = CardObj_ResetFields;
    g_MemCardObjReadPayloadByteFn = CardObj_ReadPayloadByte;
    g_PadPortActuatorMapFn = LIBPAD_PADPORTD_text_388;
    g_CardGetChannelIdFn = CardObj_GetChannelId;
    g_MemCardObjLookupFn = CardObj_LookupByChannelId;
    swapCommand = CardObj_SwapByteField;
    (clearObj = obj);
    g_MemCardObjArray = obj;
    g_CardSwapByteFieldFn = swapCommand;

    bzero(clearObj, sizeof(CardObj) * 2);
    i = 0;
    fill = 0xFF;
    field40 = &obj->statusBuffer;
    table40 = g_MemCardObjStatusBuffers;
    table3c = g_MemCardObjResponseBuffers;
    cursor = (u8 *)&obj[0].output_30;
    *(u8 **)cursor = savedOutput0;
    cursor = (u8 *)&obj[1].output_30;
    *(u8 **)cursor = savedOutput1;
    do {
        CARD_OBJ_FROM_STATUS_BUFFER(field40)->subPorts = 0;
        CARD_OBJ_FROM_STATUS_BUFFER(field40)->ownerPort = obj;
        CARD_OBJ_FROM_STATUS_BUFFER(field40)->output_30[0] = fill;
        CARD_OBJ_FROM_STATUS_BUFFER(field40)->output_30[1] = 0;
        CARD_OBJ_FROM_STATUS_BUFFER(field40)->response_3c = table3c;
        *field40 = table40;
        cursor = obj->actuatorMap;
        count = 5;
        do {
            *cursor = fill;
            count--;
            cursor++;
        } while (count >= 0);
        table40 += 0x23;
        table3c += 0x23;
        i++;
        field40 = &CARD_OBJ_FROM_STATUS_BUFFER(field40)[1].statusBuffer;
        obj++;
    } while (i < 2);

    _padSetVsyncParam();
    g_MemCardServiceReady = 1;
    return 1;
}

void CardObj_ResetFields(CardObj *arg0) {
    int count;
    int value;
    char *ptr;

    if (arg0->communicationState != 0) {
        ptr = (char *)arg0->actuatorMap;
        value = 0xFF;
        count = 5;
        arg0->communicationState = 0;
        arg0->sequenceStep = 0;
        arg0->currentExtendedId = 0;
        arg0->fn_14 = 0;
        arg0->fn_18 = 0;
        arg0->modeCount = 0;
        arg0->currentModeIndex = 0;
        arg0->currentExtendedId = 0;
        arg0->actuatorCount = 0;
        arg0->combinationCount = 0;
        arg0->modeTable = 0;
        arg0->capabilities = 0;
        arg0->combinations = 0;

        do {
            *ptr = value;
            count -= 1;
            ptr += 1;
        } while (count >= 0);
    }
}

void _dirFailAuto(CardObj *obj);
int _padInitSioMode(CardObj *obj);

int MemCard_DmaCompleteCallback(int result) {
    int callbackResult = result;
    CardObj *objects;
    int ignoredResult;
    int *channelResults;
    CardObj *obj;
    int index;
    MemCardSioRegisters *control;
    int limit;
    int offset;
    int nextResult;
    int cleared;

    objects = &D_800A5B70[0];
    ignoredResult = -9;
    channelResults = &g_MemCardChannelCounts[0];

    do {
        index = g_PadActivePortIndex;
        offset = index * sizeof(CardObj);
        obj = (CardObj *)(offset + (int)objects);

        if (callbackResult != ignoredResult) {
            cleared = callbackResult == 0;
            if (cleared) {
                channelResults[index] = 0;
            } else {
                _dirFailAuto(obj);
                CardObj_SwapByteField(obj);
            }
        }

        (control = g_CardSioRegsPtr, index = g_PadActivePortIndex);
        g_MemCardCommandStep = 0;
        control->control = 0;
        limit = g_MemCardPort2Present;
        index += 1;
        g_PadActivePortIndex = index;
        if (limit >= index) {
            nextResult = _padInitSioMode(
                (CardObj *)((((index << 4) - index) << 4) + (int)objects));
        } else {
            nextResult = 1;
        }
        callbackResult = 0xFFFF;
    } while (nextResult == 0);

    return nextResult;
}

void CardObj_SwapByteField(CardObj *obj) {
    u8 command;

    command = obj->command;
    obj->command = 0;
    obj->saved_command = command;
}

int CardObj_ReadPayloadByte(CardObj *arg0, int needsAck) {
    int index;
    int mode;
    u8 *ptr;

    index = arg0->payload_index - 3;
    mode = arg0->command;
    if (mode != 0) {
        if (mode == 0x4D) {
            goto mode4D;
        }

    }
    else
    {
mode0:
    if (index < 6) {
        if (arg0->actuatorEnabled[index] == 0) {
            return 0;
        }
    }

    if (index >= arg0->payload_28_len) {
        return 0;
    }
    ptr = arg0->payload_28 + index;

load:
    return *ptr;

mode4D:
    if (index < arg0->payload_2c_len) {
        ptr = arg0->payload_2c + index;
        goto load;
    }
    return 0xFF;

    }
other:
    if (index >= arg0->payload_2c_len) {
        return 0;
    }
    return arg0->payload_2c[index];
}

void bzero(void *, int);
void LIBPAD_PADPORTD_text_388(CardObj *port) {
    int limit, i, j, mask, active;
    int count;
    int offset;
    int one;
    u8 *map, *data;
    bzero(port->actuatorEnabled, 6);
    if (port->currentExtendedId && port->payload_28) {
        limit = 6;
        if (port->payload_28_len < 7) {
            limit = port->payload_28_len;
        }
        i = 0;
        if (port->actuatorCount) {
            one = 1;
            offset = 0;
            do {
                CardObjAddress capability;
                active = 0;
                mask = 1;
                capability.capability = port->capabilities;
                capability.word = offset + capability.word;
                if (capability.capability->payloadBytes)
                    mask = 255;
                map = port->actuatorMap;
                data = port->payload_28;
                j = 0;
                if (limit) {
                    while (1) {
                        if (*map != i || !(*data & mask)) {
                            map++;
                            j++;
                            data++;
                            if (j < limit)
                                continue;
                        } else
                            goto selected;
                        break;
                    }
                }
            search_done:
                if (active) {
                    CardObjAddress costRecord;
                    int total;
                    costRecord.capability = port->capabilities;
                    costRecord.word = offset + costRecord.word;
                    total = g_PadActuatorCostTotal + costRecord.capability->activationCost;
                    if (total < 61)
                        g_PadActuatorCostTotal = total;
                    else {
                        goto deny;
                    selected:
                        active = 1;
                        goto search_done;
                    deny:
                        active = 0;
                    }
                    if (active) {
                        map = port->actuatorMap;
                        data = port->actuatorEnabled;
                        j = 0;
                        if (limit)
                            do {
                                int same = *map == i;
                                map++;
                                if (same)
                                    *data = one;
                                j++;
                                data++;
                            } while (j < limit);
                    }
                }
                count = port->actuatorCount;
                i++;
                offset += 5;
            } while (i < count);
        }
    } else {
        if (((unsigned)(port->currentId - 4) < 2 || port->currentId == 7) &&
            !port->currentExtendedId && port->payload_28_len >= 2) {
            if ((port->payload_28[0] & 192) == 64 && (port->payload_28[1] & 1) &&
                g_PadActuatorCostTotal + 10 < 61) {
                port->actuatorEnabled[1] = 1;
                port->actuatorEnabled[0] = 1;
                g_PadActuatorCostTotal += 10;
            }
        } else {
            if (port->currentId == 3)
                port->actuatorEnabled[0] = 1;
            else if (!port->currentExtendedId) {
                int value = 1;
                int k = 5;
                /* Byte cursor over the object, offset by the flag index. */
                CardObjAddress cursor;
                cursor.obj = port;
                cursor.word += 5;
                do {
                    cursor.bytes[PE1_OFFSETOF(CardObj, actuatorEnabled)] = value;
                    k--;
                    cursor.bytes--;
                } while (k >= 0);
            }
        }
    }
}

int CardObj_GetChannelId(CardObj *entry) {
    CardObj *candidate;
    int index;
    int value;

    index = 0;
    value = 0x10;
    candidate = D_800A5B70;
    for (; index < 2; index++) {
        if (entry == candidate) {
            return value;
        }
        value += 0x10;
        candidate += 1;
    }

    return 0xFF;
}

CardObj *CardObj_LookupByChannelId(int value) {
    CardObj *entry = D_800A5B70;

    if ((value & 0xF0) != 0) {
        entry += 1;
    }
    return entry;
}
