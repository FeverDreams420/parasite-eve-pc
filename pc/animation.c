#include "animation.h"
#include <stdio.h>
static uint16_t half(const uint8_t *p){return (uint16_t)(p[0]|(unsigned)p[1]<<8);}
int pe_animation_open(const uint8_t *bytes,size_t size,PeAnimation *animation,char *error,size_t error_size){
    if(size<12||size>UINT32_MAX)goto malformed;
    PeAnimation next={0};next.bytes=bytes;next.size=size;next.encoding=bytes[0];next.bones=(unsigned)bytes[1]+1;next.frames=bytes[2];
    for(unsigned i=0;i<4;++i)next.body[i]=half(bytes+4+i*2);
    size_t cursor=12;
    for(unsigned channel=0;channel<3+next.bones*3;++channel){
        unsigned width=channel<3||(next.encoding&3)==2?2:1;
        if(cursor>size||width>size-cursor)goto malformed;
        unsigned marker=width==2?half(bytes+cursor):bytes[cursor];
        size_t stride=marker?4:((unsigned)(next.frames>>(width==2?1:2))+1)*4;
        if(stride>size-cursor)goto malformed;
        next.samples[channel]=(uint32_t)(cursor+width);next.constant[channel]=marker!=0;cursor+=stride;
    }
    *animation=next;if(error_size)error[0]=0;return 1;
malformed:
    snprintf(error,error_size,"Malformed animation channel stream");return 0;
}
int pe_animation_frame(const PeAnimation *animation,unsigned frame,PeAnimationFrame *output){
    if(animation->bones>PE_ANIMATION_BONES)return 0;
    PeAnimationFrame next={0};next.bones=animation->bones;
    for(unsigned i=0;i<4;++i)next.body[i]=animation->body[i];
    for(unsigned channel=0;channel<3+animation->bones*3;++channel){
        unsigned width=channel<3||(animation->encoding&3)==2?2:1;
        if(!animation->constant[channel]&&frame>=(animation->frames?animation->frames:1))return 0;
        uint64_t offset=animation->samples[channel]+(uint64_t)(animation->constant[channel]?0:frame)*width;
        if(offset>animation->size||width>animation->size-offset)return 0;
        unsigned value=width==2?half(animation->bytes+(size_t)offset):animation->bytes[offset];
        if(channel<3)next.translation[channel]=value&32768?(int)value-65536:(int)value;
        else next.angles[(channel-3)/3][(channel-3)%3]=(uint16_t)(width==1?value*16:value);
    }
    *output=next;return 1;
}
