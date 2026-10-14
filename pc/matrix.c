#include "matrix.h"
#include <stdlib.h>
#include <string.h>
static int64_t signed_word(uint32_t v){return v&UINT32_C(0x80000000)?(int64_t)v-INT64_C(0x100000000):v;}
static int signed_half(uint32_t v){v&=65535;return v&32768?(int)v-65536:(int)v;}
static int64_t shift12(int64_t v){return v>=0?v/4096:-((-v+4095)/4096);}
void pe_matrix_identity(PeMatrix *matrix){
    memset(matrix,0,sizeof(*matrix));for(unsigned i=0;i<3;++i)matrix->rotation[i][i]=4096;
}
void pe_matrix_compose(const PeMatrix *parent,const PeMatrix *source,PeMatrix *output){
    PeMatrix next={0};
    for(unsigned row=0;row<3;++row){
        for(unsigned column=0;column<3;++column){
            int64_t sum=0;for(unsigned k=0;k<3;++k)sum+=(int64_t)parent->rotation[row][k]*source->rotation[k][column];
            sum=shift12(sum);if(sum>32767)sum=32767;else if(sum<-32768)sum=-32768;
            next.rotation[row][column]=(int16_t)sum;
        }
        int64_t sum=signed_word(parent->translation[row])*4096;
        for(unsigned k=0;k<3;++k)sum+=(int64_t)parent->rotation[row][k]*signed_half(source->translation[k]);
        next.translation[row]=(uint32_t)shift12(sum);
    }
    *output=next;
}
void pe_matrix_point(const PeMatrix *matrix,const int16_t point[3],PeTransformedPoint *output){
    PeTransformedPoint next={0};
    for(unsigned row=0;row<3;++row){
        int64_t sum=signed_word(matrix->translation[row])*4096;
        for(unsigned k=0;k<3;++k)sum+=(int64_t)matrix->rotation[row][k]*point[k];
        uint32_t word=(uint32_t)shift12(sum);
        int64_t value=signed_word(word);
        next.word[row]=word;
        if(value>32767)value=32767;else if(value<-32768)value=-32768;
        next.saturated[row]=(int16_t)value;
    }
    *output=next;
}
int pe_model_matrices(const PeModel *model,const PeMatrix *base,const PeMatrix *sources,
                       unsigned source_count,PeMatrix *output,unsigned capacity,unsigned *written){
    unsigned depth=0,max_depth=0,count=0;
    for(unsigned i=0;i<model->matrix_bytes;++i){
        PeMatrixCommand command;if(!pe_model_matrix_command(model,i,&command))return 0;
        if(command.kind==PE_MATRIX_PUSH){if(++depth>max_depth)max_depth=depth;}
        else if(command.kind==PE_MATRIX_POP){if(!depth)return 0;--depth;}
        else {PeModelPart part;if(command.part>=source_count || !pe_model_part(model,command.part,&part) || ++count>capacity || count>model->part_count)return 0;}
    }
    PeMatrix *next=count?malloc((size_t)count*sizeof(*next)):NULL;
    PeMatrix *stack=max_depth?malloc((size_t)max_depth*sizeof(*stack)):NULL;
    if((count&&!next)||(max_depth&&!stack)){free(next);free(stack);return 0;}
    PeMatrix current=*base;depth=0;unsigned emitted=0;
    for(unsigned i=0;i<model->matrix_bytes;++i){
        PeMatrixCommand command;if(!pe_model_matrix_command(model,i,&command))goto failed;
        if(command.kind==PE_MATRIX_PUSH){if(depth>=max_depth)goto failed;stack[depth++]=current;}
        else if(command.kind==PE_MATRIX_POP){if(!depth)goto failed;current=stack[--depth];}
        else {
            PeModelPart part;if(emitted>=count || command.part>=source_count || !pe_model_part(model,command.part,&part))goto failed;
            PeMatrix source=sources[command.part];
            if(command.part){source.translation[0]=source.translation[1]=0;source.translation[2]=part.translation_z;}
            pe_matrix_compose(&current,&source,&next[emitted]);current=next[emitted++];
        }
    }
    if(count)memcpy(output,next,(size_t)count*sizeof(*next));
    *written=count;free(next);free(stack);return 1;
failed:
    free(next);free(stack);return 0;
}

static int volume_part(const PeModel *model,unsigned index,PeModelPart *part,PeModelBounds *bounds){
    return pe_model_part(model,index,part)&&pe_model_bounds(model,index,bounds);
}
int pe_model_hitvolumes(const PeModel *model,const PeMatrix *pose,unsigned pose_count,
                         PeHitVolume *output,unsigned capacity,unsigned *written){
    unsigned count=0;
    for(unsigned i=0;i<model->part_count;++i){
        PeModelPart part;PeModelBounds bounds;if(!volume_part(model,i,&part,&bounds))return 0;
        if(part.visible==1&&bounds.attributes>=0)++count;
    }
    if(count>capacity||count>model->animation_entries)return 0;
    unsigned transformed=0,pose_index=0;
    for(unsigned i=0;i<model->matrix_bytes;++i){
        PeMatrixCommand command;if(!pe_model_matrix_command(model,i,&command))return 0;
        if(command.kind!=PE_MATRIX_PART)continue;
        if(pose_index++>=pose_count)return 0;
        PeModelPart part;PeModelBounds bounds;if(!volume_part(model,command.part,&part,&bounds))return 0;
        if(part.visible==1&&bounds.attributes>=0)++transformed;
    }
    if(transformed!=count)return 0;
    PeHitVolume *next=count?calloc(count,sizeof(*next)):NULL;if(count&&!next)return 0;
    unsigned emitted=0;
    for(unsigned i=0;i<model->part_count;++i){
        PeModelPart part;PeModelBounds bounds;if(!volume_part(model,i,&part,&bounds))goto failed_volume;
        if(part.visible==1&&bounds.attributes>=0){
            next[emitted].part=(uint16_t)i;next[emitted].radius=bounds.radius;next[emitted++].attributes=bounds.attributes;
        }
    }
    emitted=0;pose_index=0;
    for(unsigned i=0;i<model->matrix_bytes;++i){
        PeMatrixCommand command;if(!pe_model_matrix_command(model,i,&command))goto failed_volume;
        if(command.kind!=PE_MATRIX_PART)continue;
        if(pose_index>=pose_count)goto failed_volume;
        PeModelPart part;PeModelBounds bounds;if(!volume_part(model,command.part,&part,&bounds))goto failed_volume;
        if(part.visible==1&&bounds.attributes>=0){
            if(emitted>=count)goto failed_volume;
            PeTransformedPoint point;pe_matrix_point(&pose[pose_index],bounds.lower,&point);
            memcpy(next[emitted++].center,point.saturated,sizeof(point.saturated));
        }
        ++pose_index;
    }
    if(count)memcpy(output,next,(size_t)count*sizeof(*next));
    *written=count;free(next);return 1;
failed_volume:
    free(next);return 0;
}
