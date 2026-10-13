#ifndef PE1_ASSET_FIND_H
#define PE1_ASSET_FIND_H

#include "pe1/asset_table.h"

/* Positional effect and sound triggers looked up by asset id
 * (cdrom/scene_assets.c). Callers pass the coordinates as ints;
 * the definitions narrow them to halfwords. */
int Asset_Find08w(int id, int mode, int x, int y, int z);
int Asset_Find08Alt(int id, int mode, int x, int y, int z);

#endif
