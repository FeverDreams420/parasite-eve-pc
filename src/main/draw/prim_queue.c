#include "common.h"
#include "pe1/boot.h"

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
    base = g_DrawPrimGeneratorWords + 16;
    head_slot = g_DrawPrimGeneratorWords + 14;
    tail_slot = g_DrawPrimGeneratorWords + 15;
    head = (s32)*head_slot;
    tail = (s32)*tail_slot;
    head_ptr = base + head / 4;
    tail_ptr = base + tail / 4;

    {
        u32 previousValue = *head_ptr;
        tail_value = *tail_ptr;
        head_value = previousValue + tail_value;
    }
    *head_ptr = head_value;
    /* Finish the entry write and result copy before updating queue offsets. */
    ret = head_value;

    head = head - 4;
    tail = tail - 4;
    if (head < 0) {
        head = 0x40;
    }
    *head_slot = head;
    if (tail < 0) {
        /* Retail ORs the negative value; it does not reset it to +0x40.
         * This cycles through -4..-64, including both cursors and the
         * instruction constants retained in the same backing array. */
        tail |= 0x40;
    }
    *tail_slot = tail;
    return ret;
}
