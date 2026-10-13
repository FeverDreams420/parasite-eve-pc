#ifndef PE1_EXT_E19
#define PE1_EXT_E19

#include "common.h"
#include "pe1/battle.h"
#include "pe1/field_color.h"
#include "pe1/gte_short_vector.h"
#include "pe1/gte_types.h"
#include "pe1/room_orbit_trail.h"
#include "pe1/room_spark.h"

extern GteRotation g_SceneE19ZeroSpin;
extern int g_SceneE19BlastGlow;
extern GteRotation g_SceneE19RollTilt;
extern RenderColor g_SceneE19HomingParticleColor;
extern void FieldFx_DrawPulledSprite(GteShortVector *, int, int, int,
                         int, int, int, int, int);
extern RenderColor g_SceneE19EmberParticleColor;
extern GteShortVector g_SceneE19OrbitTrailCenter;
extern u8 g_SceneE19OrbitTrailColorTrack[];
extern void LoadAverageShort12(void *from, void *to, int weightFrom, int weightTo,
                          void *out);
extern void FieldFx_DrawGlowLine(void *, void *, void *, void *, int, int, int);
extern RenderColor g_SceneE19OrbitTrailBurstColor;
extern RenderColor g_SceneE19OrbitTrailBurstRingColor;
extern int SceneE19_OrbitTrailParticleCallback(int mode, RoomOrbitTrailParticle *p);
extern RenderColor g_SceneE19GlintRibbonSparkColor;
extern s16 g_SceneE19GlintRibbonJoints[3];
extern GteShortVector g_SceneE19JointOrigin;
extern GteShortVector g_SceneE19JointAnchor;
extern RenderColor g_SceneE19GlintRibbonHeadColor;
extern RenderColor g_SceneE19GlintRibbonMidColor;
extern RenderColor g_SceneE19GlintRibbonTailColor;
extern int SceneE19_GlintRibbonSparkCallback(int mode, RoomDampedSpark *spark);
extern int SceneE19_JitterSparkCallback(int mode, RoomJitterSpark *spark);
#endif /* PE1_EXT_E19 */
