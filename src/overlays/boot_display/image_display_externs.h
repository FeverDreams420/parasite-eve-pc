#ifndef PE1_EXT_IMAGE_DISPLAY
#define PE1_EXT_IMAGE_DISPLAY

#include "common.h"

extern void *g_BootDisplayCreditImages[];
extern u32 g_FieldPadBits;
extern u32 D_8009D1F4;
extern u32 g_BootDisplayFrameCounter;
extern char *g_BootDisplayTransferCursor;
extern u32 func_8010BF94(void *);
extern void DecDCTvlcSize2(u32);
extern s32 DecDCTvlc2(void *, void *, void *);
extern void DecDCTin(void *, s32);
extern void DecDCTout(void *, s32);
extern s32 DecDCToutSync(s32);
#endif /* PE1_EXT_IMAGE_DISPLAY */
