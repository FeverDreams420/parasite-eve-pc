#ifndef SCENE_E22_EMBER_DATA_H
#define SCENE_E22_EMBER_DATA_H

#include "pe1/scene_e22_ember_burst.h"

/* scene_e22's own templates, colours, particle tables and particle
 * callbacks for the ember, link beam, swirl ring, ember column, twist model
 * and sweep bolt controllers. scene_e19 reuses the controller layouts from
 * pe1/scene_e22_ember_burst.h with its own data at these addresses. */
extern RenderColor D_8018F208;
extern RenderColor D_8018F20C;
extern RenderColor D_8018F204;
extern u8 g_SceneE22EmberColumnColorTrack[];
extern int SceneE22_EmberDriftParticleCallback(int mode, RoomOrbitTrailParticle *p);
extern SceneE22LinkJoints g_SceneE22LinkBeamJoints[];
extern GteShortVector D_8018F1D8;
extern GteShortVector D_8018F1E0;
extern RenderColor D_8018F1E8;
extern RenderColor D_8018F1EC;
extern RenderColor D_8018F1F0;
extern int SceneE22_LinkBeamSparkCallback(int mode, RoomOrbitTrailParticle *p);
extern GteRotation D_8018F1F4;
extern RenderColor D_8018F214;
extern u8 D_80199434[];
extern int SceneE22_SwirlRingParticleCallback(int mode, RoomOrbitTrailParticle *p);
extern GteRotation D_8018F1FC;
extern u8 g_SceneE22EmberDriftColorTrack[];
extern u8 D_80199190[];
extern RenderColor D_8018F210;
extern int SceneE22_EmberColumnParticleCallback(int mode, RoomOrbitTrailParticle *p);
extern void *g_SceneE22BoltModel;
extern RenderColor D_8018F218;
extern int SceneE22_TwistTrailCallback(int mode, RoomOrbitTrailParticle *p);
extern RenderColor D_8018F224;
extern s16 D_80199504;
extern int SceneE22_TwistBeamCallback(int mode, RoomOrbitTrailParticle *p);

#endif /* SCENE_E22_EMBER_DATA_H */
