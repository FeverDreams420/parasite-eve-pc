#include "input.h"
#include <stdio.h>
#include <stdlib.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);exit(1);}}while(0)
static void word(uint8_t *p,uint32_t v){for(unsigned i=0;i<4;++i)p[i]=(uint8_t)(v>>(i*8));}
int main(void){
    uint8_t bytes[44]={0};word(bytes+8,16);word(bytes+16,17|(3u<<13)|(2u<<23));word(bytes+36,32);
    PeRoomBlob blob={0};blob.bytes=bytes;blob.size=sizeof(bytes);
    PeScriptProgram program={0};program.blob=&blob;program.entries=1;
    PeScriptBanks banks={0};PeScriptTask task;PeInput input={0};
    pe_input_update(&input,3);pe_input_update(&input,1);
    for(unsigned mode=0;mode<3;++mode){
        word(bytes+24,mode);word(bytes+28,mode==2?2:1);word(bytes+32,0);
        CHECK(pe_script_task_init(&task,&program,0,&banks));task.dispatch=pe_input_dispatch;task.dispatch_user=&input;
        CHECK(pe_script_task_tick(&task,100)==PE_SCRIPT_STOPPED&&banks.variables[0]==(mode!=1));
    }
    word(bytes+24,3);word(bytes+28,1);
    CHECK(pe_script_task_init(&task,&program,0,&banks));task.dispatch=pe_input_dispatch;task.dispatch_user=&input;
    CHECK(pe_script_task_tick(&task,100)==PE_SCRIPT_STOPPED&&banks.variables[0]==2&&bytes[28]==31);
    word(bytes+28,0);CHECK(pe_script_task_init(&task,&program,0,&banks));task.dispatch=pe_input_dispatch;task.dispatch_user=&input;
    CHECK(pe_script_task_tick(&task,100)==PE_SCRIPT_ERROR&&banks.variables[0]==2);
    word(bytes+24,99);word(bytes+28,1);
    CHECK(pe_script_task_init(&task,&program,0,&banks));task.dispatch=pe_input_dispatch;task.dispatch_user=&input;
    CHECK(pe_script_task_tick(&task,100)==PE_SCRIPT_STOPPED&&banks.variables[0]==2);
    word(bytes+24,3);word(bytes+28,0x80000000);input.held=0x80000000;input.held_frames[31]=7;
    CHECK(pe_script_task_init(&task,&program,0,&banks));task.dispatch=pe_input_dispatch;task.dispatch_user=&input;
    CHECK(pe_script_task_tick(&task,100)==PE_SCRIPT_STOPPED&&banks.variables[0]==7&&bytes[31]==0x80);
    input.held_frames[0]=UINT32_MAX;pe_input_update(&input,1);CHECK(!input.held_frames[0]);
    puts("Script input: held/pressed/released queries, counters, CLZ operand writeback and errors passed");return 0;
}
