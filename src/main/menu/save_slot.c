/* CC1_FLAGS: -G8 */
/* MASPSX_FLAGS: -G8 --use-comm-section */
/* MASPSX_FORCE_G0: 1 */

#include "common.h"
#include "pe1/save.h"
#include "pe1/save_slot_metadata.h"
#include "pe1/textbox.h"

/* COMMON supplies small-data metadata to MASPSX: these words are reached
 * through $gp, every other extern stays absolute. */
unsigned char g_MenuActiveMode;
unsigned char g_SaveOverlayTimer;
u8 D_8009D1CE;
void *D_8009D1F8;

void Save_DrawSlotMetadata(void)
{
    u32 state;
    u32 next;
    int word;
    int phase;
    int prompt;
    short colors[3];

    if (D_8009D1A8.summary == 0 || *D_8009D1A8.summary == 0 || (*D_8009D1A8.summary)->primaryValue <= 0) {
        Draw_ResetTextboxes();
        D_8009D1A8.prompt.word &= ~0x300;
    }

    state = D_8009D1A8.prompt.word;
    phase = (state >> 8) & 3;
    switch (phase) {
    case 1:
        colors[0] = (*D_8009D1A8.summary)->primaryValue;
        colors[1] = (*D_8009D1A8.summary)->secondaryValue;
        colors[2] = 0;
        Draw_ResetTextboxes();
        Menu_SetTextCursorRect(Save_GetMetadataWindowIndex() != 0 ? 0x14 : 0x61, g_MenuActiveMode < 2 ? 0xF : 0xC3, 0, 0);
        Render_SetupColorTable(0, 2, colors);
        prompt = (D_8009D1A8.prompt.word >> 10) & 3;
        if (prompt != 0) {
            if (prompt == phase) {
                g_TextboxEntries[0].message = g_AnalyzeHpFormatText;
            }
        } else {
            g_TextboxEntries[0].message = D_80091464;
        }
        g_TextboxEntries[0].state = 2;
        g_TextboxEntries[0].control.flags |= 0x2000000;
        next = D_8009D1A8.prompt.word;
        word = next & ~0x300;
        word |= ((((next >> 8) & 3) + 1) & 3) << 8;
        D_8009D1A8.prompt.word = word;
        break;
    case 2:
        if (D_8009D1A8.prompt.timer == 0) {
            D_8009D1A8.prompt.word = (state & ~0x300) | 0x200;
            D_8009D1A8.prompt.timer = 0x4B;
            state = D_8009D1A8.prompt.word;
            if (state & 0x1000) {
                g_TextboxEntries[0].message = g_TextSusceptibleToPoison[Save_GetMetadataWindowIndex()];
                D_8009D1A8.prompt.word &= ~0x1000;
            } else if (state & 0x2000) {
                g_TextboxEntries[0].message = g_TextSusceptibleToAcid[Save_GetMetadataWindowIndex()];
                D_8009D1A8.prompt.word &= ~0x2000;
            } else if (state & 0x4000) {
                g_TextboxEntries[0].message = D_800914D4[Save_GetMetadataWindowIndex()];
                D_8009D1A8.prompt.word &= ~0x4000;
            } else if (state & 0x8000) {
                g_TextboxEntries[0].message = g_AnalyzeHeatWeakTexts[Save_GetMetadataWindowIndex()];
                D_8009D1A8.prompt.word &= ~0x8000;
            } else if (state & 0x10000) {
                g_TextboxEntries[0].message = D_80091520[Save_GetMetadataWindowIndex()];
                D_8009D1A8.prompt.word &= ~0x10000;
            } else if (state & 0x20000) {
                g_TextboxEntries[0].message = D_80091544[Save_GetMetadataWindowIndex()];
                D_8009D1A8.prompt.word &= ~0x20000;
            } else if (state & 0x40000) {
                g_TextboxEntries[0].message = D_80091570[Save_GetMetadataWindowIndex()];
                D_8009D1A8.prompt.word &= ~0x40000;
            } else {
                Draw_ResetTextboxes();
                D_8009D1A8.prompt.word &= ~0x300;
            }
        }
        break;
    }
    D_8009D1A8.prompt.timer--;
    Draw_SetCursor(0, g_MenuActiveMode < 2 ? 0xB : 0xBF);
    Draw_AllocColorGradient(0x140, 0x14, 0, 0);
}

void Menu_SaveOverlayDraw(void)
{
    short color;
    int y;
    int value;
    switch (D_8009D1CE) {
    case 1:
        color = -1;
        Draw_ResetTextboxes();
        value = Save_GetMetadataWindowIndex() ? 20 : 97;
        y = g_MenuActiveMode < 2 ? 15 : 195;
        Menu_SetTextCursorRect(value, y, 0, 0);
        Render_SetupColorTable(0, 2, &color);
        value = D_8009D1CE;
        g_TextboxEntries[0].state = 2;
        g_SaveOverlayTimer = 75;
        g_TextboxEntries[0].message = D_8009D1F8;
        value++;
        D_8009D1CE = value;
        g_TextboxEntries[0].control.flags |= 0x2000000;
        break;
    case 2:
        if (!g_SaveOverlayTimer) {
            Draw_ResetTextboxes();
            D_8009D1CE = 0;
        }
        break;
    }
    --g_SaveOverlayTimer;
    Draw_SetCursor(0, g_MenuActiveMode < 2 ? 11 : 191);
    Draw_AllocColorGradient(320, 20, 0, 0);
}
