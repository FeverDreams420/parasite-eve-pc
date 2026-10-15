#ifndef PE1_SAVE_SLOT_METADATA_H
#define PE1_SAVE_SLOT_METADATA_H

#include "common.h"

/*
 * Save prompt state word at 0x8009D1AC: the low byte counts frames down,
 * bits 8-9 hold the phase, bits 10-11 the prompt kind and bits 12-18 one
 * pending message each. Retail reaches the frame counter through the same
 * object as the word (it reloads the word after a counter store), so both
 * views share one union, following the native target pointer.
 */
typedef union SavePromptState {
    u32 word;
    u8 timer;
} SavePromptState;

struct BattleEntity;
typedef struct SavePromptBlock {
    struct BattleEntity *target; /* Scan target, whose first field is the enemy core. */
    SavePromptState prompt;    /* 0x8009D1AC */
} SavePromptBlock;
/* g_BattleActiveEntity (the Scan target, PE_SPELL_SCAN in battle/attack.c)
 * and g_BattleResultFlags as the record the Scan HP panel reads. */
extern SavePromptBlock g_BattleScanPanel;
#define g_BattleActiveEntity (g_BattleScanPanel.target)
#define g_BattleResultFlags (g_BattleScanPanel.prompt.word)

/* Prompt texts: two fixed lines, then per-window tables. */
extern u8 g_AnalyzeHpUnknownText[];
extern u8 g_AnalyzeHpFormatText[];
extern u8 g_TextSusceptibleToPoison[][22];
extern u8 g_TextSusceptibleToAcid[][20];
extern u8 g_AnalyzeTranquilizerWeakTexts[][20];
extern u8 g_AnalyzeHeatWeakTexts[][18];
extern u8 g_AnalyzeColdWeakTexts[][18];
extern u8 g_AnalyzeStealWeakTexts[][22];
extern u8 g_AnalyzeConfusionWeakTexts[][21];

#endif
