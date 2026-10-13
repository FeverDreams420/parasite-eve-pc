/* PSY-Q LIBPAD WAITRC2: setRC2wait, chkRC2wait. */

#include "pe1/psyq_pad_main.h"

#include "waitrc2_externs.h"

void setRC2wait(int limit) {
    int start = *(volatile u16 *)0x1F801120;
    g_TimerTimeoutLimit = limit;
    g_TimerTimeoutStart = start;
}

int chkRC2wait(void) {
    u32 current;
    u32 raw;
    u32 base;
    u32 limit;

    raw = *(volatile u16 *)0x1F801120;
    base = g_TimerTimeoutStart;
    current = raw & 0xFFFF;
    if (current < base) {
        if (*(volatile u16 *)0x1F801128 != 0) {
            current += *(volatile u16 *)0x1F801128;
        } else {
            current += 0x10000;
        }
    }

    if ((*(volatile u16 *)0x1F801124 & 0x200) != 0) {
        limit = g_TimerTimeoutLimit;
        return !((current - g_TimerTimeoutStart) < limit);
    } else {
        limit = g_TimerTimeoutLimit;
        return !(((current - g_TimerTimeoutStart) >> 3) < limit);
    }
}
