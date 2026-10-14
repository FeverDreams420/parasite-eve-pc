#include "scene.h"
#include <stdlib.h>
#include <string.h>

void pe_scene_close(PeScene *scene) {
    if(scene->actors){for(unsigned i=0;i<PE_ACTOR_CAPACITY;++i)pe_actor_model_close(&scene->actors->slots[i]);free(scene->actors);}
    for(unsigned i=0;i<2;++i)pe_music_close(scene->audio.music[i]);
    pe_collision_close(&scene->collision);
    pe_room_close(&scene->room);
    free(scene->vram);
    free(scene->spu);
    free(scene->scheduler);
    memset(scene, 0, sizeof(*scene));
}

int pe_scene_prepare(PeRoomCatalog *catalog, unsigned map, PeScene *scene,
                       char *error, size_t error_size) {
    PeScene next = {0};
    if (!pe_room_load(catalog, map, &next.room)) goto catalog_error;
    if (!pe_room_collision(&next.room, &next.collision, error, error_size)) goto failed;
    if (!pe_room_banks(&next.room, &next.banks, error, error_size)) goto failed;
    next.vram = malloc(sizeof(*next.vram));
    next.spu = malloc(sizeof(*next.spu));
    if (!next.vram || !next.spu) {
        snprintf(error, error_size, "Cannot allocate scene VRAM/SPU state");
        goto failed;
    }
    if (scene->vram) memcpy(next.vram, scene->vram, sizeof(*next.vram));
    else memset(next.vram, 0, sizeof(*next.vram));
    if (scene->spu) {
        memcpy(next.spu, scene->spu, sizeof(*next.spu));
        next.sample_stats = scene->sample_stats;
    } else {
        memset(next.spu, 0, sizeof(*next.spu));
        if (!pe_spu_bank_load_startup(catalog, next.spu, &next.sample_stats)) goto catalog_error;
    }
    if (!pe_room_upload_vram(&next.room, next.vram, &next.texture_stats, error, error_size) ||
        !pe_script_program_open(&next.room, &next.scripts, error, error_size)) goto failed;
    next.script_banks = scene->script_banks;
    next.script_banks.actor_serial = 1; /* Draw_InitPipeline starts room actor IDs at one. */
    next.akao = catalog->akao;
    next.scheduler = malloc(sizeof(*next.scheduler));
    next.script_actor_count = next.scripts.actors;
    next.actors=malloc(sizeof(*next.actors));
    if (!next.scheduler || !next.actors) {
        free(next.actors);next.actors=NULL;
        snprintf(error,error_size,"Cannot allocate actor/task pools");goto failed;
    }
    pe_script_scheduler_init(next.scheduler,&next.scripts,&next.script_banks);
    next.scheduler->sequence=1;
    pe_actors_init(next.actors,next.scheduler);
    next.scheduler->dispatch=pe_scene_dispatch;next.scheduler->dispatch_user=&next;
    next.audio.room=&next.room;next.audio.resident=next.spu;next.audio.game_flags=&next.script_banks.game_flags;
    pe_scene_audio_reset(&next.audio);
    for(unsigned i=0;i<next.script_actor_count;++i){
        unsigned entry,sub_id;
        if(!pe_script_actor(&next.scripts,i,&entry,&sub_id)||
           pe_actors_spawn(next.actors,(uint8_t)entry,(uint8_t)sub_id,-1,0,0,0)<0){
            snprintf(error,error_size,"Invalid initial actor or pool exhaustion");goto failed;
        }
    }
    pe_scene_close(scene);
    *scene = next;
    /* The program borrows the room member; rebind after moving the owner. */
    scene->scripts.blob = &scene->room.data;
    scene->scheduler->program = scene->scripts;
    scene->scheduler->banks = &scene->script_banks;
    scene->scheduler->dispatch_user=scene;
    scene->audio.room=&scene->room;scene->audio.game_flags=&scene->script_banks.game_flags;
    for (unsigned i = 0; i < PE_SCRIPT_TASK_CAPACITY; ++i) {
        if (!scene->scheduler->slots[i].used) continue;
        scene->scheduler->slots[i].task.program = scene->scripts;
        scene->scheduler->slots[i].task.banks = &scene->script_banks;
    }
    if (error_size) error[0] = 0;
    return 1;
catalog_error:
    snprintf(error, error_size, "%s", catalog->error);
failed:
    pe_scene_close(&next);
    return 0;
}

PeScriptDispatchResult pe_scene_dispatch(void *user,PeScriptTask *task,const PeScriptCommand *command){
    PeScene *scene=user;
    PeScriptDispatchResult result=pe_actors_dispatch(scene->actors,task,command);
    return result==PE_DISPATCH_UNSUPPORTED?pe_scene_audio_dispatch(&scene->audio,task,command):result;
}
int pe_scene_actor_scripts(PeScene *scene,unsigned slot,PeRoomCatalog *catalog,unsigned instruction_limit){
    if(!scene->actors||slot>=PE_ACTOR_CAPACITY||!scene->actors->slots[slot].used)return 0;
    scene->audio.catalog=catalog;
    int result=pe_script_scheduler_tick(scene->scheduler,&scene->actors->slots[slot].script,instruction_limit);
    scene->audio.catalog=NULL;return result;
}
