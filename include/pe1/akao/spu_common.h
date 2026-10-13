#ifndef PE1_AKAO_SPU_COMMON_H
#define PE1_AKAO_SPU_COMMON_H

#include "pe1/psyq_spu_api.h"

/* SPU common attributes the sound driver hands to SpuSetCommonAttr:
 * Spu_InitVoiceState fills them at start-up and Seq_ApplyGlobalPitch
 * rewrites the CD volume and reverb. */
extern SpuCommonAttr g_AkaoSpuCommonAttr;

#endif
