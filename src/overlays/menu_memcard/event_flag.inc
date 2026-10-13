#ifndef MENUCARD_EVENT_FLAG_NAME
#define MENUCARD_EVENT_FLAG_NAME MenuCard_UpdateEventFlag
#define MENUCARD_EVENT_FLAG D_801D0DC0
#endif

/* Raises the flag when the requested state differs from the current event
 * state; both copies of the video player carry this function. */
void MENUCARD_EVENT_FLAG_NAME(s8 enabled) {
    if ((enabled && !D_800B0DBB) || (!enabled && D_800B0DBB)) {
        MENUCARD_EVENT_FLAG = 1;
    }
}
