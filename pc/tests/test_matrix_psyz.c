#include "matrix.h"
#include <libgte.h>
#include <psyz/gte.h>
#include <stdio.h>
#include <stdlib.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);exit(1);}}while(0)
static uint32_t state=0x9a517c23;
static uint32_t random_word(void){state^=state<<13;state^=state>>17;state^=state<<5;return state;}
static int16_t half(uint32_t v){v&=65535;return (int16_t)(v&32768?(int)v-65536:(int)v);}
static int word(uint32_t v){return (int)(v&0x80000000?(int64_t)v-INT64_C(0x100000000):v);}
int main(void){
    InitGeom();
    for(unsigned sample=0;sample<65536;++sample){
        uint16_t angles[3]={(uint16_t)sample,(uint16_t)random_word(),(uint16_t)random_word()};
        SVECTOR vector={half(angles[0]),half(angles[1]),half(angles[2]),0};
        MATRIX oracle={0};PeMatrix matrix={0};matrix.translation[0]=0x12345678;
        RotMatrixYXZ(&vector,&oracle);pe_matrix_rotation_yxz(angles,&matrix);
        for(unsigned r=0;r<3;++r)for(unsigned c=0;c<3;++c)CHECK(matrix.rotation[r][c]==oracle.m[r][c]);
        CHECK(matrix.translation[0]==0x12345678);
    }
    puts("Native YXZ rotations match PsyZ for 65536 signed-angle cases, headless");
    for(unsigned sample=0;sample<10000;++sample){
        PeMatrix matrix={0};MATRIX oracle={0};int16_t point[3];
        for(unsigned r=0;r<3;++r){
            for(unsigned c=0;c<3;++c)oracle.m[r][c]=matrix.rotation[r][c]=half(random_word());
            matrix.translation[r]=random_word();oracle.t[r]=word(matrix.translation[r]);point[r]=half(random_word());
        }
        SetRotMatrix(&oracle);SetTransMatrix(&oracle);
        Psyz_GteDataWrite(0,(uint16_t)point[0]|(uint32_t)(uint16_t)point[1]<<16);
        Psyz_GteDataWrite(1,(uint16_t)point[2]);Psyz_GteRt();
        PeTransformedPoint result;pe_matrix_point(&matrix,point,&result);
        for(unsigned r=0;r<3;++r){
            CHECK(result.word[r]==Psyz_GteDataRead(25+r));
            CHECK((uint16_t)result.saturated[r]==(uint16_t)Psyz_GteDataRead(9+r));
        }
        PeMatrix source={0},composed;
        for(unsigned r=0;r<3;++r){
            source.translation[r]=random_word();
            for(unsigned c=0;c<3;++c)source.rotation[r][c]=half(random_word());
        }
        pe_matrix_compose(&matrix,&source,&composed);
        for(unsigned column=0;column<3;++column){
            for(unsigned row=0;row<3;++row)Psyz_GteDataWrite(9+row,(uint16_t)source.rotation[row][column]);
            Psyz_GteRtir();
            for(unsigned row=0;row<3;++row)
                CHECK((uint16_t)composed.rotation[row][column]==(uint16_t)Psyz_GteDataRead(9+row));
        }
        Psyz_GteDataWrite(0,(uint16_t)source.translation[0]|(uint32_t)(uint16_t)source.translation[1]<<16);
        Psyz_GteDataWrite(1,(uint16_t)source.translation[2]);Psyz_GteRt();
        for(unsigned row=0;row<3;++row)CHECK(composed.translation[row]==Psyz_GteDataRead(25+row));
    }
    puts("Native point and matrix composition match PsyZ RT/RTIR for 10000 deterministic extreme cases, headless");
    return 0;
}
