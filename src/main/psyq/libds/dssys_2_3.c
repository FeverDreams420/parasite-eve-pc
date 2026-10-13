/* Psy-Q LIBDS DSSYS_2.OBJ, part 3 of 3: DsClose, DsCommand, DsReady, DsFlush, DsSystemStatus, DsQueueLen, DsStatus, DsShellOpen, DsLastCom, CQ_vsync_system, CQ_ready_system, LIBDS_DSSYS_2_text_13CC and the object's zero tail. */

#include "pe1/cdrom.h"
#include "pe1/psyq_cd.h"
#include "pe1/psyq_ds.h"
#include "pe1/psyq_ds_queue.h"

#include "dssys_2_3_externs.h"
#include "dssys_2_3_externs_1.h"
#include "dssys_2_3_externs_2.h"
#include "dssys_2_3_externs_3.h"
#include "dssys_2_3_externs_5.h"

void DS_close(void);

void DsClose(void) {
    DS_close();
}

/* Empty constraints preserve the retail schedule. */

int DsCommand(int inCommand, void *inParameter, DslCB inCallback,
                             int inCount) {
    void *parameter = inParameter;
    DslCB callback = inCallback;
    int command = inCommand;
    int count = inCount;
    unsigned previous;
    unsigned serial;
    unsigned result;
    unsigned ticket;
    CdDsReadQueueEntry *entry;
    if (g_DsCommandHasParam[command & 255] && parameter) {

        if (g_CdPendingReadCount < 8) {
            previous = g_DsCommandSerial;
            serial = previous + 1;
            g_DsCommandSerial = serial;
            if (!serial) {
                serial = previous + 2;
                g_DsCommandSerial = serial;
            }
            ticket = serial;
            entry = CQ_last_queue();
            entry->active = ticket;
            entry->command = 2;
            if (parameter) {
                parcpy(entry->payload, parameter);
                entry->parameter = parameter;
            } else {
                entry->parameter = 0;
            }
            entry->callback = 0;
            entry->count = 0;
            g_CdPendingReadCount++;
            if (DS_system_status(0) == 1 && g_CdDsReadQueue[g_CdDsReadIndex].active == ticket)
                CQ_execute();
            result = ticket;
        } else
            result = 0;

        if (!result) {
            return 0;
        }
    }

    if (g_CdPendingReadCount < 8) {
        previous = g_DsCommandSerial;
        serial = previous + 1;
        g_DsCommandSerial = serial;
        if (!serial) {
            serial = previous + 2;
            g_DsCommandSerial = serial;
        }
        ticket = serial;
        entry = CQ_last_queue();
        entry->active = ticket;
        entry->command = command;
        if (parameter) {
            parcpy(entry->payload, parameter);
            entry->parameter = parameter;
        } else {
            entry->parameter = 0;
        }
        entry->callback = callback;
        entry->count = count;
        g_CdPendingReadCount++;
        if (DS_system_status(0) == 1 && g_CdDsReadQueue[g_CdDsReadIndex].active == ticket)
            CQ_execute();
        result = ticket;
    } else
        result = 0;
    return result;
}

/* Empty constraints preserve the retail schedule. */

int DsPacket(int inMode, DslLOC *inPosition, int inCommand,
                              DslCB inCallback, int inCount) {
    DsPacketCommand commands[5];
    CdlLOC *position = inPosition;
    CdlLOC *callPosition;
    DslCB callback = inCallback;
    int mode = inMode;
    int command = inCommand;
    unsigned count = inCount;
    int n;
    int i, total;
    DsPacketCommand *ptr, *list, *item;
    CdDsReadQueueEntry *entry;
    DslCB *field;
    unsigned previous, next;
    unsigned ticket;
    unsigned result;
    unsigned char *payload;
    n = 0;
    ptr = commands;
    do {
        ptr->command = 0;
        ptr->parameter = 0;
        ptr->callback = 0;
        n++;
        ptr++;
    } while (n < 5);
    n = 2;
    callPosition = position;
    commands[0].command = 9;
    next = 14;
    ptr = &commands[1];
    commands[1].command = next;
    ptr->payload.minute = mode;
    ptr->parameter = &commands[1].payload;
    if (DsPosToInt(callPosition) < 0) {
        result = 0; return result;
    }
    commands[2].command = n;
    commands[2].payload = *position;
    commands[2].parameter = &commands[2].payload;
    n = 3;
    switch (command & 255) {
    case 21:
    case 22:

        item = commands;
        list = item;
        item = &list[n];
        item->command = command;
        item->callback = callback;
        total = n + 1;
        if (g_CdPendingReadCount + total >= 9) {
            result = 0; return result;
        }
        previous = g_DsCommandSerial;
        next = previous + 1;
        g_DsCommandSerial = next;
        ticket = next;
        if (!next) {
            next = previous + 2;
            g_DsCommandSerial = next;
            ticket = next;
        }
        i = 0;
        if (total > 0) {
            field = &list->callback;
            do {
                entry = CQ_last_queue();
                if (!entry) {
                    result = 0; return result;
                }
                entry->active = ticket;
                entry->command = list->command;
                payload = entry->payload;
                if (field[-1]) {
                    parcpy(payload, &list->payload);
                    entry->parameter = payload;
                } else
                    entry->parameter = 0;
                entry->callback = *field;
                field += 4;
                entry->count = count;
                next = g_CdPendingReadCount;
                i++;
                g_CdPendingReadCount = next + 1;
                list++;
            } while (i < total);
        }
        break;
    case 3:
    case 6:
    case 27:

        item = commands;
        list = item;
        item = &list[n];
        item->command = command;
        item->callback = callback;
        total = n + 1;
        if (g_CdPendingReadCount + total >= 9) {
            result = 0; return result;
        }
        previous = g_DsCommandSerial;
        next = previous + 1;
        g_DsCommandSerial = next;
        ticket = next;
        if (!next) {
            next = previous + 2;
            g_DsCommandSerial = next;
            ticket = next;
        }
        i = 0;
        if (total > 0) {
            field = &list->callback;
            do {
                entry = CQ_last_queue();
                if (!entry) {
                    result = 0; return result;
                }
                entry->active = ticket;
                entry->command = list->command;
                payload = entry->payload;
                if (field[-1]) {
                    parcpy(payload, &list->payload);
                    entry->parameter = payload;
                } else
                    entry->parameter = 0;
                entry->callback = *field;
                field += 4;
                entry->count = count;
                next = g_CdPendingReadCount;
                i++;
                g_CdPendingReadCount = next + 1;
                list++;
            } while (i < total);
        }
        break;
    default:
        goto failure;
    }
dispatch:
    if (DS_system_status(0) == 1 && g_CdDsReadQueue[g_CdDsReadIndex].active == ticket)
        CQ_execute();
    result = ticket;
    return result;
failure:
    result = 0;
done:
    return result;
}

/* Empty constraints preserve the retail schedule. */

s32 DsSync(s32 inId, void *inResult) {
    s32 id = inId;
    void *result = inResult;
    DsResult *selected;
    s32 cursor;
    s32 scanned;
    s32 searched;
    s32 offset;
    s32 status;
    s32 entryIdOrOffset;
    s32 forward;
    s32 readiness;
    s32 backward;

    if (id != 0) {
        DS_sync(0);
        forward = g_DsReadCallbackCursor;
        scanned = 0;
        entryIdOrOffset = forward * 0x10;
    scanForward:
        {
            DsSlotAddress at;
            at.slot = g_DsReadCallbackSlots;
            at.word += entryIdOrOffset;
            entryIdOrOffset = *at.words;
        }
        forward += 1;
        if (entryIdOrOffset != id) {
            if (forward >= 8) {
                forward = 0;
            }
            scanned += 1;
            if (scanned >= 8) {
                readiness = 0;
                {
                    DsSlotAddress at;
                    at.slot = g_DsReadCallbackSlots;
                    at.word += g_DsReadCallbackCursor * 0x10;
                    if (id < *at.words) {
                        goto completed;
                    }
                }
            } else {
                entryIdOrOffset = forward * 0x10;
                goto scanForward;
            }
        } else {
        completed:
            readiness = 7;
        }
        if (readiness != 7) {
            status = 0;
            return status;
        }
        {
            cursor = g_DsReadCallbackCursor;
            backward = cursor - 1;
            if (backward < 0) {
                backward = 7;
            }
            searched = 0;
            if (id != 0) {
                offset = backward * 0x10;
                while (1) {
                    DsSlotAddress at;
                    backward -= 1;
                    at.slot = g_DsReadCallbackSlots;
                    at.word += offset;
                    if (*at.words != id) {
                        if (backward < 0) {
                            backward = 7;
                        }
                        searched += 1;
                        offset = backward * 0x10;
                        if (searched >= 8) {
                            selected = 0;
                        } else {
                            continue;
                        }
                    } else {
                        goto publishResult;
                    }
                    break;
                }
            } else {
                DsSlotAddress at;
                offset = backward * 0x10;
                selected = 0;
                at.slot = g_DsReadCallbackSlots;
                at.word += offset;
                if (*at.words == 0) {

                } else {
                    goto publishResult;
                }
            }
            goto copyResult;
        }
    }
    cursor = g_DsReadCallbackCursor - 1;
    offset = cursor * 0x10;
    if (cursor < 0) {
        cursor = 7;
        offset = cursor * 0x10;
    }
    selected = 0;
    {
    DsSlotAddress at;
    at.slot = g_DsReadCallbackSlots;
    at.word += offset;
    if (*at.words != 0) {
    publishResult:
        {
            int *dst = g_DsSlotResultCopy.words;
            int first;
            int second;
            int third;
            {
                DsSlotAddress word0, word1, word2;
                word0.slot = g_DsReadCallbackSlots;
                word0.word += offset;
                first = *word0.words;
                word1.words = D_800A3614;
                word1.word += offset;
                second = *word1.words;
                word2.words = D_800A3618;
                word2.word += offset;
                third = *word2.words;
            }
            dst[0] = first;
            dst[1] = second;
            dst[2] = third;
            {
                DsSlotAddress word3;
                word3.words = D_800A361C;
                word3.word += offset;
                dst[3] = *word3.words;
            }
        }
        selected = &g_DsSlotResultCopy;
    }
    }
copyResult:
    if (selected != 0) {
        rescpy(result, selected->slot.payload);
        status = selected->slot.command;
    } else {
        status = 6;
    }
done:
    return status;
}

int DsReady(u8 *destination) {
    CdQueuedCmdSlot *slot;
    u8 *copyDestination;
    int selector;

    slot = (CdQueuedCmdSlot *)destination;
    DS_ready(0);
    if (D_800A3530.state == 1) {
        selector = 4;
    } else {
        selector = D_800A3520.state == 1;
    }

    if (selector == 4) {
        copyDestination = (u8 *)slot;
        slot = &D_800A3530;
    } else if (selector == 1) {
        copyDestination = (u8 *)slot;
        slot = &D_800A3520;
    } else {
        return 0;
    }

    slot->state = 0;
    rescpy(copyDestination, slot->payload);
    return slot->result;
}

void DsFlush(void) {
    int i;
    CdDsReadQueueEntry *p;
    int j;
    unsigned char *q;

    DS_stop();

    i = 0;
    g_CdPendingReadCount = 0;
    g_CdDsReadIndex = 0;
    g_CdDsReadQueueState = 0;
    p = g_CdDsReadQueue;

    while (i < 8) {
        j = 3;
        q = (unsigned char *)p + 3;
        p->active = 0;
        p->command = 0;
        for (; j >= 0; j--, q--) {
            q[5] = 0;
        }
        p->parameter = 0;
        p->callback = 0;
        p->count = 0;
        p++;
        i++;
    }

    DsEndReadySystem();
    DS_restart();
}

int DS_system_status(int arg0);

int DsSystemStatus(void) {
    int status = DS_system_status(0);

    if (status == 1 && DsQueueLen() > 0) {
        status = 2;
    }

    return status;
}

int DS_status(void);

int DS_shell_open(void);

int DsQueueLen(void) {
    return g_CdPendingReadCount;
}

int DsStatus(void) {
    return DS_status() & 0xFF;
}

int DsShellOpen(void) {
    return DS_shell_open();
}

int DsLastCom(void) {
    return DS_lastcom() & 0xFF;
}

void CQ_vsync_system(void) {
    int status;
    s32 *pending;
    s32 *workAddress;
    s32 work;
    unsigned int result;

    status = DS_system_status(0);
    if (status == 1) {
        pending = &g_CdPendingReadCount;
        if (*pending > 0) {
            result = DS_system_status(0);
            if (result == status) {
                workAddress = &g_CdDsReadIndex;
                work = *workAddress;
                result = work * 3;
                result <<= 3;
                work = (s32)CD_DS_QUEUE_FROM_PENDING(pending);
                work = result + work;
                if (((CdDsReadQueueEntry *)work)->active != 0) {
                    DS_cw(((CdDsReadQueueEntry *)work)->command,
                                      ((CdDsReadQueueEntry *)work)->parameter);
                }
            }
        }
    }
}

void CQ_error_flush(int command);

int CQ_ready_system(int command, u8 *payload) {
    int (*callbackFn)(u8, u8 *);
    CdQueuedCmdSlot *slot;
    u8 *copy_destination;
    u8 command_byte;

    command_byte = command;
    if (command_byte == 5 && (payload[0] & 0x10)) {
        CQ_error_flush(5);
    }

    switch (command_byte) {
    case 1:
    case 5:
                slot = &D_800A3520;
                copy_destination = slot->payload;
        break;
    case 4:
        slot = &D_800A3530;
        copy_destination = slot->payload;
        break;
    default:
        callbackFn = (int (*)(u8, u8 *))g_DsReadCallbackState.ready;
        if (callbackFn != 0)
            return callbackFn(command_byte, payload);
        return 0;
    }

    slot->state = 1;
    slot->result = command_byte;
    rescpy(copy_destination, payload);

callback:
    callbackFn = (int (*)(u8, u8 *))g_DsReadCallbackState.ready;
    if (callbackFn != 0)
        return callbackFn(command_byte, payload);
    return 0;
}

void LIBDS_DSSYS_2_text_13CC(int arg0) {
    if (g_DsStartCallback != 0) {
        g_DsStartCallback((unsigned char)arg0);
    }
}

unsigned int LIBDS_DSSYS_2_pad[] = {
    0x00000000,
    0x00000000,
};
