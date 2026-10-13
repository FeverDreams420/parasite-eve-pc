#ifndef PE1_FIELDFX_SLOT_H
#define PE1_FIELDFX_SLOT_H

#include "common.h"

/* One dispatch slot in g_FieldFxSlotTable (0x40 entries, 6 bytes each).
 * The field engine walks the table every frame: flag==1 dispatches the
 * handler_id'th handler, flag==2 marks the slot for teardown. */
typedef struct FieldFxSlot {
    /* 0x0 */ unsigned char handler_id;
    /* 0x1 */ signed char flag;
    /* 0x2 */ unsigned short counter;
    /* 0x4 */ short data_offset;
} FieldFxSlot;

/* A slot address as a pointer or a word: the dispatch passes add the scaled
 * slot index to the table address as an integer, index first. */
typedef union FieldFxSlotAddress {
    FieldFxSlot *slot;
    u32 word;
} FieldFxSlotAddress;

PE1_STATIC_ASSERT(sizeof(FieldFxSlot) == 6, fieldfx_slot_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(FieldFxSlot, flag) == 1, fieldfx_slot_flag);
PE1_STATIC_ASSERT(PE1_OFFSETOF(FieldFxSlot, counter) == 2, fieldfx_slot_counter);
PE1_STATIC_ASSERT(PE1_OFFSETOF(FieldFxSlot, data_offset) == 4,
                  fieldfx_slot_data_offset);

/* Clears an object's script and dispatch storage, then returns its script
 * table pointer slot at object + 0x78. */
struct FieldFxObject;
void **FieldFx_GetSlot(struct FieldFxObject *object);

#endif
