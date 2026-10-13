#ifndef MENUCARD_EVENT_FLAG_NAME
#define MENUCARD_EVENT_FLAG_NAME MenuCard_UpdateEventFlag
#define MENUCARD_EVENT_FLAG g_MenuCardScreenVideoDepthFlip
#endif

/* Raises the flag when the requested state differs from the current event
 * state; both copies of the video player carry this function. */
void MENUCARD_EVENT_FLAG_NAME(s8 enabled) {
    if ((enabled && !g_MovieWide) || (!enabled && g_MovieWide)) {
        MENUCARD_EVENT_FLAG = 1;
    }
}
