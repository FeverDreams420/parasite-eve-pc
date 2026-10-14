#ifndef PE_PC_SCRIPT_SCHEDULER_H
#define PE_PC_SCRIPT_SCHEDULER_H

#include "script_vm.h"

#define PE_SCRIPT_TASK_CAPACITY 72u

typedef struct PeScriptSlot {
    PeScriptTask task;
    int next, previous;
    unsigned group, used;
} PeScriptSlot;

typedef struct PeScriptScheduler {
    PeScriptProgram program;
    PeScriptBanks *banks;
    PeScriptSlot slots[PE_SCRIPT_TASK_CAPACITY];
    int free_next[PE_SCRIPT_TASK_CAPACITY], free_head;
    uint16_t sequence;
    unsigned live;
    PeScriptDispatch dispatch;
    void *dispatch_user;
    char error[256];
} PeScriptScheduler;

/* Scheduler and actors stay at stable addresses until their tasks are freed.
 * The caller owns actor creation/order and calls tick once per actor per game
 * tick. Three callback buckets and deferred collection follow the original. */
void pe_script_scheduler_init(PeScriptScheduler *scheduler,
                               const PeScriptProgram *program, PeScriptBanks *banks);
int pe_script_scheduler_start(PeScriptScheduler *scheduler, PeScriptActor *actor,
                               unsigned group, uint32_t offset);
int pe_script_scheduler_tick(PeScriptScheduler *scheduler, PeScriptActor *actor,
                              unsigned instruction_limit);
int pe_script_scheduler_collect(PeScriptScheduler *scheduler, PeScriptActor *actor);
/* Drain events after actor callbacks; matches type/sub-ID or low-byte selector.
 * Validate all matches/capacity before allocating any task. */
int pe_script_scheduler_events(PeScriptScheduler *scheduler, PeScriptActor *actors, unsigned count);

#endif
