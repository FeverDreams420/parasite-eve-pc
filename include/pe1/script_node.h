#ifndef PE1_SCRIPT_NODE_H
#define PE1_SCRIPT_NODE_H

/* Task/scene node: elements of the free list (g_ScriptNodeFreeListHead), the current-node
 * pointer (g_ScriptNodePool @ 0x8009D300), and the per-entity bucket lists
 * (FieldActor.script_node_lists @ 0xA0). Doubly-linked via next/prev. Size 0x2C. */
typedef union ScriptNodeTarget18 {
    int coordinate;
    void *actor;
} ScriptNodeTarget18;

/* A node carries either an event value or an interpreter script cursor. */
typedef union ScriptNodeCurrent {
    int value;
    unsigned int *script;
} ScriptNodeCurrent;

typedef struct ScriptNode {
    /* 0x00 */ ScriptNodeCurrent current;
    /* 0x04 */ int next_value;
    /* 0x08 */ unsigned short flags;   /* |= 0x10 marks node free */
    /* 0x0A */ unsigned short seq;      /* sequence/id, matched in Entity_FindNodeById */
    /* 0x0C */ int trigger_value;    /* event task value, or the contacting actor's field_sfx_id */
    /* 0x10 */ int active;              /* set 1 on alloc/yield */
    /* 0x14 */ int target14;
    /* 0x18 */ ScriptNodeTarget18 target18;
    /* 0x1C */ int target1c;
    /* 0x20 */ int target20;   /* fourth operand slot, cleared when a move-to task starts */
    /* 0x24 */ struct ScriptNode *next;
    /* 0x28 */ struct ScriptNode *prev;
} ScriptNode;

/* A node-pool address with byte offsets added to it word by word. */
typedef union ScriptNodeAddress {
    ScriptNode *node;
    int *words;
    unsigned int word;
} ScriptNodeAddress;

/* The node the task interpreter is currently running. */
extern ScriptNode *g_ScriptNodePool;
extern ScriptNode g_ScriptNodes[72];

ScriptNode *Script_AllocNode(unsigned int *script, ScriptNode *previous);
void Script_InitNodePool(void);
void Script_InitNodeFreeList(void);
void Script_GarbageCollectNodes(void);

/* One word per script node, cleared when the pools are reset. */
extern int g_ScriptNodeActiveFlags[64];

#endif /* PE1_SCRIPT_NODE_H */
