#ifndef PE1_RENDER_MATRIX_COPY_H
#define PE1_RENDER_MATRIX_COPY_H

#include "pe1/render_object.h"

void Render_CopyMatrixBlock(RenderMatrix *source, RenderMatrix *destination,
                            s16 count);

/* Matrix blocks copied word by word into model and player matrices. */
extern s32 g_IdentityMatrixSource[8];

#endif
