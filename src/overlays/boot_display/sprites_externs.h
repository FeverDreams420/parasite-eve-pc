#ifndef PE1_EXT_SPRITES
#define PE1_EXT_SPRITES

#include "common.h"

extern s32 g_ActiveDrawSlot;
extern void *g_BootDisplayCreditImages[];
extern s32 g_BootDisplayImageBrightness;
extern u32 g_BootDisplayFrameCounter;
extern char *g_BootDisplayTransferCursor;
extern u32 MdecDiag_ReadHalfword(void *);
extern void DecDCTvlcSize2(u32);
extern s32 DecDCTvlc2(void *, void *, void *);
extern void DecDCTin(void *, s32);
extern void DecDCTout(void *, s32);
extern s32 DecDCToutSync(s32);
#endif /* PE1_EXT_SPRITES */
