#include "pe1/asset_tim.h"
#include "pe1/gte_types.h"

#include "fx_common.h"
#include "fx_common_setup.h"
#include "fx_common_vectors.h"

void FxCommon_StepCameraPath(void)
{
    FxCommonVec3 point;
    GteShortVector scratch;
    void *lookup;

    if (D_8019C040 > 1) {
        D_8019C040--;
        D_8019C041++;

        lookup = Draw_GetTimTableEntry(&g_FxCommonTimTable, 2);
        FxCommon_SamplePath(D_8019C041 << 8, D_8019C042 + 1,
                       lookup, &point, &scratch);
        D_8019C810 = point.x;
        D_8019C814 = point.y;
        D_8019C818 = point.z;

        lookup = Draw_GetTimTableEntry(&g_FxCommonTimTable, 2);
        FxCommon_SamplePath(D_8019C041 << 8, D_8019C042,
                       lookup, &point, &scratch);
        D_8019C330 = point.x;
        D_8019C334 = point.y;
        D_8019C338 = point.z;
    }
}

void FxCommon_InitCameraPath(int index, int unused1, int unused2, int source)
{
    FxCommonVec3 point;
    GteShortVector scratch;
    short savedIndex = index;
    void *lookup;
    int result;
    int shortIndex;

    lookup = Draw_GetTimTableEntry(&g_FxCommonTimTable, 2);
    shortIndex = savedIndex;
    FxCommon_SamplePath(source, shortIndex + 1, lookup, &point, &scratch);
    D_8019C054 = 0;
    D_8019C810 = point.x;
    D_8019C814 = point.y;
    D_8019C818 = point.z;

    lookup = Draw_GetTimTableEntry(&g_FxCommonTimTable, 2);
    result = FxCommon_SamplePath(source, shortIndex, lookup, &point, &scratch);
    D_8019C040 = result - 2;
    D_8019C050 = 0;
    D_8019C041 = 0;
    D_8019C042 = savedIndex;
    g_FxCommonMarkerFade = 0;
    D_8019C330 = point.x;
    D_8019C334 = point.y;
    D_8019C338 = point.z;
}
