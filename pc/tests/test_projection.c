#include "projection.h"
#include <stdio.h>
#include <stdlib.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);exit(1);}}while(0)
int main(void){
    PeMatrix matrix;pe_matrix_identity(&matrix);PeProjection projection={160u<<16,120u<<16,0,200,0};
    int16_t point[3]={100,50,500};PeProjectedVertex vertex;pe_project_point(&projection,&matrix,point,&vertex);
    CHECK(vertex.screen[0]==199&&vertex.screen[1]==139&&vertex.depth==500);
    point[0]=32767;point[1]=-32768;point[2]=1;pe_project_point(&projection,&matrix,point,&vertex);
    CHECK(vertex.screen[0]==1023&&vertex.screen[1]==-1024&&vertex.depth==1&&(vertex.flags&(1u<<17)));
    point[2]=-1;projection.dqb=0x1000000;pe_project_point(&projection,&matrix,point,&vertex);
    CHECK(!vertex.depth&&vertex.interpolation==4096&&(vertex.flags&(1u<<17)));projection.dqb=0;
    uint8_t bytes[132]={0};bytes[2]=1;bytes[6]=2;bytes[24]=1;bytes[30]=1;bytes[32]=1;
    bytes[40]=100;bytes[42]=50;bytes[44]=244;bytes[45]=1;
    PeModel model;char error[128];CHECK(pe_model_open(bytes,sizeof(bytes),&model,error,sizeof(error)));
    PeProjectedVertex output[2]={0};output[1].screen[0]=77;unsigned written=99;
    CHECK(pe_model_project(&model,&matrix,&matrix,1,&projection,output,2,&written));
    CHECK(written==1&&output[0].screen[0]==199&&output[1].screen[0]==77);
    written=99;CHECK(!pe_model_project(&model,&matrix,&matrix,0,&projection,output,2,&written)&&written==99&&output[0].screen[0]==199);
    puts("Headless projection: reciprocal rounding, screen/depth saturation, flags and transactional model batch passed");return 0;
}
