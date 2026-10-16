#ifndef PE1_GAME_TIMERS_H
#define PE1_GAME_TIMERS_H

#include "common.h"

/* Four three-word frame timers beginning at 0x800A76A0, stepped once per
 * frame by Scene_TickTimers and read and written by the scene scripts.
 * flags bit 0 runs the timer, bit 1 counts down instead of up, bit 2 marks
 * that the limit was reached. Timer 0 is the game clock, timer 2 the play
 * time saved in the memory card header (in frames), timer 3
 * the script countdown that Script_ComputeAudioTimer arms with the frames
 * left in current and a limit of 0. */
typedef struct GameTimerEntry {
    /* 0x00 */ u32 flags;
    /* 0x04 */ int current;
    /* 0x08 */ int limit;
} GameTimerEntry;

PE1_STATIC_ASSERT(sizeof(GameTimerEntry) == 0x0C, game_timer_entry_size);

extern GameTimerEntry g_GameTimers[4];
void GameTime_InitCounters(void);
void GameTime_InitCounter0(void);
void GameTime_InitCounter1(void);
void GameTime_InitCounter2(void);
void GameTime_PackTime(const u32 *src, u32 *out);
void Scene_TickTimers(void);

/* Interior clock aliases used by existing callers; ordinary host memory. */
#define g_GameClockFlags (g_GameTimers[0].flags)

#define g_GameTimeTable (g_GameTimers[0].current)
#define g_GameClockLimit (g_GameTimers[0].limit)
#define GAME_TIME_COUNTER(index) (g_GameTimers[index].current)

/* Legacy name for authored story progress (script variable 74), used to
 * classify the save header. Elapsed play frames are timer 2 above. */
extern u32 g_ScriptVariableBank[];
#define g_PlayTimeFrameCounter (g_ScriptVariableBank[74])

/* Frames left on the timed state that Entity_UpdateTimers clears with flag 0x100. */
extern u16 g_ActorEffectFlag100Timer;

#endif
