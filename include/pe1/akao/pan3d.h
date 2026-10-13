#ifndef PE1_AKAO_PAN3D_H
#define PE1_AKAO_PAN3D_H

#include "common.h"
#include "pe1/akao/pos.h"
#include "pe1/gte_types.h"
#include "pe1/projection_slot.h"

/* Projects a world position and derives the stereo pan (0..255) and the
 * distance volume (0..127) from the scene's audio depth range. */
int Akao_Calc3DPan(AkaoPackedRect3 *pos, int *pan, int *volume);
s32 Akao_SendPositionalCmd(void *table, s32 command, s32 arg, AkaoPosCoord x, AkaoPosCoord y, AkaoPosCoord z);

#endif /* PE1_AKAO_PAN3D_H */
