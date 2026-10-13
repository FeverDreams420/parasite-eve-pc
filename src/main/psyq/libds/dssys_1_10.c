/* ASSEMBLER: GNU */
/* GCC_VERSION: 2.8.1 */
/* CC1_FLAGS: -mno-split-addresses */
/* Psy-Q LIBDS DSSYS_1.OBJ, part 10 of 10: tipDsSystem and the object's zero tail. */

#include "pe1/psyq_bios.h"
#include "pe1/psyq_ds.h"

#include "dssys_1_10_externs.h"

void tipDsSystem(void) {
    enum {
        PARAM_OFFSET = PE1_OFFSETOF(CdRomEventCommandState, pendingParamBytes) -
                       PE1_OFFSETOF(CdRomEventCommandState, command.read)
    };
    DsReadStatusBlock *state;
    CdRomEventCommandState *command;
    int b0;
    int b1;
    int b2;
    int b3;
    int b4;
    char *busy;

    printf(g_DsStatusHeaderText);

    state = &g_DsReadStatusBlock;
    printf(g_DsCommandStatusFormat, state->status, state->command, state->sector);
    command = (CdRomEventCommandState *)((char *)state -
        PE1_OFFSETOF(CdRomEventCommandState, command.read));
    b0 = command->pendingCommand;
    b1 = ((u_char *)state)[PARAM_OFFSET + 0];
    b2 = ((u_char *)state)[PARAM_OFFSET + 1];
    b3 = ((u_char *)state)[PARAM_OFFSET + 2];
    b4 = ((u_char *)state)[PARAM_OFFSET + 3];
    printf(g_DsLastCommandFormat, b0, b1, b2, b3, b4);
    printf(g_DsCommandRemainFormat, state->readyResult);
    printf(g_DsSystemTimerFormat, state->syncResult);
    {
        DsCallbackValue poll, sync, ready;
        poll.poll = g_DsPollCallback;
        sync.event = g_DsSyncCallback;
        ready.event = g_DsReadyCallback;
        printf(g_DsCallbacksFormat, poll.word, sync.word, ready.word);
    }

    {
        int active = ER_active();
        busy = g_DsNoneText;
        if (active != 0) busy = g_DsActiveText;
    }
    printf(g_DsReadySystemFormat, busy);
}

unsigned int LIBDS_DSSYS_1_pad[] __attribute__((section(".text"))) = {
    0x00000000,
    0x00000000,
};
