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
extern FieldGlowSprite g_SceneE08GlowSpritesSprite, g_SceneE08RingSpritesFlashSprite, g_SceneE08RingSpritesGlowSprite, g_SceneE08PlayerOrbPointSprite;
extern void FieldFx_SetScriptContext(void *);
extern u8 g_SceneE08RingSpritesSource[];
extern struct FieldActor *g_PlayerEntity;
extern char *g_SceneE08GlowModel;
extern FieldGlowSprite g_SceneE08GlowModelHaloSprite;
#include "pe1/player_entity.h"
extern u8 g_SceneE08RadialRibbonsRamp[];
extern u8 g_SceneE08RingEffectSource[];
extern FieldRingGeometry g_SceneE08RingEffectRings[3];
extern u8 g_SceneE08SpiralRibbonsRamp[];
extern SceneE08FourBytes g_SceneE08ScreenFadeColor;
extern u8 g_SceneE08OrbitParticlesRamp[];
extern int rand(void);
extern FieldGlowSprite g_SceneE08LayeredUnusedSprite, g_SceneE08TrailGlowFloorSprite;
extern RoomFxVec4 g_SceneE08LayeredSpriteScale;
extern u8 g_SceneE08SpinningGlowRamp[16];
/* Original overlapping sprite tails store the scene's X/Z origins. */
typedef union SceneE08PositionedSprite {
    FieldGlowSprite sprite;
    struct { u8 prefix[12]; s32 origin; } anchor;
} SceneE08PositionedSprite;
typedef struct SceneE08LayeredColors {
    RenderColor colors[4];
    s16 offset, depth;
    u8 reserved[4];
} SceneE08LayeredColors;
extern SceneE08PositionedSprite g_SceneE08LayeredSpriteStorage, g_SceneE08SpinningGlowStorage;
extern SceneE08LayeredColors g_SceneE08LayeredUnusedColors;
extern FieldGlowSprite g_SceneE08GlowModelUnusedSpriteRecord, g_SceneE08CentreGlowSpriteRecord, g_SceneE08TrailGlowSpriteRecord;
PE1_STATIC_ASSERT(sizeof(SceneE08PositionedSprite)==16, scene_e08_positioned_sprite_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(SceneE08PositionedSprite, anchor.origin)==12, scene_e08_origin_alias);
PE1_STATIC_ASSERT(sizeof(SceneE08LayeredColors)==24, scene_e08_layered_colors_size);

#define g_SceneE08GlowModelUnusedSpriteR (g_SceneE08GlowModelUnusedSpriteRecord.r)
#define g_SceneE08GlowModelUnusedSpriteG (g_SceneE08GlowModelUnusedSpriteRecord.g)
#define g_SceneE08GlowModelUnusedSpriteB (g_SceneE08GlowModelUnusedSpriteRecord.b)
#define g_SceneE08GlowModelUnusedSpriteCell (g_SceneE08GlowModelUnusedSpriteRecord.cell)
#define g_SceneE08GlowModelUnusedSpriteClut (g_SceneE08GlowModelUnusedSpriteRecord.clut)
#define g_SceneE08GlowModelUnusedSpriteFlip (g_SceneE08GlowModelUnusedSpriteRecord.flip)
#define g_SceneE08GlowModelUnusedSpriteOffset (g_SceneE08GlowModelUnusedSpriteRecord.offset)
#define g_SceneE08GlowModelUnusedSpriteDepth (*(s16 *)&g_SceneE08GlowModelUnusedSpriteRecord.depth)
#define g_SceneE08CentreGlowSpriteR (g_SceneE08CentreGlowSpriteRecord.r)
#define g_SceneE08CentreGlowSpriteG (g_SceneE08CentreGlowSpriteRecord.g)
#define g_SceneE08CentreGlowSpriteB (g_SceneE08CentreGlowSpriteRecord.b)
#define g_SceneE08CentreGlowSpriteCell (g_SceneE08CentreGlowSpriteRecord.cell)
#define g_SceneE08CentreGlowSpriteClut (g_SceneE08CentreGlowSpriteRecord.clut)
#define g_SceneE08CentreGlowSpriteFlip (g_SceneE08CentreGlowSpriteRecord.flip)
#define g_SceneE08CentreGlowSpriteOffset (g_SceneE08CentreGlowSpriteRecord.offset)
#define g_SceneE08CentreGlowSpriteDepth (*(s16 *)&g_SceneE08CentreGlowSpriteRecord.depth)
#define g_SceneE08LayeredSpriteR (g_SceneE08LayeredSpriteStorage.sprite.r)
#define g_SceneE08LayeredSpriteG (g_SceneE08LayeredSpriteStorage.sprite.g)
#define g_SceneE08LayeredSpriteB (g_SceneE08LayeredSpriteStorage.sprite.b)
#define g_SceneE08LayeredSpriteCell (g_SceneE08LayeredSpriteStorage.sprite.cell)
#define g_SceneE08LayeredSpriteClut (g_SceneE08LayeredSpriteStorage.sprite.clut)
#define g_SceneE08LayeredSpriteFlip (g_SceneE08LayeredSpriteStorage.sprite.flip)
#define g_SceneE08LayeredSpriteOffset (g_SceneE08LayeredSpriteStorage.sprite.offset)
#define g_SceneE08LayeredSpriteDepth (*(s16 *)&g_SceneE08LayeredSpriteStorage.sprite.depth)
#define g_SceneE08TrailGlowSpriteR (g_SceneE08TrailGlowSpriteRecord.r)
#define g_SceneE08TrailGlowSpriteG (g_SceneE08TrailGlowSpriteRecord.g)
#define g_SceneE08TrailGlowSpriteB (g_SceneE08TrailGlowSpriteRecord.b)
#define g_SceneE08TrailGlowSpriteCell (g_SceneE08TrailGlowSpriteRecord.cell)
#define g_SceneE08TrailGlowSpriteClut (*(volatile u8 *)&g_SceneE08TrailGlowSpriteRecord.clut)
#define g_SceneE08TrailGlowSpriteFlip (g_SceneE08TrailGlowSpriteRecord.flip)
#define g_SceneE08TrailGlowSpriteOffset (g_SceneE08TrailGlowSpriteRecord.offset)
#define g_SceneE08TrailGlowSpriteDepth (*(s16 *)&g_SceneE08TrailGlowSpriteRecord.depth)
#define g_SceneE08SpinningGlowSprite (g_SceneE08SpinningGlowStorage.sprite)
#define g_SceneE08CentreX (g_SceneE08LayeredSpriteStorage.anchor.origin)
#define g_SceneE08CentreZ (g_SceneE08SpinningGlowStorage.anchor.origin)
#define g_SceneE08CentreGlowSprite ((u8 *)&g_SceneE08CentreGlowSpriteRecord)
#define g_SceneE08ParticleClusterSpriteDepth (*(s16 *)&g_SceneE08ParticleClusterSprite.depth)
#define g_SceneE08GlowSpritesSpriteDepth (*(s16 *)&g_SceneE08GlowSpritesSprite.depth)
#define g_SceneE08ArmGlowSpriteDepth (*(s16 *)&g_SceneE08ArmGlowSprite.depth)
#define g_SceneE08LayeredUnusedColor0R (g_SceneE08LayeredUnusedColors.colors[0].r)
#define g_SceneE08LayeredUnusedColor0G (g_SceneE08LayeredUnusedColors.colors[0].g)
#define g_SceneE08LayeredUnusedColor0B (g_SceneE08LayeredUnusedColors.colors[0].b)
#define g_SceneE08LayeredUnusedColor1R (g_SceneE08LayeredUnusedColors.colors[1].r)
#define g_SceneE08LayeredUnusedColor1G (g_SceneE08LayeredUnusedColors.colors[1].g)
#define g_SceneE08LayeredUnusedColor1B (g_SceneE08LayeredUnusedColors.colors[1].b)
#define g_SceneE08LayeredUnusedColor2R (g_SceneE08LayeredUnusedColors.colors[2].r)
#define g_SceneE08LayeredUnusedColor2G (g_SceneE08LayeredUnusedColors.colors[2].g)
#define g_SceneE08LayeredUnusedColor2B (g_SceneE08LayeredUnusedColors.colors[2].b)
#define g_SceneE08LayeredUnusedColor3R (g_SceneE08LayeredUnusedColors.colors[3].r)
#define g_SceneE08LayeredUnusedColor3G (g_SceneE08LayeredUnusedColors.colors[3].g)
#define g_SceneE08LayeredUnusedColor3B (g_SceneE08LayeredUnusedColors.colors[3].b)
#define g_SceneE08LayeredUnusedOffset (g_SceneE08LayeredUnusedColors.offset)
#define g_SceneE08LayeredUnusedDepth (g_SceneE08LayeredUnusedColors.depth)
#define g_SceneE08RingSpritesRing0Color0R (g_SceneE08RingSpritesRings[0].color0[0])
#define g_SceneE08RingSpritesRing0Color0G (g_SceneE08RingSpritesRings[0].color0[1])
#define g_SceneE08RingSpritesRing0Color0B (g_SceneE08RingSpritesRings[0].color0[2])
#define g_SceneE08RingSpritesRing0Color1R (g_SceneE08RingSpritesRings[0].color1[0])
#define g_SceneE08RingSpritesRing0Color1G (g_SceneE08RingSpritesRings[0].color1[1])
#define g_SceneE08RingSpritesRing0Color1B (g_SceneE08RingSpritesRings[0].color1[2])
#define g_SceneE08RingSpritesRing0Mode (g_SceneE08RingSpritesRings[0].mode)
#define g_SceneE08RingSpritesRing0Extent0 (g_SceneE08RingSpritesRings[0].extent0)
#define g_SceneE08RingSpritesRing0Extent1 (g_SceneE08RingSpritesRings[0].extent1)
#define g_SceneE08RingSpritesRing0Offset (g_SceneE08RingSpritesRings[0].offset)
#define g_SceneE08RingSpritesRing0Intensity (g_SceneE08RingSpritesRings[0].intensity)
#define g_SceneE08RingEffectRing0Mode (g_SceneE08RingEffectRings[0].mode)

#endif /* PE1_EXT_E08 */
