#include "pe1/pm.h"

/* Original script-to-process command wrappers from system_opcodes.c. */
typedef struct WrapperArgs {
    int *arg0;
    int *arg2;
    int *arg3;
    int *arg4;
    int *arg5;
} WrapperArgs;

int Pm_ScriptSendCommand0(WrapperArgs *args) {
    int *ptr0;
    int *ptr2;
    int *ptr4;
    int value4;
    int value0;
    int value2;

    ptr0 = args->arg0;
    ptr4 = args->arg4;
    ptr2 = args->arg2;
    value4 = *ptr4;
    value0 = *ptr0;
    value2 = *ptr2;
    Pm_SendCmd(value0, 0, value2, *args->arg3, value4, *args->arg5);
    return 1;
}

int Pm_ScriptSendCommand1(int **arg0) {
    int **base;
    int *op0;
    int *op1;
    int *stack0;
    int first;
    int second;

    base = arg0;
    op0 = base[0];
    op1 = base[1];
    stack0 = base[3];
    first = *op0;
    second = *op1;

    Pm_SendCmd(first, 1, second, (intptr_t)base[2], (intptr_t)stack0, (intptr_t)base[4]);
    return 1;
}

int Pm_ScriptSetState(int **arg0) {
    Pm_SetGetState(*arg0[0], 0, *arg0[1]);
    return 1;
}

int Pm_ScriptGetState(int **arg0) {
    Pm_SetGetState(*arg0[0], 1, (intptr_t)arg0[1]);
    return 1;
}

