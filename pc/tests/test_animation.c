#include "animation.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);exit(1);}}while(0)
static void half(uint8_t*p,unsigned v){p[0]=(uint8_t)v;p[1]=(uint8_t)(v>>8);}
int main(void){
    uint8_t bytes[40]={0};bytes[2]=3;half(bytes+4,0x8000);half(bytes+10,42);
    half(bytes+14,65535);half(bytes+16,2);half(bytes+18,3);
    half(bytes+20,1);half(bytes+22,0x8000);half(bytes+24,65535);half(bytes+26,0x7fff);
    bytes[29]=0;bytes[30]=128;bytes[31]=255;bytes[32]=1;bytes[33]=17;
    bytes[37]=1;bytes[38]=2;bytes[39]=3;
    PeAnimation animation={0};PeAnimationFrame frame;char error[128];
    CHECK(pe_animation_open(bytes,sizeof(bytes),&animation,error,sizeof(error)));
    CHECK(pe_animation_frame(&animation,2,&frame));
    CHECK(frame.bones==1&&frame.translation[0]==3&&frame.translation[1]==-32768&&frame.translation[2]==32767);
    CHECK(frame.angles[0][0]==4080&&frame.angles[0][1]==272&&frame.angles[0][2]==48);
    CHECK(frame.body[0]==0x8000&&frame.body[3]==42);
    CHECK(!pe_animation_frame(&animation,3,&frame)&&frame.translation[0]==3);
    CHECK(!pe_animation_open(bytes,39,&animation,error,sizeof(error))&&animation.size==40);
    uint8_t shorts[44]={0};shorts[0]=2;shorts[2]=2;
    for(unsigned c=0;c<3;++c){half(shorts+12+c*4,1);half(shorts+14+c*4,c+10);}
    half(shorts+26,0x1234);half(shorts+28,0xfedc); /* variable x angle, padded stride 8 */
    half(shorts+32,1);half(shorts+34,0xffff);
    half(shorts+38,10);half(shorts+40,20);
    CHECK(pe_animation_open(shorts,sizeof(shorts),&animation,error,sizeof(error)));
    CHECK(pe_animation_frame(&animation,1,&frame));
    CHECK(frame.translation[2]==12&&frame.angles[0][0]==0xfedc&&frame.angles[0][1]==0xffff&&frame.angles[0][2]==20);
    uint8_t constants[36]={0};
    for(unsigned c=0;c<3;++c){half(constants+12+c*4,1);half(constants+14+c*4,c+1);constants[24+c*4]=1;constants[25+c*4]=(uint8_t)(c+2);}
    CHECK(pe_animation_open(constants,sizeof(constants),&animation,error,sizeof(error)));
    CHECK(pe_animation_frame(&animation,999,&frame)&&frame.translation[0]==1&&frame.angles[0][2]==64);
    memset(&frame,0,sizeof(frame));frame.bones=2;frame.translation[0]=-7;
    frame.angles[1][0]=65535;
    PeMatrix matrices[2]={0};matrices[1].translation[2]=123;
    PeRotationOverride overrides[2]={{1,9,{1024,0,0}},{1,1,{3072,0,0}}};
    CHECK(pe_animation_matrices(&frame,overrides,matrices,2));
    CHECK(matrices[0].translation[0]==(uint32_t)-7&&matrices[1].translation[2]==123);
    CHECK(matrices[1].rotation[0][0]==4096&&matrices[1].rotation[1][1]==4096);
    PeMatrix saved[2];memcpy(saved,matrices,sizeof(saved));overrides[1].matrix_index=2;
    CHECK(!pe_animation_matrices(&frame,overrides,matrices,2)&&!memcmp(saved,matrices,sizeof(saved)));
    overrides[1].matrix_index=0;overrides[0].matrix_index=0;
    CHECK(pe_animation_matrices(&frame,overrides,matrices,2));
    CHECK(!pe_animation_matrices(&frame,NULL,matrices,1));
    puts("Animation: channels, ordered overrides, root/non-root translations and transactional failure passed");
    return 0;
}
