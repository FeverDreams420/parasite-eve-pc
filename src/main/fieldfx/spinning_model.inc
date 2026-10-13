/* Spinning model flare: the stage 0 leader follows SPINNING_MODEL_ANCHOR and
 * drops shrinking stage 1 copies; each draws the D_800F32D8 model with a
 * size-dependent colour, scaled vertically by its height.
 * SPINNING_MODEL_FUNC       entry point
 * SPINNING_MODEL_ANCHOR     s16[3] anchor position
 * SPINNING_MODEL_PALETTE    palette offset (the special palette adds 4)
 * SPINNING_MODEL_COLOR(s)   r, g, b arguments for FieldFx_ModelSetColors */
#include "pe1/render_object.h"
#include "pe1/fieldfx_task.h"
#include "pe1/field_model_draw.h"
#include "pe1/gte.h"
#include "pe1/psyq_gpu.h"
#include "pe1/render_prim.h"

int SPINNING_MODEL_FUNC(int mode, FieldFxSpinningModel *state)
{
    GteShortVector angles;
    GteMatrix matrix;
    GteVector scale;
    FieldFxSpinningModel *copy;
    int tpage;
    int kind;
    int palette;

    switch (mode) {
    case 1:
        if (state->stage == 0) {
            state->x = SPINNING_MODEL_ANCHOR[0];
            state->z = SPINNING_MODEL_ANCHOR[2];
            state->y = SPINNING_MODEL_ANCHOR[1];
            state->angle += 32 + g_FieldFxCurrentAge * 4;
            state->size = rsin((g_FieldFxCurrentAge << 11) / 24) / 32;
            state->height = (g_FieldFxCurrentAge << 12) / 24;
            if (g_FieldFxCurrentAge & 1) {
                copy = FieldFx_AcquireCallbackSlot(g_FieldFxActiveTaskSlot->end);
                if (copy) {
                    copy->stage = 1;
                    copy->x = state->x;
                    copy->y = state->y;
                    copy->z = state->z;
                    copy->size = state->size;
                    copy->height = state->height;
                }
            }
            if (g_FieldFxCurrentAge >= 24)
                return 1;
        } else {
            state->angle += 3;
            state->size = state->size * 3 / 4;
            if (g_FieldFxCurrentAge >= 4)
                return 1;
        }
        state->ticks++;
        break;
    case 2:
        scale.z = 0x400;
        scale.x = 0x400;
        scale.y = state->height / 4;
        angles.x = 0;
        angles.y = state->angle;
        angles.z = 0;
        RotMatrixYXZ(&angles, &matrix);
        ScaleMatrix(&matrix, &scale);
        matrix.t[0] = state->x;
        matrix.t[1] = state->y;
        matrix.t[2] = state->z;
        g_FieldFxSpriteParams.tpage = g_FieldFxTpageTable[g_FieldFxTpageSlots[9]];
        g_FieldFxSpriteParams.palette = 1;
        FieldFx_EnsureEffectTexturePage(1);
        g_FieldFxSpriteParams.parameter06 = 1;
        gte_ldrotmatrix(g_ViewMatrixSlot.value);
        gte_ldtransmatrix(g_ViewMatrixSlot.value);
        tpage = (u16)(g_FieldFxTpageTable[g_FieldFxTpageSlots[9]] | GetTPage(0, 1, 0, 0));
        kind = g_FieldFxSpriteParams.palette;
        palette = g_FieldFxPaletteTable[kind];
        FieldFx_ModelSetTexture(tpage, GetClut(0, (kind == 4 && g_FieldFxAltPaletteFlag) ? palette + SPINNING_MODEL_PALETTE + 4 : palette + SPINNING_MODEL_PALETTE));
        FieldFx_ModelSetSemiTrans(1);
        FieldFx_ModelSaveColors(D_800F32D8);
        FieldFx_ModelSetColors(D_800F32D8, SPINNING_MODEL_COLOR(state->size));
        FieldFx_ModelDraw((union FieldTexturedModel *)D_800F32D8, &matrix);
        FieldFx_ModelRestoreColors(D_800F32D8);
        break;
    }
    return 0;
}

#undef SPINNING_MODEL_FUNC
#undef SPINNING_MODEL_ANCHOR
#undef SPINNING_MODEL_PALETTE
#undef SPINNING_MODEL_COLOR
