#ifndef PE1_EXT_E19
#define PE1_EXT_E19

#include "common.h"
#include "pe1/battle.h"
#include "pe1/field_color.h"
#include "pe1/gte_short_vector.h"
#include "pe1/gte_types.h"
#include "pe1/room_orbit_trail.h"
#include "pe1/room_spark.h"

extern GteRotation D_8018F1CC;
extern int g_SceneE19BlastGlow;
extern GteRotation D_8018F210;
extern RenderColor D_8018F218;
extern void FieldFx_DrawPulledSprite(GteShortVector *, int, int, int,
                         int, int, int, int, int);
extern RenderColor D_8018F234;
extern GteShortVector g_SceneE19OrbitTrailCenter;
extern u8 g_SceneE19OrbitTrailColorTrack[];
extern void LoadAverageShort12(void *from, void *to, int weightFrom, int weightTo,
                          void *out);
extern void FieldFx_DrawGlowLine(void *, void *, void *, void *, int, int, int);
extern RenderColor D_8018F23C;
extern RenderColor D_8018F240;
extern int SceneE19_OrbitTrailParticleCallback(int mode, RoomOrbitTrailParticle *p);
extern RenderColor D_8018F244;
extern s16 g_SceneE19GlintRibbonJoints[3];
extern GteShortVector D_8018F248;
extern GteShortVector D_8018F250;
extern RenderColor D_8018F258;
extern RenderColor D_8018F25C;
extern RenderColor D_8018F260;
extern int SceneE19_GlintRibbonSparkCallback(int mode, RoomDampedSpark *spark);
extern int SceneE19_JitterSparkCallback(int mode, RoomJitterSpark *spark);
#endif /* PE1_EXT_E19 */
