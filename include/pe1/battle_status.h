#ifndef PE1_BATTLE_STATUS_H
#define PE1_BATTLE_STATUS_H

#include "pe1/battle.h"
#include "pe1/render_prim.h"

typedef struct BattleStatusLinePrim {
    u8 pad00[8];
    u16 x0;
    u16 y0;
    u16 x1;
    u16 y1;
} BattleStatusLinePrim;

typedef struct BattleStatusMarkerBody {
    u8 pad00[8];
    u16 x;
    u16 y;
    u8 pad0C[0x10];
} BattleStatusMarkerBody;

/* Texture-page command followed by a variable-size UI sprite. */
typedef RenderPagedSpritePacket BattleGaugePrim;
/* A gauge packet addressed from an untyped buffer by byte offset. */
typedef union BattleGaugePrimAddress {
    void *raw;
    BattleGaugePrim *prim;
    RenderSpritePacket *sprite;
    u32 word;
} BattleGaugePrimAddress;
/* A word that holds a status packet offset, then the packet's address. */
typedef union BattleSpriteWord {
    s32 value;
    u8 *bytes;
    RenderSpritePacket *sprite;
} BattleSpriteWord;

/* A HUD packet address as a pointer or a word: retail adds the scaled slot
 * and index offsets to the buffer address as integers, offsets first. */
typedef union BattleStatusPrimAddress {
    BattleStatusLinePrim *line;
    BattleGaugePrim *gauge;
    RenderSpritePacket *sprite;
    unsigned int *words;
    u8 *bytes;
    u8 **byteOrdering;
    u32 **ordering;
    u32 address;
} BattleStatusPrimAddress;

extern BattleStatusLinePrim g_BattleStatusBarTiles[6];
/* The same packets seen as the three shaded status-bar tiles of each draw
 * buffer (Battle_DrawHPBar). */
#define BATTLE_STATUS_TILES ((RenderColorTilePacket (*)[3])g_BattleStatusBarTiles)
extern BattleStatusMarkerBody g_BattleStatusValueMarkerSlots[2];
extern RenderPagedSpritePacket g_BattleStatusValueDigits[2][2];  /* ammo digits */
extern BattleGaugePrim g_BattleTargetHighlightSprites[2][3];
extern BattleGaugePrim g_BattleStatusPointerSprites[2];
/* Status symbols use the same texture-page and sprite packet layout.
 * Label sprite (u 0x68, v 0xF4, 24x4) Battle_DrawStatusOverlay mode 3 places
 * at the status bar before the status value; one per draw buffer. */
extern BattleGaugePrim g_BattleStatusValueLabelSprites[2];
extern BattleGaugePrim g_BattleAmmoLabelSprites[2];
extern RenderPagedSpritePacket g_BattleAmmoDigitSprites[2][4];  /* ammo count digits */

void AddPrim(unsigned int *orderingEntry, unsigned int *primitive);
void Battle_LayoutStatusPrimRow(int bottomY);
void Battle_DrawTargetHighlight(void);
void Battle_DrawActiveStatus(void);
void Battle_DrawEnemyHP(s16 maximum, s16 current);
void Draw_StatusIcons(void);
int Menu_GetItemContextFlag(void);

/* Active menu window origin (Menu_ActiveWindowMode): x at g_MenuActiveWindowXOffset and y
 * in the next halfword. The HUD reads it as one record, so the stores to the
 * packets make GCC reload it. */
typedef struct BattleHudOrigin {
    u16 x;
    u16 y;
} BattleHudOrigin;

extern u16 g_MenuActiveWindowXOffset;
#define g_HudOrigin (*(BattleHudOrigin *)&g_MenuActiveWindowXOffset)

/* Per draw buffer HUD packets (Battle_DrawHPBar fills them once). */
typedef struct BattleModeTilePacket {
    RenderDrawModePacket mode;
    RenderColorTilePacket tile;
} BattleModeTilePacket;

PE1_STATIC_ASSERT(sizeof(BattleModeTilePacket) == 0x18,
                  battle_mode_tile_packet_size);

extern BattleModeTilePacket g_BattleStatusBackgrounds[2];      /* status background */
/* The background tiles seen from the tile: each is followed by the mode
 * words of the next packet. Battle_DrawActiveStatus addresses them this way,
 * keeping the tile base in a register. */
typedef struct BattleTileSlot {
    RenderColorTilePacket tile;
    RenderDrawModePacket nextMode;
} BattleTileSlot;
extern BattleTileSlot g_BattleStatusBackgroundTiles[2];
extern RenderColorTilePacket g_BattleAtbTracks[2];     /* ATB gauge track */
extern RenderPagedSpritePacket g_BattleAtbLabels[2];   /* ATB label */
extern RenderPagedSpritePacket g_BattlePeLabels[2];   /* PE label */
extern RenderGouraudQuad g_BattleAtbGauges[2];         /* ATB gauge fill */
extern RenderGouraudQuad g_BattlePeGauges[2][2];      /* PE gauge fill, remainder */
extern RenderPagedSpritePacket g_BattleAtbMarkers[2];   /* ATB gauge marker */
/* Color a gauge quad: vertices 0 and 2 take the first color, 1 and 3 the
 * second (the PSY-Q setRGB0..setRGB3 sequence). A macro rather than an
 * inline function: the constants must stay shared with neighbouring stores. */
#define BATTLE_SET_GAUGE_GRADIENT(quad, r0_, g0_, b0_, r1_, g1_, b1_) \
    ((quad).r0 = (r0_), (quad).g0 = (g0_), (quad).b0 = (b0_),         \
     (quad).r1 = (r1_), (quad).g1 = (g1_), (quad).b1 = (b1_),         \
     (quad).r2 = (r0_), (quad).g2 = (g0_), (quad).b2 = (b0_),         \
     (quad).r3 = (r1_), (quad).g3 = (g1_), (quad).b3 = (b1_))

/* Color of a marker or label sprite (setRGB0). */
#define BATTLE_SET_SPRITE_COLOR(sprite, r_, g_, b_)                   \
    ((sprite).color.bytes.r = (r_), (sprite).color.bytes.g = (g_),    \
     (sprite).color.bytes.b = (b_))

/* HP digit packets: current HP has four per draw buffer, maximum HP five
 * (digits and slash). Battle_DrawEnemyHP addresses the sprites from the
 * sprite, each followed by the texture-page command of the next packet (the
 * BattleTileSlot view), so the sprite arrays keep their own symbols. */
typedef struct BattleDigitSlot {
    RenderSpritePacket sprite;
    RenderTexturePagePacket nextPage;
} BattleDigitSlot;
extern RenderPagedSpritePacket g_BattleHpDigits[2][4], g_BattleMaxHpDigits[2][5];
extern BattleDigitSlot g_BattleHpDigitSlots[2][4], g_BattleMaxHpDigitSlots[2][5];
/* HP label packets. */
extern RenderPagedSpritePacket g_BattleHpLabels[2];
/* Further HUD sprite packets Battle_DrawHPBar initializes. */
extern RenderPagedSpritePacket g_BattleRemainingActionSprites[2][10];
extern RenderPagedSpritePacket g_BattleStatusValueMarkers[2];
extern RenderPagedSpritePacket g_BattleStatusSymbolSprites[2];
/* The sprites of a paged-sprite array seen from the first sprite: each is
 * followed by the texture-page command of the next packet. The view keeps
 * the sprite base (array + 8) in its own register, as Battle_DrawHPBar's
 * loops do. */
#define BATTLE_SPRITE_SLOTS(packets, count) \
    ((BattleDigitSlot (*)[count])&(packets)[0][0].sprite)
/* Gray levels of the three status-bar tiles (Battle_DrawHPBar). */
typedef struct BattleHudShades {
    s8 values[3];
} BattleHudShades;
extern BattleHudShades g_BattleStatusShades[]; /* one record; unsized keeps it off $gp */
/* Status-panel digit sprites: two draw buffers, ten panels of five digits. */
extern RenderPagedSpritePacket g_BattlePanelDigits[2][10][5];
/* End-of-battle panel quads, one per draw buffer. */
extern RenderTexturedQuad g_BattleEndPanels[2];
/* Corner coordinates of a quad (the PSY-Q setXY4 sequence). */
#define BATTLE_SET_QUAD_XY(quad, x0_, y0_, x1_, y1_, x2_, y2_, x3_, y3_)  \
    ((quad).x0 = (x0_), (quad).y0 = (y0_), (quad).x1 = (x1_),           \
     (quad).y1 = (y1_), (quad).x2 = (x2_), (quad).y2 = (y2_),           \
     (quad).x3 = (x3_), (quad).y3 = (y3_))
void Battle_DrawStatusValue(int value, int yOffset);
/* Returns the highest digit index (number of rendered digits minus one). */
int Battle_DrawDecimalNumber(void *buffer, s16 x, s16 y, s16 value, s16 mode);

PE1_STATIC_ASSERT(sizeof(BattleStatusLinePrim) == 0x10,
                  battle_status_line_prim_size);
PE1_STATIC_ASSERT(sizeof(BattleStatusMarkerBody) == 0x1C,
                  battle_status_marker_body_stride);
PE1_STATIC_ASSERT(PE1_OFFSETOF(BattleGaugePrim, sprite) == 8,
                  battle_gauge_sprite_offset);
PE1_STATIC_ASSERT(sizeof(BattleGaugePrim) == 0x1C,
                  battle_gauge_prim_stride);

extern s32 g_BattleStatusPanelSlot;
/* The MISS sprite: the sprite of g_BattlePanelDigits[slot][panel][0], which
 * Battle_DrawStatusPanel draws for a negative value. Views of the record,
 * its colour and its texture coordinates: retail addresses each byte with
 * its own relocation, and the indexed field form changes the function. */
extern RenderSpritePacket g_BattleMissSprite[];
extern u8 g_BattleMissSpriteR[], g_BattleMissSpriteG[], g_BattleMissSpriteB[];
extern u8 g_BattleMissSpriteU[], g_BattleMissSpriteV[];
void Battle_DrawStatusPanel(int mode, BattleStatusPanel *panel);


/* Target pointer drawn by Battle_BuildStatusPrimHeader: a flat-shaded
 * triangle (POLY_F3 layout) plus two flat lines per draw slot that join the
 * pointer to the status panel. */
typedef struct BattleStatusPointerPrim {
    u32 tag;
    u8 r, g, b, code;
    s16 x0, y0, x1, y1, x2, y2;
} BattleStatusPointerPrim;

extern RenderLinePacket g_BattleStatusPointerLines[2][2];
extern BattleStatusPointerPrim g_BattleStatusPointerTriangles[2];
/* Frame counter; drives the pointer's rotation. */
extern u32 g_FrameRngCounter;

void RotTrans(const GteShortVector *v, GteVector *out, s32 *flag);

PE1_STATIC_ASSERT(sizeof(BattleStatusPointerPrim) == 0x14,
                  battle_status_pointer_prim_stride);

#endif
