#ifndef PE_PC_CAMERA_H
#define PE_PC_CAMERA_H
#include "matrix.h"
#include "room.h"
typedef struct PeCameraBank { const uint8_t *bytes;size_t size;uint32_t views;unsigned count; } PeCameraBank;
typedef struct PeCamera { PeMatrix view;uint16_t distance,width,height;int16_t min_x,max_x,min_y,max_y; } PeCamera;
int pe_camera_bank_open(const uint8_t *bytes,size_t size,PeCameraBank *bank);
int pe_room_cameras(const PeRoom *room,PeCameraBank *bank);
int pe_camera_group(const PeCameraBank *bank,unsigned group,PeCamera *camera);
/* Original halfword coordinates; disabled flag 0x40 preserves position. */
void pe_camera_clamp(const PeCamera *camera,uint32_t flags,uint32_t x,uint32_t y,int16_t position[2]);
#endif
