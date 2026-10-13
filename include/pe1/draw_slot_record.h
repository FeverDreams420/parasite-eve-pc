#ifndef PE1_DRAW_SLOT_RECORD_H
#define PE1_DRAW_SLOT_RECORD_H

/* g_ActiveDrawSlot (0x8009CDDC) as a one-field record (see
 * battle_word_records.h): a read through it is a structure access, so GCC
 * reloads the slot after each store into a packet structure, as retail
 * does, instead of reusing the first load. */
typedef struct DrawSlotRecord {
    int slot;
} DrawSlotRecord;

extern DrawSlotRecord g_ActiveDrawSlotRecord;

#endif
