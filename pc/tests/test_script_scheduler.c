#include "script_scheduler.h"
#include <stdlib.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "line %d: %s\n", __LINE__, #expr); exit(1); } } while (0)

static void word(uint8_t *p, uint32_t value) {
    for (unsigned i = 0; i < 4; ++i) p[i] = (uint8_t)(value >> (i * 8));
}

int main(void) {
    uint8_t bytes[256] = {0};
    PeRoomBlob blob = {0}; blob.bytes = bytes; blob.size = sizeof(bytes);
    PeScriptProgram program = {0}; program.blob = &blob; program.entries = 1;
    word(bytes + 8, 16);
    word(bytes + 16, 18 | (1u << 13)); word(bytes + 24, 32); /* child at 80 */
    word(bytes + 28, 2 | (1u << 13)); word(bytes + 36, 1);
    word(bytes + 40, 10 | (2u << 13) | (2u << 17) | (1u << 20));
    word(bytes + 48, 0); word(bytes + 52, 0);
    word(bytes + 56, 32);
    word(bytes + 80, 10 | (2u << 13) | (1u << 17));
    word(bytes + 88, 0); word(bytes + 92, 99);
    word(bytes + 96, 32);
    PeScriptBanks banks = {0};
    PeScriptActor actor, other;
    PeScriptScheduler *scheduler = malloc(sizeof(*scheduler)); CHECK(scheduler);
    for (unsigned at_head = 0; at_head < 2; ++at_head) {
        pe_script_scheduler_init(scheduler, &program, &banks);
        pe_script_actor_init(&actor, 16);
        pe_script_actor_init(&other, 16);
        int parent = pe_script_scheduler_start(scheduler, &actor, 2, 16);
        CHECK(parent == 0);
        scheduler->slots[parent].task.flags = (uint16_t)at_head;
        CHECK(pe_script_scheduler_tick(scheduler, &actor, 100));
        CHECK(scheduler->live == 2 && actor.locals[0] == (at_head ? 0u : 99u));
        CHECK(!other.locals[0]);
        CHECK(scheduler->slots[1].task.sequence == 1);
        CHECK(scheduler->slots[1].task.base == 16 && scheduler->slots[1].task.actor == &actor);
        CHECK(scheduler->slots[1].task.ticks == (at_head ? 0u : 1u));
        CHECK(pe_script_scheduler_tick(scheduler, &actor, 100));
        CHECK(actor.locals[0] == 99 && banks.variables[0] == 99);
        pe_script_scheduler_collect(scheduler, &actor);
        CHECK(!scheduler->live && actor.heads[2] == -1);
    }
    pe_script_scheduler_init(scheduler, &program, &banks);
    pe_script_actor_init(&actor, 16);
    scheduler->sequence = 65535;
    for (unsigned i = 0; i < PE_SCRIPT_TASK_CAPACITY; ++i) {
        int index = pe_script_scheduler_start(scheduler, &actor, 1, 80);
        CHECK(index == (int)i);
        CHECK(scheduler->slots[index].task.sequence == (uint16_t)(65535u + i));
    }
    int head = actor.heads[1];
    CHECK(pe_script_scheduler_start(scheduler, &actor, 1, 80) == -1);
    CHECK(scheduler->live == 72 && actor.heads[1] == head && strstr(scheduler->error, "exhausted"));
    CHECK(pe_script_scheduler_tick(scheduler, &actor, 100));
    pe_script_scheduler_collect(scheduler, &actor);
    CHECK(!scheduler->live && actor.heads[1] == -1);
    CHECK(pe_script_scheduler_start(scheduler, &actor, 0, 80) >= 0);
    actor.flags = 0x10;
    pe_script_scheduler_collect(scheduler, &actor);
    CHECK(!scheduler->live);
    pe_script_scheduler_init(scheduler, &program, &banks);
    pe_script_actor_init(&actor, 16);
    word(bytes + 16, 1); /* mark actor dead, then process the remaining node */
    CHECK(pe_script_scheduler_start(scheduler, &actor, 2, 80) == 0);
    CHECK(pe_script_scheduler_start(scheduler, &actor, 2, 16) == 1);
    CHECK(pe_script_scheduler_tick(scheduler, &actor, 100));
    CHECK(actor.flags == 0x10 && actor.locals[0] == 99);
    CHECK(pe_script_scheduler_collect(scheduler, &actor) && !scheduler->live);
    /* Invalid spawn fails before mutating the list or sequence. */
    pe_script_scheduler_init(scheduler, &program, &banks);
    pe_script_actor_init(&actor, 16);
    word(bytes + 16, 18 | (1u << 13));
    CHECK(pe_script_scheduler_start(scheduler, &actor, 0, 16) == 0);
    word(bytes + 24, UINT32_C(0xfffffc18)); /* -1000 halfwords */
    CHECK(!pe_script_scheduler_tick(scheduler, &actor, 100));
    CHECK(scheduler->live == 1 && scheduler->sequence == 1 && scheduler->slots[0].next == -1);
    word(bytes + 24, 1000);
    pe_script_scheduler_init(scheduler, &program, &banks);
    pe_script_actor_init(&actor, 16);
    CHECK(pe_script_scheduler_start(scheduler, &actor, 0, 16) == 0);
    CHECK(!pe_script_scheduler_tick(scheduler, &actor, 100));
    CHECK(scheduler->live == 1 && scheduler->sequence == 1);
    pe_script_scheduler_init(scheduler, &program, &banks);
    pe_script_actor_init(&actor, 16);
    actor.heads[0] = 999;
    CHECK(!pe_script_scheduler_tick(scheduler, &actor, 100));
    CHECK(!pe_script_scheduler_collect(scheduler, &actor));
    CHECK(pe_script_scheduler_start(scheduler, &actor, 0, 80) == -1);
    actor.heads[0] = -1;
    scheduler->free_head = 999;
    CHECK(pe_script_scheduler_start(scheduler, &actor, 0, 80) == -1);
    pe_script_scheduler_init(scheduler, &program, &banks);
    PeScriptActor actors[3];
    for (unsigned i = 0; i < 3; ++i) {
        pe_script_actor_init(&actors[i], 16);
        actors[i].cursors[2] = 80; actors[i].script_entry = 7;
        actors[i].sub_id = i < 2 ? 3 : 4; actors[i].field_id = 11 + i;
    }
    banks.events[0] = (PeScriptEvent){7,3,6,0,99};
    banks.events[1] = (PeScriptEvent){0,0,9,13,55}; banks.event_count = 2;
    CHECK(pe_script_scheduler_events(scheduler, actors, 3));
    CHECK(!banks.event_count && scheduler->live == 3);
    for (unsigned i = 0; i < 3; ++i) {
        PeScriptTask *event_task = &scheduler->slots[i].task;
        CHECK(event_task->actor == &actors[i] && event_task->flags == 4);
        CHECK(event_task->target14 == (i < 2 ? 6u : 9u));
        CHECK(event_task->trigger_value == (i < 2 ? 99u : 55u));
        CHECK(!event_task->ticks && event_task->pc == 80);
    }
    while (scheduler->live < PE_SCRIPT_TASK_CAPACITY - 1)
        CHECK(pe_script_scheduler_start(scheduler, &actors[0], 2, 80) >= 0);
    int previous_head = actors[0].heads[2];
    banks.event_count = 1;
    CHECK(!pe_script_scheduler_events(scheduler, actors, 3));
    CHECK(banks.event_count == 1 && scheduler->live == PE_SCRIPT_TASK_CAPACITY - 1);
    CHECK(actors[0].heads[2] == previous_head);
    free(scheduler);
    puts("Script scheduler: same-tick child order, shared actors, deferred collection, pool exhaustion and sequence wrap passed");
    return 0;
}
