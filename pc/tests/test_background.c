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
    word(bytes+104,200);CHECK(!pe_background_open(&bank,&bg)&&bg.count==1);
    CHECK(!pe_background_tile(&bg,0,0,&tile)&&tile.x==3);
    puts("Background: relative tile spans, packed position/UV/CLUT/page and transactional bounds passed");return 0;
}
