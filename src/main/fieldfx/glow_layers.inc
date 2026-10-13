/* Field glow layers: compose the effect's axis matrix with two fixed spins
 * and draw the sprite once per spin, scaled by a fixed vector.
 * GLOW_LAYERS_FUNC    entry point
 * GLOW_LAYERS_SPIN_A  GteShortVector first spin
 * GLOW_LAYERS_SPIN_B  GteShortVector second spin
 * GLOW_LAYERS_SCALE   GteVector sprite scale
 * GLOW_LAYERS_SPRITE  FieldGlowSprite parameter block */
#include "pe1/field_glow_layers.h"
#include "pe1/gte.h"

/* Two layers: the effect's axis matrix composed with each spin
 * (gte_CompMatrix), scaled, and drawn as the glow sprite. */
void GLOW_LAYERS_FUNC(void *object, void *slot, FieldGlowLayers *glow)
{
    GteMatrix matrix;
    GteMatrix rotation;
    GteShortVector spinA;
    GteShortVector spinB;
    GteVector scaleA;
    GteVector scaleB;

    spinA = GLOW_LAYERS_SPIN_A;
    spinB = GLOW_LAYERS_SPIN_B;
    FieldFx_SetSpriteTexturePreset(3);
    FieldFx_SetSpriteColorMode(0x10);
    FieldFx_SetSpriteCellSize(0x20, 0x10);
    FieldFx_SetSpriteBlendMode(2);
    GLOW_LAYERS_SPRITE.depth = glow->depth;

    RotMatrixYXZ(&spinA, &rotation);
    matrix = glow->matrix;
    matrix.t[0] = glow->x;
    matrix.t[1] = glow->y;
    matrix.t[2] = glow->z;
    rotation.t[0] = 0;
    rotation.t[1] = 0;
    rotation.t[2] = 0;
    gte_CompMatrix(&matrix, &rotation, &matrix);
    scaleA = GLOW_LAYERS_SCALE;
    ScaleMatrix(&matrix, &scaleA);
    FieldFx_DrawGlowSprite(&GLOW_LAYERS_SPRITE, &matrix, 0);

    RotMatrixYXZ(&spinB, &rotation);
    matrix = glow->matrix;
    matrix.t[0] = glow->x;
    matrix.t[1] = glow->y;
    matrix.t[2] = glow->z;
    gte_CompMatrix(&matrix, &rotation, &matrix);
    scaleB = GLOW_LAYERS_SCALE;
    ScaleMatrix(&matrix, &scaleB);
    FieldFx_DrawGlowSprite(&GLOW_LAYERS_SPRITE, &matrix, 0);
}

#undef GLOW_LAYERS_FUNC
#undef GLOW_LAYERS_SPIN_A
#undef GLOW_LAYERS_SPIN_B
#undef GLOW_LAYERS_SCALE
#undef GLOW_LAYERS_SPRITE
