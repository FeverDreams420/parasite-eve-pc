#ifndef PE1_DRAW_STATE_H
#define PE1_DRAW_STATE_H

#include "common.h"
#include "pe1/render_prim.h"

/* Lookup entries have an eight-byte stride; the final byte is not read here. */
typedef struct DrawGlyphDescriptor {
    u8 u, v;
    u16 clut;
    u8 width, height, mode, reserved;
} DrawGlyphDescriptor;

typedef struct DrawTextCursorPair {
    u32 x;
    u32 y;
} DrawTextCursorPair;

/* The draw-enabled byte is read signed by the display queries and unsigned
 * by the sequence-elapsed query. */
typedef union GpuDrawEnabledFlag {
    signed char value;
    unsigned char bits;
} GpuDrawEnabledFlag;

/* Wipe bar vertex coordinates, stored y first. */
typedef struct {
    s16 y, x;
} __attribute__((packed)) DrawVertexPair;

PE1_STATIC_ASSERT(sizeof(DrawTextCursorPair) == 8, draw_text_cursor_pair_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(DrawTextCursorPair, y) == 4,
                  draw_text_cursor_pair_y_offset);

PE1_STATIC_ASSERT(sizeof(DrawGlyphDescriptor) == 8, draw_glyph_descriptor_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(DrawGlyphDescriptor, mode) == 6, draw_glyph_mode_offset);
DrawGlyphDescriptor *Draw_LookupGlyphDescriptor(int index);
void Menu_SetSaveSlotBlendBase(int color);

/* A GPU packet's address as the low 24 bits of an ordering-table link. */
typedef union DrawPrimLink {
    void *packet;
    u32 word;
} DrawPrimLink;

/* Semantic C names for the existing, independently addressed linker objects. */
extern unsigned char *g_DrawPacketCursor;
extern unsigned char *g_DrawPacketArenaBase;
extern int g_DrawBufferIndex;
extern unsigned int g_DrawPrimColor;
extern unsigned int g_DrawColorShaded;
extern unsigned int *g_DrawOrderingTableEntry;

#define g_DrawColorSelect g_DrawTextDimmed
#define g_DrawPrimaryColor g_DrawPrimColor
#define g_DrawAlternateColor g_DrawColorShaded
#define g_TextCursorStack g_TextCursorStackPtr
#define g_DrawSpriteX g_TextCursorX
#define g_DrawSpriteY g_TextCursorY

extern int g_TextCursorStackBottom[], g_TextCursorStackTop[];
extern int *g_TextCursorStackPtr;

/* Byte (x, y) pairs terminated by 0xFF, followed by polygon edges. */
void Draw_AllocColorGradient(int width, int height, u8 *points, int textured);
void Draw_AllocColorTri(int width, int height, int mode);
void Draw_AllocColorRect(int firstVertex, int secondVertex, int width, int mode);
void Draw_EmitWipeBar(u8 *edges, int mode);
void Draw_AllocSprite(int sprite);
void Draw_EmitDigitSprite(int digit);
void Draw_AllocTexturedQuad(int glyph);
void Draw_AllocTexturedRect(int value, int width);
int Draw_LookupGlyphMetrics(int glyph);
void Draw_PrintTextWrapped(u8 *text, int width);
void Draw_PrintCenteredTextInWidth(u8 *text, int width);
void MenuWidget_DrawCenteredText(u8 *text);
extern int g_TextRenderMode, g_DrawGlyphAdvance;
/* Width the centred text routines centre within, and the dimmed flag the
 * widget renderer sets per list row. */
extern int g_DrawTextBoxWidth;
extern int g_DrawTextDimmed;
extern int g_TextCursorX;
extern int g_TextCursorY;
extern int g_DrawDigitFontBaseTexU, g_DrawDigitFontBaseTexV;
extern int g_DrawDigitFontTpageClut;
void Draw_BlendColor(int color);
int Draw_GetBaseY(void);
void Draw_SetBaseOffsetPosition(int x, int y);

void Draw_PrintNumberWidth4Unk(int value);
void Draw_PrintSignedNumberWidth4(int value);

void Draw_SetStatCompareColor(int current, int candidate);
void Draw_PrintNumberWidth4(int value);
void Draw_PrintNumberWidth2(int value);
void Draw_PrintNumberWidth3(int value);
void Draw_PrintNumberWidth5(int value);
void Draw_PrintNumberWidth6(int value);
void Draw_PrintNumberWidth3Unk(int value);
void Draw_PrintRawText(u8 *text);
void Draw_SetColor(int color);
void Draw_SetTextDimmed(int dimmed);
void Draw_SetFontVariant(int variant);
int Draw_GetBlendColor(void);
void Draw_PrintSignedNumberWidth3(int value);
void Draw_EmitGlyph(int glyph, int mode);

void Draw_EmitWipeBarRect(int width, int height, int mode);

extern signed char g_DrawBaseX[];
extern s32 g_DrawBufferIndex;
/* g_DrawBlendColor and the halved blend Draw_BlendColor stores after it
 * (g_DrawScratchBuffer) as one pair; the wipe bars pick one by mode. */
extern u32 g_DrawBlendColorPair[];
extern GpuDrawEnabledFlag g_DrawEnabled;
extern short g_SeqElapsed;
extern volatile unsigned char g_SceneInitSubphase;

/* Battle status icons: per draw buffer, thirteen paged sprites (poison,
 * confusion and the other ailment symbols Draw_StatusIcons lists). */
extern RenderPagedSpritePacket g_StatusIconPackets[2][13];
/* Status icon floated over an entity by Draw_SetupSprites and raised two
 * pixels a frame by Draw_QueuePrimitive. */
extern RenderPagedSpritePacket g_StatusMarkerPackets[2];

#endif /* PE1_DRAW_STATE_H */
