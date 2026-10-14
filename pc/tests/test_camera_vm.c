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
    pe_scene_close(&scene);
    puts("Camera VM: real polling branch/wait loop resumes only after transition completion, headless");return 0;
}
