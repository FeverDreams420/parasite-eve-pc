#ifndef MENU_MEMCARD_GLINT_H
#define MENU_MEMCARD_GLINT_H

#include "pe1/akao.h"
#include "pe1/field_effect_pool.h"
#include "pe1/field_sprite_draw.h"
#include "pe1/fieldfx_calls.h"
#include "pe1/gte.h"
#include "pe1/room_spark.h"

#include "menu_memcard_offset.h"

/* Scene object reached through the primary channel pool. */
typedef struct MemcardGlintAnchor {
    u8 reserved[0x268];
    s16 x, y, z;                  /* 0x268 */
} MemcardGlintAnchor;

typedef struct MemcardGlintBurst {
    s16 x, y, z;
} MemcardGlintBurst;

/* Spinning particle: a position (state 0) or the spin rates that drive
 * its rotation (states 1 and 2). */
typedef struct MemcardSpinParticle {
    s16 x, y, z, reserved06;      /* 0x00 */
    GteRotation rotation;         /* 0x08 */
    s16 state;                    /* 0x10 */
    u16 timer;                    /* 0x12 */
} MemcardSpinParticle;

typedef struct MemcardSpinBurst {
    s16 x, y, z, reserved06;      /* 0x00 */
    GteRotation rotation;         /* 0x08 */
} MemcardSpinBurst;

/* Ring burst controller: position plus a signed state and frame timer. */
typedef struct MemcardRingBurst {
    s16 x, y, z;                  /* 0x00 */
    s16 reserved06[5];
    s16 state;                    /* 0x10 */
    s16 timer;                    /* 0x12 */
} MemcardRingBurst;

typedef struct MemcardModelMatrix {
    s16 m[3][3];
    s16 reserved12;
    s32 t[3];                     /* 0x14 */
} MemcardModelMatrix;

typedef struct MemcardModelScale {
    s32 x, y, z, reserved0C;
} MemcardModelScale;

extern RenderColor g_MenuCardGlintBurstFlashColor, g_MenuCardSpinRingColorA, g_MenuCardSpinRingColorB;
extern GteRotation g_MenuCardEffectSpinRotation, g_MenuCardEffectTiltRotation;
extern RenderColor g_MenuCardRingBurstGlowColor, g_MenuCardRingBurstRingColor, g_MenuCardRingBurstBandColor;
extern u8 g_MenuCardDriftGlowColorTrack[];
extern u8 g_MenuCardRisingEmberColorTrack[];
extern u8 g_MenuCardSpinRingColorTrack[];
extern GteShortVector g_MemcardGlintOrigin;
extern s16 g_MenuCardCrossFlashRaised;
extern s16 g_BattleCrossFlashStop;

/* Cross flash: two beams between the actor's hands, each drawn as two
 * mirrored trails. */
typedef struct MemcardCrossFlash {
    s16 attachA;                  /* 0x00 */
    s16 attachB;                  /* 0x02 */
    s16 reserved04;
    u16 timer;                    /* 0x06 */
    u8 trailA[0x90];              /* 0x08 */
    u8 trailB[0x90];              /* 0x98 */
    u8 trailC[0xB0];              /* 0x128 */
    u8 trailD[0x90];              /* 0x1D8 */
} MemcardCrossFlash;

extern MemcardJointOffset g_MenuCardCrossFlashTipAOffset, g_MenuCardCrossFlashTipBOffset, g_MenuCardCrossFlashCenterOffset;
extern RenderColor g_MenuCardCrossFlashInnerColor, g_MenuCardCrossFlashOuterColor, g_MenuCardCrossFlashEdgeColor;
extern u8 g_MenuCardGlintBurstColorTrack[];

int MenuCard_FadingGlintParticle(int mode, RoomDampedSpark *spark);
int MenuCard_GlintBurstController(int mode, MemcardGlintBurst *burst);
int MenuCard_SpinRingParticle(int mode, MemcardSpinParticle *p);
int MenuCard_SpinBurstController(int mode, MemcardSpinBurst *burst);
int MenuCard_DriftGlowParticle(int mode, RoomDampedSpark *spark);
int MenuCard_RisingEmberParticle(int mode, RoomDampedSpark *spark);
int MenuCard_RingBurstController(int mode, MemcardRingBurst *burst);

GteShortVector *MenuCard_GetGlintOriginA(void);
GteShortVector *MenuCard_GetGlintOriginB(void);
GteShortVector *MenuCard_GetGlintOriginC(void);
GteShortVector *MenuCard_GetGlintOriginD(void);
GteShortVector *MenuCard_GetGlintOriginE(void);
GteShortVector *MenuCard_GetGlintOriginF(void);
int MenuCard_ConsumeCrossFlashFlag(int mode);
int MenuCard_CrossFlashController(int mode, MemcardCrossFlash *flash);

#endif
