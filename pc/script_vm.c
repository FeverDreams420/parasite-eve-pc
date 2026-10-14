#include "script_vm.h"
#include <string.h>

static uint32_t *bank_word(PeScriptTask *task, const PeScriptArgument *arg) {
    switch (arg->mode) {
    case 1: return &task->locals[arg->value];
    case 2: return &task->banks->variables[arg->value];
    case 3: return &task->banks->operands[arg->value];
    case 4: return &task->banks->node_flags[arg->value];
    default: return NULL;
    }
}

uint32_t pe_script_arg_read(PeScriptTask *task, const PeScriptArgument *arg) {
    return arg->mode ? *bank_word(task, arg) : arg->value;
}

void pe_script_arg_write(PeScriptTask *task, const PeScriptArgument *arg, uint32_t value) {
    if (arg->mode) *bank_word(task, arg) = value;
    else {
        uint8_t *dest = task->program.blob->bytes + arg->offset;
        for (unsigned i = 0; i < 4; ++i) dest[i] = (uint8_t)(value >> (i * 8));
    }
}

static int64_t signed_word(uint32_t word) {
    return word & UINT32_C(0x80000000) ? (int64_t)word - INT64_C(4294967296) : word;
}

static uint32_t arithmetic_shift(uint32_t word, unsigned shift) {
    shift &= 31;
    if (!shift) return word;
    return (word >> shift) | (word & UINT32_C(0x80000000) ? UINT32_MAX << (32 - shift) : 0);
}

static int expression(unsigned op, uint32_t lhs, uint32_t rhs, uint32_t *result) {
    int64_t a = signed_word(lhs), b = signed_word(rhs);
    switch (op) {
    case 0: *result = lhs + rhs; break;
    case 1: *result = lhs - rhs; break;
    case 2: *result = lhs | rhs; break;
    case 3: *result = lhs & rhs; break;
    case 4: *result = lhs ^ rhs; break;
    case 5: *result = lhs || rhs; break;
    case 6: *result = lhs && rhs; break;
    case 7: *result = !lhs; break;
    case 8: *result = ~lhs; break;
    case 9: *result = a > b; break;
    case 10: *result = a < b; break;
    case 11: *result = lhs == rhs; break;
    case 12: *result = a >= b; break;
    case 13: *result = a <= b; break;
    case 14: *result = lhs != rhs; break;
    case 15: *result = lhs * rhs; break;
    case 16: if (!b) return 0; *result = (uint32_t)(a / b); break;
    case 17: *result = lhs << (rhs & 31); break;
    case 18: *result = arithmetic_shift(lhs, rhs); break;
    case 19: *result = lhs; break;
    case 20: {
        int64_t product = a * b;
        /* Floor division implements the signed 64-bit arithmetic shift. */
        int64_t scaled = product / 65536;
        if (product < 0 && product % 65536) --scaled;
        *result = (uint32_t)scaled;
        break;
    }
    case 21: {
        /* Original 24.8 divide discards the divisor's fraction byte, then
         * scales the truncated integer quotient. This is not a/b in 16.16. */
        int64_t divisor = signed_word(arithmetic_shift(rhs, 8));
        if (!divisor) return 0;
        *result = (uint32_t)(a / divisor) * 256u;
        break;
    }
    case 22: if (!b) return 0; *result = (uint32_t)(a % b); break;
    case 23: *result = 0u - lhs; break;
    default: return 0;
    }
    return 1;
}

static int target(PeScriptTask *task, uint32_t halfwords, uint32_t *offset) {
    int64_t address = (int64_t)task->base + signed_word(halfwords) * 2;
    if (address < 0 || (uint64_t)address > task->program.blob->size ||
        task->program.blob->size - (uint64_t)address < 8) return 0;
    *offset = (uint32_t)address;
    return 1;
}

int pe_script_task_init(PeScriptTask *task, const PeScriptProgram *program,
                         unsigned entry, PeScriptBanks *banks) {
    uint32_t offset;
    if (!banks || !pe_script_entry(program, entry, &offset)) return 0;
    memset(task, 0, sizeof(*task));
    task->program = *program;
    task->banks = banks;
    task->base = task->pc = offset;
    task->active = 1;
    for (unsigned i = 0; i < 3; ++i) task->cursors[i] = UINT32_MAX;
    return 1;
}

PeScriptStep pe_script_task_tick(PeScriptTask *task, unsigned instruction_limit) {
    if (task->error[0]) return PE_SCRIPT_ERROR;
    if ((task->flags & 0x10) || (task->actor_flags & 0x10)) return PE_SCRIPT_STOPPED;
    ++task->ticks;
    if ((task->flags & 0x40) ||
        (((task->actor_flags & 0x1000) ||
          ((task->banks->scene_flags & 0x100) && !task->is_battle_actor)) && !(task->flags & 0x80)))
        return PE_SCRIPT_WAIT;
    if (!task->active || --task->active) return PE_SCRIPT_WAIT;
    unsigned current_opcode = 0;
    for (unsigned step = 0; step < instruction_limit; ++step) {
        PeScriptCommand command;
        if (!pe_script_decode(&task->program, task->pc, &command,
                              task->error, sizeof(task->error))) return PE_SCRIPT_ERROR;
        current_opcode = command.opcode;
        uint32_t args[15] = {0};
        for (unsigned i = 0; i < command.count; ++i) args[i] = pe_script_arg_read(task, &command.args[i]);
        unsigned required;
        switch (command.opcode) {
        case 0: case 2: case 19: case 25: case 31: case 43: case 44: case 45:
        case 82: case 83: required = 1; break;
        case 5: case 10: case 20: case 40: case 42: case 81: required = 2; break;
        case 41: required = 3; break;
        case 9: required = 4; break;
        case 1: case 30: case 32: case 50: case 55: case 57: case 63: case 64:
        case 65: case 66: case 76: case 79: case 80: required = 0; break;
        default:
            if (task->dispatch) {
                PeScriptDispatchResult status = task->dispatch(task->dispatch_user, task, &command);
                if (status == PE_DISPATCH_CONTINUE || status == PE_DISPATCH_YIELD) {
                    task->pc = command.next;
                    ++task->instructions;
                    if (status == PE_DISPATCH_YIELD) {
                        if (!task->active) task->active = 1;
                        return PE_SCRIPT_WAIT;
                    }
                    continue;
                }
                if (status == PE_DISPATCH_RETRY) {
                    task->active = 1;
                    return PE_SCRIPT_WAIT;
                }
                if (status == PE_DISPATCH_ERROR) {
                    if (!task->error[0]) snprintf(task->error, sizeof(task->error),
                        "Subsystem error for opcode %u at 0x%x", command.opcode, task->pc);
                    return PE_SCRIPT_ERROR;
                }
                if (status != PE_DISPATCH_UNSUPPORTED) {
                    snprintf(task->error, sizeof(task->error), "Invalid subsystem dispatch result");
                    return PE_SCRIPT_ERROR;
                }
            }
            snprintf(task->error, sizeof(task->error), "Unimplemented script opcode %u at 0x%x",
                     command.opcode, task->pc);
            return PE_SCRIPT_ERROR;
        }
        if (command.count < required) goto invalid;
        uint32_t next = command.next, result;
        int yield = 0;
        switch (command.opcode) {
        case 0: if (!target(task, args[0], &next)) goto invalid; break;
        case 1: task->actor_flags |= 0x10; yield = 1; break;
        case 2: task->active = (uint16_t)args[0]; yield = 1; break;
        case 5:
            /* Script_JumpIfZero writes the interpreter cursor address;
             * the native runtime keeps that address as one checked offset. */
            if (!args[0] && !target(task, args[1], &next)) goto invalid;
            break;
        case 9:
            if (!expression(args[0], args[2], args[3], &result)) goto invalid;
            pe_script_arg_write(task, &command.args[1], result);
            break;
        case 10: pe_script_arg_write(task, &command.args[0], args[1]); break;
        case 19: task->actor_flags |= args[0]; break;
        case 20:
            if (args[0] >= 1 && args[0] <= 3) {
                result = UINT32_MAX;
                if (signed_word(args[1]) >= 0 && !target(task, args[1], &result)) goto invalid;
                task->cursors[args[0] - 1] = result;
            }
            break;
        case 25: pe_script_arg_write(task, &command.args[0], task->sequence); break;
        case 30: task->actor_flags |= 0x80; break;
        case 31: pe_script_arg_write(task, &command.args[0], task->target14); break;
        case 32: task->flags |= 0x10; yield = 1; break;
        case 40: case 41: case 42:
            result = UINT32_C(1) << (args[1] & 31);
            if (command.opcode == 40) pe_script_arg_write(task, &command.args[0], args[0] & ~result);
            else if (command.opcode == 41) pe_script_arg_write(task, &command.args[2], args[0] & result);
            else pe_script_arg_write(task, &command.args[0], args[0] | result);
            break;
        case 43: task->banks->move_lock &= ~args[0]; break;
        case 44: pe_script_arg_write(task, &command.args[0], task->banks->move_lock); break;
        case 45: task->banks->move_lock |= args[0]; break;
        case 50: task->actor_flags &= ~UINT32_C(0x80); break;
        case 55: task->actor_flags |= 0x4000; break;
        case 57: task->actor_flags &= ~UINT32_C(0x4000); break;
        case 63: task->banks->move_lock &= ~UINT32_C(1); break;
        case 64: task->banks->move_lock |= 1; break;
        case 65: task->actor_flags |= 0x40; break;
        case 66: task->actor_flags &= ~UINT32_C(0x40); break;
        case 76: break; /* Original Script_Noop, explicitly defined. */
        case 79: task->actor_flags |= 0x100; break;
        case 80: task->actor_flags &= ~UINT32_C(0x100); break;
        case 81: pe_script_arg_write(task, &command.args[1], (task->banks->scene_flags & args[0]) != 0); break;
        case 82: task->banks->scene_flags |= args[0]; break;
        case 83: task->banks->scene_flags &= ~args[0]; break;
        }
        task->pc = next;
        ++task->instructions;
        if (yield) return (task->flags & 0x10) || (task->actor_flags & 0x10) ?
                          PE_SCRIPT_STOPPED : PE_SCRIPT_WAIT;
    }
    snprintf(task->error, sizeof(task->error), "Script instruction limit %u exceeded at 0x%x",
             instruction_limit, task->pc);
    return PE_SCRIPT_ERROR;
invalid:
    snprintf(task->error, sizeof(task->error), "Invalid operands for opcode %u at 0x%x",
             current_opcode, task->pc);
    return PE_SCRIPT_ERROR;
}
