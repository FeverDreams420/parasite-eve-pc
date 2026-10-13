/* Psy-Q LIBPAD PADMAIN.OBJ: PadEnableCom, _padSetVsyncParam, _padChkVsync, _padStartCom, _padStopCom, _padInitSioMode, _padSioRW and their local helpers. */

#include "common.h"
#include "pe1/psyq_bios.h"
#include "pe1/psyq_pad_main.h"

#include "padmain_externs.h"
#include "padmain_externs_1.h"
#include "padmain_externs_2.h"
#include "padmain_externs_3.h"

int PadEnableCom(int portMask) {
    int currentMask;
    int timerValue;
    int *timer1;
    int *timer2;
    currentMask = (g_MemCardPort2Present << 1)
        | (g_MemCardPort1Present == 0);
    if (currentMask != portMask) {
        g_MemCardServiceReady = 0;

        if (portMask & 1) {
            timer1 = &D_800A5AC0[0];
            timerValue = *timer1;
            g_MemCardPort1Present = 0;
            if (timerValue >= 150) {
                g_MemCardObjResetFn(g_MemCardObjArray);
            }
            *timer1 = 0;
        } else {
            g_MemCardPort1Present = 1;
        }

        if (portMask & 2) {
            timer2 = &D_800A5AC0[1];
            timerValue = *timer2;
            g_MemCardPort2Present = 1;
            if (timerValue >= 150) {
                g_MemCardObjResetFn(g_MemCardObjArray + 1);
            }
            *timer2 = 0;
        } else {
            g_MemCardPort2Present = 0;
        }

        g_MemCardServiceReady = 1;
    }
    return currentMask;
}

int MemCard_TimerCallback(void);
int MemCard_TimerReadyCallback(void);

void _padSetVsyncParam(void) {
    void **table;

    table = D_800A5AB4;
    table[0] = MemCard_TimerCallback;
    table[1] = MemCard_TimerReadyCallback;
    table[-1] = 0;
    table[2] = 0;
}

void SysDeqIntRP(int index, void *queue);
void SysEnqIntRP(int index, void *queue);

int _padInitSioMode(CardObj *port);

int MemCard_TimerReadyCallback(void) {
    MemCardInterruptRegisters *state = g_MemCardState;

    if ((state->mask & 1) == 0) {
        return 0;
    }
    if ((state->status & 1) == 0) {
        return 0;
    }
    if (g_PadMainHook != 0) {
        g_PadMainHook();
    }
    return 1;
}

int MemCard_TimerCallback(void) {
    int index;
    int limit;
    int *timer;
    void *obj;
    int active;
    int one;

    active = g_MemCardPort1Present;
    one = 1;
    g_MemCardCallbackPending = one;
    if (active != 0) {
        timer = &D_800A5AC0[0];
        active = *timer;
        if (active < 0x96) {
            one = active + 1;
            *timer = one;
        }
    }

    if (g_MemCardPort2Present == 0) {
        timer = &D_800A5AC0[1];
        active = *timer;
        if (active < 0x96) {
            one = active + 1;
            *timer = one;
        }
    }

    if (g_MemCardServiceReady != 0) {
        index = g_MemCardPort1Present;
        limit = g_MemCardPort2Present;
        if (limit >= index) {
            obj = (void *)((int)g_MemCardObjArray + (((index << 4) - index) << 4));
            g_MemCardCommandStep = 0;
            g_PadActivePortIndex = index;
            if (_padInitSioMode(obj) == 0) {
                g_PadDmaCompleteFn(0xFFFF);
            }

            g_PadActuatorCostTotal = 0;
            while (g_MemCardPort2Present >= g_PadActivePortIndex) {
                index = g_PadActivePortIndex;
                obj = (void *)((int)g_MemCardObjArray + (((index << 4) - index) << 4));
                MemCard_RunCommandStep(obj);
            }
            g_MemCardSioRegs->baud = 0x88;
        }
    }

    return 0;
}

int _padChkVsync(void) {
    int old = g_MemCardCallbackPending;

    g_MemCardCallbackPending = 0;
    return old;
}

/* SDK _padStartCom (PADMAIN.OBJ). Constraints are tracked in crutch debt. */
void _padStartCom(void) {
    g_MemCardServiceReady = 0;
    EnterCriticalSection();
    SysDeqIntRP(2, g_PadVsyncIntQueueNode);
    SysEnqIntRP(2, g_PadVsyncIntQueueNode);
    D_8009B784->status = -2;
    D_8009B784->mask |= 1;
    ChangeClearRCnt(3, 0);
    ExitCriticalSection();
    g_MemCardObjResetFn(g_MemCardObjArray);
    g_MemCardObjResetFn(g_MemCardObjArray + 1);
    {
        int *clear = D_800A5AC0;
        clear[1] = 0;
        clear[0] = 0;
    }
    g_MemCardServiceReady = 1;
    return;
}

void _padStopCom(void) {
    EnterCriticalSection();
    ChangeClearRCnt(3, 1);
    SysDeqIntRP(2, g_PadVsyncIntQueueNode);
    ExitCriticalSection();
}

void MemCard_WaitStatusBit2(void);
int _padInitSioMode(CardObj *port) {
    MemCardSioRegisters *sio = D_8009B788;
    sio->control = 64;
    sio->control = 0;
    sio->mode = 13;
    sio->baud = 136;
    setRC2wait(port->currentId == 8 ? 80 : 145);
    {
        int index = g_PadActivePortIndex;
        {
            MemCardSioRegisters *regs = D_8009B788;
            int control = 0x1003;
            if (index)
                control = 0x3003;
            regs->control = control;
        }
        {
            int *address = g_MemCardChannelCounts + index;
            int remaining = *address;
            if (remaining >= 0) {
                if (remaining > 0) {
                    int *counts = g_MemCardChannelCounts;
                    do {
                        CardObjAddress count;
                        count.count = counts;
                        count.word = (g_PadActivePortIndex * 4) + count.word;
                        (*count.count)--;
                        g_MemCardResponseHandler(port->subPorts + *count.count);
                    } while (counts[g_PadActivePortIndex] > 0);
                }
                {
                    int index = g_PadActivePortIndex;
                    int *table;
                    int *count;
                    table = g_MemCardChannelCounts;
                    count = &table[index];
                    if (*count == 0) {
                        int value = -1;
                        CardObj *arg = port;
                        void (*callback)(CardObj *);
                        callback = g_MemCardResponseHandler;
                        *count = value;
                        callback(arg);
                        g_CardSwapByteFieldFn(port);
                    }
                }
            }
        }
    }
    sio = D_8009B788;
    if (sio->status & 0x200) {
        {
            int control = sio->control;
            sio->control = control | 16;
        }
        if (sio->status & 0x200) {
            sio->data = 1;
            MemCard_WaitStatusBit2();
            (void)D_8009B788->data;
            return 0;
        }
        D_8009B784->status = ~128;
    }
    if (port->pad_50[0] && port->command)
        return 0;
    *port->response_3c = 0;
    return 1;
}

/* Adjacent PADMAIN.OBJ state-machine and SIO exchange functions.
 * Empty constraints are tracked in crutch debt. */
void MemCard_RunCommandStep(CardObj *port) {
    int *index = &g_MemCardCommandStep;
    int count;
    int result;
    int (*next)(void);
    count = *index;
    next = g_MemCardCommandStepHandlers[count];
    *index = count + 1;
    result = next();
    if (result >= 0) {
        if (g_MemCardCommandStep) {
            setRC2wait(60);
            if (!MemCard_WaitReadyForTransfer())
                g_PadDmaCompleteFn(-3);
        }
        if (g_MemCardCommandStep >= 5)
            g_MemCardCommandStep--;
    } else
        g_PadDmaCompleteFn(result);
}

s32 _padSioRW(CardObj *inObj, s32 inByte) {
    CardObj *port = inObj;
    s32 outgoing = inByte;
    u8 received;
    s32 deviceId;
    u32 baud;
    s32 replyByte;
    s32 result;

    if (outgoing < 0) {
        received = D_8009B788->data;
        port->response_index = 0xFF;
        port->payload_index = 1;
        *port->statusBuffer = ~outgoing;
        replyByte = received;
        if (!(D_8009B788->status & 1)) {
            do {

            } while (!(D_8009B788->status & 1));
        }
        do {

        } while (chkRC2wait() == 0);
        result = ~outgoing;
        D_8009B788->data = result;
        result = replyByte;
        return result;
    }
    {
        int expected;
        {
            u8 *response = port->response_3c;
            deviceId = *response;
        }
        expected = 8;
        baud = 0x88;
        if ((deviceId >> 4) == expected) {
            if ((u8)port->response_index >= 9U) {
                baud = 0x22;
            }
        }
    }
    {
        MemCardSioRegisters *registers = D_8009B788;
        u32 timer = *(volatile u16 *)0x1F801120;
        u32 ready = registers->status;
        MemCardSioRegisters *cursor;
        g_TimerTimeoutLimit = 0x1AE;
        g_TimerTimeoutStart = timer;
        if (!(ready & 2)) {
            cursor = registers;
            do {
            } while (!(cursor->status & 2));
        } else
            cursor = registers;
    }
    {
        MemCardSioRegisters *sio = D_8009B788;
        MemCardInterruptRegisters *irq = D_8009B784;
        received = sio->data;
        replyByte = received;
        sio->baud = baud;
        if (!(irq->status & 0x80)) {
            do {
                if (chkRC2wait() != 0) {
                    result = -20;
                    return result;
                }
            } while ((D_8009B784->status & 0x80) == 0);
        }
        D_8009B788->data = (u8)outgoing;
        port->payload_index += 1;
        port->response_3c[port->response_index] = replyByte;
        result = port->response_index;
        result++;
        port->response_index = result;
        result = replyByte;
    }
done:
    return result;
}

int MemCard_WriteByte(CardObj *obj, int value)
{
    MemCardSioRegisters *sio;
    MemCardInterruptRegisters *irq;
    volatile u16 *counter;
    volatile u16 *target;
    volatile u16 *mode;
    u32 start;
    u32 limit;
    u32 ticks;
    u32 wrap;
    int baud;
    int received;
    int first_byte;
    u16 poll_status;

    first_byte = obj->response_3c[0];
    baud = 0x88;
    if ((first_byte >> 4) == 8 && obj->response_index >= 9)
        baud = 0x22;

    {
        MemCardSioRegisters *ready_sio = D_8009B788;
        do {
            poll_status = ready_sio->status;
        } while ((poll_status & 2) == 0);
    }
    setRC2wait(400);

    sio = D_8009B788;
    received = sio->data;
    if (obj->response_index != 0 || (received >> 4) != 8) {
    sio->baud = baud;
    } else {
    sio->baud = 0x22;
    }

    irq = D_8009B784;
    if (!(irq->status & 0x80)) {
        MemCardInterruptRegisters *poll_irq;
        counter = (volatile u16 *)0x1F801120;
        target = (volatile u16 *)0x1F801128;
        wrap = 0x10000;
        mode = (volatile u16 *)0x1F801124;
        poll_irq = irq;
        start = g_TimerTimeoutStart;
        limit = g_TimerTimeoutLimit;
        do {
            ticks = *counter;
            if (ticks < start) {
                if (*target != 0)
                    ticks += *target;
                else
                    ticks += wrap;
            }
            if (*mode & 0x200) {
                if (ticks - start >= limit)
                    return -2;
            }
            if (((ticks - start) >> 3) >= limit)
                return -2;
        } while (!(poll_irq->status & 0x80));
    }

    if (obj->currentId != 8 && g_MemCardCommandStep == 2) {
        setRC2wait(60);
        while (!chkRC2wait()) { }
    }

    D_8009B788->data = value;
    obj->payload_index++;
    if (obj->response_index != 0xFF) {
        /* The response cursor is read again after the payload update. */
        obj->response_3c[obj->response_index] = received;
    }
    obj->response_index++;
    return received;
}

int MemCard_WaitReadyForTransfer(void)
{
  MemCardInterruptRegisters *state;
  MemCardSioRegisters *regs;
  MemCardSioRegisters *check_regs;
  int value;
  unsigned char status;
  state = g_MemCardState;
  regs = g_MemCardSioRegs;
  value = -0x81;
  state->status = value;
  status = regs->status;
  status &= 0x80;
  while (status != 0)
  {
    if (chkRC2wait() != 0)
    {
      return 0;
    }
    check_regs = g_MemCardSioRegs;
    status = check_regs->status;
    status &= 0x80;
  }
  regs = g_MemCardSioRegs;

  regs->control |= 0x10;
  return 1;
}

void MemCard_WaitStatusBit2(void) {
    MemCardSioRegisters *ptr = g_MemCardSioRegs;

    /* Keep the poll loop target on the lhu, not the load-delay nop. */
    while ((ptr->status & 2) == 0) {
    }
}
