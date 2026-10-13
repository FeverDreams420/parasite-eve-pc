#ifndef PE1_STAT_MODIFIERS_H
#define PE1_STAT_MODIFIERS_H

/* Base scaling entry followed by the per-category stat modifiers: seven
 * words cleared before scanning the equipped item record. */
extern int g_StatScaleBase[7];
int *Battle_GetModifierTable(void);

#endif
