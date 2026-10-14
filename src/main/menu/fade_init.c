#include "pe1/menu_fade_state.h"

int g_MenuSaveBgFadeState;
int g_MenuSaveBgFadeStep;
int g_MenuSaveBgFadeHeight;
int g_MenuSaveBgFadeLutLen;
int g_MenuSaveBgFadeTint;
int g_MenuItemContextFlag;
u8 g_MenuSaveBgFadeLut[16];

void Menu_SaveBgInitFade(void) {
    g_MenuSaveBgFadeState = 0;
    g_MenuSaveBgFadeLutLen = 0;
    g_MenuSaveBgFadeStep = 0;
    g_MenuSaveBgFadeHeight = 0x20;
    Menu_ComputeGammaLut(0x90, 0xFF);
    g_MenuSaveBgFadeTint = 0x48;
}


void Menu_ComputeGammaLut(int arg0, int arg1) {
    u8 *base;
    u8 *ptr;
    int scale;
    int limit;
    u8 *iter_end;
    u8 *check_end;

    base = g_MenuSaveBgFadeLut;
    base[0] = 0;
    ptr = base;
    limit = 0x100;
    scale = limit - arg0;
    check_end = ptr + 0xF;
    arg0 <<= 8;
    if (ptr < check_end) {
        iter_end = check_end;
        do {
            if (*ptr >= arg1) {
                base = g_MenuSaveBgFadeLut;
                limit = (ptr - base) + 1;
                g_MenuSaveBgFadeLutLen = limit;
                return;
            }
            ptr[1] = (arg0 + (scale * *ptr)) >> 8;
            ptr++;
        } while (ptr < iter_end);
    }

out:
    base = g_MenuSaveBgFadeLut;
    limit = (ptr - base) + 1;
    g_MenuSaveBgFadeLutLen = limit;
}


int Menu_ClampRange(int arg0) {
    arg0 &= 0x1FF;
    g_MenuItemContextFlag = arg0;
    if (arg0 == 0) {
        g_MenuItemContextFlag = 0x1FF;
    }
    return 0;
}

