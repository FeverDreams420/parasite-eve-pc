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

extern FieldGlowSprite D_801994A8;
extern FieldGlowSprite g_SceneE08GlowRingsSprite;
extern RoomFxSeed8 g_SceneE08GlowRingsRotationSeed;
extern u8 g_SceneE08GlowRingsGlowRamp[], g_SceneE08GlowRingsRingRamp[];
extern s16 D_801994B2;
extern FieldGlowSprite D_801994B8, g_SceneE08RingSpritesFlashSprite, g_SceneE08RingSpritesGlowSprite, g_SceneE08PlayerOrbPointSprite;
extern void FieldFx_SetScriptContext(void *);
extern s16 D_801994C2;
extern u8 D_801995D4[];
extern u8 g_SceneE08RingSpritesSource[];
extern u8 D_801995D0, D_801995D1, D_801995D2;
extern u8 D_801995CC, D_801995CD, D_801995CE;
extern u16 D_801995DA, D_801995DC, D_801995D6, D_801995D8;
extern struct FieldActor *g_PlayerEntity;
extern int g_SceneE08CentreX, g_SceneE08CentreZ;
extern u8 *g_SceneE08GlowModel;
extern u8 D_80199680, D_80199681, D_80199682, D_80199684, D_80199685, D_80199686;
extern s16 D_80199688, D_8019968A;
extern u8 D_801996A0[];
extern u8 D_801996A1, D_801996A2, D_801996A4, D_801996A5, D_801996A6;
extern s16 D_801996A8, D_801996AA;
extern FieldGlowSprite D_801996B0;
extern struct BattleEntity *D_8009D254;
extern u8 g_SceneE08RadialRibbonsRamp[];
extern u16 D_80199604;
extern u8 g_SceneE08RingEffectSource[];
extern char g_SceneE08RingEffectRings[];
extern s32 g_SceneE08CentreX, g_SceneE08CentreZ;
extern u8 g_SceneE08SpiralRibbonsRamp[];
extern SceneE08FourBytes g_SceneE08ScreenFadeColor;
extern char g_SceneE08OrbitParticlesRamp[];
extern int rand(void);
extern FieldGlowSprite D_80199540, g_SceneE08SpinningGlowSprite, g_SceneE08TrailGlowFloorSprite;
extern u8 D_80199560, D_80199561, D_80199562, D_80199564, D_80199565, D_80199566;
extern s16 D_80199568, D_8019956A;
extern s16 D_80199558;
extern volatile u8 D_80199555;
extern u8 D_80199554;
extern s16 D_8019955A;
extern u8 D_80199550;
extern u8 D_80199551;
extern u8 D_80199552;
extern u8 D_80199556;
extern s16 D_801994A0;
extern s16 D_801994A2;
extern u8 D_80199490;
extern u8 D_80199491;
extern u8 D_80199492;
extern u8 D_80199494;
extern u8 D_80199495;
extern u8 D_80199496;
extern u8 D_80199498;
extern u8 D_80199499;
extern u8 D_8019949A;
extern u8 D_8019949C;
extern u8 D_8019949D;
extern u8 D_8019949E;
extern RoomFxVec4 g_SceneE08LayeredSpriteScale;
extern u8 g_SceneE08SpinningGlowRamp;
extern s16 D_801994D2;
#endif /* PE1_EXT_E08 */
