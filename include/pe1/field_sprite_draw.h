#ifndef PE1_FIELD_SPRITE_DRAW_H
#define PE1_FIELD_SPRITE_DRAW_H

/* Field engine sprite, shape and color helpers shared by the main field
 * effects and the room overlays. Only forward-declared types appear here so
 * room headers can include this without the full render_object.h. */
struct GteShortVector;
struct GteRotation;
struct RenderColor;
struct FieldTrailPair;

void FieldFx_GetActorPosition(char *object, int mode, short *position);

/* Builds the tpage words of the two effect texture pages (0x380 and 0x340). */
void FieldFx_InitTexturePageTable(void);

/* Loads texture block index (0 or 1) into VRAM and marks it resident. */
void FieldFx_LoadEffectTexturePage(int index);

/* Loads texture block index (0 or 1) into VRAM unless already resident. */
void FieldFx_EnsureEffectTexturePage(int index);

/* Draws the quads of shape texture (pe1/field_shape_quads.h); a null
 * rotation selects the default. */
void FieldFx_DrawShapeQuads(struct GteShortVector *position, struct GteRotation *rotation,
                   int scale_x, int scale_y, int texture, int clut,
                   int page, int intensity, struct RenderColor *color);

/* Initializes cached key times, clamps time at the end and blends colors.
 * The track (a RenderColorTrack) requires at least one positive-duration
 * key and a zero-duration terminator carrying the final RGB value. Call
 * with nonnegative time. */
void FieldFx_SampleColorTrack(void *track, void *color, int time);

/* Colour track of the rising ember sparks (fieldfx/sprite_effects.c). */
extern char g_EmberColorTrack[];

/* Rotates (0, 0, distance) by the angles (z forced to 0) into out. */
void FieldFx_AnglesToVector(struct GteShortVector *angles, short distance,
                   struct GteShortVector *out);

void FieldFx_DrawStarFan(struct GteShortVector *position, int width, int height,
                   int segments, struct GteRotation *rotation, int scale_x,
                   int scale_y, struct RenderColor *color0,
                   struct RenderColor *color1, int intensity, int mode);
void FieldFx_DrawRingBand(struct GteShortVector *position, int inner, int outer,
                   int segments, struct GteRotation *rotation, int scale_x,
                   int scale_y, struct RenderColor *color0,
                   struct RenderColor *color1, int intensity, int mode);
void FieldFx_DrawRotatedTriangle(struct GteShortVector *position, struct GteRotation *rotation,
                   int length, int breadth, struct RenderColor *color0,
                   struct RenderColor *color1, struct RenderColor *color2,
                   int intensity, int mode);
void FieldFx_DrawHaloedPoint(void *position, void *color, int scale, int flags);
void FieldFx_DrawRotatedQuad(struct GteShortVector *position, struct GteRotation *rotation,
                   int width, int height, int u, int v, int texture_width,
                   int texture_height, int clut, struct RenderColor *color0,
                   struct RenderColor *color1, int intensity, int mode);
void FieldFx_DrawOrbitPoint(int x, int y, void *color, int scale, int mode);

void FieldFx_DrawScreenTint(unsigned char *color, int scale, int abr, unsigned int depth);

void FieldFx_DrawPointTile(struct GteShortVector *position, unsigned char *color,
                   int scale, int abr);

void FieldFx_DrawHistoryTrail(struct GteShortVector *head, struct GteShortVector *tail,
                   unsigned int count, unsigned char *color0,
                   unsigned char *color1, int scale,
                   struct FieldTrailPair *history, int mode);

void FieldFx_DrawTexturedRibbon(struct GteShortVector *points, short count, int width, int u,
                   int v, int du, int height, int tpage, int clut, int scale,
                   struct RenderColor *color0, struct RenderColor *color1,
                   int mode);

void FieldFx_EaseAngles(unsigned short *from, unsigned short *to, unsigned short speed);

#endif /* PE1_FIELD_SPRITE_DRAW_H */
