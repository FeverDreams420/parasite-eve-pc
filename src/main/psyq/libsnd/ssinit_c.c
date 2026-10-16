/* USA 80036DC8 initializes game timers, independently of PSY-Q sound. */
#include "pe1/game_timers.h"

void GameTime_InitCounters(void) {
    GameTime_InitCounter0();
    GameTime_InitCounter2();
    GameTime_InitCounter1();
}
