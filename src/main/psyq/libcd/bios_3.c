/* ASSEMBLER: GNU */
/* GCC_VERSION: 2.8.1 */
/* CC1_FLAGS: -fno-expensive-optimizations -mno-split-addresses */
/* Psy-Q LIBCD BIOS.OBJ, part 3 of 7: CD_cw, CD_vol, CD_flush. */

#include "bios_internal.h"

/* Separate captures preserve stock-compiler argument lifetimes. */
int CD_cw(int command, void *parameters, u8 *result, int mode)
{
  u8 *payload;
  u8 *parameter = parameters;
  int opcode;
  int i;

  opcode = command;
  if (CD_debug >= 2)
  {
    printf(g_CdCommandTraceFormat, g_CdCommandNames[(u8) opcode]);
  }
  payload = parameter;
  i = 0;
  if (g_CdCommandTables.parameter_counts[(u8) opcode] && (!payload))
  {
    if (CD_debug > 0)
    {
      printf(g_CdNoParamFormat, g_CdCommandNames[(u8) opcode]);
    }
    return -2;
  }
  CD_sync(i, 0);

  if (((u8) opcode) == 2)
  {
    for (i = 0; i < 4; i++)
    {
      ((u8 *)&g_CdLastPos)[i] = payload[i];
    }

  }
  if (((u8) opcode) == 14)
  {
    g_CdMode = payload[0];
  }
  {
    CdCommandTables *tables = &g_CdCommandTables;
    g_CdInterruptEvents.sync = 0;
    if (tables->ready_flags[(u8) opcode])
    {
      g_CdInterruptEvents.ready = 0;
    }
    *g_CdRegIndexBase = 0;
    {
      int *counts = tables->parameter_counts;
      i = 0;
      if (counts[(u8) opcode] > 0)
      {
        int *count = &counts[(u8) opcode];
        do
        {
          *g_CdRegDataWrite = payload[i];
        }
        while ((++i) < (*count));
      }
    }
  }
  g_CdLastCom = opcode;
  *g_CdRegPort1 = opcode;
  if (mode)
  {
    return 0;
  }
  g_CdTimeoutDeadlineVsync = VSync(-1) + 0x3C0;
  g_CdTimeoutPollCount = 0;
  g_CdTimeoutFuncName = g_CdCwName;
  if (!g_CdInterruptEvents.sync)
  {
    const char **commands = g_CdCommandNames;
    const char **events = g_CdInterruptNames;
    u8 *sync = (u8 *) (&g_CdInterruptEvents);
    u8 *ready = (u8 *)&((CdInterruptEvents *)sync)->ready;
    do
    {
      if (timed_out(commands, events, (CdInterruptEvents *)sync))
      {
        return -1;
      }
      if (CheckCallback())
      {
        dispatch_interrupts(sync, ready);
      }
    }
    while (!(*((volatile u8 *) sync)));
  }
  copy_result(result, g_CdSyncResult);
  {
    unsigned long status = 0;
    unsigned short event = g_CdInterruptEvents.sync;
    if (event == 5)
    {
      status = -1;
    }
    return status;
  }
}

int CD_vol(CdlATV *vol) {
    *g_CdRegIndexBase = 2;
    *g_CdRegDataWrite = vol->val0;
    *g_CdRegResponse = vol->val1;
    *g_CdRegIndexBase = 3;
    *g_CdRegPort1 = vol->val2;
    *g_CdRegDataWrite = vol->val3;
    *g_CdRegResponse = 0x20;
    return 0;
}

void CD_flush(void) {
    CdInterruptEvents *state;

    *g_CdRegIndexBase = 1;
    while (*g_CdRegResponse & 7) {
        *g_CdRegIndexBase = 1;
        *g_CdRegResponse = 7;
        *g_CdRegDataWrite = 7;
    }
    state = &g_CdInterruptEvents;
    state->end = 0;
    state->ready = state->end;
    {
        volatile u8 *index = g_CdRegIndexBase;
        state->sync = 2;
        *index = 0;
    }
    *g_CdRegResponse = 0;
    *g_CdRegRequest = 0x1325;
}
