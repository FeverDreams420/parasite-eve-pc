#include "pe1/display_env.h"
#include "pe1/psyq_gpu.h"
#include "pe1/gpu_image.h"
#include "pe1/game_state.h"
#ifndef MENUCARD_RESET_DISPLAY_STATE_NAME
#define MENUCARD_RESET_DISPLAY_STATE_NAME MenuCard_ResetDisplayState
#endif
#ifndef MENUCARD_DISPLAY_PTR0
#define MENUCARD_DISPLAY_PTR0 D_801D0DE8
#define MENUCARD_DISPLAY_PTR1 D_801D0DEC
#define MENUCARD_DISPLAY_PTR2 D_801D0DF0
#define MENUCARD_DISPLAY_PTR3 D_801D0DF4
#define MENUCARD_DISPLAY_PTR4 D_801D0DF8
#define MENUCARD_DISPLAY_PTR5 D_801D0DFC
#define MENUCARD_SLOT20_SOURCE g_MenuCardScreenSavedDispEnv
#define MENUCARD_SLOT92_SOURCE D_801D13AC
#endif

typedef struct MemcardRect {
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} MemcardRect;


#include "display_reset_externs.h"

void SetDispMask(s32 enabled);

void MENUCARD_RESET_DISPLAY_STATE_NAME(void) {
    s8 i;
    u8 *active;
    MemcardRect rect;

    active = &D_800B0DBA;
    if (*active != 0) {
        MENUCARD_DISPLAY_PTR0 = 0;
        MENUCARD_DISPLAY_PTR1 = 0;
        MENUCARD_DISPLAY_PTR5 = 0;
        MENUCARD_DISPLAY_PTR4 = 0;
        MENUCARD_DISPLAY_PTR2 = 0;
        MENUCARD_DISPLAY_PTR3 = 0;
        *active = 0;

        DrawSync(0);
        VSync(0);
        SetDispMask(0);

        i = 0;
        do {
            g_RenderDispEnvArray[i] = MENUCARD_SLOT20_SOURCE[i];
            g_RenderDrawEnvArray[i] = MENUCARD_SLOT92_SOURCE[i];
            i++;
        } while (i < 2);

        rect.x = 0x200;
        rect.y = 0;
        rect.w = 0xC0;
        rect.h = 0x100;
        MoveImage(&rect, 0x140, 0);

        if ((D_800B0CD8 & 0x8000000) == 0) {
            rect.x = 0x200;
            rect.y = 0x100;
            rect.w = 0x140;
            rect.h = 0x40;
            MoveImage(&rect, 0, 0x1C0);
        }
    }
}

#undef MENUCARD_SLOT92_SOURCE
#undef MENUCARD_SLOT20_SOURCE
#undef MENUCARD_DISPLAY_PTR5
#undef MENUCARD_DISPLAY_PTR4
#undef MENUCARD_DISPLAY_PTR3
#undef MENUCARD_DISPLAY_PTR2
#undef MENUCARD_DISPLAY_PTR1
#undef MENUCARD_DISPLAY_PTR0
#undef MENUCARD_RESET_DISPLAY_STATE_NAME
