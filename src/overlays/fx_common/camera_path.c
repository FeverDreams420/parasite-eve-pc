#include "pe1/asset_tim.h"
#include "pe1/gte_types.h"

#include "fx_common.h"
#include "fx_common_motion.h"
#include "fx_common_setup.h"
#include "fx_common_vectors.h"

void FxCommon_StepCameraPath(void)
{
    FxCommonVec3 point;
    GteShortVector scratch;
    void *lookup;

    if (g_FxCommonCameraPathRemaining > 1) {
        g_FxCommonCameraPathRemaining--;
        g_FxCommonCameraPathStep++;

        lookup = Draw_GetTimTableEntry(&g_FxCommonTimTable, 2);
        FxCommon_SamplePath(g_FxCommonCameraPathStep << 8, g_FxCommonCameraPathIndex + 1,
                       lookup, &point, &scratch);
        g_FxCommonCameraTarget.x = point.x;
        g_FxCommonCameraTarget.y = point.y;
        g_FxCommonCameraTarget.z = point.z;

        lookup = Draw_GetTimTableEntry(&g_FxCommonTimTable, 2);
        FxCommon_SamplePath(g_FxCommonCameraPathStep << 8, g_FxCommonCameraPathIndex,
                       lookup, &point, &scratch);
        g_FxCommonCameraPosition.x = point.x;
        g_FxCommonCameraPosition.y = point.y;
        g_FxCommonCameraPosition.z = point.z;
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
    g_FxCommonMotionRemaining0 = 0;
    g_FxCommonCameraTarget.x = point.x;
    g_FxCommonCameraTarget.y = point.y;
    g_FxCommonCameraTarget.z = point.z;

    lookup = Draw_GetTimTableEntry(&g_FxCommonTimTable, 2);
    result = FxCommon_SamplePath(source, shortIndex, lookup, &point, &scratch);
    g_FxCommonCameraPathRemaining = result - 2;
    g_FxCommonMotionRemaining1 = 0;
    g_FxCommonCameraPathStep = 0;
    g_FxCommonCameraPathIndex = savedIndex;
    g_FxCommonMarkerFade = 0;
    g_FxCommonCameraPosition.x = point.x;
    g_FxCommonCameraPosition.y = point.y;
    g_FxCommonCameraPosition.z = point.z;
}
