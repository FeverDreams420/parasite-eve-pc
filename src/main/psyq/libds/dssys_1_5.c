/* Psy-Q LIBDS DSSYS_1.OBJ, part 5 of 10: LIBDS_DSSYS_1_text_8B8, LIBDS_DSSYS_1_text_A9C, LIBDS_DSSYS_1_text_D24. */

#include "common.h"
#include "pe1/psyq_cd.h"
#include "pe1/psyq_ds.h"

#include "dssys_1_5_externs.h"

/* Contiguous private routines from Psy-Q LIBDS/DSSYS_1.OBJ:
 * text_8B8 is at 0x80080220 and text_A9C follows at 0x80080404.
 * text_4A4 is non-contiguous and remains in dssys1_vsync.c. */
/* Psy-Q LIBDS/DSSYS_1.OBJ private text_8B8. */
#define STATE_FROM_MODE(p)                                                             \
    ((CdRomEventCommandState *)((u8 *)(p) - PE1_OFFSETOF(CdRomEventCommandState,       \
                                                         command.read.commandMode)))
#define STATE_FROM_RETRY(p)                                                            \
    ((CdRomEventCommandState *)((u8 *)(p) - PE1_OFFSETOF(CdRomEventCommandState,       \
                                                         command.read.retryCount)))
#define STATE_FROM_PARAM(p)                                                            \
    ((CdRomEventCommandState *)((u8 *)(p) - PE1_OFFSETOF(CdRomEventCommandState,       \
                                                         command.read.commandParam)))

#define STATE(p)                                                                       \
    ((CdRomCommandState *)((u8 *)(p) - PE1_OFFSETOF(CdRomCommandState, read.command)))
#define SYSTEM(p)                                                                      \
    ((CdRomSystemState *)((u8 *)(p) -                                                  \
                          CDROM_SYSTEM_READ_COMMAND_OFFSET))

void LIBDS_DSSYS_1_text_8B8(int inEvent, u8 *inResult) {
    u8 *commandByte;
    int event = inEvent;
    u8 *result = inResult;
    int masked = (u8)event;
    int two;
    two = 2;
    if (masked == two) {
        CdRomEventCommandState *state = &g_CdRomEventCommandState;
        int pending;
        int command;
        int value;
        pending = state->pendingCommand;
        value = 14;
        command = (u8)pending;
        if (command == value) {
            if ((state->command.read.commandMode ^ state->pendingParamBytes[0]) & 128) {
                state->command.read.command = 15;
                state->command.read.status = two;
                state->command.read.syncResult = 3;
            } else {
                state->command.read.status = 1;
                state->command.read.command = 11;
            }
            {
                commandByte = &g_CdRomEventCommandState.command.read.commandMode;
                *commandByte = STATE_FROM_MODE(commandByte)->pendingParamBytes[0];
            }
            goto dispatch;
        }
        if (command == 3) {
            state->command.read.command = 16;
            state->command.read.status = two;
            state->command.read.commandParam = pending;
            state->command.reserved34 = 1200;
            goto dispatch;
        } else if (command == 6 || command == 27) {
            state->command.read.command = 17;
            state->command.read.status = two;
            state->command.read.commandParam = pending;
            state->command.reserved34 = 1200;
            goto dispatch;
        }
        switch (state->pendingCommand) {
        case 2:
            g_CdCurPosPtr = *(CdlLOC *)g_CdRomEventCommandState.pendingParamBytes;
            break;
        case 21:
        case 22: {
            commandByte = &g_CdRomEventCommandState.command.read.retryCount;
            *commandByte = STATE_FROM_RETRY(commandByte)->pendingCommand;
            break;
        }
        case 3:
        case 6:
        case 27: {
            commandByte = &g_CdRomEventCommandState.command.read.commandParam;
            *commandByte = STATE_FROM_PARAM(commandByte)->pendingCommand;
            break;
        }
        }
        {
            DsReadStatusBlock *read = &g_DsReadStatusBlock;
            read->status = 1;
            read->command = 11;
        }
    } else {
        CdRomCommandState *state = &g_CdSeekState;
        if (state->eventStatus & 16) {
            state->read.status = two;
            state->read.command = 12;
        } else {
            state->read.status = 1;
            state->read.command = 11;
        }
    }
dispatch:
    if (g_DsSyncCallback && g_DsReadSysEnabled.enabled) {
        DsEventCallback cb = g_DsSyncCallback;
        cb(event, result);
    }
}

void LIBDS_DSSYS_1_text_A9C(int event, u8 *inResult) {
    u8 *result = inResult;
    u32 *command = &D_8009B578;
    int twelve;
    int old;
    int two;
    DsEventCallback callback;
    int callbackEvent;
    old = *command;
    twelve = 12;
    if (old == twelve)
        *command = 13;
    {
        int masked = (u8)event;
        two = 2;
        if (masked == two) {
            int current = *command;
            u32 range;
            if (current == 13) {
                if (!(STATE(command)->eventStatus & 16)) {
                    *command = 14;
                    STATE(command)->read.status = two;
                    STATE(command)->read.sector = 21;
                    STATE(command)->read.discType++;
                }
                return;
            } else {
                if (current == 14) {
                    int step = STATE(command)->read.sector;
                    int next;
                    if (step == 21) {
                        if (!(STATE(command)->eventStatus & 16)) {
                            STATE(command)->read.status = two;
                            *command = current;
                            STATE(command)->read.sector = 22;
                            STATE(command)->read.reserved18 = 0;
                        }
                    } else if (step == 22) {
                        if (STATE(command)->eventStatus & 2) {
                            next = 23;
                            goto advance;
                        }
                        if ((int)STATE(command)->read.reserved18 >= 301) {
                            STATE(command)->read.status = 3;
                            if (g_DsDispatchCallback) {
                                callbackEvent = 5;
                                callback = g_DsDispatchCallback;
                                goto notify;
                            }
                        }
                    } else if (step == 23) {
                        next = 24;
                    advance:
                        STATE(command)->read.status = two;
                        *command = current;
                        STATE(command)->read.sector = next;
                    } else if (step == 24) {
                        if (STATE(command)->eventStatus == two) {
                            DsEventCallback available = g_DsDispatchCallback;
                            int value = 1;
                            STATE(command)->read.status = value;
                            value = 11;
                            *command = value;
                            STATE(command)->read.sector = 0;
                            if (available) {
                                callbackEvent = 2;
                                callback = g_DsDispatchCallback;
                                goto notify;
                            }
                        }
                    }
                    return;
                } else {
                    range = current - 16;
                    if (range < 2) {
                        if (STATE(command)->reserved34 == 0 &&
                            !(STATE(command)->eventStatus & 2)) {
                            {
                                DsEventCallback available = g_DsReadyCallback;
                                int value = 1;
                                STATE(command)->read.status = value;
                                value = 11;
                                *command = value;
                                if (available && SYSTEM(command)->enabled)
                                    g_DsReadyCallback(5, result);
                            }
                            if (g_DsSyncCallback && g_DsReadSysEnabled.enabled) {
                                callbackEvent = 5;
                                callback = g_DsSyncCallback;
                            notify:
                                callback(callbackEvent, result);
                            }
                        }
                        return;
                    }
                }
            }
        } else {
            if (STATE(command)->eventStatus & 16) {
                STATE(command)->read.status = two;
                *command = twelve;
            } else if ((*command - 16) < 2) {
                STATE(command)->read.status = 1;
                *command = 11;
            }
        }
    }
done:;
}


void LIBDS_DSSYS_1_text_D24(int event, u8 *result) {
    u32 event_reg;
    CdRomEventCommandState *cmd_state;
    int value;
    int status;
    CdRomCommandState *ready;

    event_reg = event & 0xFF;
    if (event_reg == 2) {
        cmd_state = &g_CdRomEventCommandState;
        status = cmd_state->pendingCommand;
        value = 0xE;
        if (status == value) {
            value = cmd_state->command.read.commandMode;
            status = cmd_state->pendingParamBytes[0];
            if (((value ^ status) & 0x80) != 0) {
                cmd_state->command.read.command = 0xF;
                cmd_state->command.read.status = event_reg;
                cmd_state->command.read.syncResult = 3;
            }
            cmd_state->command.read.commandMode = status;
        }
    }

    status = event & 0xFF;
    if (status == 5) {
        ready = &g_CdSeekState;
        if (ready->eventStatus & 0x10) {
            ready->read.status = 2;
            ready->read.command = 12;
            if (g_DsSyncCallback &&
                ((CdRomSystemState *)((char *)ready -
                    CDROM_SYSTEM_COMMAND_OFFSET))->enabled) {
                g_DsSyncCallback(5, result);
            }
        } else {
            ready->read.status = 1;
            ready->read.command = 11;
        }
    }
}
