#include "camera.h"
#include <stdio.h>
#include <stdlib.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);exit(1);}}while(0)
static void half(uint8_t*p,unsigned v){p[0]=(uint8_t)v;p[1]=(uint8_t)(v>>8);}
int main(void){
    uint8_t bytes[112]={0};bytes[0]=112;bytes[10]=1;bytes[28]=60;
    half(bytes+60,320);half(bytes+62,4096);half(bytes+70,4096);half(bytes+78,4096);
    bytes[80]=255;half(bytes+100,640);half(bytes+102,240);half(bytes+104,65535);half(bytes+106,321);
    PeCameraBank bank={0};PeCamera camera={0};
    CHECK(pe_camera_bank_open(bytes,sizeof(bytes),&bank)&&bank.count==1);
    CHECK(pe_camera_group(&bank,0,&camera)&&camera.distance==320&&camera.view.rotation[2][2]==4096);
    CHECK(camera.view.translation[0]==255&&camera.width==640&&camera.min_x==-1&&camera.max_x==321);
    CHECK(!pe_camera_group(&bank,1,&camera)&&camera.distance==320);
    camera.min_y=-20;camera.max_y=20;int16_t position[2]={123,456};
    pe_camera_clamp(&camera,0,0x8000,123,position);CHECK(position[0]==123&&position[1]==456);
    pe_camera_clamp(&camera,0x40,0x8000,123,position);CHECK(position[0]==-1&&position[1]==20);
    pe_camera_clamp(&camera,0x40,0x10064,0xffffffe2,position);CHECK(position[0]==100&&position[1]==-20);
    bytes[10]=2;CHECK(!pe_camera_bank_open(bytes,sizeof(bytes),&bank)&&bank.count==1);
    puts("Camera: group matrix, projection distance, viewport bounds and transactional truncation passed");return 0;
}
