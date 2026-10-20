#ifndef PE1_EXT_E08
#define PE1_EXT_E08

#include "common.h"
#include "pe1/battle.h"
#include "pe1/field_actor.h"
#include "pe1/field_glow_sprite.h"
#include "pe1/room_fx.h"

typedef struct {
    char bytes[4];
} SceneE08FourBytes;

extern FieldGlowSprite g_SceneE08ParticleClusterSprite;
extern FieldGlowSprite g_SceneE08GlowRingsSprite;
extern RoomFxSeed8 g_SceneE08GlowRingsRotationSeed;
extern u8 g_SceneE08GlowRingsGlowRamp[], g_SceneE08GlowRingsRingRamp[];
extern s16 g_SceneE08ParticleClusterSpriteDepth;
extern FieldGlowSprite g_SceneE08GlowSpritesSprite, g_SceneE08RingSpritesFlashSprite, g_SceneE08RingSpritesGlowSprite, g_SceneE08PlayerOrbPointSprite;
extern void FieldFx_SetScriptContext(void *);
extern s16 g_SceneE08GlowSpritesSpriteDepth;
extern u8 g_SceneE08RingSpritesRing0Mode[];
extern u8 g_SceneE08RingSpritesSource[];
extern u8 g_SceneE08RingSpritesRing0Color1R, g_SceneE08RingSpritesRing0Color1G, g_SceneE08RingSpritesRing0Color1B;
extern u8 g_SceneE08RingSpritesRing0Color0R, g_SceneE08RingSpritesRing0Color0G, g_SceneE08RingSpritesRing0Color0B;
extern u16 g_SceneE08RingSpritesRing0Offset, g_SceneE08RingSpritesRing0Intensity, g_SceneE08RingSpritesRing0Extent0, g_SceneE08RingSpritesRing0Extent1;
extern struct FieldActor *g_PlayerEntity;
extern int g_SceneE08CentreX, g_SceneE08CentreZ;
extern char *g_SceneE08GlowModel;
extern u8 g_SceneE08GlowModelUnusedSpriteR, g_SceneE08GlowModelUnusedSpriteG, g_SceneE08GlowModelUnusedSpriteB, g_SceneE08GlowModelUnusedSpriteCell, g_SceneE08GlowModelUnusedSpriteClut, g_SceneE08GlowModelUnusedSpriteFlip;
extern s16 g_SceneE08GlowModelUnusedSpriteOffset, g_SceneE08GlowModelUnusedSpriteDepth;
extern u8 g_SceneE08CentreGlowSprite[];
extern u8 g_SceneE08CentreGlowSpriteG, g_SceneE08CentreGlowSpriteB, g_SceneE08CentreGlowSpriteCell, g_SceneE08CentreGlowSpriteClut, g_SceneE08CentreGlowSpriteFlip;
extern s16 g_SceneE08CentreGlowSpriteOffset, g_SceneE08CentreGlowSpriteDepth;
extern FieldGlowSprite g_SceneE08GlowModelHaloSprite;
#include "pe1/player_entity.h"
extern u8 g_SceneE08RadialRibbonsRamp[];
extern u16 g_SceneE08RingEffectRing0Mode;
extern u8 g_SceneE08RingEffectSource[];
extern FieldRingGeometry g_SceneE08RingEffectRings[3];
extern s32 g_SceneE08CentreX, g_SceneE08CentreZ;
extern u8 g_SceneE08SpiralRibbonsRamp[];
extern SceneE08FourBytes g_SceneE08ScreenFadeColor;
extern u8 g_SceneE08OrbitParticlesRamp[];
extern int rand(void);
extern FieldGlowSprite g_SceneE08LayeredUnusedSprite, g_SceneE08SpinningGlowSprite, g_SceneE08TrailGlowFloorSprite;
extern u8 g_SceneE08LayeredSpriteR, g_SceneE08LayeredSpriteG, g_SceneE08LayeredSpriteB, g_SceneE08LayeredSpriteCell, g_SceneE08LayeredSpriteClut, g_SceneE08LayeredSpriteFlip;
extern s16 g_SceneE08LayeredSpriteOffset, g_SceneE08LayeredSpriteDepth;
extern s16 g_SceneE08TrailGlowSpriteOffset;
extern volatile u8 g_SceneE08TrailGlowSpriteClut;
extern u8 g_SceneE08TrailGlowSpriteCell;
extern s16 g_SceneE08TrailGlowSpriteDepth;
extern u8 g_SceneE08TrailGlowSpriteR;
extern u8 g_SceneE08TrailGlowSpriteG;
extern u8 g_SceneE08TrailGlowSpriteB;
extern u8 g_SceneE08TrailGlowSpriteFlip;
extern s16 g_SceneE08LayeredUnusedOffset;
extern s16 g_SceneE08LayeredUnusedDepth;
extern u8 g_SceneE08LayeredUnusedColor0R;
extern u8 g_SceneE08LayeredUnusedColor0G;
extern u8 g_SceneE08LayeredUnusedColor0B;
extern u8 g_SceneE08LayeredUnusedColor1R;
extern u8 g_SceneE08LayeredUnusedColor1G;
extern u8 g_SceneE08LayeredUnusedColor1B;
extern u8 g_SceneE08LayeredUnusedColor2R;
extern u8 g_SceneE08LayeredUnusedColor2G;
extern u8 g_SceneE08LayeredUnusedColor2B;
extern u8 g_SceneE08LayeredUnusedColor3R;
extern u8 g_SceneE08LayeredUnusedColor3G;
extern u8 g_SceneE08LayeredUnusedColor3B;
extern RoomFxVec4 g_SceneE08LayeredSpriteScale;
extern u8 g_SceneE08SpinningGlowRamp;
extern s16 g_SceneE08ArmGlowSpriteDepth;
#endif /* PE1_EXT_E08 */
