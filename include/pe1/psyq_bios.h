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

#define PE1_STRINGIFY_INNER(value) #value
#define PE1_STRINGIFY(value) PE1_STRINGIFY_INNER(value)

/* PSY-Q's assembled BIOS veneers preserve all arguments and tail-call the
 * selected A0/B0/C0 service through the documented t2/t1 ABI. */
#define PSYQ_BIOS_TRAMPOLINE(name, table, selector) \
    __asm__( \
        ".section .text, \"ax\"\n" \
        ".set noreorder\n" \
        ".globl " #name "\n" \
        ".type " #name ", @function\n" \
        #name ":\n" \
        ".ent " #name "\n" \
        "addiu $10, $0, " PE1_STRINGIFY(table) "\n" \
        "jr $10\n" \
        "addiu $9, $0, " PE1_STRINGIFY(selector) "\n" \
        ".size " #name ", . - " #name "\n" \
        ".end " #name "\n" \
        ".set reorder\n")

/* Critical-section BIOS ABI: selector in a0, result in v0. The caller
 * binds these operands to $4/$2; C owns selector setup and the return.
 * This is a legacy hardware ABI, replaced by native services in the port. */
#define PSYQ_BIOS_SYSCALL(result, selector) \
    __asm__ volatile("syscall" : "=r"(result), "=r"(selector) \
                     : "1"(selector) : "memory")

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
