#ifndef PE_PC_ACTORS_H
#define PE_PC_ACTORS_H
#include "script_scheduler.h"
#include "collision.h"
#include "animation.h"
#include "room_banks.h"
#define PE_ACTOR_CAPACITY 14u
typedef struct PeActorModel {
    PeModel model;PeRoomSlice actions[48];
    PeMatrix sources[PE_ANIMATION_BONES],pose[PE_ANIMATION_BONES];
    PeHitVolume volumes[PE_ANIMATION_BONES];
    PeRotationOverride overrides[2];uint16_t body[4];
    unsigned pose_count,volume_count,slot;
    PeMatrix world;
} PeActorModel;
typedef struct PeActor {
    PeScriptActor script;
    uint32_t position[3], gravity_y, move_factor, move_speed, animation_step;
    int next, previous;
    unsigned used;
    uint32_t floor_triangle, floor_triangle_mirror; /* UINT32_MAX means no face */
    PeAnimation animation; /* borrowed stable room/entity-bank bytes */
    PeAnimationClock animation_clock;
    unsigned action;
    uint16_t rotation[3];
    PeActorModel *render; /* owned matrices; model/action bytes remain borrowed */
} PeActor;
typedef struct PeActors {
    PeActor slots[PE_ACTOR_CAPACITY];
    int free_next[PE_ACTOR_CAPACITY], free_head, head, player;
    unsigned live;
    PeScriptScheduler *scheduler; /* borrowed stable scheduler */
    char error[256];
} PeActors;
/* Fresh room only. Owner stays at a stable address while tasks borrow slots.
 * Complete spawn opcode 8 still requires scene bank selection, physical
 * simulation, callbacks and render packet setup. */
void pe_actors_init(PeActors *actors, PeScriptScheduler *scheduler);
int pe_actors_spawn(PeActors *actors, uint8_t type, uint8_t sub_id, int after,
                    uint32_t x, uint32_t y, uint32_t z);
/* Post-callback collection. Actor flag 0x10 requests removal; task references
 * and owned model state are released before recycling. Parent hierarchies pending. */
int pe_actors_collect(PeActors *actors);
/* Original FindFloor state update; no match or bypass preserves face/Y. */
int pe_actor_find_floor(PeActor *actor, const PeCollision *collision);
/* Local animation binding; parent propagation remains separate. */
int pe_actor_set_animation(PeActor *actor,const PeAnimation *animation,unsigned action,int force);
int pe_actor_advance_animation(PeActor *actor);
/* Transactional model/action binding and pose refresh. Base world transform
 * is supplied explicitly; rendering and alternate skin modes are separate. */
int pe_actor_model_bind(PeActor *actor,const PeRoomBanks *banks,unsigned slot);
void pe_actor_model_close(PeActor *actor);
int pe_actor_set_action(PeActor *actor,unsigned action,int force);
int pe_actor_model_pose(PeActor *actor,const PeMatrix *base);
/* Original signed integer position, actor rotation and halfword scale.
 * Flags 0x10040 preserve the last world matrix. */
int pe_actor_model_world_pose(PeActor *actor);
PeScriptDispatchResult pe_actors_dispatch(void *user,PeScriptTask *task,const PeScriptCommand *command);
#endif
