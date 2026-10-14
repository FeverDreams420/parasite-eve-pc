#include "input.h"
#include <stdio.h>
#include <stdlib.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);exit(1);}}while(0)
typedef struct Calls { unsigned calls,fallback; } Calls;
static void action(void *context,int *mode){((Calls*)context)->calls++;*mode=1;}
static void fallback(void *context,int *mode){(void)mode;((Calls*)context)->fallback++;}
int main(void){
    PeInput input={0};pe_input_update(&input,3);CHECK(input.held==3&&input.pressed==3&&!input.released);
    pe_input_update(&input,3);CHECK(!input.pressed&&!input.released);
    pe_input_update(&input,6);CHECK(input.pressed==4&&input.released==1);
    pe_input_update(&input,0);CHECK(input.released==6&&!input.pressed);
    PeInputRule records[]={{1|4,3,4,action},{0,0,0,fallback}};
    PeInputRules table[2]={{records,2},{NULL,0}};Calls calls={0};int mode=0;
    input=(PeInput){7,4,0};CHECK(pe_input_rules(&input,&calls,table,2,&mode)&&calls.calls==1&&!calls.fallback&&mode==1);
    calls=(Calls){0};mode=0;input.held=4;
    CHECK(pe_input_rules(&input,&calls,table,2,&mode)&&!calls.calls&&calls.fallback==1);
    records[0].flags=2|8;records[0].any=3;input=(PeInput){1,0,2};calls=(Calls){0};mode=0;
    CHECK(pe_input_rules(&input,&calls,table,2,&mode)&&calls.calls==1&&!calls.fallback);
    records[0].flags=1|2|4|8;records[0].all=1;calls=(Calls){0};mode=0;
    CHECK(pe_input_rules(&input,&calls,table,2,&mode)&&!calls.calls&&calls.fallback==1);
    records[0].flags=8;input=(PeInput){0,0,2};calls=(Calls){0};mode=0;
    CHECK(pe_input_rules(&input,&calls,table,2,&mode)&&calls.calls==1);
    PeInputRule continuation[]={{0x30,0,0,action},{0x10,0,0,action},{0,0,0,fallback}};
    table[0]=(PeInputRules){continuation,3};calls=(Calls){0};mode=0;
    CHECK(pe_input_rules(&input,&calls,table,2,&mode)&&calls.calls==1&&calls.fallback==1);
    continuation[1].flags=1;continuation[1].all=8;calls=(Calls){0};mode=0;
    CHECK(pe_input_rules(&input,&calls,table,2,&mode)&&calls.calls==1&&!calls.fallback);
    table[0].count=1;calls=(Calls){0};mode=0;
    CHECK(!pe_input_rules(&input,&calls,table,2,&mode)&&!calls.calls);
    continuation[0].callback=NULL;
    CHECK(pe_input_rules(&input,&calls,table,2,&mode)&&!calls.calls);
    mode=-1;CHECK(!pe_input_rules(&input,&calls,table,2,&mode));
    puts("Input: edges, all/any masks, priority, continuation, fallback, mode changes and bounded failure passed");
    return 0;
}
