#include "background.h"
#include <stdlib.h>
#include <string.h>
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
    next.modulus[0]=(uint16_t)half(p+4);next.modulus[1]=(uint16_t)half(p+6);
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
static int signed_half(unsigned value){value&=65535;return value&32768?(int)value-65536:(int)value;}
int pe_background_motion(const PeBackground *bg,unsigned index,PeBackgroundMotion *motion){
    PeBackgroundEntry entry;if(!pe_background_entry(bg,index,&entry))return 0;
    const uint8_t *p=bg->bank.bytes+bg->entries+(size_t)index*56;PeBackgroundMotion next={0};next.flags=p[0];
    for(unsigned i=0;i<2;++i){next.modulus[i]=(uint16_t)half(p+4+i*2);next.base[i]=(int16_t)signed_half(half(p+8+i*2));
        next.position[i]=(int16_t)signed_half(half(p+12+i*2));next.speed[i]=(int16_t)signed_half(half(p+28+i*2));next.fraction[i]=(uint16_t)half(p+32+i*2);}
    *motion=next;return 1;
}
static int32_t shift8(uint32_t value){
    int64_t v=value&0x80000000?(int64_t)value-INT64_C(0x100000000):value;
    return (int32_t)(v>=0?v/256:-((-v+255)/256));
}
int pe_background_motion_step(PeBackgroundMotion *motion,const PeCameraScroll *camera,uint32_t scene_flags){
    if(scene_flags&0x104)return 1;
    if((motion->flags&4)&&(!motion->modulus[0]||!motion->modulus[1]))return 0;
    PeBackgroundMotion next=*motion;
    for(unsigned axis=0;axis<2;++axis){
        if(motion->flags&4){
            uint32_t position=((uint32_t)(int32_t)motion->position[axis]*256)|(motion->fraction[axis]&255);
            position+=(uint32_t)(int32_t)motion->speed[axis];
            next.position[axis]=(int16_t)signed_half((unsigned)(shift8(position)%motion->modulus[axis]));next.fraction[axis]=(uint16_t)(position&255);
        }
        if(motion->flags&8){
            int delta=(int)camera->position[axis]-camera->origin[axis];
            uint32_t position=(uint32_t)(int32_t)motion->base[axis]*256+(uint32_t)delta*(uint32_t)(int32_t)motion->speed[axis];
            next.position[axis]=(int16_t)signed_half((unsigned)shift8(position));next.fraction[axis]=(uint16_t)(position&255);
        }
    }
    *motion=next;return 1;
}
int pe_background_draw_tiles(const PeBackground *bg,unsigned group,const int16_t display[2],
                              PeBackgroundDrawTile *output,unsigned capacity,unsigned *written){
    if((uint64_t)capacity*sizeof(*output)>SIZE_MAX)return 0;
    PeBackgroundDrawTile *next=capacity?malloc((size_t)capacity*sizeof(*next)):NULL;
    if(capacity&&!next)return 0;unsigned count=0;
    for(unsigned i=0;i<bg->count;++i){PeBackgroundEntry entry;if(!pe_background_entry(bg,i,&entry))goto failed;
        if(!(entry.flags&2)||entry.group!=group)continue;
        int scroll[2]={entry.screen[0]+(uint16_t)display[0],entry.screen[1]+(uint16_t)display[1]};
        if(entry.flags&4)for(unsigned axis=0;axis<2;++axis){
            int modulus=entry.modulus[axis],extent=axis?224:320;if(!modulus)goto failed;
            scroll[axis]=(signed_half((unsigned)scroll[axis])-extent+modulus)%modulus-(modulus-extent);
        }
        for(unsigned j=0;j<entry.tiles;++j){PeBackgroundTile tile;if(!pe_background_tile(bg,i,j,&tile))goto failed;
            int x=scroll[0]+tile.x,y=scroll[1]+tile.y;
            if(entry.flags&4){
                if(signed_half((unsigned)x)>=320)x-=entry.modulus[0];else if(signed_half((unsigned)x)<-15)x+=entry.modulus[0];
                if(signed_half((unsigned)y)>=224)y-=entry.modulus[1];else if(signed_half((unsigned)y)<-15)y+=entry.modulus[1];
            }
            unsigned depth=half(bg->bank.bytes+38)+((entry.flags>>8)&4095)+(tile.position&4095);
            if((uint16_t)(x+15)>=335||(uint16_t)(y+15)>=239||(uint16_t)(depth-8)>=4081)continue;
            if(count==capacity)goto failed;
            next[count++]=(PeBackgroundDrawTile){tile,(int16_t)signed_half((unsigned)x),(int16_t)signed_half((unsigned)y),(uint16_t)depth,i,j};
        }
    }
    if(count)memcpy(output,next,(size_t)count*sizeof(*next));*written=count;free(next);return 1;
failed:free(next);return 0;
}
