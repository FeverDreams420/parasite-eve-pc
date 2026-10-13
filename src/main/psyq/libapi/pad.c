/* PSY-Q LIBAPI PAD: SetInitPadFlag, ReadInitPadFlag, PAD_init, InitPAD,
 * StartPAD, StopPAD, Sys_SetInterruptHandler, RemovePatchPad and the two
 * interrupt handlers it installs. PAD_init and InitPAD instantiate
 * Pad_InitCommon.inc. */

#include "common.h"
#include "pe1/pad_internal.h"
#include "pe1/psyq_bios.h"
#include "pe1/psyq_pad_main.h"

#include "pad_externs.h"

long StartPAD2(void);
void EnablePAD(void);
int RemovePatchPad(void);
void SysDeqIntRP(int index, void *queue);
void SysEnqIntRP(int index, void *queue);
int LIBAPI_PAD_text_254(void);
int LIBAPI_PAD_text_2BC(void);

void SetInitPadFlag(int value) {
    g_InitPadFlag = value;
}

int ReadInitPadFlag(void) {
    return g_InitPadFlag;
}

#define PAD_INIT_COMMON_NAME PAD_init
#define PAD_INIT_COMMON_CALL PAD_init2
#include "Pad_InitCommon.inc"

#define PAD_INIT_COMMON_NAME InitPAD
#define PAD_INIT_COMMON_CALL InitPAD2
#include "Pad_InitCommon.inc"

long StartPAD(void) {
    StartPAD2();
    ChangeClearPAD(0);
    EnablePAD();
    return 1;
}

void StopPAD(void) {
    DisablePAD();
    StopPAD2();
    RemovePatchPad();
    g_InitPadFlag = 0;
}

int Sys_SetInterruptHandler(void) {
    void *queue;
    int (**slot)(void);
    int (**handlers)(void);
    EnterCriticalSection();
    slot = D_800A34B4;
    handlers = slot;
    queue = (char *)handlers - 4;
    slot = handlers++;
    *slot = LIBAPI_PAD_text_254;
    *handlers = LIBAPI_PAD_text_2BC;
    g_PadIntQueueNode = 0;
    D_800A34BC = 0;
    SysDeqIntRP(1, queue);
    SysEnqIntRP(1, queue);
    ExitCriticalSection();
    return 1;
}

int RemovePatchPad(void) {
    EnterCriticalSection();
    SysDeqIntRP(1, &g_PadIntQueueNode);
    ExitCriticalSection();
    return 1;
}

int LIBAPI_PAD_text_254(void) {
    volatile int delay[4];
    PadWork *work;

    work = g_PadSioBasePtr;
    work->unk_a = 0;

    delay[0] = 10;
    if (--delay[0] != -1) {
        do {
        } while (--delay[0] != -1);
    }

    return 0;
}

int LIBAPI_PAD_text_2BC(void)
{
  int *state;
  int result;
  int new_var;
  state = g_PadIntrRegsPtr;
  new_var = state[1];
  result = new_var;
  result = result & 1;
  if ((new_var & 1) == 0)
  {
    result = 0;
  }
  else if ((state[0] & 1) != 0)
  {
    new_var = 1;
    result = new_var;
  }
  else
  {
    result = (long) 0;
  }
  return result;

}
