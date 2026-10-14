#ifndef PE1_ASSET_TABLE_H
#define PE1_ASSET_TABLE_H

#include "common.h"

/* Returns the payload of a 12-byte record with a 32-bit key in an asset table. */
void *Asset_FindTable08ByU32Key(void *base, int key);

/* Room names are six base-32 digits drawn from this 32-character alphabet. */
extern u8 g_Base32CharTable[32];
int Text_EncodeBase32(char *out, unsigned int value);
int Text_DecodeBase32(unsigned int value);
int Text_ParseBase32Id(signed char *name);
int Text_ParseMapNumber(signed char *name);

#endif
