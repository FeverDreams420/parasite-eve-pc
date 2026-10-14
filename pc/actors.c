#include "actors.h"
#include <string.h>
#include <stdlib.h>

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
    actor->floor_triangle = actor->floor_triangle_mirror = UINT32_MAX;
    actor->gravity_y = 0x4000; actor->move_speed = 0x1000; actor->animation_step = 0x10000;
    actor->action=UINT32_MAX;
    actor->parent=-1;
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
    unsigned remove=0;
    unsigned visited = 0; int previous = -1, index = actors->head;
    /* Check list linkage before unlinking or recycling any actor. */
    while (index >= 0) {
        if ((unsigned)index >= PE_ACTOR_CAPACITY || ++visited > PE_ACTOR_CAPACITY ||
            !actors->slots[index].used || actors->slots[index].previous != previous) goto invalid;
        PeActor *actor=&actors->slots[index];
        if(actor->parent < -1 || (actor->parent>=0&&((unsigned)actor->parent>=PE_ACTOR_CAPACITY||
           actors->slots[actor->parent].script.field_id!=actor->parent_id)))goto invalid;
        if((actor->script.flags&0x10)||(actor->parent>=0&&(actors->slots[actor->parent].script.flags&0x10)))remove|=1u<<index;
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
        uint32_t saved_flags=actor->script.flags;
        if(remove&(1u<<index))actor->script.flags|=0x10;
        if (!pe_script_scheduler_collect(actors->scheduler, &actor->script)) {
            actor->script.flags=saved_flags;
            snprintf(actors->error, sizeof(actors->error), "%s", actors->scheduler->error);
            return 0;
        }
        actor->script.flags=saved_flags;
        if (remove&(1u<<index)) {
            pe_actor_model_close(actor);
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

int pe_actor_find_floor(PeActor *actor, const PeCollision *collision) {
    PeFloorHit hit;
    if (!pe_collision_find_floor(collision, actor->position[0], actor->position[1],
                                 actor->position[2], actor->script.flags, &hit)) return 0;
    actor->floor_triangle = actor->floor_triangle_mirror = hit.triangle;
    actor->position[1] = hit.height;
    return 1;
}
int pe_actor_set_animation(PeActor *actor,const PeAnimation *animation,unsigned action,int force){
    if(!animation->frames||action>65535)return 0;
    PeAnimationFrame frame;
    if(!pe_animation_frame(animation,0,&frame))return 0;
    if(force||actor->action!=action){
        actor->animation=*animation;actor->action=action;
        actor->animation_clock.frame=actor->animation_clock.previous=0;
        actor->animation_clock.last=(uint16_t)(animation->frames-1);
    }
    actor->script.flags&=~UINT32_C(0x200);return 1;
}
int pe_actor_advance_animation(PeActor *actor){
    if(!actor->animation.bytes)return 0;
    return pe_animation_advance(&actor->animation_clock,actor->animation_step,&actor->script.flags);
}
void pe_actor_model_close(PeActor *actor){
    free(actor->render);actor->render=NULL;memset(&actor->animation,0,sizeof(actor->animation));
    actor->action=UINT32_MAX;
}
int pe_actor_model_bind(PeActor *actor,const PeRoomBanks *banks,unsigned slot){
    if(slot>=10||!banks->roots[slot].present)return 0;
    PeActorModel *next=calloc(1,sizeof(*next));if(!next)return 0;
    char error[128];
    if(!pe_model_open(banks->roots[slot].bytes,banks->roots[slot].size,&next->model,error,sizeof(error))){free(next);return 0;}
    memcpy(next->actions,banks->rows[slot],sizeof(next->actions));next->slot=slot;
    pe_matrix_identity(&next->world);
    for(unsigned i=0;i<next->model.part_count;++i)pe_matrix_identity(&next->sources[i]);
    pe_actor_model_close(actor);actor->render=next;return 1;
}
int pe_actor_set_action(PeActor *actor,unsigned action,int force){
    if(!actor->render||action>=48||!actor->render->actions[action].present)return 0;
    PeRoomSlice *slice=&actor->render->actions[action];PeAnimation animation;char error[128];
    if(!pe_animation_open(slice->bytes,slice->size,&animation,error,sizeof(error))||
       animation.bones>actor->render->model.part_count)return 0;
    return pe_actor_set_animation(actor,&animation,action,force);
}
int pe_actor_model_pose(PeActor *actor,const PeMatrix *base){
    if(!actor->render||!actor->animation.bytes)return 0;
    PeActorModel *render=actor->render;
    PeMatrix sources[PE_ANIMATION_BONES],pose[PE_ANIMATION_BONES];PeHitVolume volumes[PE_ANIMATION_BONES];
    unsigned pose_count=0,volume_count=0;PeAnimationFrame frame;
    unsigned index=actor->animation_clock.frame>>16;
    if(!pe_animation_frame(&actor->animation,index,&frame))return 0;
    memcpy(sources,render->sources,(size_t)render->model.part_count*sizeof(*sources));
    if(!pe_animation_pose(&render->model,&actor->animation,index,render->overrides,base,
                          sources,render->model.part_count,pose,PE_ANIMATION_BONES,&pose_count)||
       !pe_model_hitvolumes(&render->model,pose,pose_count,volumes,PE_ANIMATION_BONES,&volume_count))return 0;
    memcpy(render->sources,sources,(size_t)render->model.part_count*sizeof(*sources));
    memcpy(render->pose,pose,(size_t)pose_count*sizeof(*pose));
    memcpy(render->volumes,volumes,(size_t)volume_count*sizeof(*volumes));
    memcpy(render->body,frame.body,sizeof(render->body));render->pose_count=pose_count;render->volume_count=volume_count;
    return 1;
}
int pe_actor_model_world_pose(PeActor *actor){
    if(!actor->render)return 0;
    PeMatrix world=actor->render->world;
    if(!(actor->script.flags&0x10040)){
        pe_matrix_rotation_xyz(actor->rotation,&world);
        for(unsigned i=0;i<3;++i){
            unsigned value=actor->position[i]>>16;
            world.translation[i]=value&32768?(uint32_t)((int)value-65536):value;
        }
        PeMatrix scale={0};unsigned half=actor->move_speed&65535;
        int16_t value=(int16_t)(half&32768?(int)half-65536:(int)half);
        for(unsigned i=0;i<3;++i)scale.rotation[i][i]=value;
        pe_matrix_compose(&world,&scale,&world);
    }
    if(!pe_actor_model_pose(actor,&world))return 0;
    actor->render->world=world;return 1;
}
PeScriptDispatchResult pe_actors_dispatch(void *user,PeScriptTask *task,const PeScriptCommand *command){
    if(command->opcode<46||command->opcode>48)return PE_DISPATCH_UNSUPPORTED;
    PeActors *actors=user;PeActor *actor=NULL;
    for(unsigned i=0;i<PE_ACTOR_CAPACITY;++i)
        if(actors->slots[i].used && &actors->slots[i].script==task->actor){actor=&actors->slots[i];break;}
    if(!actor||(command->opcode!=48&&!command->count))goto invalid;
    if(command->opcode==46){
        if(actor->script.flags&0x100000)goto invalid; /* parent propagation pending */
        unsigned action=pe_script_arg_read(task,&command->args[0])&65535;
        if(!pe_actor_set_action(actor,action,1))goto invalid;
        actor->script.flags&=~UINT32_C(0x100);return PE_DISPATCH_CONTINUE;
    }
    if(command->opcode==47){
        unsigned stop=pe_script_arg_read(task,&command->args[0])&65535;
        if(stop>actor->animation_clock.last)stop=actor->animation_clock.last;
        actor->animation_clock.stop=(uint16_t)stop;actor->script.flags|=0x200;
        return PE_DISPATCH_CONTINUE;
    }
    task->active=1;
    return (actor->animation_clock.frame>>16)==actor->animation_clock.stop?PE_DISPATCH_YIELD:PE_DISPATCH_RETRY;
invalid:
    snprintf(task->error,sizeof(task->error),"Invalid or unsupported actor animation opcode %u",command->opcode);
    return PE_DISPATCH_ERROR;
}
int pe_actors_events(PeActors *actors){
    PeScriptActor *targets[PE_ACTOR_CAPACITY];unsigned count=0,seen=0;
    int index=actors->head,previous=-1;
    while(index>=0){
        if((unsigned)index>=PE_ACTOR_CAPACITY||count>=PE_ACTOR_CAPACITY||
           (seen&(1u<<index))||!actors->slots[index].used||actors->slots[index].previous!=previous)goto invalid;
        seen|=1u<<index;targets[count++]=&actors->slots[index].script;
        previous=index;index=actors->slots[index].next;
    }
    if(index!=-1||count!=actors->live)goto invalid;
    if(!pe_script_scheduler_event_targets(actors->scheduler,targets,count)){
        snprintf(actors->error,sizeof(actors->error),"%s",actors->scheduler->error);return 0;
    }
    actors->error[0]=0;return 1;
invalid:snprintf(actors->error,sizeof(actors->error),"Invalid actor list during event delivery");return 0;
}
void pe_actor_motion_snapshot(PeActor *actor){
    memcpy(actor->saved_position,actor->position,sizeof(actor->position));
    memcpy(actor->saved_rotation,actor->rotation,sizeof(actor->rotation));
}
void pe_actor_motion_apply(PeActor *actor){
    if(actor->script.flags&2){
        actor->motion[0]+=actor->gravity_x;actor->motion[1]+=actor->gravity_y;actor->motion[2]+=actor->gravity_z;
    }
    for(unsigned i=0;i<3;++i){
        actor->motion[i]+=actor->acceleration[i];
        actor->position[i]+=actor->motion[i];actor->position[i]+=actor->delta[i];
    }
}
static int parent_chain(const PeActors *actors,unsigned child,int parent){
    unsigned seen=1u<<child;
    while(parent>=0){
        if((unsigned)parent>=PE_ACTOR_CAPACITY||!actors->slots[parent].used||(seen&(1u<<parent)))return 0;
        seen|=1u<<parent;
        const PeActor *actor=&actors->slots[parent];int next=actor->parent;
        if(next>=0&&((unsigned)next>=PE_ACTOR_CAPACITY||!actors->slots[next].used||
                     actors->slots[next].script.field_id!=actor->parent_id))return 0;
        parent=next;
    }
    return parent==-1;
}
int pe_actors_set_parent(PeActors *actors,unsigned child,int parent){
    if(child>=PE_ACTOR_CAPACITY||!actors->slots[child].used||!parent_chain(actors,child,parent))return 0;
    actors->slots[child].parent=parent;
    actors->slots[child].parent_id=parent>=0?actors->slots[parent].script.field_id:0;return 1;
}
static int validate_hierarchy(PeActors *actors){
    unsigned count=0,seen=0;int index=actors->head,previous=-1;
    while(index>=0){
        if((unsigned)index>=PE_ACTOR_CAPACITY||(seen&(1u<<index))||!actors->slots[index].used||
           actors->slots[index].previous!=previous)goto invalid;
        PeActor *actor=&actors->slots[index];seen|=1u<<index;++count;
        if(!parent_chain(actors,(unsigned)index,actor->parent))goto invalid;
        if(actor->parent>=0&&actors->slots[actor->parent].script.field_id!=actor->parent_id)goto invalid;
        previous=index;index=actor->next;
    }
    if(index!=-1||count!=actors->live)goto invalid;
    actors->error[0]=0;return 1;
invalid:snprintf(actors->error,sizeof(actors->error),"Invalid actor parent hierarchy");return 0;
}
int pe_actors_copy_parent_positions(PeActors *actors){
    if(!validate_hierarchy(actors))return 0;
    int index;
    for(index=actors->head;index>=0;index=actors->slots[index].next){
        PeActor *actor=&actors->slots[index];
        if(actor->parent>=0&&(actor->script.flags&0x400000)){
            memcpy(actor->position,actors->slots[actor->parent].position,sizeof(actor->position));
            memcpy(actor->rotation,actors->slots[actor->parent].rotation,sizeof(actor->rotation));
        }
    }
    actors->error[0]=0;return 1;
}
static void restore_position(PeActor *actor){
    memcpy(actor->position,actor->saved_position,sizeof(actor->position));
    actor->floor_triangle=actor->floor_triangle_mirror;
}
int pe_actors_rollback_position(PeActors *actors,unsigned slot){
    if(slot>=PE_ACTOR_CAPACITY||!actors->slots[slot].used||!validate_hierarchy(actors))return 0;
    int member=0;for(int index=actors->head;index>=0;index=actors->slots[index].next)if((unsigned)index==slot)member=1;
    if(!member){snprintf(actors->error,sizeof(actors->error),"Rollback actor absent from list");return 0;}
    int chain[PE_ACTOR_CAPACITY];unsigned depth=0;
    for(int index=(int)slot;index>=0;index=actors->slots[index].parent)chain[depth++]=index;
    PeActor *root=&actors->slots[chain[depth-1]];restore_position(root);root->script.flags|=0x40000;
    while(--depth){
        int parent=actors->slots[chain[depth-1]].parent;
        for(int index=actors->head;index>=0;index=actors->slots[index].next)
            if(actors->slots[index].parent==parent)restore_position(&actors->slots[index]);
    }
    actors->error[0]=0;return 1;
}
