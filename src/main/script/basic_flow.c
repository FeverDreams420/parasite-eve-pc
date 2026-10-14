#include "pe1/script_queue.h"

extern FieldActor *g_CurrentEntity;
ScriptCursorValue g_ScriptCursorValue;

int Script_JumpToEntityOffset(int **arg0)
{
    g_ScriptCursor = (u32 *)(g_CurrentEntity->script_base + (*arg0[0] * 2));
    return 1;
}

int Entity_MarkCurrentDead(int **unused) {
    (void)unused;
    g_CurrentEntity->flags |= 0x10;
    return 0;
}

int Entity_SetCurrentNodeActive(int **arg0) {
    g_ScriptNodePool->active = (u16)*arg0[0];
    return 0;
}

int Entity_MarkCurrentNodeFree(int **unused) {
    (void)unused;
    g_ScriptNodePool->flags |= 0x10;
    return 0;
}

int Script_AssignValue(int **arg0) {
    *arg0[0] = *arg0[1];
    return 1;
}
