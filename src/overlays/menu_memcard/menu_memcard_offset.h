#ifndef MENU_MEMCARD_OFFSET_H
#define MENU_MEMCARD_OFFSET_H

#include "common.h"
#include "pe1/gte_short_vector.h"

/* Joint-space offset handed to FieldFx_TransformMatrixPoint. */
typedef GteShortVector MemcardJointOffset;

/* Hand offset shared by the link, trail and cross flash effects. */
extern MemcardJointOffset g_MenuCardHandOffset;

#endif /* MENU_MEMCARD_OFFSET_H */
