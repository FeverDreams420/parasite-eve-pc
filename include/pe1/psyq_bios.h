#ifndef PE1_PSYQ_BIOS_H
#define PE1_PSYQ_BIOS_H


#include <stddef.h>
#include <psyz/system.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <strings.h>
#else
#define bzero(destination, length) memset((destination), 0, (length))
#endif

/* File services used by the memory-card manager. */
int erase(char *path);

/* Program control and cache services. */
void FlushCache(void);

/* Entry-hook and automatic IRQ acknowledgement services used by LIBETC. */
void PS1_EnterCriticalSection(void);
void PS1_ExitCriticalSection(void);
#define EnterCriticalSection PS1_EnterCriticalSection
#define ExitCriticalSection PS1_ExitCriticalSection
int ResetEntryInt(void);
void ReturnFromException(void);
void HookEntryInt(void *entry);
void ChangeClearPAD(int mode);
int ChangeClearRCnt(int counter, int enabled);

void DeliverEvent(unsigned int event, unsigned int spec);

long OpenEvent(unsigned long desc, long spec, long mode, long (*func)(void));
long CloseEvent(long event);
long EnableEvent(long event);
long TestEvent(long event);
long DisableEvent(long event);
void UnDeliverEvent(unsigned int event, unsigned int spec);

/* LIBAPI root counters (counter.c). */
int SetRCnt(unsigned int counter, int target, int mode);
int GetRCnt(unsigned int counter);
int StartRCnt(unsigned int counter);
int StopRCnt(unsigned int counter);

/* LIBAPI.H descriptor, event and counter mode constants. */
#define RCntCNT2 0xF2000002
#define RCntMdINTR 0x1000
#define EvSpINT 0x0002
#define EvMdINTR 0x1000

#endif
