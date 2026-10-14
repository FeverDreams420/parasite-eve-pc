#include "camera.h"
static uint16_t half(const uint8_t *p){return (uint16_t)(p[0]|(unsigned)p[1]<<8);}
static uint32_t word(const uint8_t *p){return half(p)|(uint32_t)half(p+2)<<16;}
static int16_t sign(uint16_t v){return (int16_t)(v&32768?(int)v-65536:(int)v);}
int pe_camera_bank_open(const uint8_t *bytes,size_t size,PeCameraBank *bank){
    if(size<60)return 0;
    uint32_t declared=word(bytes),offset=word(bytes+28);unsigned count=half(bytes+10);
    if(declared<60||declared>size||offset<60||offset>declared||!count||count>(declared-offset)/52)return 0;
    *bank=(PeCameraBank){bytes,declared,offset,count,half(bytes+44),half(bytes+46)};return 1;
}
int pe_room_cameras(const PeRoom *room,PeCameraBank *bank){
    const uint8_t *records;unsigned count;
    if(!pe_room_blob_table(&room->data,7,8,&records,&count)||!count)return 0;
    size_t size=word(records)&0xffffff;uint32_t offset=word(records+4)&0xffffff;
    if(offset>room->data.size||size>room->data.size-offset)return 0;
    return pe_camera_bank_open(room->data.bytes+offset,size,bank);
}
int pe_camera_group(const PeCameraBank *bank,unsigned group,PeCamera *camera){
    if(group>=bank->count)return 0;
    uint64_t offset=bank->views+(uint64_t)group*52;
    if(offset>bank->size||52>bank->size-offset)return 0;
    const uint8_t *p=bank->bytes+(size_t)offset;PeCamera next={0};next.distance=half(p);
    for(unsigned i=0;i<9;++i)next.view.rotation[i/3][i%3]=sign(half(p+2+i*2));
    for(unsigned i=0;i<3;++i)next.view.translation[i]=word(p+20+i*4);
    next.width=half(p+40);next.height=half(p+42);
    next.min_x=sign(half(p+44));next.max_x=sign(half(p+46));next.min_y=sign(half(p+48));next.max_y=sign(half(p+50));
    *camera=next;return 1;
}
void pe_camera_clamp(const PeCamera *camera,uint32_t flags,uint32_t x,uint32_t y,int16_t position[2]){
    if(!(flags&0x40))return;
    int16_t sx=sign((uint16_t)x),sy=sign((uint16_t)y);
    position[0]=sx<camera->min_x?camera->min_x:sx>camera->max_x?camera->max_x:sx;
    position[1]=sy<camera->min_y?camera->min_y:sy>camera->max_y?camera->max_y:sy;
}
void pe_camera_scroll_init(const PeCameraBank *bank,const PeCamera *camera,PeCameraScroll *scroll){
    int x=((int)camera->min_x+camera->max_x)/2,y=((int)camera->min_y+camera->max_y)/2;
    *scroll=(PeCameraScroll){.position={(int16_t)x,(int16_t)y},.origin={(int16_t)x,(int16_t)y},
        .bounds={camera->min_x,camera->max_x,camera->min_y,camera->max_y},
        .display={sign((uint16_t)(bank->source_x-(x-160))),sign((uint16_t)(bank->source_y-(y-112)))},.flags=0x70};
}
void pe_camera_background_offset(const PeCameraBank *bank,PeCameraScroll *scroll){
    scroll->display[0]=sign((uint16_t)(bank->source_x-((uint16_t)scroll->position[0]-160)));
    scroll->display[1]=sign((uint16_t)(bank->source_y-((uint16_t)scroll->position[1]-112)));
}
int pe_camera_transition_step(const PeCamera *camera,PeCameraScroll *scroll,uint32_t *flags){
    if(!(*flags&0x40))return -20;
    unsigned mode=*flags&7;if(mode<1||mode>3)return 0;
    int duration=sign(scroll->duration),time=sign(scroll->elapsed);if(!duration)return -1;
    int weight=0;
    if(*flags&8){
        uint16_t angles[3]={0,(uint16_t)(time*2048/duration+2048),0};PeMatrix rotation={0};
        pe_matrix_rotation_xyz(angles,&rotation);weight=rotation.rotation[0][0]+4096;
    }
    for(unsigned axis=0;axis<2;++axis){
        int delta=sign((uint16_t)(scroll->target[axis]-scroll->start[axis]));
        int moved=*flags&8?delta*weight/8192:delta*time/duration;
        scroll->position[axis]=sign((uint16_t)(scroll->start[axis]+moved));
    }
    scroll->screen_offset[0]=(uint16_t)(160-(scroll->position[0]-sign(camera->width)/2));
    scroll->screen_offset[1]=(uint16_t)(112-(scroll->position[1]-sign(camera->height)/2));
    if(mode==1)*flags=(*flags&~UINT32_C(7))|2;
    scroll->elapsed=(uint16_t)(scroll->elapsed+1);
    if(sign(scroll->elapsed)>duration)*flags=(*flags&~UINT32_C(7))|((*flags&7)==2?0x84:0x80);
    return 0;
}
int pe_camera_transition_start(PeCameraScroll *scroll,uint32_t *flags,uint16_t x,uint16_t y,uint16_t duration,int smooth){
    if(!(*flags&0x40))return -19;
    if(!duration)return -1;
    for(unsigned i=0;i<2;++i)scroll->start[i]=(uint16_t)scroll->position[i];
    scroll->target[0]=x;scroll->target[1]=y;scroll->duration=duration;scroll->elapsed=1;
    *flags=(*flags&~UINT32_C(15))|(smooth?9:1);return 0;
}
