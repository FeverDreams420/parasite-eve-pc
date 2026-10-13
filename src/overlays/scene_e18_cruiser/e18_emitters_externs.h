#ifndef PE1_EXT_E18_EMITTERS
#define PE1_EXT_E18_EMITTERS

#include "common.h"
#include "pe1/scene_e18_pulse.h"

extern u16 g_SceneE18ExpandingRingRemaining;
int SceneE18_PulseRingCallback(int mode, SceneE18PulseRing *ring);
extern u32 g_SceneE18WaveFanColor0[], g_SceneE18WaveFanColor1[];
#endif /* PE1_EXT_E18_EMITTERS */
