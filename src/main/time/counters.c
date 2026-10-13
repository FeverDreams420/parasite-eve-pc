#include "pe1/game_state.h"
#include "pe1/game_timers.h"
#include "pe1/save.h"

/* Restarts the game clock, counting up to 100 hours of frames. */
void GameTime_InitCounter0(void) {
    int value;

    value = 100 * 60 * 60 * 60;
    g_GameClockLimit = 0;
    g_GameClockLimit = value;
    value = 1;
    g_GameTimeTable = 0;
    D_800A76A0 = 0;
    D_800A76A0 = value;
}

/* Restarts the play time counter with no limit. */
void GameTime_InitCounter2(void) {
    int value;

    value = 1;
    g_GameTimers[2].current = 0;
    g_GameTimers[2].limit = 0;
    g_GameTimers[2].flags = value;
}

void GameTime_InitCounter1(void) {
    int value;

    value = 1;
    g_GameTimers[1].current = 0;
    g_GameTimers[1].limit = 0;
    g_GameTimers[1].flags = value;
}

/* Packed clock word with 20/6/6-bit fields, each filled from src[1]. */
typedef struct PackedTime {
    u32 frames : 20;
    u32 minutes : 6;
    u32 seconds : 6;
} PackedTime;

void GameTime_PackTime(u32 *src, PackedTime *out) {
    out->frames = src[1] % 60 * 60 * 60 * 60;
    out->minutes = src[1] % 60 * 60 * 60;
    out->seconds = src[1] % 60 * 60;
}

int Draw_CheckDrawStatus(void);

void Scene_TickTimers(void) {
    GameTimerEntry *entry;
    int *current_p;
    u32 *flags_p;
    int *limit_p;
    unsigned int i;
    u32 flags;
    int direct;
    int current;

    if ((g_GameStateFlags & 0x41) != 0) {
        return;
    }
    if ((Draw_CheckDrawStatus() << 24) != 0) {
        return;
    }

    i = 0;
    flags_p = &g_GameTimers[0].flags;
    entry = g_GameTimers;
    limit_p = &entry->limit;
    current_p = &entry->current;

    do {
        flags = *flags_p;
        if ((flags & 1) != 0) {
            if ((flags & 4) == 0) {
                if ((flags & 2) != 0) {
                    if ((flags & 5) != 0) {
                        direct = *current_p;
                        entry->current = direct - 1;
                    } else {
                        current = *current_p;
                        if ((unsigned int)*limit_p < (unsigned int)current) {
                            direct = current - 1;
                            entry->current = direct;
                        } else {
                            direct = flags | 4;
                            *flags_p = direct;
                        }
                    }
                } else {
                    if ((flags & 5) != 0) {
                        direct = *current_p;
                        entry->current = direct + 1;
                    } else {
                        current = *current_p;
                        if ((unsigned int)current < (unsigned int)*limit_p) {
                            /* Flag tests are finished; reuse the temporary for the count. */
                            flags = current;
                            direct = flags + 1;
                            entry->current = direct;
                        } else {
                            direct = flags | 4;
                            *flags_p = direct;
                        }
                    }
                }
            }
        }
        flags_p += sizeof(GameTimerEntry) / sizeof(*flags_p);
        entry++;
        limit_p += sizeof(GameTimerEntry) / sizeof(*limit_p);
        i++;
        current_p += sizeof(GameTimerEntry) / sizeof(*current_p);
    } while (i < 4);
}
