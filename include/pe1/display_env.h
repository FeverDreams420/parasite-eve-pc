#ifndef PE1_DISPLAY_ENV_H
#define PE1_DISPLAY_ENV_H

#include "common.h"
#include "pe1/psyq_gpu.h"

/* Double-buffered display and draw environments, indexed by the active
 * draw slot (g_ActiveDrawSlot). Boot fills them, the frame loop puts them, and
 * the memcard video player saves, retunes and restores them. */
extern DISPENV g_RenderDispEnvArray[2];
extern DRAWENV g_RenderDrawEnvArray[2];

#endif /* PE1_DISPLAY_ENV_H */
