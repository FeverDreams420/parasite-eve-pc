#include "fx_common.h"
#include "fx_common_motion.h"

void FxCommon_SetMotionEndpoints(FxCommonShortVec3 *a, FxCommonShortVec3 *b, int bi, int ai)
{
    int x, y, z, originComponent, originX, ay, az, u, v, w, one;
    u16 bzCopy, ayCopy, azCopy;
    volatile FxCommonShortVec3 *origin0 = &g_FxCommonMotionOrigin0;
    volatile FxCommonShortVec3 *target0 = &g_FxCommonMotionTarget0;
    volatile FxCommonShortVec3 *origin1 = &g_FxCommonMotionOrigin1;
    volatile FxCommonShortVec3 *target1 = &g_FxCommonMotionTarget1;
    int firstOriginX;
    int firstOriginY;

    x = b->x;
    originX = g_FxCommonCameraTarget.x;
    firstOriginX = originX;
    originComponent = g_FxCommonCameraTarget.y;
    firstOriginY = originComponent;
    g_FxCommonMotionStep0X = x - firstOriginX;
    y = b->y;
    originComponent = g_FxCommonCameraTarget.z;
    g_FxCommonMotionStep0Y = y - firstOriginY;
    z = b->z;
    g_FxCommonMotionOrigin0X = firstOriginX;
    originX = g_FxCommonCameraPosition.x;
    origin0->y = firstOriginY;
    ay = g_FxCommonCameraPosition.y;
    g_FxCommonMotionAccum0X = 0;
    g_FxCommonMotionAccum0Y = 0;
    g_FxCommonMotionAccum0Z = 0;
    origin0->z = originComponent;
    g_FxCommonMotionStep0Z = z - originComponent;
    /* These are separate halfword snapshots, after the signed delta reads. */
    g_FxCommonMotionTarget0X = b->x;
    target0->y = b->y;
    bzCopy = b->z;
    bi &= 255;
    g_FxCommonMotionShift0 = bi;
    target0->z = bzCopy;

    u = a->x;
    g_FxCommonMotionStep1X = u - originX;
    v = a->y;
    az = g_FxCommonCameraPosition.z;
    g_FxCommonMotionStep1Y = v - ay;
    one = 1;
    g_FxCommonMotionRemaining0 = one << bi;
    g_FxCommonMotionAccum1X = 0;
    w = a->z;
    g_FxCommonMotionStep1Z = w - az;
    g_FxCommonMotionAccum1Y = 0;
    g_FxCommonMotionAccum1Z = 0;
    g_FxCommonMotionOrigin1X = originX;
    origin1->y = ay;
    origin1->z = az;
    g_FxCommonMotionTarget1X = (u16)a->x;
    ayCopy = a->y;
    ai &= 255;
    target1->y = ayCopy;
    azCopy = a->z;
    g_FxCommonMotionShift1 = ai;
    g_FxCommonMotionRemaining1 = one << ai;
    target1->z = azCopy;
}
