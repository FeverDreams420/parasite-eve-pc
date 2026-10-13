#include "pe1/boot_display.h"
#include "pe1/psyq_ds.h"
#include "pe1/psyq_ds_queue.h"

s32 BootDisplay_InitPlaybackFile(void) {
    DslFILE file;
    s32 found;
    /* Preserve separate zero/-1 retries and the ready-constant reload.
     * Pins and empty barriers are tracked in crutch debt. */
    s32 ready;
    s32 failure;
    volatile s32 *start;
    DsInit();
    DsSetDebug(0);
    for (;;) {
        ready = 1;
        if (DsSystemStatus() != ready || DsQueueLen() != 0)
            return VSync(0);
        failure = 0;
        found = (s32)DsSearchFile(&file, g_BootDisplayPlaybackFilePath);
        if (found == failure) continue;
        failure = -1;
        if (found != failure) break;
    }
    start = &g_BootDisplayPlaybackStartPos;
    *start = DsPosToInt(&file.pos);
    g_BootDisplayPlaybackEndPos = DsPosToInt(&file.pos) + (file.size >> 11);
    g_BootDisplayPlaybackCurrentPos = *start;
    g_BootDisplayPlaybackReadCount = 0;
    g_BootDisplayPlaybackRingIndex = 0;
    D_80172CFA = 0;
    g_BootDisplayPlaybackSectorBuffer = g_CdLoadAreaBase + (((g_MainAssetLbaTbl[0] - g_BootDisplayDataLbaTbl) << 11) + 0x34800);
    return 0;
}
