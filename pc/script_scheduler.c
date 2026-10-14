#include "script_scheduler.h"
#include <string.h>

static PeScriptDispatchResult dispatch(void *user, PeScriptTask *task,
                                       const PeScriptCommand *command);

void pe_script_scheduler_init(PeScriptScheduler *scheduler,
                               const PeScriptProgram *program, PeScriptBanks *banks) {
    memset(scheduler, 0, sizeof(*scheduler));
    scheduler->program = *program;
    scheduler->banks = banks;
    scheduler->free_head = 0;
    for (unsigned i = 0; i < PE_SCRIPT_TASK_CAPACITY; ++i)
        scheduler->free_next[i] = i + 1 < PE_SCRIPT_TASK_CAPACITY ? (int)i + 1 : -1;
}

static int allocate(PeScriptScheduler *scheduler, PeScriptActor *actor,
                     unsigned group, uint32_t offset, int previous) {
    if (group >= 3 || offset > scheduler->program.blob->size ||
        scheduler->program.blob->size - offset < 8 || (offset & 1)) {
        snprintf(scheduler->error, sizeof(scheduler->error), "Invalid script task group or offset");
        return -1;
    }
    int index = scheduler->free_head;
    if (index < 0) {
        snprintf(scheduler->error, sizeof(scheduler->error), "Script task pool exhausted (%u)", PE_SCRIPT_TASK_CAPACITY);
        return -1;
    }
    int next = previous < 0 ? actor->heads[group] :
               (unsigned)previous < PE_SCRIPT_TASK_CAPACITY ? scheduler->slots[previous].next : -2;
    if ((unsigned)index >= PE_SCRIPT_TASK_CAPACITY || scheduler->slots[index].used ||
        next < -1 || (next >= 0 && ((unsigned)next >= PE_SCRIPT_TASK_CAPACITY ||
        !scheduler->slots[next].used || scheduler->slots[next].task.actor != actor ||
        scheduler->slots[next].group != group)) ||
        (previous >= 0 && ((unsigned)previous >= PE_SCRIPT_TASK_CAPACITY ||
        !scheduler->slots[previous].used || scheduler->slots[previous].task.actor != actor ||
        scheduler->slots[previous].group != group))) {
        snprintf(scheduler->error, sizeof(scheduler->error), "Invalid task pool or actor list");
        return -1;
    }
    PeScriptSlot *slot = &scheduler->slots[index];
    if (!pe_script_task_init(&slot->task, &scheduler->program, 0, scheduler->banks)) {
        snprintf(scheduler->error, sizeof(scheduler->error), "Cannot initialize script task");
        return -1;
    }
    slot->task.actor = actor;
    slot->task.base = actor->base;
    slot->task.pc = offset;
    slot->task.sequence = scheduler->sequence++;
    slot->task.dispatch = dispatch;
    slot->task.dispatch_user = scheduler;
    slot->group = group;
    slot->used = 1;
    slot->previous = previous;
    scheduler->free_head = scheduler->free_next[index];
    if (previous < 0) {
        slot->next = actor->heads[group];
        actor->heads[group] = index;
    } else {
        slot->next = scheduler->slots[previous].next;
        scheduler->slots[previous].next = index;
    }
    if (slot->next >= 0) scheduler->slots[slot->next].previous = index;
    ++scheduler->live;
    scheduler->error[0] = 0;
    return index;
}

int pe_script_scheduler_start(PeScriptScheduler *scheduler, PeScriptActor *actor,
                               unsigned group, uint32_t offset) {
    return allocate(scheduler, actor, group, offset, -1);
}

static int event_matches(const PeScriptEvent *event, const PeScriptActor *actor) {
    return actor->cursors[2] != UINT32_MAX && (event->selector ? actor->field_id == event->selector :
        actor->script_entry == event->type && actor->sub_id == event->sub_id);
}

int pe_script_scheduler_events(PeScriptScheduler *scheduler, PeScriptActor *actors, unsigned count) {
    unsigned needed = 0;
    if (scheduler->banks->event_count > PE_SCRIPT_EVENT_CAPACITY || scheduler->live > PE_SCRIPT_TASK_CAPACITY) goto invalid_events;
    for (unsigned event = 0; event < scheduler->banks->event_count; ++event) {
        for (unsigned actor = 0; actor < count; ++actor) {
            if (!event_matches(&scheduler->banks->events[event], &actors[actor])) continue;
            uint32_t offset = actors[actor].cursors[2];
            if ((offset & 1) || offset > scheduler->program.blob->size ||
                scheduler->program.blob->size - offset < 8 || ++needed > PE_SCRIPT_TASK_CAPACITY - scheduler->live)
                goto invalid_events;
            int head = actors[actor].heads[2];
            if (head < -1 || (head >= 0 && ((unsigned)head >= PE_SCRIPT_TASK_CAPACITY ||
                !scheduler->slots[head].used || scheduler->slots[head].group != 2 ||
                scheduler->slots[head].task.actor != &actors[actor]))) goto invalid_events;
            if (scheduler->banks->events[event].selector) break;
        }
    }
    unsigned seen[PE_SCRIPT_TASK_CAPACITY] = {0}; int free_slot = scheduler->free_head;
    for (unsigned i = 0; i < needed; ++i) {
        if (free_slot < 0 || (unsigned)free_slot >= PE_SCRIPT_TASK_CAPACITY ||
            seen[free_slot] || scheduler->slots[free_slot].used) goto invalid_events;
        seen[free_slot] = 1; free_slot = scheduler->free_next[free_slot];
    }
    for (unsigned event = 0; event < scheduler->banks->event_count; ++event) {
        const PeScriptEvent *entry = &scheduler->banks->events[event];
        for (unsigned actor = 0; actor < count; ++actor) {
            if (!event_matches(entry, &actors[actor])) continue;
            int slot = allocate(scheduler, &actors[actor], 2, actors[actor].cursors[2], -1);
            if (slot < 0) return 0;
            scheduler->slots[slot].task.flags |= 4;
            scheduler->slots[slot].task.trigger_value = entry->value;
            scheduler->slots[slot].task.target14 = entry->argument;
            if (entry->selector) break;
        }
    }
    scheduler->banks->event_count = 0;
    return 1;
invalid_events:
    snprintf(scheduler->error, sizeof(scheduler->error), "Invalid script event cursor or insufficient task capacity");
    return 0;
}

static PeScriptDispatchResult dispatch(void *user, PeScriptTask *task,
                                       const PeScriptCommand *command) {
    PeScriptScheduler *scheduler = user;
    if (command->opcode != 18)
        return scheduler->dispatch ? scheduler->dispatch(scheduler->dispatch_user, task, command) :
                                     PE_DISPATCH_UNSUPPORTED;
    if (!command->count) {
        snprintf(task->error, sizeof(task->error), "Spawn-child opcode requires an offset");
        return PE_DISPATCH_ERROR;
    }
    uint32_t word = pe_script_arg_read(task, &command->args[0]);
    int64_t relative = word & UINT32_C(0x80000000) ? (int64_t)word - INT64_C(4294967296) : word;
    int64_t offset = (int64_t)task->actor->base + relative * 2;
    if (offset < 0 || offset > UINT32_MAX) {
        snprintf(task->error, sizeof(task->error), "Invalid child script offset");
        return PE_DISPATCH_ERROR;
    }
    int parent = -1;
    for (unsigned i = 0; i < PE_SCRIPT_TASK_CAPACITY; ++i)
        if (scheduler->slots[i].used && &scheduler->slots[i].task == task) { parent = (int)i; break; }
    if (parent < 0) {
        snprintf(task->error, sizeof(task->error), "Spawn requires a scheduled parent");
        return PE_DISPATCH_ERROR;
    }
    int at_head = (task->flags & 3) != 0;
    unsigned group = at_head ? 2 : scheduler->slots[parent].group;
    if (allocate(scheduler, task->actor, group, (uint32_t)offset, at_head ? -1 : parent) < 0) {
        snprintf(task->error, sizeof(task->error), "%s", scheduler->error);
        return PE_DISPATCH_ERROR;
    }
    return PE_DISPATCH_CONTINUE;
}

int pe_script_scheduler_tick(PeScriptScheduler *scheduler, PeScriptActor *actor,
                              unsigned instruction_limit) {
    for (unsigned group = 0; group < 3; ++group) {
        int index = actor->heads[group];
        if (index < -1) {
            snprintf(scheduler->error, sizeof(scheduler->error), "Invalid actor task head");
            return 0;
        }
        unsigned visited = 0;
        while (index >= 0) {
            if ((unsigned)index >= PE_SCRIPT_TASK_CAPACITY || ++visited > PE_SCRIPT_TASK_CAPACITY ||
                !scheduler->slots[index].used || scheduler->slots[index].task.actor != actor ||
                scheduler->slots[index].group != group) {
                snprintf(scheduler->error, sizeof(scheduler->error), "Invalid script task list");
                return 0;
            }
            PeScriptSlot *slot = &scheduler->slots[index];
            if (pe_script_task_tick(&slot->task, instruction_limit) == PE_SCRIPT_ERROR) {
                snprintf(scheduler->error, sizeof(scheduler->error), "%s", slot->task.error);
                return 0;
            }
            /* Read after execution: a newly inserted child runs this tick,
             * while a new head before the current node waits until next tick. */
            index = slot->next;
        }
    }
    return 1;
}

int pe_script_scheduler_collect(PeScriptScheduler *scheduler, PeScriptActor *actor) {
    for (unsigned group = 0; group < 3; ++group) {
        int index = actor->heads[group];
        if (index < -1) {
            snprintf(scheduler->error, sizeof(scheduler->error), "Invalid actor task head");
            return 0;
        }
        unsigned visited = 0;
        while (index >= 0) {
            if ((unsigned)index >= PE_SCRIPT_TASK_CAPACITY || ++visited > PE_SCRIPT_TASK_CAPACITY ||
                !scheduler->slots[index].used || scheduler->slots[index].task.actor != actor ||
                scheduler->slots[index].group != group) {
                snprintf(scheduler->error, sizeof(scheduler->error), "Invalid script task list during collection");
                return 0;
            }
            PeScriptSlot *slot = &scheduler->slots[index];
            int next = slot->next;
            if (next < -1 || (next >= 0 && ((unsigned)next >= PE_SCRIPT_TASK_CAPACITY ||
                !scheduler->slots[next].used || scheduler->slots[next].previous != index ||
                scheduler->slots[next].task.actor != actor || scheduler->slots[next].group != group)) ||
                slot->previous < -1 || (slot->previous >= 0 &&
                ((unsigned)slot->previous >= PE_SCRIPT_TASK_CAPACITY ||
                 !scheduler->slots[slot->previous].used || scheduler->slots[slot->previous].next != index ||
                 scheduler->slots[slot->previous].task.actor != actor || scheduler->slots[slot->previous].group != group))) {
                snprintf(scheduler->error, sizeof(scheduler->error), "Broken script task links");
                return 0;
            }
            if ((actor->flags & 0x10) || (slot->task.flags & 0x10)) {
                if (slot->previous < 0) actor->heads[group] = next;
                else scheduler->slots[slot->previous].next = next;
                if (next >= 0) scheduler->slots[next].previous = slot->previous;
                slot->used = 0;
                scheduler->free_next[index] = scheduler->free_head;
                scheduler->free_head = index;
                --scheduler->live;
            }
            index = next;
        }
    }
    return 1;
}
