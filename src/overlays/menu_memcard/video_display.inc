#ifndef MENUCARD_SET_VIDEO_DISPLAY_NAME
#define MENUCARD_SET_VIDEO_DISPLAY_NAME MenuCard_SetVideoDisplay
#define MENUCARD_VIDEO_DISPLAY_MODE D_801D0DBE
#endif

void MENUCARD_SET_VIDEO_DISPLAY_NAME(s8 index, s8 wide) {
    s16 y[2];

    if (index == 0) {
        y[0] = 240;
        y[1] = 0;
    } else {
        y[0] = 0;
        y[1] = 240;
    }
    if (wide) {
        MENUCARD_VIDEO_DISPLAY_MODE = 3;
        SetDefDispEnv(&g_RenderDispEnvArray[index], 0, y[0], 480, 240);
        g_RenderDispEnvArray[index].isrgb24 = 1;
        g_RenderDispEnvArray[index].disp.w = g_RenderDispEnvArray[index].disp.w * 2 / 3;
        SetDefDrawEnv(&g_RenderDrawEnvArray[index], 0, y[1], 480, 240);
        g_RenderDrawEnvArray[index].isbg = 1;
        g_RenderDrawEnvArray[index].dtd = 1;
        g_RenderDrawEnvArray[index].dfe = 0;
        g_RenderDrawEnvArray[index].r0 = 0;
        g_RenderDrawEnvArray[index].g0 = 0;
        g_RenderDrawEnvArray[index].b0 = 0;
        g_RenderDrawEnvArray[index].clip.w = g_RenderDrawEnvArray[index].clip.w * 2 / 3;
    } else {
        MENUCARD_VIDEO_DISPLAY_MODE = 2;
        SetDefDispEnv(&g_RenderDispEnvArray[index], 0, y[0], 320, 240);
        SetDefDrawEnv(&g_RenderDrawEnvArray[index], 0, y[1], 320, 240);
        g_RenderDrawEnvArray[index].isbg = 1;
        g_RenderDrawEnvArray[index].dtd = 1;
        g_RenderDrawEnvArray[index].dfe = 0;
        g_RenderDrawEnvArray[index].r0 = 0;
        g_RenderDrawEnvArray[index].g0 = 0;
        g_RenderDrawEnvArray[index].b0 = 0;
    }
}

#undef MENUCARD_VIDEO_DISPLAY_MODE
#undef MENUCARD_SET_VIDEO_DISPLAY_NAME
