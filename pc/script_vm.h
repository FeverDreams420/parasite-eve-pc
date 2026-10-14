#ifndef PE_PC_SCRIPT_VM_H
#define PE_PC_SCRIPT_VM_H

#include "script.h"

typedef struct PeScriptBanks {
    uint32_t variables[512], operands[16], node_flags[64];
    uint32_t move_lock;
} PeScriptBanks;

typedef struct PeScriptTask {
    PeScriptProgram program;
    PeScriptBanks *banks;
    uint32_t locals[56], actor_flags;
    uint32_t base, pc, cursors[3];
    uint16_t active, flags;
    uint64_t ticks, instructions;
    char error[256];
} PeScriptTask;

typedef enum PeScriptStep {
    PE_SCRIPT_ERROR = -1,
    PE_SCRIPT_WAIT = 0,
    PE_SCRIPT_STOPPED = 1
} PeScriptStep;

/* One native task. Banks are shared across tasks, literals write back to the
 * room's owned mutable script bytes, matching original pointer operands.
 * Actor/node allocation and external subsystems are not yet dispatched. */
int pe_script_task_init(PeScriptTask *task, const PeScriptProgram *program,
                         unsigned entry, PeScriptBanks *banks);
PeScriptStep pe_script_task_tick(PeScriptTask *task, unsigned instruction_limit);

#endif
