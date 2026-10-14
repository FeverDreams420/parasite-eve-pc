#include "input.h"
PeScriptDispatchResult pe_input_dispatch(void *user,PeScriptTask *task,const PeScriptCommand *command){
    if(command->opcode!=17)return PE_DISPATCH_UNSUPPORTED;
    if(command->count<3)return PE_DISPATCH_ERROR;
    PeInput *input=user;uint32_t mode=pe_script_arg_read(task,&command->args[0]);
    uint32_t mask=pe_script_arg_read(task,&command->args[1]),value;
    if(mode>3)return PE_DISPATCH_CONTINUE; /* Original unknown-mode no-op. */
    if(mode<3){
        uint32_t state=mode==0?input->held:mode==1?input->pressed:input->released;
        value=(state&mask)==mask;
    }else{
        if(!mask)return PE_DISPATCH_ERROR; /* Original invalid counter index -1. */
        value=0;
        if((input->held&mask)==mask){
            unsigned bit=31;while(!(mask&(UINT32_C(1)<<bit)))--bit;
            if(mask!=UINT32_C(0x80000000))pe_script_arg_write(task,&command->args[1],31-bit);
            value=input->held_frames[bit];
        }
    }
    pe_script_arg_write(task,&command->args[2],value);return PE_DISPATCH_CONTINUE;
}
