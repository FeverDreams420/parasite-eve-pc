#ifndef MENU_MEMCARD_PULSE_H
#define MENU_MEMCARD_PULSE_H

#include "pe1/gte_types.h"
#include "pe1/psyq_gpu.h"
#include "pe1/render_object.h"

typedef struct MemcardPulseState {
    GteShortVector position;
    GteRotation rotation;
    s16 phase, timer;
} MemcardPulseState;
extern u8 g_MenuCardPulseColorTrack[];
extern RenderColor g_MemcardCosineColor;
extern RenderColor g_MemcardSineColor;

s32 MenuCard_PulseImage(int mode, void *state, intptr_t argument);
s32 MenuCard_CosineImage(int mode, void *state, intptr_t argument);
s32 MenuCard_SineRotatingImage(int mode, void *state, intptr_t argument);

#endif
