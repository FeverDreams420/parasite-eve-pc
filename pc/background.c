#include "background.h"
static unsigned half(const uint8_t *p){return p[0]|(unsigned)p[1]<<8;}
static uint32_t word(const uint8_t *p){return half(p)|(uint32_t)half(p+2)<<16;}
static int relative(const PeBackground *bg,uint64_t base,uint32_t offset,unsigned count,unsigned stride,uint32_t *out){
    int64_t signed_offset=offset&0x80000000?(int64_t)offset-INT64_C(0x100000000):offset;
    int64_t address=(int64_t)base+signed_offset;
    if(address<0||(uint64_t)address>bg->bank.size||(uint64_t)count*stride>bg->bank.size-(uint64_t)address)return 0;
    *out=(uint32_t)address;return 1;
}
int pe_background_entry(const PeBackground *bg,unsigned index,PeBackgroundEntry *entry){
    if(index>=bg->count)return 0;
    uint64_t offset=bg->entries+(uint64_t)index*56;
    if(offset>bg->bank.size||56>bg->bank.size-offset)return 0;
    const uint8_t *p=bg->bank.bytes+(size_t)offset;PeBackgroundEntry next={0};
    next.flags=word(p);next.screen[0]=(uint16_t)half(p+12);next.screen[1]=(uint16_t)half(p+14);
    next.group=p[36];next.tiles=(uint16_t)half(p+38);
    if(!relative(bg,offset,word(p+40),next.tiles,4,&next.positions)||!relative(bg,offset,word(p+44),next.tiles,8,&next.textures))return 0;
    *entry=next;return 1;
}
int pe_background_open(const PeCameraBank *bank,PeBackground *background){
    if(!bank->bytes||bank->size<60||bank->size>UINT32_MAX)return 0;
    PeBackground next={*bank,word(bank->bytes+20),half(bank->bytes+6)};
    if(next.entries>bank->size||(uint64_t)next.count*56>bank->size-next.entries)return 0;
    for(unsigned i=0;i<next.count;++i){PeBackgroundEntry entry;if(!pe_background_entry(&next,i,&entry))return 0;}
    *background=next;return 1;
}
int pe_background_tile(const PeBackground *bg,unsigned index,unsigned tile,PeBackgroundTile *output){
    PeBackgroundEntry entry;if(!pe_background_entry(bg,index,&entry)||tile>=entry.tiles)return 0;
    const uint8_t *p=bg->bank.bytes+entry.textures+(size_t)tile*8;
    PeBackgroundTile next={0};next.position=word(bg->bank.bytes+entry.positions+(size_t)tile*4);
    next.texture[0]=word(p);next.texture[1]=word(p+4);next.x=(uint16_t)(next.position>>22);next.y=(uint16_t)((next.position>>12)&1023);
    next.u=p[4];next.v=p[3];next.semitransparent=(next.texture[1]&0x10000000)!=0;
    next.clut=(uint16_t)((next.texture[0]&511)<<6|((next.texture[0]>>9)&63));
    unsigned abr=(next.texture[0]>>22)&3,x=(next.texture[0]>>10)&0x3c0,y=(next.texture[0]>>7)&256;
    next.page=(uint16_t)((128|(abr<<5)|((y&256)>>4)|(x>>6))&0x9ff);
    *output=next;return 1;
}
