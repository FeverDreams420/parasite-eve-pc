/* Psy-Q LIBDS DSSYS_1.OBJ, part 4 of 10: LIBDS_DSSYS_1_text_368, LIBDS_DSSYS_1_text_4A4, LIBDS_DSSYS_1_text_774, LIBDS_DSSYS_1_text_7FC. */

#include "pe1/psyq_cd.h"
#include "pe1/psyq_ds.h"

#include "dssys_1_4_externs.h"

int LIBDS_DSSYS_1_text_368(unsigned char command, void *param) {
    CdRomEventCommandState *state;
    int *timeout;
    CD_flush();
    state = &g_CdRomEventCommandState;
    state->pendingCommand = command;
    if (param) {
        unsigned char *copy = state->pendingParamBytes;
        parcpy(copy, param);
        state->pendingParams = copy;
    } else state->pendingParams = 0;
    timeout = &g_CdRomCmdTimeout;
    CDROM_EVENT_STATE_FROM_TIMEOUT(timeout)->command.read.readyResult = g_CdRomCmdLongTimeoutTable[CDROM_EVENT_STATE_FROM_TIMEOUT(timeout)->pendingCommand] ? 960 : 30;
    CDROM_EVENT_STATE_FROM_TIMEOUT(timeout)->command.retryAttempts = 0;
    switch (CDROM_EVENT_STATE_FROM_TIMEOUT(timeout)->pendingCommand) {
    case 7:
        if (CDROM_EVENT_STATE_FROM_TIMEOUT(timeout)->command.read.eventFlags.bit1 == 1) {
            CDROM_EVENT_STATE_FROM_TIMEOUT(timeout)->pendingCommand = 1;
            CDROM_EVENT_STATE_FROM_TIMEOUT(timeout)->pendingParams = 0;
        }
        break;
    case 8:
        if (CDROM_EVENT_STATE_FROM_TIMEOUT(timeout)->command.read.eventFlags.bit1 != 1) {
            CDROM_EVENT_STATE_FROM_TIMEOUT(timeout)->pendingCommand = 1;
            CDROM_EVENT_STATE_FROM_TIMEOUT(timeout)->pendingParams = 0;
        }
        break;
    }
    {
        CdRomEventCommandState *current = &g_CdRomEventCommandState;
        if (CD_cw(current->pendingCommand, current->pendingParams, 0, 1)) {
            current->command.retryAttempts = 0;
            current->command.read.readyResult = 0;
            return 0;
        } else {
            current->command.read.lastCommand = current->pendingCommand;
            return 1;
        }
    }
}
s32 LIBDS_DSSYS_1_text_774(void);
#define READ(p)                                                                        \
    ((DsReadStatusBlock *)((u8 *)(p) - PE1_OFFSETOF(DsReadStatusBlock, syncResult)))
#define CMD(p)                                                                         \
    ((CdRomCommandState *)((u8 *)(p) -                                                 \
                           PE1_OFFSETOF(CdRomCommandState, read.syncResult)))
void LIBDS_DSSYS_1_text_4A4(void) {
    u8 parameter;
    int pending;
    int one;
    int *timer;
    int *retry = &g_CdRomCmdTimeout;
    if (*retry > 0 && --*retry == 0) {
        LIBDS_DSSYS_1_text_774();
        return;
    }
    timer = &g_DsSyncResultCountdown;
        if (*timer > 0)
        --*timer;
    if ((int)CMD(timer)->reserved34 > 0)
        --CMD(timer)->reserved34;
    one = 1;
    if (READ(timer)->status != one && READ(timer)->status == 2) {
        int state = READ(timer)->command;
        int eleven = 11;
        if (state != eleven) {
        if (state == 12) {
            if (g_CdReadCommandPollToggle) {
                u8 *arg = &parameter;
                parameter = 0;
                if (g_CdRomCmdTimeout <= 0) {
                    CMD(timer)->eventValue = 32;
                    LIBDS_DSSYS_1_text_368(14, arg);
                }
                g_CdReadCommandPollToggle = 0;
            } else {
                if (g_CdRomCmdTimeout <= 0) {
                    CMD(timer)->eventValue = 32;
                    LIBDS_DSSYS_1_text_368(1, 0);
                }
                g_CdReadCommandPollToggle = one;
            }
        } else if (state == 13) {
            pending = g_CdRomCmdTimeout;
            g_CdReadCommandPollToggle = 0;
            if (pending <= 0) {
                CMD(timer)->eventValue = 32;
                LIBDS_DSSYS_1_text_368(1, 0);
            }
        } else if (state == 14) {
            int step = READ(timer)->sector;
            if (step == 21) {
                pending = g_CdRomCmdTimeout;
                if (pending <= 0) {
                    CMD(timer)->eventValue = 32;
                    LIBDS_DSSYS_1_text_368(1, 0);
                }
            } else if (step == 22) {
                READ(timer)->reserved18++;
                if (g_CdRomCmdTimeout <= 0) {
                    CMD(timer)->eventValue = 32;
                    LIBDS_DSSYS_1_text_368(1, 0);
                }
            } else if (step == 23) {
                if (g_CdRomCmdTimeout <= 0) {
                    CMD(timer)->eventValue = 32;
                    LIBDS_DSSYS_1_text_368(19, 0);
                }
            } else if (step == 24) {
                pending = g_CdRomCmdTimeout;
                if (pending <= 0) {
                    CMD(timer)->eventValue = 32;
                    LIBDS_DSSYS_1_text_368(1, 0);
                }
            }
        } else if (state == 15) {
            if (!*timer) {
                READ(timer)->status = one;
                READ(timer)->command = eleven;
            }
        } else if (state == 16) {
            if (!READ(timer)->eventFlags.bit7) {
                pending = g_CdRomCmdTimeout;
                if (pending <= 0) {
                    CMD(timer)->eventValue = 32;
                    LIBDS_DSSYS_1_text_368(1, 0);
                }
            } else {
                READ(timer)->status = one;
                READ(timer)->command = eleven;
            }
        } else if (state == 17) {
            if (!READ(timer)->eventFlags.bit5)
                goto poll;
            READ(timer)->status = one;
        complete:
            READ(timer)->command = eleven;
        }
        goto callbacks;
    poll:
        pending = g_CdRomCmdTimeout;
    check_timeout:
        if (pending <= 0) {
    send_status:
        CMD(timer)->eventValue = 32;
        LIBDS_DSSYS_1_text_368(1, 0);
        }
    }
        }
callbacks:
    if (g_DsPollCallback && g_DsReadSysEnabled.enabled)
        g_DsPollCallback();
    {
        DsReadStatusBlock *read = &g_DsReadStatusBlock;
                if ((read->status == 1 && !read->eventFlags.bit1) || read->status == 3) {
            if (g_CdRomCmdTimeout <= 0) {
                g_CdRomCommandEventValue = 33;
                LIBDS_DSSYS_1_text_368(1, 0);
            }
        }
    }
done:;
}

s32 LIBDS_DSSYS_1_text_774(void) {
    int *retry;
    s32 value;
    s32 idx;

    CD_flush();
    retry = &g_CdRomCmdRetryState;
    value = *retry;
    idx = CDROM_EVENT_STATE_FROM_RETRY(retry)->pendingCommand;
    value += 1;
    idx <<= 2;
    *retry = value;
    {
        CdTimeoutFlagAddress at;
        at.flag = g_CdRomCmdLongTimeoutTable;
        at.word += idx;
        value = *at.flag;
    }
    idx = 30;
    if (value != 0) {
        idx = 960;
    }
    CDROM_EVENT_STATE_FROM_RETRY(retry)->command.read.readyResult = idx;
    value = CDROM_EVENT_STATE_FROM_RETRY(retry)->pendingCommand;
    CD_cw(value, CDROM_EVENT_STATE_FROM_RETRY(retry)->pendingParams, 0, 1);
    return 0;
}

void LIBDS_DSSYS_1_text_7FC(int inputEvent, void *inputResult) {
    void *result = inputResult;
    int event = inputEvent;
    int *timeout;

    LIBDS_DSSYS_1_text_EA4((unsigned char)event, result);
    /* The timeout word anchors the surrounding CD command state. */
    timeout = &g_CdRomCmdTimeout;
    CDROM_EVENT_STATE_FROM_TIMEOUT(timeout)->command.retryAttempts = 0;
    *timeout = 0;
    if (CDROM_EVENT_STATE_FROM_TIMEOUT(timeout)->command.eventStatus & 0x10) event = 5;
    switch (CDROM_EVENT_STATE_FROM_TIMEOUT(timeout)->command.eventValue) {
    case 0x1F:
        LIBDS_DSSYS_1_text_8B8((unsigned char)event, result);
        break;
    case 0x20:
        LIBDS_DSSYS_1_text_A9C((unsigned char)event, result);
        break;
    default:
        LIBDS_DSSYS_1_text_D24((unsigned char)event, result);
        break;
    }
    timeout = &g_CdRomCmdTimeout;
    if (!*timeout) CDROM_EVENT_STATE_FROM_TIMEOUT(timeout)->command.eventValue = 0x21;
}
