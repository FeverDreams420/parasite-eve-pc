#ifndef PE1_EXT_IMAGE_DISPLAY
#define PE1_EXT_IMAGE_DISPLAY

#include "common.h"
#include "pe1/psyq_press.h"

extern void *g_BootDisplayCreditImages[];
extern u32 g_FieldPadBits;
extern u32 g_FieldPadPressed;
extern u32 g_BootDisplayFrameCounter;
extern char *g_BootDisplayTransferCursor;
extern u32 MdecDiag_ReadHalfword(void *);

extern void DecDCTin(void *, s32);
extern void DecDCTout(void *, s32);
extern s32 DecDCToutSync(s32);
#endif /* PE1_EXT_IMAGE_DISPLAY */
