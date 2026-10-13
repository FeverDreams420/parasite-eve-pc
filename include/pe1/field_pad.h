#ifndef PE1_FIELD_PAD_H
#define PE1_FIELD_PAD_H

/* Pad bits sampled for the field each frame (FIELD_PAD_* in
 * pe1/field_movement.h); the menu input mapper reads them too. */
extern unsigned int g_FieldPadBits;
/* Pad bits newly pressed this frame. */
extern unsigned int D_8009D1F4;

#endif
