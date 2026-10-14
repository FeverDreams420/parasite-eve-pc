#ifndef PE_PC_CAMERA_H
#define PE_PC_CAMERA_H
#include "matrix.h"
#include "room.h"
typedef struct PeCameraBank { const uint8_t *bytes;size_t size;uint32_t views;unsigned count;uint16_t source_x,source_y; } PeCameraBank;
typedef struct PeCamera { PeMatrix view;uint16_t distance,width,height;int16_t min_x,max_x,min_y,max_y; } PeCamera;
typedef struct PeCameraScroll { int16_t position[2],origin[2],bounds[4],display[2];uint16_t start[2],target[2],elapsed,duration,screen_offset[2];uint32_t flags; } PeCameraScroll;
int pe_camera_bank_open(const uint8_t *bytes,size_t size,PeCameraBank *bank);
int pe_room_cameras(const PeRoom *room,PeCameraBank *bank);
int pe_camera_group(const PeCameraBank *bank,unsigned group,PeCamera *camera);
/* Original halfword coordinates; absent flag 0x40 preserves position. */
void pe_camera_clamp(const PeCamera *camera,uint32_t flags,uint32_t x,uint32_t y,int16_t position[2]);
/* Original DrawMeshList startup centre and display-source translation. */
void pe_camera_scroll_init(const PeCameraBank *bank,const PeCamera *camera,PeCameraScroll *scroll);
/* -20: disabled; -1: invalid zero duration; otherwise original step result. */
int pe_camera_transition_step(const PeCamera *camera,PeCameraScroll *scroll,uint32_t *flags);
#endif
