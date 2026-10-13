#ifndef PE1_ROOM_REARM_TIMER_H
#define PE1_ROOM_REARM_TIMER_H

/* The re-arming timer tick a room effect's update list ends with: counts
 * the state's hold timer down, and when the owner sets the request flag,
 * clears it and, once the timer has run out, reloads the timer and re-sends
 * the effect (func_800C6C18). Ends the object when the script reports the
 * effect done. The state keeps the reload value at `reload`, the timer
 * right after it and the request flag after that; the same code in every
 * effect, with only that offset differing. The caller declares
 * func_800C6C18 and FieldFx_IsLastSlotHalted. */
#define ROOM_REARM_TIMER_TICK(name, reload) \
    void name(int arg0, char *arg1, char *arg2) { \
        short *state = arg2; \
        \
        if (state[(reload) / 2 + 1] != 0) { \
            state[(reload) / 2 + 1]--; \
        } \
        \
        if (state[(reload) / 2 + 2] == 1) { \
            int timer = state[(reload) / 2 + 1]; \
            \
            state[(reload) / 2 + 2] = 0; \
            if (timer == 0) { \
                state[(reload) / 2 + 1] = *(unsigned short *)(arg2 + (reload)); \
                func_800C6C18(arg0); \
            } \
        } \
        \
        if (FieldFx_IsLastSlotHalted() == 1) { \
            arg1[1] = 2; \
        } \
    }

#endif /* PE1_ROOM_REARM_TIMER_H */
