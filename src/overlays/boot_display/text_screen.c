#include "pe1/boot_display.h"

#include "text_screen_externs.h"

void BootDisplay_RunDisplay(void) {
    u32 state;
    s32 fade;
    if (g_GameStateFlags & 0x10000) {
        g_BootDisplayCallbacks = &g_BootDisplayImageCallbacks;
    } else {
        g_BootDisplayCallbacks = &g_BootDisplaySpriteCallbacks;
        if (!(g_GameStateFlags & 0x20000)) g_BootDisplaySpriteCallbacks.text = 0;
    }
    g_BootDisplayCallbacks->init();
    if (!(g_GameStateFlags & 0x10000)) {
        func_8012562C();
        BootDisplay_StartPlayback();
        if (g_GameStateFlags & 0x20000) {
            Spu_SetVoiceVolume(144);
            BootDisplay_SetCdFilter(1);
        } else Spu_SetVoiceVolume(218);
    }
    state = 0;
    fade = 0;
    g_BootDisplayFrameCounter = 420;
    g_BootDisplayPlaybackEnded = 0;
    g_BootDisplayDone = 0;
    do {
        if (g_BootDisplayCallbacks->step) g_BootDisplayCallbacks->step();
        if (g_BootDisplayCallbacks->text) g_BootDisplayCallbacks->text();
        switch (state) {
        case 0:
            if (BootDisplay_DrawTextBlock()) {
                state = (g_GameStateFlags & 0x20000) ? 1 : 4;
                fade = 0;
            }
            break;
        case 1:
            g_BootDisplayCallbacks->transition((u8)fade);
            fade += 4;
            if (fade >= 256) { state = 2; fade = 0; }
            break;
        case 2:
            g_BootDisplayCallbacks->transition(255);
            if (g_BootDisplayPlaybackEnded && (g_FieldPadBits & 0xF0000004)) { state = 3; fade = 255; }
            break;
        case 3:
            g_BootDisplayCallbacks->transition((u8)fade);
            fade -= 4;
            if (fade < 0) state = 4;
            break;
        case 4: {
            u32 flags = g_GameStateFlags;
            s32 finished;
            if (flags & 0x30000) finished = 1;
            else finished = g_BootDisplayPlaybackEnded;
            g_BootDisplayDone = finished;
            break;
        }
        }
        Field_HandleStateTransition();
        if (g_GameStateFlags & 0x10000) {
            if (D_8009D1F4 & 0x4000000) BootDisplay_SetCdFilter(0);
            if (D_8009D1F4 & 0x8000000) BootDisplay_SetCdFilter(1);
        }
        g_BootDisplayCallbacks->present();
        ++g_BootDisplayFrameCounter;
    } while (g_BootDisplayDone == 0);
    BootDisplay_WaitForCdAndRestoreCallbacks();
    g_BootDisplayCallbacks->shutdown();
    g_GameStateFlags &= ~0x10000;
}

s32 BootDisplay_DrawTextBlock(void) {
    const u8 *line;
    u32 x = 24;
    u32 width;
    const u8 *scan;
    u32 rows = 0;
    s32 y = g_BootDisplayTextScrollX >> 8;
    g_BootDisplayTextCursor = g_BootDisplayCreditLines;
    line = *g_BootDisplayTextCursor;
    while (*line != 255) {
        if (y >= -7) {
            if (y >= 225) break;
            for (; *line; ++line) {
                switch (*line) {
                case ' ':
                    break;
                case '\\':
                    width = 0;
                    if (line[1]) {
                        scan = line + 1;
                        do {
                            width += BootDisplay_GetGlyphWidth(*scan++);
                        } while (*scan);
                    }
                    x = (320 - width) >> 1;
                    continue;
                case '\t':
                    x += BootDisplay_GetGlyphWidth(' ') * 14;
                    continue;
                default:
                    g_BootDisplayCallbacks->glyph((s16)x, (s16)y, *line);
                    break;
                }
                x += BootDisplay_GetGlyphWidth(*line);
            }
            g_BootDisplayFontStyle = 0;
            x = 24;
            ++rows;
        }
        ++g_BootDisplayTextCursor;
        line = *g_BootDisplayTextCursor;
        y += 18;
    }
    g_BootDisplayTextScrollX += g_BootDisplayTextScrollStepX;
    return rows < 1;
}

/* Fallthrough follows consecutive punctuation slots in the font atlas. */
#define PUNCTUATION_INDEX(base) { \
    index = -1; \
    switch (saved) { \
    case ':': ++index; \
    case '?': ++index; \
    case '/': ++index; \
    case '.': ++index; \
    case ',': ++index; \
    case '\'': ++index; \
    case '-': ++index; \
    case ')': ++index; \
    case '(': ++index; \
    case '&': ++index; \
    case '#': ++index; \
    case '@': ++index; \
    case '!': ++index; \
    case '~': ++index; \
    default: index += (base); \
    } \
}
s32 BootDisplay_GetGlyphIndex(u8 character) {
    u16 style = g_BootDisplayFontStyle;
    u8 saved = character;
    s32 index;
    if (style) {
        if ((u8)(character - 'a') < 26) index = saved - 32;
        else if ((u8)(character - 'A') < 26) index = saved - 65;
        else if ((u8)(character - '0') < 10) index = saved + 58;
        else PUNCTUATION_INDEX(26);
    } else {
        if ((u8)(character - 'A') < 26) index = saved - 25;
        else if ((u8)(character - 'a') < 26) index = saved - 31;
        else if ((u8)(character - '0') < 10) index = saved + 58;
        else PUNCTUATION_INDEX(92);
    }
    return (u8)index;
}

#undef PUNCTUATION_INDEX

/* Four width choices per alphabetic glyph. The lowercase base accounts for
 * the extra 32 ASCII code points; odd styles select lowercase widths. */

u16 BootDisplay_GetGlyphWidth(u8 character) {
    u16 style = g_BootDisplayFontStyle;
    u8 saved;
    s32 width;
    if (style == 2) {
        saved = character;
        if ((u8)(character - 'a') < 26)
            saved = character - 32;
        if ((u8)(saved - 'A') < 26)
            width = D_80125B1E[saved][0];
        else switch (saved) {
        case '!': case '\'': case '.': case ':': width = 3; break;
        case '#': width = 7; break;
        case '&': width = 11; break;
        case '(': case ')': case ',': case '-': case '/': width = 4; break;
        case '?': width = 6; break;
        case '@': width = 9; break;
        case '~': width = 8; break;
        default: width = 8; break;
        }
    } else {
        if ((u8)(character - 'A') < 26)
            width = D_80125B1C[character][style];
        else if ((u8)(character - 'a') < 26)
            width = g_BootDisplayLowercaseWidths[character][style | 1];
        else switch (character) {
        case '!': case '.': case ':': width = 2; break;
        case '\'': case ',': width = 3; break;
        case '(': case ')': case '-': case '/': case '?': width = 4; break;
        case '&': case '@': width = 9; break;
        case '~': width = 6; break;
        default: width = 6; break;
        }
    }
    return width;
}
