#include "pe1/boot_display.h"
#include "pe1/display_env.h"
#include "pe1/gpu_image.h"
#include "pe1/psyq_gpu.h"

#include "room_m272.h"
#include "room_m272_phase.h"

void RoomM272_Nop_8018F00C(void) {}

void RoomM272_Cinematic(PmSlotHeader *slot) {
    RECT b1 = RoomM272_BlobA;
    RECT b2 = RoomM272_BlobB;
    RECT b3 = RoomM272_BlobC;

    if (g_RoomM272Phase == 0) {
        StoreImage(&b1, RoomM272_Rec);
        MoveImage(&b2, 0x300, 0x100);
        DrawSync(0);
    }
    if (g_RoomM272Phase == 1) {
        LoadImage(&b1, RoomM272_Rec);
        MoveImage(&b3, 0x140, 0x100);
        DrawSync(0);
        VSync(2);
        PutDispEnv(&g_RenderDispEnvArray[g_ActiveDrawSlot]);
        SetDispMask(1);
    }
}

int RoomM272_SetPhase(PmSlotHeader *slot, int cancel, int mode, intptr_t phase, intptr_t b, intptr_t c) {
    g_RoomM272Phase = (s32)phase;
    return 0;
}

int RoomM272_Nop_8018F188(PmSlotHeader *slot) {
    return 0;
}

int RoomM272_Nop_8018F190(PmSlotHeader *slot) {
    return 0;
}

int RoomM272_Nop_8018F198(PmSlotHeader *slot) {
    return 0;
}

void RoomM272_Nop_8018F1A0(void) {}

/* Original M272 data at 8018EFF4..8018F004 and its seven-entry descriptor.
 * The image save buffer is host memory; its pixels retain the PS1 layout. */
RECT RoomM272_BlobA = {0,448,320,64};
RECT RoomM272_BlobB = {320,256,192,256};
RECT RoomM272_BlobC = {768,256,192,256};
u16 RoomM272_Rec[320 * 64];
int g_RoomM272Phase;
PmCommand g_RoomM272Handlers = {
    RoomM272_Nop_8018F00C, RoomM272_Cinematic, RoomM272_SetPhase,
    RoomM272_Nop_8018F188, RoomM272_Nop_8018F190,
    RoomM272_Nop_8018F198, RoomM272_Nop_8018F1A0,
};
