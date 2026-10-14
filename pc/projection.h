#ifndef PE_PC_PROJECTION_H
#define PE_PC_PROJECTION_H
#include "matrix.h"
typedef struct PeProjection { uint32_t offset_x,offset_y,dqb;uint16_t distance;int16_t dqa; } PeProjection;
typedef struct PeProjectedVertex { int16_t screen[2];uint16_t depth,interpolation;uint32_t flags; } PeProjectedVertex;
/* Uses the renderer's single-threaded PsyZ GTE owner; no GPU/window required.
 * Offsets are raw signed 16.16 words. Depth/flags retain original clipping. */
void pe_project_point(const PeProjection *projection,const PeMatrix *matrix,const int16_t point[3],PeProjectedVertex *output);
int pe_model_project(const PeModel *model,const PeMatrix *view,const PeMatrix *pose,unsigned pose_count,
                     const PeProjection *projection,PeProjectedVertex *output,unsigned capacity,unsigned *written);
#endif
