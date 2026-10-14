#include "actors.h"
#include <string.h>

void pe_actors_init(PeActors *actors, PeScriptScheduler *scheduler) {
    memset(actors, 0, sizeof(*actors));
    actors->scheduler = scheduler;
    actors->head = actors->player = -1;
    actors->free_head = 0;
    for (unsigned i = 0; i < PE_ACTOR_CAPACITY; ++i)
        actors->free_next[i] = i + 1 < PE_ACTOR_CAPACITY ? (int)i + 1 : -1;
    scheduler->banks->actor_serial = 1;
}

int pe_actors_spawn(PeActors *actors, uint8_t type, uint8_t sub_id, int after,
                    uint32_t x, uint32_t y, uint32_t z) {
    int index = actors->free_head;
    uint32_t offset;
    if (index < 0) {
        snprintf(actors->error, sizeof(actors->error), "Actor pool exhausted (%u)", PE_ACTOR_CAPACITY);
        return -1;
    }
    if ((unsigned)index >= PE_ACTOR_CAPACITY || actors->slots[index].used || after < -1 ||
        (after >= 0 && ((unsigned)after >= PE_ACTOR_CAPACITY || !actors->slots[after].used)) ||
        !pe_script_entry(&actors->scheduler->program, type, &offset)) goto invalid;
    int next = after < 0 ? actors->head : actors->slots[after].next;
    if (next < -1 || (next >= 0 && ((unsigned)next >= PE_ACTOR_CAPACITY || !actors->slots[next].used ||
        actors->slots[next].previous != after))) goto invalid;
    PeActor *actor = &actors->slots[index];
    memset(actor, 0, sizeof(*actor));
    pe_script_actor_init(&actor->script, offset);
    actor->script.script_entry = type; actor->script.sub_id = sub_id;
    actor->script.flags = 0x10000000;
    actor->script.field_id = actors->scheduler->banks->actor_serial;
    if (pe_script_scheduler_start(actors->scheduler, &actor->script, 2, offset) < 0) {
        snprintf(actors->error, sizeof(actors->error), "%s", actors->scheduler->error);
        return -1;
    }
    ++actors->scheduler->banks->actor_serial;
    actor->position[0] = x; actor->position[1] = y; actor->position[2] = z;
    actor->gravity_y = 0x4000; actor->move_speed = 0x1000; actor->animation_step = 0x10000;
    actor->move_factor = type ? 0x50000 : 0x10000;
    actor->used = 1; actor->previous = after; actor->next = next;
    if (next >= 0) actors->slots[next].previous = index;
    if (after >= 0) actors->slots[after].next = index;
    else actors->head = index;
    if (!type) actors->player = index;
    actors->free_head = actors->free_next[index]; ++actors->live;
    actors->error[0] = 0;
    return index;
invalid:
    snprintf(actors->error, sizeof(actors->error), "Invalid actor type or list linkage");
    return -1;
}

int pe_actors_collect(PeActors *actors) {
    unsigned visited = 0; int previous = -1, index = actors->head;
    /* Check list linkage before unlinking or recycling any actor. */
    while (index >= 0) {
        if ((unsigned)index >= PE_ACTOR_CAPACITY || ++visited > PE_ACTOR_CAPACITY ||
            !actors->slots[index].used || actors->slots[index].previous != previous) goto invalid;
        previous = index; index = actors->slots[index].next;
    }
    if (index != -1 || visited != actors->live) goto invalid;
    unsigned free_count = 0; index = actors->free_head;
    while (index >= 0) {
        if ((unsigned)index >= PE_ACTOR_CAPACITY || ++free_count > PE_ACTOR_CAPACITY ||
            actors->slots[index].used) goto invalid;
        index = actors->free_next[index];
    }
    if (index != -1 || free_count + actors->live != PE_ACTOR_CAPACITY) goto invalid;
    index = actors->head;
    while (index >= 0) {
        PeActor *actor = &actors->slots[index]; int next = actor->next;
        actor->script.flags &= ~UINT32_C(0x800000);
        if (!pe_script_scheduler_collect(actors->scheduler, &actor->script)) {
            snprintf(actors->error, sizeof(actors->error), "%s", actors->scheduler->error);
            return 0;
        }
        if (actor->script.flags & 0x10) {
            if (actor->previous < 0) actors->head = next;
            else actors->slots[actor->previous].next = next;
            if (next >= 0) actors->slots[next].previous = actor->previous;
            if (actors->player == index) {
                actors->player = -1;
                actors->scheduler->banks->move_lock &= ~UINT32_C(0xd);
            }
            actor->used = 0;
            actors->free_next[index] = actors->free_head; actors->free_head = index;
            --actors->live;
        }
        index = next;
    }
    actors->error[0] = 0;
    return 1;
invalid:
    snprintf(actors->error, sizeof(actors->error), "Invalid actor list during collection");
    return 0;
}
