#include "common.h"

/* Table entry address: the queue offsets are byte offsets into the table. */
typedef union DrawPrimGeneratorAddress {
    u32 *pointer;
    u32 address;
} DrawPrimGeneratorAddress;

u32 Draw_FlushPrimQueue(void) {
    u32 *base;
    u32 *head_slot;
    u32 *tail_slot;
    s32 head;
    s32 tail;
    u32 *head_ptr;
    u32 *tail_ptr;
    u32 head_value;
    u32 tail_value;
    u32 ret;
    /* Match debt: read architectural zero without emitting an instruction;
     * OR/ORI preserve the original encodings of the return and wrap value. */
    u32 zero = 0;
        base = (u32 *)0x80070E0C;
    head_slot = (u32 *)0x80070E04;
    tail_slot = (u32 *)0x80070E08;
    head = *head_slot;
    tail = *tail_slot;
    /* Keep the base as a operand rather than a folded constant. */

    /* Match note: preserve target operand order in the address adds. */
    {
        DrawPrimGeneratorAddress entry;

        entry.pointer = base;
        entry.address += head;
        head_ptr = entry.pointer;
    }
    {
        DrawPrimGeneratorAddress entry;

        entry.pointer = base;
        entry.address += tail;
        tail_ptr = entry.pointer;
    }

    {
        u32 previousValue = *head_ptr;
        tail_value = *tail_ptr;
        head_value = previousValue + tail_value;
    }
    *head_ptr = head_value;
    /* Finish the entry write and result copy before updating queue offsets. */
    ret = zero | head_value;

    head = head - 4;
    tail = tail - 4;
    if (head < 0) {
        head = zero | 0x40;
    }
    *head_slot = head;
    if (tail < 0) {
        tail |= 0x40;
    }
    *tail_slot = tail;
    return ret;
}
