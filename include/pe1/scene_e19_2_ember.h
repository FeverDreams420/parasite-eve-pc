#ifndef PE1_SCENE_E19_2_EMBER_H
#define PE1_SCENE_E19_2_EMBER_H

#include "pe1/scene_e22_ember_burst.h"

/* Ember family (scene_e19_2): the scene_e22 ember controller layout with
 * this overlay's spin templates, colour seeds and colour track. */
extern GteRotation g_SceneE19ZeroSpin;
extern GteRotation g_SceneE19FlatTilt;
extern RenderColor g_SceneE19EmberParticleColor;
extern RenderColor g_SceneE19EmberFlareColor;
extern u8 g_SceneE19EmberFlareColorTrack[];

extern int SceneE19_EmberFlareParticleCallback(int mode, void *memory, intptr_t argument);

#endif
