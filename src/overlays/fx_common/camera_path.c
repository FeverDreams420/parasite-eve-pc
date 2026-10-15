#include "pe1/asset_tim.h"
#include "pe1/gte_types.h"

#include "fx_common.h"
#include "fx_common_motion.h"
#include "fx_common_setup.h"
#include "fx_common_vectors.h"

int FxCommon_StepCameraPath(void)
{
    FxCommonVec3 point;
    GteShortVector scratch;
    void *lookup;

    if (g_FxCommonCameraPathRemaining > 1) {
        if(g_FxCommonCameraPathIndex>=79)return -1;
        u8 step=(u8)(g_FxCommonCameraPathStep+1),wrapped=g_FxCommonPathWrapped;
        FxCommonVec3 target;
        lookup = Draw_GetTimTableEntry(g_FxCommonTimTable, 2);
        if(FxCommon_SamplePath((u32)step<<8,g_FxCommonCameraPathIndex+1,lookup,&target,&scratch)<0 ||
           FxCommon_SamplePath((u32)step<<8,g_FxCommonCameraPathIndex,lookup,&point,&scratch)<0){
            g_FxCommonPathWrapped=wrapped;return -1;
        }
        --g_FxCommonCameraPathRemaining;g_FxCommonCameraPathStep=step;
        g_FxCommonCameraTarget=target;g_FxCommonCameraPosition=point;
        return 1;
    }
    return 0;
}

int FxCommon_InitCameraPath(int index, int unused1, int unused2, int source)
{
    (void)unused1;(void)unused2;
    s16 savedIndex=(s16)index;
    if(savedIndex<0 || savedIndex>=79)return -1;
    FxCommonVec3 point,target;
    GteShortVector scratch;
    void *lookup=Draw_GetTimTableEntry(g_FxCommonTimTable,2);
    u8 wrapped=g_FxCommonPathWrapped;
    if(FxCommon_SamplePath((u32)source,savedIndex+1,lookup,&target,&scratch)<0){g_FxCommonPathWrapped=wrapped;return -1;}
    int count=FxCommon_SamplePath((u32)source,savedIndex,lookup,&point,&scratch);
    if(count<0){g_FxCommonPathWrapped=wrapped;return -1;}
    g_FxCommonMotionRemaining0=0;g_FxCommonMotionRemaining1=0;
    g_FxCommonCameraTarget=target;g_FxCommonCameraPosition=point;
    g_FxCommonCameraPathRemaining=(u8)(count-2);
    g_FxCommonCameraPathStep=0;g_FxCommonCameraPathIndex=(u8)savedIndex;
    g_FxCommonMarkerFade=0;
    return 0;
}

u8 g_FxCommonCameraPathRemaining,g_FxCommonCameraPathStep,g_FxCommonCameraPathIndex;
