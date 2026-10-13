#include "pe1/battle_status.h"
#include "pe1/boot.h"
#include "pe1/boot_disc_check.h"
#include "pe1/draw_buffers.h"
#include "pe1/game_state.h"
#include "pe1/menu_background.h"
#include "pe1/psyq_gpu.h"
#include "pe1/render_packets.h"

int g_ActiveDrawSlot;

short Seq_GetElapsed(void);

static inline DISPENV *DispAddress(int index) {
    return &g_RenderDispEnvArray[index];
}

static inline DRAWENV *DrawAddress(int index) {
    return &g_RenderDrawEnvArray[index];
}

void Draw_RenderFrame(void) {
    RenderBufferPrefix *buffers;
    int displayFlags;
    int idx;
    int state;
    int status;
    RenderFrameState *state_ptr;

    DrawSync(0);
    Menu_DrawSaveBg();

    displayFlags = g_GameState.flags & 0x200;
    if (displayFlags) {
        VSync(4);
        if (Seq_GetElapsed() >= 3) {
            SetDispMask(1);
        }
    } else {
        VSync(2);
    }

    ResetGraph(1);

    idx = g_ActiveDrawSlot;
    PutDispEnv(DispAddress(idx));

    status = Draw_CheckDrawStatus();
    if ((status << 24) != 0 ||
        (state_ptr = (RenderFrameState *)&g_GameState,
         state = state_ptr->flags,
         (state & 0x200) != 0)) {
        idx = g_ActiveDrawSlot;
        PutDrawEnv(DrawAddress(idx));
        g_ActiveDrawSlot = g_ActiveDrawSlot < 1U;
        return;
    }
draw_buffer:
    {
        int drawSlot;
        char *orderingTable;

        drawSlot = g_ActiveDrawSlot;
        buffers = &state_ptr->buffers;
        orderingTable = buffers->ordering[drawSlot];
        DrawOTagEnv(orderingTable + 0x3FFC, &g_RenderDrawEnvArray[drawSlot]);
    }

done:
    g_ActiveDrawSlot = g_ActiveDrawSlot < 1U;
}
