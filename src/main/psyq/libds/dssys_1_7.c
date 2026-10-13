/* Psy-Q LIBDS DSSYS_1.OBJ, part 7 of 10: LIBDS_DSSYS_1_text_EA4. */

#include "common.h"
#include "pe1/psyq_cd.h"
#include "pe1/psyq_ds_queue.h"

#include "dssys_1_7_externs.h"

void LIBDS_DSSYS_1_text_EA4(int event, u8 *data) {
    u8 *data_reg;
    DsDecodedEventFlags *state;
    CdRomEventCommandState *eventState;
    u8 value;
    u8 *src;
    int index;

    data_reg = data;
    event &= 0xFF;
    if (event != 5) {

    index = g_DsCommandStatusByteIndex[g_CdRomEventCommandState.pendingCommand] - 1;
    if (index < 0) {
        return;
    }

    } else {
    index = 0;
    }
    src = data_reg + index;
    value = src[0];
    state = &g_CdRomEventCommandState.command.read.eventFlags;
    state->bit7 = value >> 7;
    state->bit6 = (value >> 6) & 1;
    state->bit5 = (value >> 5) & 1;
    state->bit1 = (value >> 1) & 1;
    eventState = (CdRomEventCommandState *)((u8 *)state -
        (PE1_OFFSETOF(CdRomEventCommandState, command) +
         PE1_OFFSETOF(CdRomCommandState, read) +
         PE1_OFFSETOF(DsReadStatusBlock, eventFlags)));
    eventState->command.eventStatus = value;
    rescpy(eventState->eventResult, data_reg);
}
