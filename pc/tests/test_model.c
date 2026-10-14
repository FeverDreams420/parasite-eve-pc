#include "model.h"
#include <stdio.h>
#include <stdlib.h>
#define CHECK(x) do { if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);exit(1);} } while(0)
static void half(uint8_t *p,unsigned v){p[0]=(uint8_t)v;p[1]=(uint8_t)(v>>8);}
int main(void){
    uint8_t bytes[144]={0}; PeModel model={0}; char error[128];
    half(bytes,8);bytes[2]=1;bytes[3]=3;half(bytes+6,3);half(bytes+14,1);half(bytes+24,3);
    half(bytes+30,3);
    CHECK(pe_model_open(bytes,sizeof(bytes),&model,error,sizeof(error)));
    CHECK(model.part_count==1 && model.animation_entries==3 && model.vertex_count==3 && model.packets[3]==1);
    CHECK(model.parts==28 && model.vertices==40 && model.colors==64 && model.descriptors==76);
    CHECK(model.section==88 && model.bounds==104 && model.origin==120 && model.commands==128 && model.textures==132);
    CHECK(model.bytes==bytes && model.primitive_blocks==8);
    CHECK(!pe_model_open(bytes,27,&model,error,sizeof(error)) && model.size==sizeof(bytes));
    CHECK(!pe_model_open(bytes,131,&model,error,sizeof(error)) && model.textures==132);
    half(bytes+28,2);half(bytes+30,2);
    CHECK(!pe_model_open(bytes,sizeof(bytes),&model,error,sizeof(error)) && model.vertices==40);
    half(bytes+28,0);half(bytes+30,3);half(bytes+24,65535);
    CHECK(!pe_model_open(bytes,sizeof(bytes),&model,error,sizeof(error)) && model.matrix_bytes==3);
    puts("Model prefix: contiguous sections, matrix-byte rounding, part bounds and transactional failure passed");
    return 0;
}
