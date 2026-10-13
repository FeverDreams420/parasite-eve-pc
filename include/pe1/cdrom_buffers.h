#ifndef PE1_CDROM_BUFFERS_H
#define PE1_CDROM_BUFFERS_H

#include "common.h"

extern u8 *g_StrFileDirBuffer;
/* PE.IMG sector range (start, end) of the stream file directory. */
extern u16 g_StrFileDirLba[];
extern u8 *g_MdecOverlayLoadBase;
extern u8 *g_CdLoadAreaBase;

#endif
