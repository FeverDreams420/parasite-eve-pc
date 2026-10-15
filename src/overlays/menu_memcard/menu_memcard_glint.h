#ifndef MENU_MEMCARD_GLINT_H
#define MENU_MEMCARD_GLINT_H

#include "pe1/akao.h"
#include "pe1/field_effect_pool.h"
#include "pe1/field_history_trail.h"
#include "pe1/field_sprite_draw.h"
#include "pe1/fieldfx_calls.h"
#include "pe1/gte.h"
#include "pe1/room_spark.h"

#include "menu_memcard_offset.h"

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

extern RenderColor g_MenuCardGlintBurstFlashColor, g_MenuCardSpinRingColorA, g_MenuCardSpinRingColorB;
extern GteRotation g_MenuCardEffectSpinRotation, g_MenuCardEffectTiltRotation;
extern RenderColor g_MenuCardRingBurstGlowColor, g_MenuCardRingBurstRingColor, g_MenuCardRingBurstBandColor;
extern u8 g_MenuCardDriftGlowColorTrack[];
extern u8 g_MenuCardRisingEmberColorTrack[];
extern u8 g_MenuCardSpinRingColorTrack[];
extern GteShortVector g_MemcardGlintOrigin;
extern s16 g_MenuCardCrossFlashRaised;
extern u16 g_BattleCrossFlashStop;

/* Cross flash: two beams between the actor's hands, each drawn as two
 * mirrored trails. */
typedef struct MemcardCrossFlash {
    s16 attachA;                  /* 0x00 */
    s16 attachB;                  /* 0x02 */
    s16 reserved04;
    u16 timer;                    /* 0x06 */
    FieldTrailPair trailA[9];              /* 0x08 */
    FieldTrailPair trailB[9];              /* 0x98 */
    FieldTrailPair trailC[11];              /* 0x128 */
    FieldTrailPair trailD[11];              /* 0x1D8 */
} MemcardCrossFlash;

PE1_STATIC_ASSERT(sizeof(MemcardCrossFlash) == 0x288, memcard_cross_flash_state_size);

extern MemcardJointOffset g_MenuCardCrossFlashTipAOffset, g_MenuCardCrossFlashTipBOffset, g_MenuCardCrossFlashCenterOffset;
extern RenderColor g_MenuCardCrossFlashInnerColor, g_MenuCardCrossFlashOuterColor, g_MenuCardCrossFlashEdgeColor;
extern u8 g_MenuCardGlintBurstColorTrack[];

int MenuCard_FadingGlintParticle(int mode, void *state, intptr_t argument);
int MenuCard_GlintBurstController(int mode, void *state, intptr_t argument);
int MenuCard_SpinRingParticle(int mode, void *state, intptr_t argument);
int MenuCard_SpinBurstController(int mode, void *state, intptr_t argument);
int MenuCard_DriftGlowParticle(int mode, void *state, intptr_t argument);
int MenuCard_RisingEmberParticle(int mode, void *state, intptr_t argument);
int MenuCard_RingBurstController(int mode, void *state, intptr_t argument);

GteShortVector *MenuCard_GetGlintOriginA(void);
GteShortVector *MenuCard_GetGlintOriginB(void);
GteShortVector *MenuCard_GetGlintOriginC(void);
GteShortVector *MenuCard_GetGlintOriginD(void);
GteShortVector *MenuCard_GetGlintOriginE(void);
GteShortVector *MenuCard_GetGlintOriginF(void);
int MenuCard_ConsumeCrossFlashFlag(int mode);
int MenuCard_CrossFlashController(int mode, void *state, intptr_t argument);

#endif
