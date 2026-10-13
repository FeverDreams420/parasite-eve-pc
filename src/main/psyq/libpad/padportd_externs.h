#ifndef PSYQ_LIBPAD_PADPORTD_EXTERNS_H
#define PSYQ_LIBPAD_PADPORTD_EXTERNS_H

#include "common.h"
#include "pe1/card_obj.h"
#include "pe1/memcard_state.h"

extern CardObj D_800A5B70[];
extern u8 g_MemCardObjResponseBuffers[];
extern u8 g_MemCardObjStatusBuffers[];
extern void (*g_PadPortActuatorMapFn)(CardObj *);
extern int (*g_CardGetChannelIdFn)(CardObj *);
extern void (*g_CardSwapByteFieldFn)(CardObj *);
extern int g_PadActivePortIndex;
extern int g_MemCardCommandStep;
extern int g_MemCardPort2Present;
extern int g_MemCardChannelCounts[];
extern MemCardSioRegisters *g_CardSioRegsPtr;
extern int g_PadActuatorCostTotal;

#endif /* PSYQ_LIBPAD_PADPORTD_EXTERNS_H */
