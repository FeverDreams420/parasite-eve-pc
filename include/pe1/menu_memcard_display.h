#ifndef PE1_MENU_MEMCARD_DISPLAY_H
#define PE1_MENU_MEMCARD_DISPLAY_H

#include "common.h"
#include "pe1/display_env.h"
#include "pe1/psyq_gpu.h"

/* Video display mode of each copied player (2 = 320 wide, 3 = 480 wide). */
extern u8 g_MenuCardScreenVideoDecodeMode;
extern u8 g_MenuCardVideoDecodeMode;

/* Prim list the memory card progress widget flushes; 0x40 marks a write. */
extern int g_MemCardProgressPrimList;


#endif
