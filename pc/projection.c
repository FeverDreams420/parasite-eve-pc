#include "projection.h"
#include <psyz/gte.h>
#include <stdlib.h>
#include <string.h>
static int16_t signed_half(unsigned value){value&=65535;return (int16_t)(value&32768?(int)value-65536:(int)value);}
/* Retail loads three packed vectors even at a part tail, including bytes
 * after the declared vertex array. Read only within the owned model blob. */
static int raw_triplet(const PeModel *model,unsigned index,int16_t points[3][3]){
    uint64_t offset=model->vertices+(uint64_t)index*8;
    if(offset>model->size||22>model->size-offset)return 0;
    for(unsigned i=0;i<3;++i)for(unsigned axis=0;axis<3;++axis){
        const uint8_t *p=model->bytes+(size_t)offset+i*8+axis*2;
        points[i][axis]=signed_half(p[0]|(unsigned)p[1]<<8);
    }
    return 1;
}
static void configure(const PeProjection *projection,const PeMatrix *matrix){
    uint16_t coefficients[10]={0};
    for(unsigned i=0;i<9;++i)coefficients[i]=(uint16_t)matrix->rotation[i/3][i%3];
    for(unsigned i=0;i<5;++i)Psyz_GteCtrlWrite(i,coefficients[i*2]|(uint32_t)coefficients[i*2+1]<<16);
    for(unsigned i=0;i<3;++i)Psyz_GteCtrlWrite(5+i,matrix->translation[i]);
    Psyz_GteCtrlWrite(24,projection->offset_x);Psyz_GteCtrlWrite(25,projection->offset_y);
    Psyz_GteCtrlWrite(26,projection->distance);Psyz_GteCtrlWrite(27,(uint16_t)projection->dqa);Psyz_GteCtrlWrite(28,projection->dqb);
}
void pe_project_point(const PeProjection *projection,const PeMatrix *matrix,const int16_t point[3],PeProjectedVertex *output){
    configure(projection,matrix);
    Psyz_GteDataWrite(0,(uint16_t)point[0]|(uint32_t)(uint16_t)point[1]<<16);Psyz_GteDataWrite(1,(uint16_t)point[2]);
    Psyz_GteRtps();uint32_t screen=Psyz_GteDataRead(14);
    *output=(PeProjectedVertex){{signed_half(screen),signed_half(screen>>16)},(uint16_t)Psyz_GteDataRead(19),(uint16_t)Psyz_GteDataRead(8),Psyz_GteCtrlRead(31)};
}
void pe_project_triplet(const PeProjection *projection,const PeMatrix *matrix,const int16_t points[3][3],PeProjectedVertex output[3]){
    configure(projection,matrix);
    for(unsigned i=0;i<3;++i){
        Psyz_GteDataWrite(i*2,(uint16_t)points[i][0]|(uint32_t)(uint16_t)points[i][1]<<16);
        Psyz_GteDataWrite(i*2+1,(uint16_t)points[i][2]);
    }
    Psyz_GteRtpt();uint32_t flags=Psyz_GteCtrlRead(31);uint16_t interpolation=(uint16_t)Psyz_GteDataRead(8);
    for(unsigned i=0;i<3;++i){uint32_t screen=Psyz_GteDataRead(12+i);
        output[i]=(PeProjectedVertex){{signed_half(screen),signed_half(screen>>16)},(uint16_t)Psyz_GteDataRead(17+i),interpolation,flags};
    }
}
int pe_camera_focus_transition(const PeCamera *camera,PeCameraScroll *scroll,uint32_t *flags,
                               const uint32_t position[3],int16_t focus_height,int32_t duration,int32_t mode){
    if(!(scroll->flags&0x40))return -21;
    uint16_t ticks=duration==-1?30:(uint16_t)duration;if(!ticks)return -1;
    int16_t point[3]={signed_half(position[0]>>16),signed_half((position[1]>>16)-(uint16_t)focus_height),signed_half(position[2]>>16)};
    PeProjection projection={160u<<16,112u<<16,0,camera->distance,0};PeProjectedVertex projected;
    pe_project_point(&projection,&camera->view,point,&projected);
    int x=signed_half(camera->width)/2-160+projected.screen[0];
    int y=signed_half(camera->height)/2-112+projected.screen[1];
    if(x<camera->min_x)x=camera->min_x;else if(x>camera->max_x)x=camera->max_x;
    if(y<camera->min_y)y=camera->min_y;else if(y>camera->max_y)y=camera->max_y;
    for(unsigned i=0;i<2;++i)scroll->start[i]=(uint16_t)scroll->position[i];
    scroll->target[0]=(uint16_t)x;scroll->target[1]=(uint16_t)y;scroll->elapsed=1;scroll->duration=ticks;
    *flags=mode==-1?(*flags&~UINT32_C(7))|3:(*flags&~UINT32_C(15))|((uint32_t)mode|3);
    return 0;
}
int pe_model_project(const PeModel *model,const PeMatrix *view,const PeMatrix *pose,unsigned pose_count,
                     const PeProjection *projection,PeProjectedVertex *output,unsigned capacity,unsigned *written){
    if(capacity<model->vertex_count)return 0;
    for(unsigned i=0;i<model->part_count;++i){PeModelPart part;
        if(!pe_model_part(model,i,&part)||(part.visible==1&&(i>=pose_count||(unsigned)part.first+part.count>model->vertex_count)))return 0;
        if(part.visible==1&&part.count){int16_t points[3][3];
            if(!raw_triplet(model,part.first+(part.count-1)/3*3,points))return 0;}
    }
    PeProjectedVertex *next=model->vertex_count?malloc((size_t)model->vertex_count*sizeof(*next)):NULL;
    if(model->vertex_count&&!next)return 0;
    if(model->vertex_count)memcpy(next,output,(size_t)model->vertex_count*sizeof(*next));unsigned count=0;
    for(unsigned i=0;i<model->part_count;++i){PeModelPart part;if(!pe_model_part(model,i,&part))goto failed;
        if(part.visible!=1)continue;
        PeMatrix matrix;pe_matrix_compose(view,&pose[i],&matrix);
        for(unsigned j=0;j<part.count;j+=3){unsigned index=part.first+j;
            int16_t points[3][3];PeProjectedVertex projected[3];
            if(!raw_triplet(model,index,points))goto failed;
            pe_project_triplet(projection,&matrix,points,projected);
            unsigned available=model->vertex_count-index;if(available>3)available=3;
            memcpy(next+index,projected,(size_t)available*sizeof(*projected));
            count+=part.count-j<3?part.count-j:3;
        }
    }
    if(model->vertex_count)memcpy(output,next,(size_t)model->vertex_count*sizeof(*next));*written=count;free(next);return 1;
failed:free(next);return 0;
}
