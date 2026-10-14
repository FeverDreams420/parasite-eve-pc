#include "scene.h"
#include <stdio.h>
#include <stdlib.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);exit(1);}}while(0)
static void word(uint8_t *p,uint32_t v){for(unsigned i=0;i<4;++i)p[i]=(uint8_t)(v>>(i*8));}
int main(void){
    uint8_t bytes[112]={0};word(bytes+8,16);
    /* Start scroll; poll variable 9; wait/jump while false; stop when true. */
    word(bytes+16,3|(3u<<13));word(bytes+24,100);word(bytes+28,200);word(bytes+32,2);
    word(bytes+36,71|(1u<<13)|(2u<<17));word(bytes+44,9);
    word(bytes+48,5|(2u<<13)|(2u<<17));word(bytes+56,9);word(bytes+60,34);
    word(bytes+64,32);
    word(bytes+84,2|(1u<<13));word(bytes+92,1);
    word(bytes+96,0|(1u<<13));word(bytes+104,10);
    PeRoomBlob blob={0};blob.bytes=bytes;blob.size=sizeof(bytes);
    PeScriptProgram program={0};program.blob=&blob;program.entries=1;
    PeScene scene={0};scene.actors=calloc(1,sizeof(*scene.actors));CHECK(scene.actors);
    scene.actors->render_flags=0x40;scene.camera.width=320;scene.camera.height=224;
    PeScriptTask task;CHECK(pe_script_task_init(&task,&program,0,&scene.script_banks));
    task.dispatch=pe_scene_dispatch;task.dispatch_user=&scene;
    CHECK(pe_script_task_tick(&task,100)==PE_SCRIPT_WAIT&&task.pc==96&&!scene.script_banks.variables[9]);
    CHECK(scene.camera_scroll.duration==2&&scene.camera_scroll.elapsed==1);
    CHECK(!pe_scene_camera_transition_step(&scene)&&scene.camera_scroll.position[0]==50&&scene.camera_scroll.position[1]==100);
    CHECK(pe_script_task_tick(&task,100)==PE_SCRIPT_WAIT&&task.pc==96&&!scene.script_banks.variables[9]);
    CHECK(!pe_scene_camera_transition_step(&scene)&&scene.camera_scroll.position[0]==100&&scene.camera_scroll.position[1]==200);
    CHECK(pe_script_task_tick(&task,100)==PE_SCRIPT_STOPPED&&scene.script_banks.variables[9]==1);
    CHECK(task.ticks==3&&task.instructions==12&&task.pc==72);
    word(bytes+16,70|(3u<<13));scene.camera_scroll=(PeCameraScroll){0};scene.actors->render_flags=0x40;
    CHECK(pe_script_task_init(&task,&program,0,&scene.script_banks));task.dispatch=pe_scene_dispatch;task.dispatch_user=&scene;
    CHECK(pe_script_task_tick(&task,100)==PE_SCRIPT_WAIT&&(scene.actors->render_flags&15)==9);
    CHECK(!pe_scene_camera_transition_step(&scene));
    CHECK(pe_script_task_tick(&task,100)==PE_SCRIPT_WAIT&&task.pc==96);
    CHECK(!pe_scene_camera_transition_step(&scene));
    CHECK(pe_script_task_tick(&task,100)==PE_SCRIPT_STOPPED&&task.ticks==3&&task.instructions==12);
    CHECK(scene.camera_scroll.position[0]==100&&scene.camera_scroll.position[1]==200);
    word(bytes+16,237|(3u<<13));word(bytes+24,2201);word(bytes+28,0x8000);word(bytes+32,0x12345678);
    CHECK(pe_script_task_init(&task,&program,0,&scene.script_banks));task.dispatch=pe_scene_dispatch;task.dispatch_user=&scene;
    CHECK(pe_script_task_tick(&task,100)==PE_SCRIPT_STOPPED&&scene.camera_scroll.origin[0]==-32768&&scene.camera_scroll.origin[1]==0x5678);
    CHECK(scene.camera_scroll.position[0]==100&&scene.camera_scroll.position[1]==200);
    uint32_t render_flags=scene.actors->render_flags;
    word(bytes+16,237|(1u<<13));word(bytes+24,2101);word(bytes+28,32);
    CHECK(pe_script_task_init(&task,&program,0,&scene.script_banks));task.dispatch=pe_scene_dispatch;task.dispatch_user=&scene;
    CHECK(pe_script_task_tick(&task,100)==PE_SCRIPT_STOPPED&&(scene.script_banks.game_flags&0x800)&&scene.actors->render_flags==render_flags);
    word(bytes+24,9999);CHECK(pe_script_task_init(&task,&program,0,&scene.script_banks));task.dispatch=pe_scene_dispatch;task.dispatch_user=&scene;
    CHECK(pe_script_task_tick(&task,100)==PE_SCRIPT_ERROR&&scene.camera_scroll.origin[0]==-32768);
    word(bytes+24,2100);CHECK(pe_script_task_init(&task,&program,0,&scene.script_banks));task.dispatch=pe_scene_dispatch;task.dispatch_user=&scene;
    CHECK(pe_script_task_tick(&task,100)==PE_SCRIPT_STOPPED&&scene.actors->render_flags==render_flags);
    scene.actors->player=0;scene.actors->slots[0].used=1;scene.actors->slots[0].position[2]=500u<<16;
    pe_matrix_identity(&scene.camera.view);scene.camera.distance=200;
    scene.camera.min_x=scene.camera.min_y=-500;scene.camera.max_x=scene.camera.max_y=500;scene.camera_scroll.flags=0x70;
    word(bytes+16,237|(3u<<13));word(bytes+24,2200);word(bytes+28,3);word(bytes+32,0);word(bytes+36,32);
    CHECK(pe_script_task_init(&task,&program,0,&scene.script_banks));task.dispatch=pe_scene_dispatch;task.dispatch_user=&scene;
    CHECK(pe_script_task_tick(&task,100)==PE_SCRIPT_STOPPED&&scene.camera_scroll.duration==3&&scene.camera_scroll.target[0]==160&&scene.camera_scroll.target[1]==112);
    scene.actors->player=-1;CHECK(pe_script_task_init(&task,&program,0,&scene.script_banks));task.dispatch=pe_scene_dispatch;task.dispatch_user=&scene;
    CHECK(pe_script_task_tick(&task,100)==PE_SCRIPT_ERROR&&scene.camera_scroll.duration==3);
    uint8_t background_bytes[160]={0};background_bytes[6]=1;background_bytes[20]=60;background_bytes[38]=8;
    background_bytes[60]=2;background_bytes[98]=1;word(background_bytes+100,60);word(background_bytes+104,64);
    word(background_bytes+120,(3u<<22)|(4u<<12));
    scene.cameras=(PeCameraBank){0};scene.cameras.bytes=background_bytes;scene.cameras.size=sizeof(background_bytes);scene.cameras.count=1;
    CHECK(pe_background_open(&scene.cameras,&scene.background));
    scene.background_motion=malloc(sizeof(*scene.background_motion));CHECK(scene.background_motion);scene.background_motion_count=1;
    CHECK(pe_background_motion(&scene.background,0,scene.background_motion));
    scene.camera_scroll.position[0]=160;scene.camera_scroll.position[1]=112;
    PeBackgroundDrawTile draw;unsigned count=99;
    CHECK(pe_scene_background_tiles(&scene,&draw,1,&count)&&count==1&&draw.x==3&&draw.y==4);
    scene.camera_scroll.position[0]=161;count=99;
    CHECK(!pe_scene_background_tiles(&scene,&draw,0,&count)&&count==99&&draw.x==3&&scene.camera_scroll.display[0]==0);
    CHECK(pe_scene_background_tiles(&scene,&draw,1,&count)&&draw.x==2&&scene.camera_scroll.display[0]==-1);
    PeBackgroundMotion *motion=scene.background_motion;motion->flags=6;motion->modulus[0]=320;motion->modulus[1]=224;motion->speed[0]=128;
    CHECK(pe_scene_background_step(&scene)&&motion->position[0]==0&&motion->fraction[0]==128);
    CHECK(pe_scene_background_step(&scene)&&motion->position[0]==1&&!motion->fraction[0]);
    CHECK(pe_scene_background_tiles(&scene,&draw,1,&count)&&draw.x==3);
    CHECK(background_bytes[72]==0); /* authored screen position remains immutable */
    scene.script_banks.scene_flags|=4;scene.camera_scroll.flags|=0x80;
    CHECK(pe_scene_background_step(&scene)&&!motion->fraction[0]&&(scene.camera_scroll.flags&0x80));
    scene.script_banks.scene_flags&=~UINT32_C(4);
    CHECK(pe_scene_background_step(&scene)&&motion->fraction[0]==128&&!(scene.camera_scroll.flags&0x80)&&scene.camera_scroll.saved[0]==161);
    motion->modulus[0]=0;scene.camera_scroll.flags|=0x80;
    CHECK(!pe_scene_background_step(&scene)&&motion->position[0]==1&&motion->fraction[0]==128&&(scene.camera_scroll.flags&0x80));
    pe_scene_close(&scene);
    puts("Camera VM: real polling branch/wait loop resumes only after transition completion, headless");return 0;
}
