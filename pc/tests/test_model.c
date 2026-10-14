#include "model.h"
#include <stdio.h>
#include <stdlib.h>
#define CHECK(x) do { if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);exit(1);} } while(0)
static void half(uint8_t *p,unsigned v){p[0]=(uint8_t)v;p[1]=(uint8_t)(v>>8);}
int main(void){
    uint8_t bytes[144]={0}; PeModel model={0}; char error[128];
    half(bytes,8);bytes[2]=1;bytes[3]=3;half(bytes+6,3);half(bytes+14,1);half(bytes+24,3);
    half(bytes+30,3);
    bytes[32]=1;half(bytes+36,0xfefe);half(bytes+38,0xffff);
    half(bytes+40,0x8000);half(bytes+42,0x7fff);half(bytes+44,0xffff);half(bytes+46,0xff00);
    half(bytes+64,0x3344);half(bytes+66,0x1122);
    bytes[79]=0x1a;half(bytes+80,0);half(bytes+82,1);half(bytes+84,2);half(bytes+86,65535);
    CHECK(pe_model_open(bytes,sizeof(bytes),&model,error,sizeof(error)));
    CHECK(model.part_count==1 && model.animation_entries==3 && model.vertex_count==3 && model.packets[3]==1);
    CHECK(model.parts==28 && model.vertices==40 && model.colors==64 && model.descriptors==76);
    CHECK(model.section==88 && model.bounds==104 && model.origin==120 && model.commands==128 && model.textures==132);
    CHECK(model.bytes==bytes && model.primitive_blocks==8);
    PeModelVertex vertex;PeModelPart part;PeModelPrimitive primitive;
    CHECK(pe_model_vertex(&model,0,&vertex) && vertex.x==INT16_MIN && vertex.y==INT16_MAX && vertex.z==-1);
    CHECK(vertex.normal==0xff00 && vertex.color==0x11223344);
    CHECK(pe_model_part(&model,0,&part) && part.first==0 && part.count==3 && part.visible==1 && part.translation_z==0xfffffefe);
    CHECK(pe_model_primitive(&model,3,0,&primitive) && primitive.vertices==3 && primitive.kind==0x1a && primitive.vertex[2]==2);
    CHECK(!pe_model_vertex(&model,3,&vertex) && vertex.color==0x11223344);
    CHECK(!pe_model_primitive(&model,4,0,&primitive) && primitive.kind==0x1a);
    half(bytes+84,3);
    CHECK(!pe_model_open(bytes,sizeof(bytes),&model,error,sizeof(error)) && model.vertex_count==3);
    CHECK(!pe_model_primitive(&model,3,0,&primitive) && primitive.vertex[2]==2);
    half(bytes+84,2);
    CHECK(!pe_model_open(bytes,27,&model,error,sizeof(error)) && model.size==sizeof(bytes));
    CHECK(!pe_model_open(bytes,131,&model,error,sizeof(error)) && model.textures==132);
    half(bytes+28,2);half(bytes+30,2);
    CHECK(!pe_model_open(bytes,sizeof(bytes),&model,error,sizeof(error)) && model.vertices==40);
    CHECK(!pe_model_part(&model,0,&part) && part.count==3);
    half(bytes+28,0);half(bytes+30,3);half(bytes+24,65535);
    CHECK(!pe_model_open(bytes,sizeof(bytes),&model,error,sizeof(error)) && model.matrix_bytes==3);
    uint8_t textured[200]={0}; textured[2]=1;half(textured+6,3);half(textured+8,1);half(textured+10,1);
    half(textured+24,3);half(textured+30,3);
    for(unsigned i=0;i<4;++i){textured[144+i*4]=(uint8_t)(i*2+1);textured[145+i*4]=(uint8_t)(i*2+2);}
    half(textured+146,0xabcd);half(textured+150,0x1234);
    for(unsigned i=0;i<3;++i){textured[160+i*4]=(uint8_t)(i*2+10);textured[161+i*4]=(uint8_t)(i*2+11);}
    half(textured+162,0x5678);half(textured+166,0x9abc);
    CHECK(pe_model_open(textured,sizeof(textured),&model,error,sizeof(error)) && model.textures==144);
    PeModelTexture texture;
    CHECK(pe_model_texture(&model,0,0,&texture) && texture.vertices==4 && texture.uv[3][0]==7);
    CHECK(texture.clut==0xabcd && texture.page==0x1234);
    CHECK(pe_model_texture(&model,1,0,&texture) && texture.vertices==3 && texture.uv[2][1]==15);
    CHECK(texture.clut==0x5678 && texture.page==0x9abc);
    CHECK(!pe_model_texture(&model,1,1,&texture) && texture.clut==0x5678);
    model.textures=200;
    CHECK(!pe_model_texture(&model,0,0,&texture) && texture.clut==0x5678);
    model.textures=144;
    CHECK(!pe_model_open(textured,171,&model,error,sizeof(error)) && model.size==sizeof(textured));
    puts("Model: checked sections, texture records, matrix rounding and transactional bounds passed");
    return 0;
}
