#ifndef PE_PC_SCRIPT_VM_H
#define PE_PC_SCRIPT_VM_H

#include "script.h"

typedef struct PeScriptBanks {
    uint32_t variables[512], operands[16], node_flags[64];
    uint32_t move_lock, scene_flags;
} PeScriptBanks;

struct PeScriptTask;
typedef enum PeScriptDispatchResult {
    PE_DISPATCH_ERROR = -1,
    PE_DISPATCH_UNSUPPORTED = 0,
    PE_DISPATCH_CONTINUE = 1,
    PE_DISPATCH_YIELD = 2,
    PE_DISPATCH_RETRY = 3
} PeScriptDispatchResult;
typedef PeScriptDispatchResult (*PeScriptDispatch)(void *user,
    struct PeScriptTask *task, const PeScriptCommand *command);

typedef struct PeScriptTask {
    PeScriptProgram program;
    PeScriptBanks *banks;
    uint32_t locals[56], actor_flags;
    uint32_t base, pc, cursors[3];
    uint32_t target14;
    uint16_t active, flags, sequence;
    unsigned is_battle_actor; /* identity selected by the actor scheduler */
    PeScriptDispatch dispatch;
    void *dispatch_user;
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
 * Subsystem handlers receive decoded operand references. RETRY preserves PC
 * and waits one tick; YIELD advances PC (active defaults to one tick unless
 * the handler sets it). Unsupported/error handlers must not mutate state.
 * Actor/node allocation is not yet implemented. */
int pe_script_task_init(PeScriptTask *task, const PeScriptProgram *program,
                         unsigned entry, PeScriptBanks *banks);
PeScriptStep pe_script_task_tick(PeScriptTask *task, unsigned instruction_limit);
uint32_t pe_script_arg_read(PeScriptTask *task, const PeScriptArgument *arg);
void pe_script_arg_write(PeScriptTask *task, const PeScriptArgument *arg, uint32_t value);

#endif
