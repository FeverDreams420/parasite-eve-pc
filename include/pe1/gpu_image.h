#ifndef PE1_GPU_IMAGE_H
#define PE1_GPU_IMAGE_H

#include "pe1/psyq_gpu.h"

/* VRAM fill and copy as the game calls them. LIBGPU SYS.OBJ defines
 * ClearImage with byte colour arguments and MoveImage returning a status;
 * the callers pass ints and ignore the result. */
void ClearImage(RECT *rect, int r, int g, int b);
void MoveImage(RECT *rect, int x, int y);

#endif
