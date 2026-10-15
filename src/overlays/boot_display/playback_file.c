#include "pe1/boot_display.h"
#include "pe1/cdrom.h"

/* Resolve the original XA path in the mounted image. There is no drive
 * negotiation, shell state or unbounded search retry on the host. */
s32 BootDisplay_InitPlaybackFile(void) {
    PeDiscFile file;
    u64 end;
    s64 bufferOffset=((s64)g_MainAssetLbaTbl[0]-g_BootDisplayDataLbaTbl)*2048+0x34800;
    if(!g_CdImage || !g_BootDisplayPlaybackFilePath ||
       !pe_disc_find(g_CdImage,g_BootDisplayPlaybackFilePath,&file))return -1;
    end=(u64)file.sector+(file.size>>11);
    if(file.directory || file.size<2048 || end>INT32_MAX ||
       !g_CdLoadAreaBase || bufferOffset<0 || bufferOffset+8*2048>BOOT_DISPLAY_WORK_SIZE)return -2;
    g_BootDisplayPlaybackFile=file;
    g_BootDisplayPlaybackStartPos=(s32)file.sector;
    g_BootDisplayPlaybackEndPos=(s32)end;
    g_BootDisplayPlaybackCurrentPos=g_BootDisplayPlaybackStartPos;
    g_BootDisplayPlaybackReadCount=0;
    g_BootDisplayPlaybackRingIndex=0;
    g_BootDisplayPlaybackAltCount=0;
    g_BootDisplayPlaybackSectorBuffer=(char *)g_CdLoadAreaBase+bufferOffset;
    return 0;
}

PeDiscFile g_BootDisplayPlaybackFile;
char *g_BootDisplayPlaybackFilePath,*g_BootDisplayPlaybackSectorBuffer;
s32 g_BootDisplayPlaybackStartPos,g_BootDisplayPlaybackEndPos,g_BootDisplayPlaybackCurrentPos;
s32 g_BootDisplayPlaybackReadCount;
u16 g_BootDisplayPlaybackRingIndex,g_BootDisplayPlaybackAltCount;
