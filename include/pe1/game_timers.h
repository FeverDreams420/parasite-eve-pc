#ifndef PE1_GAME_TIMERS_H
#define PE1_GAME_TIMERS_H

#include "common.h"

/* Four three-word frame timers beginning at 0x800A76A0, stepped once per
 * frame by Scene_TickTimers and read and written by the scene scripts.
 * flags bit 0 runs the timer, bit 1 counts down instead of up, bit 2 marks
 * that the limit was reached. Timer 0 is the game clock, timer 2 the play
 * time saved in the memory card header (in frames), timer 3
 * the combat countdown that Script_ComputeAudioTimer arms with the frames
 * left in current and a limit of 0. */
typedef struct GameTimerEntry {
    /* 0x00 */ u32 flags;
    /* 0x04 */ int current;
    /* 0x08 */ int limit;
} GameTimerEntry;

PE1_STATIC_ASSERT(sizeof(GameTimerEntry) == 0x0C, game_timer_entry_size);

extern GameTimerEntry g_GameTimers[4];

/* Interior symbol naming timer zero's flags word; its reset stores both the
 * cleared and the running value. */
extern volatile u32 g_GameClockFlags;

/* Interior symbols for the game clock words GameTime_InitCounter0 writes.
 * Retail stores the game clock limit twice on reset; the qualifier keeps
 * both. */
extern int g_GameTimeTable;
extern volatile int g_GameClockLimit;
#define GAME_TIME_COUNTER(index) (g_GameTimers[index].current)

/* Frames played; Save_ClassifyPlaytime buckets it for the save header. */
extern unsigned int g_PlayTimeFrameCounter;

/* Frames left on the timed state that Entity_UpdateTimers clears with flag 0x100. */
extern u16 g_ActorEffectFlag100Timer;

#endif
