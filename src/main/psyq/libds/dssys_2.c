/* ASSEMBLER: GNU */
/* Psy-Q LIBDS DSSYS_2.OBJ, part 1 of 3: CQ_clear_queue, CQ_delete_command,
 * CQ_last_queue, CQ_error_flush, CQ_execute, CQ_sync_system, CQ_add_result.
 * DsInit and DsReset (dssys_2_2.c) still need GCC 2.8.1. */

#include "common.h"
#include "pe1/psyq_cd.h"
#include "pe1/psyq_ds.h"
#include "pe1/psyq_ds_queue.h"

void CQ_clear_queue(void *arg0) {
    int i;
    char *ptr;
    char *bytes = arg0;
    int *words = arg0;

    words[0] = 0;
    i = 3;
    ptr = bytes + 3;
    bytes[4] = 0;
    for (; i >= 0; i--) {
        ptr[5] = 0;
        ptr--;
    }
    words[3] = 0;
    words[4] = 0;
    words[5] = 0;
}

void CQ_delete_command(void) {
    int *state;
    int *initial;
    int *end;
    int *count;
    CdDsReadQueueEntry *entries;
    CdDsReadQueueEntry *entry;
    unsigned int active;
    int i;
    int index;
    unsigned char *ptr;

    initial = &g_CdDsReadQueueState;
    count = initial + 2;
    entries = (CdDsReadQueueEntry *)((char *)initial -
        PE1_OFFSETOF(CdDsReadQueueWindow, queue_state));
    {
        int index = initial[0];
        state = initial;
        active = g_CdDsReadQueue[index].active;
    }
    while (1) {
    if (*count <= 0) break;
    index = g_CdDsReadQueueState;
    i = 3;
    entry = (CdDsReadQueueEntry *)
        ((index * sizeof(CdDsReadQueueEntry)) + (int)entries);
    entry->active = 0;
    ptr = (unsigned char *)entry + 3;
    entry->command = 0;
    do {
        ptr[5] = 0;
        i--;
        ptr--;
    } while (i >= 0);
    entry->parameter = 0;
    entry->callback = 0;
    entry->count = 0;
    state[0]++;
    if (state[0] >= 8) state[0] = 0;
    state[2]--;
    if (g_CdDsReadQueue[state[0]].active == active) continue;
    break;
    }
    end = &g_CdDsReadIndex;
    state = end - 1;
    *end = *state;
}

CdDsReadQueueEntry *CQ_last_queue(void) {
    volatile int *base;
    int count;
    int index;
    int delta;
    int scaled;
    int byte_offset;
    unsigned char *entry_base;

    base = &g_CdPendingReadCount;
    asm volatile("" : "=r"(base) : "0"(base));
    count = base[0];
    if (count >= 8) {
        return 0;
    }

    delta = ((volatile CdDsReadQueueWindow *)CD_DS_QUEUE_FROM_PENDING(base))->queue_state;
    index = delta + count;
    if (index >= 8) {
        index -= 8;
    }

    scaled = (index << 1) + index;
    byte_offset = scaled << 3;
    entry_base = (unsigned char *)CD_DS_QUEUE_FROM_PENDING(base)->entries;
    return (CdDsReadQueueEntry *)(byte_offset + (int)entry_base);
}

/* Psy-Q LIBDS/DSSYS_2.OBJ private text_170.
 * Provenance: configs/USA/psyq_provenance.json (LIBDS). */
void CQ_error_flush(int event, u8 *data) {
    DsQueueCallback callbacks[8];
    register u8 *result = data;
    register int savedEvent asm("$22") = event;
    register int i;
    register int last;
    register int index;

    register u32 id;
    u32 previous;
    {
        DsQueueCallback *init;
        i = 0;
        init = callbacks;
        for (; i < 8; i++, init++) {
            init->id = 0;
            init->callback = 0;
            asm("" : : "m"(*init));
        }
    }
    {
        register DsQueueIndices *indices asm("$3") = (DsQueueIndices *)&g_CdDsReadQueueState;
        asm("" : "=r"(indices) : "0"(indices));
        previous = 0;
        last = -1;
        index = indices->start;
        i = 0;
        if (indices->count > 0)
            do {
                DsEventCallback callback;
                id = g_CdDsReadQueue[index].active;
                if (id != previous) {
                    CQ_add_result(id, savedEvent, result);
                    previous = id;
                }
                callback = g_CdDsReadQueue[index].callback;
                if (callback) {
                    {
                        register int sentinel asm("$7") = -1;
                        if (last == sentinel) {
                            last = 0;
                            callbacks[0].id = id;
                        } else if (callbacks[last].id != id) {
                            last++;
                            callbacks[last].id = id;
                        } else {
                            goto skipCallback;
                        }
                        callbacks[last].callback = callback;
                    skipCallback:;
                    }
                }
                index++;
                if (index >= 8)
                    index = 0;
                i++;
            } while (i < g_CdPendingReadCount);
    }
    g_CdPendingReadCount = 0;
    g_CdDsReadIndex = 0;
    g_CdDsReadQueueState = 0;
    {
        register int n;
        register int j;
        register CdDsReadQueueEntry *entry;
        n = 0;
        entry = g_CdDsReadQueue;
        for (; n < 8; n++, entry++) {
            asm("" : "=r"(entry) : "0"(entry));
            entry->active = 0;
            entry->command = 0;
            {
                register u8 *clear;
                j = 3;
                asm("" : : "r"(j));
                /* Reverse byte traversal retains the queue-entry base. */
                {
                    CdDsQueueEntryAddress at;
                    at.entry = entry;
                    at.word += 3;
                    clear = at.bytes;
                }
                for (; j >= 0; j--, clear--)
                    clear[PE1_OFFSETOF(CdDsReadQueueEntry, payload)] = 0;
            }
            entry->parameter = 0;
            entry->callback = 0;
            entry->count = 0;
            asm("" : : "m"(*entry));
        }
    }
    {
        i = 0;
        if (last >= 0) {
            DsQueueCallback *call = callbacks;
            do {
                register int eventArg asm("$4") = (u8)savedEvent;
                register u8 *dataArg asm("$5") = result;
                register DsEventCallback cb;
                asm("" : : "r"(eventArg), "r"(dataArg));
                cb = call->callback;
                call++;
                i++;
                cb(eventArg, dataArg);
            } while (i <= last);
        }
    }
}

#define QUEUE_FROM_CURRENT(p)                                                          \
    ((CdDsReadQueueWindow *)((u8 *)(p) - PE1_OFFSETOF(CdDsReadQueueWindow, read_index)))

int CQ_execute(void) {
    int *current;
    CdDsQueueEntryAddress at;
    CdDsReadQueueEntry *entry;
    if (DS_system_status(0) != 1) {
        return 0;
    }

    current = &g_CdDsReadIndex;
    at.entry = QUEUE_FROM_CURRENT(current)->entries;
    at.word = *current * sizeof(CdDsReadQueueEntry) + at.word;
    entry = at.entry;

    if (entry->active != 0) {
        return DS_cw(entry->command, entry->parameter) != 0;
    }
    return 0;
}

void CQ_sync_system(int inEvent, u8 *inResult) {
    register int event = inEvent;
    register u8 *result = inResult;
    register int *current;
    register CdDsReadQueueEntry *base asm("$19");
    register CdDsReadQueueEntry *entry;
    register int savedEvent asm("$20");
    current = &g_CdDsReadIndex;
    {
        int index = *current;
        asm("" : : "r"(index));
        base = QUEUE_FROM_CURRENT(current)->entries;
        asm("" : "=r"(base) : "0"(base));
        {
            int offset = index * sizeof(CdDsReadQueueEntry);
            CdDsQueueEntryAddress at;
            at.entry = base;
            at.word = offset + at.word;
            entry = at.entry;
        }
    }
    savedEvent = event;
    if (entry->active) {
        {
            register CdQueuedCmdSlot *slot = &g_CdQueuedCmdSlots[0];
            asm("" : "=r"(slot) : "0"(slot));
            slot->state = entry->active;
            slot->result = event;
            rescpy(slot->payload, result);
        }
        switch ((u8)savedEvent) {
        case 2: {
            register int indexValue = *current;
            register int next = indexValue + 1;
            register int valid = next < 8;
            int index = next;
            if (!valid)
                index = 0;
            if (base[index].active != entry->active) {
                CQ_add_result(entry->active, 2, result);
                {
                    register DslCB cb = entry->callback;
                    if (cb)
                        cb(2, result);
                }
                CQ_delete_command();
            } else {
                *current = next;
                if (!valid) {
                    *current = 0;
                }
            }
            break;
        }
        case 5: {
            int retries = entry->count;
            if (retries > 0 || retries == -1) {
                *current = ((CdDsReadQueueWindow *)base)->queue_state;
                if (entry->count != -1)
                    entry->count--;
            } else {
                CQ_add_result(entry->active, 5, result);
                {
                    register DslCB cb = entry->callback;
                    if (cb)
                        cb(5, result);
                }
                CQ_delete_command();
            }
            break;
        }
        }
    }
    if (g_DsReadCallbackState.sync)
        g_DsReadCallbackState.sync(savedEvent, result);
    {
        int ready = DS_system_status(0);
        if (ready == 1) {
            int *pending = &g_CdPendingReadCount;
            if (*pending > 0 && DS_system_status(0) == ready) {
                register int index = g_CdDsReadIndex;
                register CdDsReadQueueEntry *item;
                {
                    int offset = index * sizeof(CdDsReadQueueEntry);
                    register u8 *queue;
                    queue = (u8 *)pending -
                            PE1_OFFSETOF(CdDsReadQueueWindow, pending_count);
                    item = (CdDsReadQueueEntry *)(offset + queue);
                }
                if (item->active)
                    DS_cw(item->command, item->parameter);
            }
        }
    }
}

void CQ_add_result(u32 value, u8 command, u8 *payload) {
    int *cursor;
    u8 *payloadBase;

    cursor = &g_DsReadCallbackCursor;
    asm volatile("" : "=r"(cursor) : "0"(cursor));

    g_DsReadCallbackSlots[*cursor].value = value;
    g_DsReadCallbackSlots[*cursor].command = command;

    payloadBase = (u8 *)cursor - 123;
    rescpy((u8 *)((*cursor << 4) + (int)payloadBase), payload);

    (*cursor)++;
    if (*cursor >= 8) {
        *cursor = 0;
    }
}
