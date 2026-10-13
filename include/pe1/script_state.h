#ifndef PE1_SCRIPT_STATE_H
#define PE1_SCRIPT_STATE_H

#include "common.h"
#include "pe1/script_node.h"

/* Task script interpreter state and the tables its opcodes read
 * (src/main/script). */

extern int g_ScriptCursorAddress;
extern short g_EntityCollisionWallParam;
extern ScriptNode *g_ScriptNodeFreeListHead;
/* Sequence number stamped into each allocated script node (a one-element
 * array: small data in the -G8 pool unit). */
extern unsigned short g_ScriptNodeSeqCounter[1];
extern u32 g_ScriptOperandTable[16];
extern u16 g_SceneInitLbaTbl0[];
extern u16 g_SceneInitLbaTbl1[];
extern u16 g_SceneAssetLbaTbl[];
extern unsigned char g_BattleEnemyEscapeFlag;

/* Sound commands an actor script queued while deferral was on
 * (Akao_ProcessCommand ops 0x196/0x197). */
typedef struct DeferredAkaoCommand {
    u8 actor_type;
    u8 actor_id;
    s8 command;
    s8 arg1;
    s16 arg2;
    s16 arg3;
} DeferredAkaoCommand;

extern DeferredAkaoCommand g_DeferredAkaoCommands[16];

u32 Draw_FlushPrimQueue(void);
void Script_SignalExit(void);

#endif /* PE1_SCRIPT_STATE_H */
