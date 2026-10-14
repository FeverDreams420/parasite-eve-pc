#ifndef PE_PC_ACTORS_H
#define PE_PC_ACTORS_H
#include "script_scheduler.h"
#include "collision.h"
#include "animation.h"
#define PE_ACTOR_CAPACITY 14u
typedef struct PeActor {
    PeScriptActor script;
    uint32_t position[3], gravity_y, move_factor, move_speed, animation_step;
    int next, previous;
    unsigned used;
    uint32_t floor_triangle, floor_triangle_mirror; /* UINT32_MAX means no face */
    PeAnimation animation; /* borrowed stable room/entity-bank bytes */
    PeAnimationClock animation_clock;
    unsigned action;
} PeActor;
typedef struct PeActors {
    PeActor slots[PE_ACTOR_CAPACITY];
    int free_next[PE_ACTOR_CAPACITY], free_head, head, player;
    unsigned live;
    PeScriptScheduler *scheduler; /* borrowed stable scheduler */
    char error[256];
} PeActors;
/* Fresh room only. Owner stays at a stable address while tasks borrow slots.
 * This is actor/script allocation state; models and physical
 * simulation must be added before dispatching complete spawn opcode 8. */
void pe_actors_init(PeActors *actors, PeScriptScheduler *scheduler);
int pe_actors_spawn(PeActors *actors, uint8_t type, uint8_t sub_id, int after,
                    uint32_t x, uint32_t y, uint32_t z);
/* Post-callback collection. Actor flag 0x10 requests removal; task references
 * are released before recycling a slot. Models/parent hierarchies pending. */
int pe_actors_collect(PeActors *actors);
/* Original FindFloor state update; no match or bypass preserves face/Y. */
int pe_actor_find_floor(PeActor *actor, const PeCollision *collision);
/* Local action binding; parent propagation/model allocation remain separate. */
int pe_actor_set_animation(PeActor *actor,const PeAnimation *animation,unsigned action,int force);
int pe_actor_advance_animation(PeActor *actor);
#endif
