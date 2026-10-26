#include "pe1/battle_status.h"
#include "pe1/boot.h"
#include "pe1/boot_disc_check.h"
#include "pe1/draw_buffers.h"
#include "pe1/game_state.h"
#include "pe1/menu_background.h"
#include "pe1/psyq_gpu.h"
#include "pe1/render_packets.h"

int g_ActiveDrawSlot;
DISPENV g_RenderDispEnvArray[2];
DRAWENV g_RenderDrawEnvArray[2];

short Seq_GetElapsed(void);

static inline DISPENV *DispAddress(int index) {
    return &g_RenderDispEnvArray[index];
}

static inline DRAWENV *DrawAddress(int index) {
    return &g_RenderDrawEnvArray[index];
}

void Draw_RenderFrame(void) {
    int displayFlags;
    int idx;
    int state;
    int status;

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
    state = g_GameState.flags;
    if ((status & 0xff) != 0 || (state & 0x200) != 0) {
        idx = g_ActiveDrawSlot;
        PutDrawEnv(DrawAddress(idx));
        Render_CommitWidescreenPicture(idx, -1);
        g_ActiveDrawSlot = g_ActiveDrawSlot < 1U;
        return;
    }
draw_buffer:
    {
        int drawSlot;
        RenderNativeTag *orderingTable;

        drawSlot = g_ActiveDrawSlot;
        orderingTable = (RenderNativeTag *)g_GameState.shared_buffers.ordering_tables[drawSlot];
        Render_CommitWidescreenPicture(drawSlot, g_RenderDrawEnvArray[drawSlot].clip.y);
        DrawOTagEnv(orderingTable + 4095, &g_RenderDrawEnvArray[drawSlot]);
    }

done:
    g_ActiveDrawSlot = g_ActiveDrawSlot < 1U;
}
