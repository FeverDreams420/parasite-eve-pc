#include "pe1/script_queue.h"
#include "pe1/script_state.h"
#include "pe1/player_entity.h"

extern FieldActor *g_CurrentEntity;

/* Original interpreter from flow_opcodes.c, with native actor references. */
void Script_RunQueue(void)
{
    int *args[16];
    ScriptNode *node;
    u8 *entity;
    u32 header, modes, tail, opcode;
    u32 *values;
    FieldActor *argument_entity;
    u16 i;

    do {
        node = g_ScriptNodePool;
        /* Retail first reads flags and sequence together as a word. */
        if (node->flags & 0x50) continue;
        entity = (u8 *)g_CurrentEntity;
        if ((((FieldActor *)entity)->flags & 0x1000) && !(node->flags & 0x80)) continue;
        if ((g_GameStateFlags & 0x100) && entity != (u8 *)g_PlayerEntity && !(node->flags & 0x80)) continue;
        node = g_ScriptNodePool;
        if (node->active == 0) continue;
        node->active--;
        if (node->active != 0) continue;
        g_ScriptCursor = node->current.script;
        do {
            header = *g_ScriptCursor;
            g_ScriptCursor++;
            values = g_ScriptCursor + 1;
            tail = *g_ScriptCursor;
            modes = header >> 17;
            opcode = header & 0x1FFF;
            header >>= 13;
            header &= 15;
            g_ScriptCursor = values + header;
            for (i = 0; i < header; i++) {
                switch (modes & 7) {
                case 0: args[i] = (int *)&values[i]; break;
                case 3: args[i] = (int *)&g_ScriptOperandTable[values[i]]; break;
                case 1:
                    /* Reload after every handler: it may switch actors. */
                    argument_entity = g_CurrentEntity;
                    args[i] = (int *)&argument_entity->pad_0AC[values[i]];
                    break;
                case 2: args[i] = (int *)&g_ScriptVariableBank[values[i]]; break;
                case 4: args[i] = &g_ScriptNodeActiveFlags[values[i]]; break;
                }
                modes >>= 3;
                if (i == 4) modes = tail;
            }
        } while (g_ScriptQueueOpcodeHandlers[opcode](args));
        /* Handlers may replace both the current node and the script cursor. */
        g_ScriptNodePool->current.script = g_ScriptCursor;
    } while ((g_ScriptNodePool = g_ScriptNodePool->next) != 0);
}
