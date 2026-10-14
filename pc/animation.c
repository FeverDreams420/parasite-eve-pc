#include "animation.h"
#include <stdio.h>
#include <string.h>
static int64_t signed_word(uint32_t value){return value&UINT32_C(0x80000000)?(int64_t)value-INT64_C(0x100000000):value;}
static int32_t whole(uint32_t value){int64_t v=signed_word(value);return (int32_t)(v>=0?v/65536:-((-v+65535)/65536));}
int pe_animation_advance(PeAnimationClock *clock,uint32_t step,uint32_t *flags){
    if(clock->last>255||(*flags&0x200000))return 0;
    PeAnimationClock next=*clock;uint32_t f=(*flags&~UINT32_C(8))|0x800000;
    uint32_t old=clock->frame;next.previous=old;
    if(f&0x100)goto done;
    if((f&0x200)&&(old>>16)==clock->stop)goto done;
    uint32_t value=old+step,end=(uint32_t)clock->stop<<16;
    if(f&0x200){
        if((signed_word(old)<signed_word(end)&&signed_word(end)<signed_word(value))||
           (signed_word(end)<signed_word(old)&&signed_word(value)<signed_word(end))){value=end;goto store;}
    }
    if((int)clock->last<whole(value)){
        int32_t limit=whole(value)%((int)clock->last+1);
        old=0;f|=8;value=(uint32_t)limit<<16;
    }else if(signed_word(value)<0){
        old=(uint32_t)clock->last<<16;value+=((uint32_t)clock->last+1)<<16;f|=8;
    }
    if(f&0x200){
        if(signed_word(end)>=signed_word(old)&&signed_word(end)<signed_word(value)){value=end;goto store;}
        if(signed_word(old)<signed_word(end))goto store;
        if(signed_word(value)<signed_word(end))value=end;
    }
store:next.frame=value;
done:*clock=next;*flags=f;return 1;
}
int pe_animation_matrices(const PeAnimationFrame *frame,const PeRotationOverride overrides[2],
                          PeMatrix *matrices,unsigned capacity){
    if(!frame->bones||frame->bones>PE_ANIMATION_BONES||frame->bones>capacity)return 0;
    uint16_t angles[PE_ANIMATION_BONES][3];memcpy(angles,frame->angles,sizeof(angles));
    if(overrides)for(unsigned i=0;i<2;++i){
        unsigned index=overrides[i].matrix_index;
        if(!index)continue;
        if(index>=frame->bones)return 0;
        for(unsigned axis=0;axis<3;++axis){
            if(overrides[i].flags&(8u<<axis))angles[index][axis]=overrides[i].angles[axis];
            else if(overrides[i].flags&(1u<<axis))angles[index][axis]=(uint16_t)(angles[index][axis]+overrides[i].angles[axis]);
        }
    }
    for(unsigned i=0;i<frame->bones;++i)pe_matrix_rotation_yxz(angles[i],&matrices[i]);
    for(unsigned axis=0;axis<3;++axis)matrices[0].translation[axis]=(uint32_t)frame->translation[axis];
    return 1;
}
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
