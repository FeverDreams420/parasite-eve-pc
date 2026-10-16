#include "pe1/game_state.h"
#include "pe1/game_timers.h"
#include "pe1/save.h"

GameTimerEntry g_GameTimers[4];

/* Restarts the game clock, counting up to 100 hours of frames. */
void GameTime_InitCounter0(void) {
    g_GameClockLimit = 100 * 60 * 60 * 60;
    g_GameTimeTable = 0;
    g_GameClockFlags = 1;
}

/* Restarts the play time counter with no limit. */
void GameTime_InitCounter2(void) {
    g_GameTimers[2].current = 0;
    g_GameTimers[2].limit = 0;
    g_GameTimers[2].flags = 1;
}

void GameTime_InitCounter1(void) {
    g_GameTimers[1].current = 0;
    g_GameTimers[1].limit = 0;
    g_GameTimers[1].flags = 1;
}

/* Original 20/6/6-bit packed word. Reload src[1] after each store: the
 * destination may alias it. Explicit masks avoid compiler bitfield layout. */
void GameTime_PackTime(const u32 *src, u32 *out) {
    u32 packed = (*out & 0xfff00000u) | (((src[1] % 60u) * 216000u) & 0xfffffu);
    *out = packed;
    packed = (packed & 0xfc0fffffu) | ((((src[1] % 60u) * 3600u) & 63u) << 20);
    *out = packed;
    packed = (packed & 0x03ffffffu) | ((((src[1] % 60u) * 60u) & 63u) << 26);
    *out = packed;
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
    entry = g_GameTimers;

    do {
        flags_p = &entry->flags;
        limit_p = &entry->limit;
        current_p = &entry->current;
        flags = *flags_p;
        if ((flags & 1) != 0) {
            if ((flags & 4) == 0) {
                if ((flags & 2) != 0) {
                    if ((flags & 5) != 0) {
                        direct = *current_p;
                        entry->current = (s32)((u32)direct - 1u);
                    } else {
                        current = *current_p;
                        if ((unsigned int)*limit_p < (unsigned int)current) {
                            direct = (s32)((u32)current - 1u);
                            entry->current = direct;
                        } else {
                            direct = flags | 4;
                            *flags_p = direct;
                        }
                    }
                } else {
                    if ((flags & 5) != 0) {
                        direct = *current_p;
                        entry->current = (s32)((u32)direct + 1u);
                    } else {
                        current = *current_p;
                        if ((unsigned int)current < (unsigned int)*limit_p) {
                            /* Flag tests are finished; reuse the temporary for the count. */
                            flags = current;
                            direct = (s32)(flags + 1u);
                            entry->current = direct;
                        } else {
                            direct = flags | 4;
                            *flags_p = direct;
                        }
                    }
                }
            }
        }
        entry++;
        i++;
    } while (i < 4);
}
