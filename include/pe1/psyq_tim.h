#ifndef PE1_PSYQ_TIM_H
#define PE1_PSYQ_TIM_H

#include "common.h"
#include "pe1/psyq_gpu.h"

/* The length includes this block's length word, rectangle and pixel bytes;
 * `bytes` views the block so the next block is bytes[length]. */
typedef union TimBlock {
    struct {
        int length;
        RECT rect;
        int pixels[1];
    } header;
    u8 bytes[1];
} TimBlock;

/* The optional CLUT block (flags bit 3) comes first, then the image block. */
typedef struct TimFile {
    int magic;
    int flags;
    TimBlock first_block;
} TimFile;

#endif
