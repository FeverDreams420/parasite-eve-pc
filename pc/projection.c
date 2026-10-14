#include "projection.h"
#include <psyz/gte.h>
#include <stdlib.h>
#include <string.h>
static int16_t signed_half(unsigned value){value&=65535;return (int16_t)(value&32768?(int)value-65536:(int)value);}
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
int pe_model_project(const PeModel *model,const PeMatrix *view,const PeMatrix *pose,unsigned pose_count,
                     const PeProjection *projection,PeProjectedVertex *output,unsigned capacity,unsigned *written){
    if(capacity<model->vertex_count)return 0;
    for(unsigned i=0;i<model->part_count;++i){PeModelPart part;
        if(!pe_model_part(model,i,&part)||(part.visible==1&&(i>=pose_count||(unsigned)part.first+part.count>model->vertex_count)))return 0;
    }
    PeProjectedVertex *next=model->vertex_count?malloc((size_t)model->vertex_count*sizeof(*next)):NULL;
    if(model->vertex_count&&!next)return 0;
    if(model->vertex_count)memcpy(next,output,(size_t)model->vertex_count*sizeof(*next));unsigned count=0;
    for(unsigned i=0;i<model->part_count;++i){PeModelPart part;if(!pe_model_part(model,i,&part))goto failed;
        if(part.visible!=1)continue;
        PeMatrix matrix;pe_matrix_compose(view,&pose[i],&matrix);
        for(unsigned j=0;j<part.count;){unsigned index=part.first+j;
            if(part.count-j>=3){
                int16_t points[3][3];PeProjectedVertex projected[3];
                for(unsigned k=0;k<3;++k){PeModelVertex vertex;if(!pe_model_vertex(model,index+k,&vertex))goto failed;
                    points[k][0]=vertex.x;points[k][1]=vertex.y;points[k][2]=vertex.z;}
                pe_project_triplet(projection,&matrix,points,projected);
                memcpy(next+index,projected,sizeof(projected));j+=3;count+=3;continue;
            }
            PeModelVertex vertex;
            if(!pe_model_vertex(model,index,&vertex))goto failed;
            int16_t point[3]={vertex.x,vertex.y,vertex.z};pe_project_point(projection,&matrix,point,&next[index]);++count;++j;
        }
    }
    if(model->vertex_count)memcpy(output,next,(size_t)model->vertex_count*sizeof(*next));*written=count;free(next);return 1;
failed:free(next);return 0;
}
