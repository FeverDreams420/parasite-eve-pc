#include "matrix.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);exit(1);}}while(0)
static void half(uint8_t*p,unsigned v){p[0]=(uint8_t)v;p[1]=(uint8_t)(v>>8);}
static void word(uint8_t*p,uint32_t v){half(p,v);half(p+2,v>>16);}
int main(void){
    PeMatrix parent,source,result;pe_matrix_identity(&parent);pe_matrix_identity(&source);
    parent.translation[0]=100;source.translation[0]=0x1ffff;
    pe_matrix_compose(&parent,&source,&result);CHECK(result.translation[0]==99 && result.rotation[0][0]==4096);
    parent.rotation[0][0]=32767;source.rotation[0][0]=32767;
    pe_matrix_compose(&parent,&source,&result);CHECK(result.rotation[0][0]==32767);
    source.rotation[0][0]=-32768;pe_matrix_compose(&parent,&source,&result);CHECK(result.rotation[0][0]==-32768);
    pe_matrix_identity(&parent);pe_matrix_identity(&source);parent.rotation[0][0]=1;source.rotation[0][0]=-1;
    pe_matrix_compose(&parent,&source,&source);CHECK(source.rotation[0][0]==-1);
    pe_matrix_identity(&parent);pe_matrix_identity(&source);parent.translation[0]=0x7fffffff;source.translation[0]=1;
    pe_matrix_compose(&parent,&source,&result);CHECK(result.translation[0]==0x80000000);
    pe_matrix_identity(&parent);parent.translation[0]=40000;parent.translation[1]=(uint32_t)-40000;
    const int16_t point[3]={1,-1,123};PeTransformedPoint transformed;
    pe_matrix_point(&parent,point,&transformed);
    CHECK(transformed.word[0]==40001 && transformed.saturated[0]==32767);
    CHECK(transformed.word[1]==(uint32_t)-40001 && transformed.saturated[1]==-32768);
    CHECK(transformed.word[2]==123 && transformed.saturated[2]==123);
    parent.translation[0]=0x7fffffff;
    pe_matrix_point(&parent,point,&transformed);
    CHECK(transformed.word[0]==0x80000000 && transformed.saturated[0]==-32768);
    uint8_t bytes[180]={0};bytes[2]=3;half(bytes+24,5);
    word(bytes+48,10);word(bytes+60,20); /* translations of parts 1 and 2 */
    bytes[136]=0;bytes[137]=255;bytes[138]=1;bytes[139]=254;bytes[140]=2;
    PeModel model;char error[128];CHECK(pe_model_open(bytes,sizeof(bytes),&model,error,sizeof(error)));
    CHECK(model.commands==136 && model.matrix_outputs==3);
    PeMatrix sources[3],output[3];for(unsigned i=0;i<3;++i)pe_matrix_identity(&sources[i]);
    sources[0].translation[2]=5;sources[1].translation[2]=999;sources[2].translation[2]=999;
    pe_matrix_identity(&parent);parent.translation[2]=100;
    unsigned written=99;CHECK(pe_model_matrices(&model,&parent,sources,3,output,3,&written));
    CHECK(written==3 && output[0].translation[2]==105 && output[1].translation[2]==115 && output[2].translation[2]==125);
    PeMatrix saved=output[0];written=77;
    CHECK(!pe_model_matrices(&model,&parent,sources,3,output,2,&written));
    CHECK(written==77 && !memcmp(&saved,&output[0],sizeof(saved)));
    bytes[136]=254;
    CHECK(!pe_model_matrices(&model,&parent,sources,3,output,3,&written) && written==77);
    bytes[136]=0;bytes[3]=2;
    bytes[32]=bytes[44]=bytes[56]=1;
    half(bytes+80,1);half(bytes+82,2);half(bytes+84,3);half(bytes+86,12);half(bytes+94,4);
    half(bytes+96,65531);half(bytes+98,65530);half(bytes+100,65529);half(bytes+102,13);half(bytes+110,5);
    half(bytes+126,65535);
    CHECK(pe_model_open(bytes,sizeof(bytes),&model,error,sizeof(error)));
    CHECK(pe_model_matrices(&model,&parent,sources,3,output,3,&written));
    PeHitVolume volumes[2];unsigned volume_count=99;
    CHECK(pe_model_hitvolumes(&model,output,3,volumes,2,&volume_count));
    CHECK(volume_count==2 && volumes[0].part==0 && volumes[0].radius==12 && volumes[0].attributes==4);
    CHECK(volumes[0].center[0]==1 && volumes[0].center[1]==2 && volumes[0].center[2]==108);
    CHECK(volumes[1].part==1 && volumes[1].radius==13 && volumes[1].center[0]==-5 && volumes[1].center[2]==108);
    PeHitVolume previous_volume=volumes[0];volume_count=77;
    CHECK(!pe_model_hitvolumes(&model,output,3,volumes,1,&volume_count) && volume_count==77);
    CHECK(!memcmp(&previous_volume,&volumes[0],sizeof(previous_volume)));
    bytes[44]=2;
    CHECK(pe_model_hitvolumes(&model,output,3,volumes,2,&volume_count) && volume_count==1);
    puts("Matrices: fixed rotation saturation, signed input, wrapped translation and transactional command hierarchy passed");
    return 0;
}
