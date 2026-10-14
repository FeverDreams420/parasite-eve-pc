#include "background.h"
#include <stdio.h>
#include <stdlib.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);exit(1);}}while(0)
static void word(uint8_t*p,uint32_t v){for(unsigned i=0;i<4;++i)p[i]=(uint8_t)(v>>(8*i));}
int main(void){
    uint8_t bytes[160]={0};bytes[6]=1;bytes[20]=60;bytes[60]=2;bytes[98]=1;
    word(bytes+100,60);word(bytes+104,64);word(bytes+120,(3u<<22)|(4u<<12));
    word(bytes+124,5|0x2000|0x20000|0x8000|(2u<<22)|(0x12u<<24));word(bytes+128,0x10000007);
    PeCameraBank bank={0};bank.bytes=bytes;bank.size=sizeof(bytes);
    PeBackground bg={0};PeBackgroundEntry entry;PeBackgroundTile tile;
    CHECK(pe_background_open(&bank,&bg)&&bg.count==1);
    CHECK(pe_background_entry(&bg,0,&entry)&&entry.positions==120&&entry.textures==124);
    CHECK(pe_background_tile(&bg,0,0,&tile)&&tile.x==3&&tile.y==4&&tile.u==7&&tile.v==0x12);
    CHECK(tile.clut==336&&tile.page==210&&tile.semitransparent);
    CHECK(!pe_background_tile(&bg,0,1,&tile)&&tile.clut==336);
    PeBackgroundMotion motion;CHECK(pe_background_motion(&bg,0,&motion)&&motion.flags==2);
    motion.flags=4;motion.modulus[0]=320;motion.modulus[1]=224;motion.position[0]=-1;motion.fraction[0]=128;motion.speed[0]=128;
    PeCameraScroll camera={0};CHECK(pe_background_motion_step(&motion,&camera,0)&&motion.position[0]==0&&!motion.fraction[0]);
    motion.speed[0]=-128;CHECK(pe_background_motion_step(&motion,&camera,0)&&motion.position[0]==-1&&motion.fraction[0]==128);
    motion.flags=12;motion.base[0]=10;motion.speed[0]=128;camera.position[0]=3;camera.origin[0]=0;
    CHECK(pe_background_motion_step(&motion,&camera,0)&&motion.position[0]==11&&motion.fraction[0]==128);
    motion.modulus[0]=0;CHECK(!pe_background_motion_step(&motion,&camera,0)&&motion.position[0]==11);
    CHECK(pe_background_motion_step(&motion,&camera,0x100)&&motion.position[0]==11);
    motion.flags=8;motion.base[0]=32767;motion.speed[0]=32767;camera.position[0]=32767;camera.origin[0]=-32768;
    CHECK(pe_background_motion_step(&motion,&camera,0));
    uint32_t wrapped=32767u*256+65535u*32767;
    int64_t signed_position=wrapped&0x80000000?(int64_t)wrapped-INT64_C(0x100000000):wrapped;
    int64_t shifted=signed_position>=0?signed_position/256:-((-signed_position+255)/256);
    CHECK((uint16_t)motion.position[0]==(uint16_t)shifted&&motion.fraction[0]==(wrapped&255));
    bytes[38]=8;int16_t display[2]={0,0};PeBackgroundDrawTile draw={0};unsigned written=99;
    CHECK(pe_background_draw_tiles(&bg,0,display,&draw,1,&written)&&written==1&&draw.x==3&&draw.y==4&&draw.depth==8);
    written=99;CHECK(!pe_background_draw_tiles(&bg,0,display,&draw,0,&written)&&written==99&&draw.x==3);
    CHECK(pe_background_draw_tiles(&bg,1,display,&draw,1,&written)&&!written&&draw.x==3);
    display[0]=317;CHECK(pe_background_draw_tiles(&bg,0,display,&draw,1,&written)&&!written);
    bytes[60]=6;bytes[64]=64;bytes[65]=1;bytes[66]=224;
    CHECK(pe_background_draw_tiles(&bg,0,display,&draw,1,&written)&&written==1&&draw.x==0&&draw.y==4);
    bytes[64]=bytes[65]=0;written=99;
    CHECK(!pe_background_draw_tiles(&bg,0,display,&draw,1,&written)&&written==99&&draw.x==0);
    bytes[60]=2;display[0]=-4;
    CHECK(pe_background_draw_tiles(&bg,0,display,&draw,1,&written)&&written==1&&draw.x==-1);
    word(bytes+104,200);CHECK(!pe_background_open(&bank,&bg)&&bg.count==1);
    CHECK(!pe_background_tile(&bg,0,0,&tile)&&tile.x==3);
    puts("Background: relative tile spans, packed position/UV/CLUT/page and transactional bounds passed");return 0;
}
