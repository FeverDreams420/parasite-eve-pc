#ifndef PE1_EXT_E18_EMITTERS
#define PE1_EXT_E18_EMITTERS

#include "common.h"
#include "pe1/scene_e18_pulse.h"

extern u16 g_SceneE18ExpandingRingRemaining;
int SceneE18_PulseRingCallback(int mode, void *memory, intptr_t argument);
extern u32 g_SceneE18WaveFanColor0[], g_SceneE18WaveFanColor1[];
#include "pe1/fieldfx_task.h"
extern FieldFxTaskProgram g_SceneE18ShellProgram;
extern FieldFxTaskProgram g_SceneE18PulseProgram;
extern FieldFxTaskProgram g_SceneE18WaveProgram;
void SceneE18_ResetModule(void);
#endif /* PE1_EXT_E18_EMITTERS */
