#ifndef PE1_FIELD_SPRITE_STATE_H
#define PE1_FIELD_SPRITE_STATE_H

#include "common.h"

/* Field engine sprite draw state, owned by fieldfx/sprite_render.c:
 * the texture page and CLUT origin the sprite draws use, the semi-transparency
 * mode and the cell size. The corners of the sprite quad that
 * FieldFx_SetSpriteCellSize sizes are declared with the shaded quad (field_shaded_quad.h).
 * Effects set it up before drawing through FieldFx_SetSpriteTexturePreset (page origin and
 * CLUT row), FieldFx_SetSpriteColorMode (colour depth), FieldFx_SetSpriteBlendMode (blend mode) and
 * FieldFx_SetSpriteCellSize (cell size in texels). */

extern u8 g_FieldFxSpriteColorMode;  /* texture colour mode: 0 = 4-bit, 1 = 8-bit */
extern u8 g_FieldFxSpriteBlendRate;  /* semi-transparency rate (GetTPage abr) */
extern u8 g_FieldFxSpriteBlendMode;  /* blend mode last set by FieldFx_SetSpriteBlendMode */
extern u8 g_FieldFxSpriteSemiTrans;  /* draw the sprites semi-transparent */
extern u16 g_FieldFxSpriteTpageX; /* texture page origin x */
extern u16 g_FieldFxSpriteTpageY; /* texture page origin y */
extern u16 g_FieldFxSpriteTpageValue; /* GetTPage value of the current state */
extern u16 g_FieldFxSpriteClutX; /* CLUT origin x */
extern u16 g_FieldFxSpriteClutY; /* CLUT origin y */
extern u8 g_FieldFxSpriteCellVBase; /* cell row base added to the cell's v */
extern u8 g_FieldFxSpriteCellUMax;  /* cell width - 1 */
extern u8 g_FieldFxSpriteCellVExtent;  /* cell height - 1 */

void FieldFx_SetSpriteTexturePreset(u8 mode);
void FieldFx_SetSpriteCellSize(int width, int height);
void FieldFx_SetSpriteColorMode(int mode);
/* Printed by FieldFx_SetSpriteColorMode for a depth other than 4 or 8 bits. */
extern char g_FieldSpriteWrongColorModeMsg[];
void FieldFx_InterpolateColorRamp(u8 *table, u32 step, u8 *out);
void FieldFx_SetSpriteBlendMode(u8 mode);

#endif
