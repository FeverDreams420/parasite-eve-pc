#ifndef PE1_AKAO_SEND_H
#define PE1_AKAO_SEND_H

/* Queues an AKAO command for a sound table (cdrom/scene_assets.c). */
int Akao_SendTableCommand(void *table, int command, int arg1, int arg2, int arg3);

/* Decoded operands of the original actor-script sound dispatcher. */
typedef struct AkaoCommandArgs {
    int *opcode;
    int *arg1;
    int *arg2;
    int *arg3;
    int *arg4;
    int *result;
} AkaoCommandArgs;

int Akao_ProcessCommand(AkaoCommandArgs *args);

#endif
