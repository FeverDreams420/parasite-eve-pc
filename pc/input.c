#include "input.h"
void pe_input_update(PeInput *input,uint32_t held){
    uint32_t changed=input->held^held;
    input->pressed=changed&held;input->released=changed&input->held;input->held=held;
    for(unsigned bit=0;bit<32;++bit)input->held_frames[bit]=held&(UINT32_C(1)<<bit)?input->held_frames[bit]+1:0;
}
static int matches(const PeInput *input,const PeInputRule *rule){
    uint32_t flags=rule->flags;
    if(flags&0x20)return 1;
    if(flags&1){
        if((input->held&rule->all)!=rule->all)return 0;
        if(flags&4)return (input->pressed&rule->any)!=0;
        if(flags&8)return (input->released&rule->any)!=0;
        return !(flags&2)||(input->held&rule->any)!=0;
    }
    if(flags&2){
        if(!(input->held&rule->any))return 0;
        if(flags&4)return (input->pressed&rule->any)!=0;
        if(flags&8)return (input->released&rule->any)!=0;
        return 1;
    }
    if(flags&4)return (input->pressed&rule->any)!=0;
    if(flags&8)return (input->released&rule->any)!=0;
    return 0;
}
int pe_input_rules(const PeInput *input,void *context,const PeInputRules *table,unsigned count,int *mode){
    if(*mode<0||(unsigned)*mode>=count)return 0;
    PeInputRules rules=table[*mode];if(!rules.records)return rules.count==0;
    size_t end=0;
    while(end<rules.count&&rules.records[end].flags&&rules.records[end].callback)++end;
    if(end==rules.count)return 0;
    int triggered=0;
    for(size_t i=0;i<end;++i){
        const PeInputRule *rule=&rules.records[i];
        if(matches(input,rule)){rule->callback(context,mode);triggered=1;}
        if(!(rule->flags&0x10)&&triggered)return 1;
    }
    if(!rules.records[end].flags&&rules.records[end].callback)rules.records[end].callback(context,mode);
    return 1;
}
