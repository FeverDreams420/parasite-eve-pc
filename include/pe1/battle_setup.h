#ifndef PE1_BATTLE_SETUP_H
#define PE1_BATTLE_SETUP_H

/* Battle action state entry points (src/main/battle/setup.c) that rooms and
 * scenes call when a grab or a scripted sequence takes and returns control. */
int Battle_BuildTargetList(void);
void Battle_BeginPlayerAction(void);
void Battle_ReturnToIdle(void);

#endif /* PE1_BATTLE_SETUP_H */
