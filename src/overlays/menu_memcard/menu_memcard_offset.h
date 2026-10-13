#ifndef MENU_MEMCARD_OFFSET_H
#define MENU_MEMCARD_OFFSET_H

#include "common.h"

/* Joint-space offset handed to FieldFx_TransformMatrixPoint. */
typedef struct MemcardJointOffset {
    s16 x, y, z, reserved06;
} MemcardJointOffset;

/* Hand offset shared by the link, trail and cross flash effects. */
extern MemcardJointOffset g_MenuCardHandOffset;

#endif /* MENU_MEMCARD_OFFSET_H */
