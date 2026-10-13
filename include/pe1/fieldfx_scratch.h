#ifndef PE1_FIELDFX_SCRATCH_H
#define PE1_FIELDFX_SCRATCH_H

#include "common.h"
#include "pe1/gte_types.h"

/* Scratchpad work area the field engine draw routines publish through
 * g_FieldFxScratch: an ordering-table depth, the current texture cell, the
 * CLUT offset, a colour channel temporary, a composed matrix and an
 * object-local matrix. */
typedef struct FieldFxScratch {
    /* 0x00 */ u8 pad00[0xC];
    /* 0x0C */ s32 depth;
    /* 0x10 */ u8 u;
    /* 0x11 */ u8 v;
    /* 0x12 */ u8 clutX;
    /* 0x13 */ u8 clutY;
    /* 0x14 */ u32 channel;
    /* 0x18 */ u8 pad18[4];
    /* 0x1C */ GteMatrix matrix;
    /* 0x3C */ GteMatrix local;
} FieldFxScratch;

#define FIELDFX_SCRATCH ((FieldFxScratch *)0x1F800000)

extern FieldFxScratch *g_FieldFxScratch;

#endif
