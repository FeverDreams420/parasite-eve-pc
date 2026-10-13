#ifndef PE1_MENU_SOUND_H
#define PE1_MENU_SOUND_H

/* Menu feedback sounds (aya/stats.c). */
void Menu_PlayMoveSound(void);
void Menu_PlayConfirmSound(void);
void Menu_PlayCancelSound(void);
void Menu_PlayErrorSound(void);

/* Battle state queries the menus gate their input on. */
int Battle_GetEnemyEscapeFlag(void);
int Battle_GetAgilityBonus(void);
unsigned int Aya_GetActiveTimeRate(void);
int Battle_IsActiveWrapped(void);
int Battle_IsInputAllowedWrapped(void);
int Battle_GetStateFlag1(void);
/* Unwrapped forms (battle/setup.c). */
int Battle_IsActive(void);
int Pad_IsMenuConfirmAvailable(void);

#endif /* PE1_MENU_SOUND_H */
