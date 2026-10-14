#include "pe1/aya.h"
#include "pe1/resident_tables.h"

ResidentDataPack g_ResidentDataPack;

void *Aya_GetStatGrowthTable(int category) {
    return &RESIDENT_TABLE(u8, statGrowth)[category << 9];
}

AyaLevelStats *Aya_LookupLevelStats(int level) {
    if (level < 0) {
        level = 0;
    } else if (level >= 0x63) {
        level = 0x62;
    }

    return &RESIDENT_TABLE(AyaLevelStats, levelStats)[level];
}

void *Aya_GetLevelExpTable(void) {
    return RESIDENT_TABLE(s32, levelExp);
}

