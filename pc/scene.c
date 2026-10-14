#include "scene.h"
#include <stdlib.h>
#include <string.h>

void pe_scene_close(PeScene *scene) {
    if(scene->actors){for(unsigned i=0;i<PE_ACTOR_CAPACITY;++i)pe_actor_model_close(&scene->actors->slots[i]);free(scene->actors);}
    for(unsigned i=0;i<2;++i)pe_music_close(scene->audio.music[i]);
    pe_collision_close(&scene->collision);
    pe_room_blob_close(&scene->entity_bank);
    pe_room_blob_close(&scene->area_bank);
    pe_room_close(&scene->room);
    free(scene->vram);
    free(scene->spu);
    free(scene->scheduler);
    memset(scene, 0, sizeof(*scene));
}

int pe_scene_prepare(PeRoomCatalog *catalog, unsigned map, PeScene *scene,
                       char *error, size_t error_size) {
    PeScene next = {0};
    next.input=scene->input;
    if (!pe_room_load(catalog, map, &next.room)) goto catalog_error;
    if(!pe_room_cameras(&next.room,&next.cameras)||!pe_camera_group(&next.cameras,0,&next.camera)){
        snprintf(error,error_size,"Invalid room camera bank");goto failed;
    }
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
    scene->actors->collision=&scene->collision;
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
    if(command->opcode==130){
        if(!command->count||!pe_scene_camera_select(scene,pe_script_arg_read(task,&command->args[0]))){
            snprintf(task->error,sizeof(task->error),"Invalid camera group");return PE_DISPATCH_ERROR;
        }
        return PE_DISPATCH_CONTINUE;
    }
    PeScriptDispatchResult input_result=pe_input_dispatch(&scene->input,task,command);
    if(input_result!=PE_DISPATCH_UNSUPPORTED)return input_result;
    PeScriptDispatchResult result=pe_actors_dispatch(scene->actors,task,command);
    return result==PE_DISPATCH_UNSUPPORTED?pe_scene_audio_dispatch(&scene->audio,task,command):result;
}
int pe_scene_actor_scripts(PeScene *scene,unsigned slot,PeRoomCatalog *catalog,unsigned instruction_limit){
    if(!scene->actors||slot>=PE_ACTOR_CAPACITY||!scene->actors->slots[slot].used)return 0;
    scene->audio.catalog=catalog;
    int result=pe_script_scheduler_tick(scene->scheduler,&scene->actors->slots[slot].script,instruction_limit);
    scene->audio.catalog=NULL;return result;
}
int pe_scene_events(PeScene *scene){return scene->actors&&pe_actors_events(scene->actors);}
void pe_scene_set_input(PeScene *scene,uint32_t held){pe_input_update(&scene->input,held);}
int pe_scene_primary_bank(PeScene *scene,PeRoomCatalog *catalog,unsigned area,char *error,size_t error_size){
    if(area<10||area>14||!scene->actors)goto invalid;
    for(unsigned i=0;i<PE_ACTOR_CAPACITY;++i)
        if(scene->actors->slots[i].used&&(scene->actors->slots[i].render||scene->actors->slots[i].animation.bytes))goto invalid;
    PeRoomBlob next={0};PeRoomBanks banks=scene->banks;
    if(!pe_sector_bank_load(catalog,area+8,&next)){
        snprintf(error,error_size,"%s",catalog->error);return 0;
    }
    if(!pe_entity_banks(&next,1,&banks,error,error_size)){pe_room_blob_close(&next);return 0;}
    /* Drop old primary views that were not overridden by the new bank. */
    if(scene->entity_bank.bytes){
        PeRoomBanks room_banks;
        if(!pe_room_banks(&scene->room,&room_banks,error,error_size)){pe_room_blob_close(&next);return 0;}
        banks=room_banks;
        if(!pe_entity_banks(&next,1,&banks,error,error_size)){pe_room_blob_close(&next);return 0;}
    }
    pe_room_blob_close(&scene->area_bank);
    pe_room_blob_close(&scene->entity_bank);scene->entity_bank=next;scene->banks=banks;scene->entity_area=area;
    scene->area_entity_id=scene->area_day=0;
    if(error_size)error[0]=0;return 1;
invalid:snprintf(error,error_size,"Invalid entity area or active actor bank references");return 0;
}
int pe_scene_area_bank(PeScene *scene,PeRoomCatalog *catalog,unsigned entity_bank,unsigned day,char *error,size_t error_size){
    if(entity_bank<10||entity_bank>13||day<1||day>8||!scene->entity_bank.bytes||!scene->actors)goto invalid;
    for(unsigned i=0;i<PE_ACTOR_CAPACITY;++i)
        if(scene->actors->slots[i].used&&(scene->actors->slots[i].render||scene->actors->slots[i].animation.bytes))goto invalid;
    PeRoomBlob next={0};PeRoomBanks banks;
    if(!pe_sector_bank_load(catalog,(entity_bank-10)/2*8+day+0x16,&next)){
        snprintf(error,error_size,"%s",catalog->error);return 0;
    }
    if(!pe_room_banks(&scene->room,&banks,error,error_size)||
       !pe_entity_banks(&scene->entity_bank,1,&banks,error,error_size)||
       !pe_entity_banks(&next,0,&banks,error,error_size)){pe_room_blob_close(&next);return 0;}
    pe_room_blob_close(&scene->area_bank);scene->area_bank=next;scene->banks=banks;
    scene->area_entity_id=entity_bank;scene->area_day=day;
    if(error_size)error[0]=0;return 1;
invalid:snprintf(error,error_size,"Invalid area/day bank or active actor references");return 0;
}
int pe_scene_entity_textures(PeScene *scene,PeRoomCatalog *catalog,unsigned area,unsigned day,char *error,size_t error_size){
    if(!scene->vram||area<10||area>14||day<1||day>8){snprintf(error,error_size,"Invalid entity texture selection");return 0;}
    PeVram *next=malloc(sizeof(*next));if(!next){snprintf(error,error_size,"Cannot allocate texture staging");return 0;}
    memcpy(next,scene->vram,sizeof(*next));PeRoomBlob blob={0};PeVramStats total={0},stats;
    unsigned indices[2]={area+3,day+0x26};unsigned count=day>=2&&day<=6?2:1;
    for(unsigned i=0;i<count;++i){
        if(!pe_sector_bank_load(catalog,indices[i],&blob)){snprintf(error,error_size,"%s",catalog->error);goto failed;}
        if(!pe_blob_upload_vram(&blob,next,&stats,error,error_size))goto failed;
        total.uploads+=stats.uploads;total.words+=stats.words;
    }
    memcpy(scene->vram,next,sizeof(*next));scene->entity_texture_stats=total;
    pe_room_blob_close(&blob);free(next);if(error_size)error[0]=0;return 1;
failed:pe_room_blob_close(&blob);free(next);return 0;
}
int pe_scene_camera_select(PeScene *scene,unsigned group){
    PeCamera camera;if(!scene->actors||!pe_camera_group(&scene->cameras,group,&camera))return 0;
    scene->camera=camera;scene->camera_group=group;scene->actors->render_flags|=0x80;return 1;
}
void pe_scene_camera_clamp(PeScene *scene,uint32_t x,uint32_t y){
    if(scene->actors)pe_camera_clamp(&scene->camera,scene->actors->render_flags,x,y,scene->camera_position);
}
