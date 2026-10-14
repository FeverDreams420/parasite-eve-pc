#include "camera.h"
static uint16_t half(const uint8_t *p){return (uint16_t)(p[0]|(unsigned)p[1]<<8);}
static uint32_t word(const uint8_t *p){return half(p)|(uint32_t)half(p+2)<<16;}
static int16_t sign(uint16_t v){return (int16_t)(v&32768?(int)v-65536:(int)v);}
int pe_camera_bank_open(const uint8_t *bytes,size_t size,PeCameraBank *bank){
    if(size<60)return 0;
    uint32_t declared=word(bytes),offset=word(bytes+28);unsigned count=half(bytes+10);
    if(declared<60||declared>size||offset<60||offset>declared||!count||count>(declared-offset)/52)return 0;
    *bank=(PeCameraBank){bytes,declared,offset,count};return 1;
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
