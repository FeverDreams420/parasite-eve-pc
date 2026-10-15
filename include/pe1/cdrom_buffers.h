#ifndef PE1_CDROM_BUFFERS_H
#define PE1_CDROM_BUFFERS_H

#include "common.h"
#include "pe1/pe_image.h"

extern u8 *g_StrFileDirBuffer;
/* g_StrFileDirLba aliases the resident PE.IMG range at sector-table slot 70. */
extern u8 *g_MdecOverlayLoadBase;
/* Boot-display data and its original image/decode scratch space. */
#define BOOT_DISPLAY_WORK_SIZE 0x100000u
extern u8 *g_CdLoadAreaBase;

#endif
