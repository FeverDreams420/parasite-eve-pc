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
    }
    puts("Native point transforms match PsyZ RT MAC/IR for 10000 deterministic extreme inputs, headless");
    return 0;
}
