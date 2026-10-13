#ifndef PE1_RANDOM_H
#define PE1_RANDOM_H

#include "common.h"

/* Byte generator cursor and its complete 521-byte state. */
extern s32 g_ItemDropRngIndex;
extern u8 g_ItemDropRngTable[521];

int rand(void);

#endif
