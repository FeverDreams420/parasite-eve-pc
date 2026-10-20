#ifndef SCENE_E22_EMBER_DATA_H
#define SCENE_E22_EMBER_DATA_H

#include "pe1/scene_e22_ember_burst.h"

/* scene_e22's own templates, colours, particle tables and particle
 * callbacks for the ember, link beam, swirl ring, ember column, twist model
 * and sweep bolt controllers. scene_e19 reuses the controller layouts from
 * pe1/scene_e22_ember_burst.h with its own data at these addresses. */
extern RenderColor g_SceneE22EmberDriftRingColor;
extern RenderColor g_SceneE22EmberParticleColor;
extern RenderColor g_SceneE22EmberDriftRingColor;
extern RenderColor g_SceneE22EmberDriftHaloColor;
extern RenderColor g_SceneE22EmberParticleColor;
extern u8 g_SceneE22EmberColumnColorTrack[];
extern int SceneE22_EmberDriftParticleCallback(int mode, void *state, intptr_t argument);
extern SceneE22LinkJoints g_SceneE22LinkBeamJoints[];
extern GteShortVector g_SceneE22JointOrigin;
extern GteShortVector g_SceneE22JointAnchor;
extern RenderColor g_SceneE22LinkBeamTailColor;
extern RenderColor g_SceneE22LinkBeamHeadColor;
extern GteShortVector g_SceneE22JointOrigin;
extern GteShortVector g_SceneE22JointAnchor;
extern RenderColor g_SceneE22LinkBeamTailColor;
extern RenderColor g_SceneE22LinkBeamMidColor;
extern RenderColor g_SceneE22LinkBeamHeadColor;
extern int SceneE22_LinkBeamSparkCallback(int mode, void *state, intptr_t argument);
extern RenderColor g_SceneE22SwirlRingColor;
extern GteRotation g_SceneE22ZeroSpin;
extern RenderColor g_SceneE22SwirlRingColor;
extern u8 g_SceneE22SwirlRingParticleColorTrack[];
extern int SceneE22_SwirlRingParticleCallback(int mode, void *state, intptr_t argument);
extern GteRotation g_SceneE22FlatTilt;
extern u8 g_SceneE22EmberDriftColorTrack[];
extern RenderColor g_SceneE22EmberColumnColor;
extern u8 g_SceneE22LinkBeamSparkColorTrack[];
extern RenderColor g_SceneE22EmberColumnColor;
extern int SceneE22_EmberColumnParticleCallback(int mode, void *state, intptr_t argument);
extern void *g_SceneE22BoltModel;
extern RenderColor g_SceneE22BoltColor;
extern int SceneE22_TwistTrailCallback(int mode, void *state, intptr_t argument);
extern RenderColor g_SceneE22SweepBoltHaloColor;
extern s16 g_SceneE22TwistBeamStop;
extern int SceneE22_TwistBeamCallback(int mode, void *state, intptr_t argument);

#endif /* SCENE_E22_EMBER_DATA_H */
