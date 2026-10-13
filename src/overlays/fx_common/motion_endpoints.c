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
    D_8019C08C = x - firstOriginX;
    y = b->y;
    originComponent = g_FxCommonCameraTarget.z;
    D_8019C090 = y - firstOriginY;
    z = b->z;
    D_8019C0AC = firstOriginX;
    originX = g_FxCommonCameraPosition.x;
    origin0->y = firstOriginY;
    ay = g_FxCommonCameraPosition.y;
    D_8019C09C = 0;
    D_8019C0A0 = 0;
    D_8019C0A4 = 0;
    origin0->z = originComponent;
    D_8019C094 = z - originComponent;
    /* These are separate halfword snapshots, after the signed delta reads. */
    D_8019C0B4 = b->x;
    target0->y = b->y;
    bzCopy = b->z;
    bi &= 255;
    D_8019C056 = bi;
    target0->z = bzCopy;

    u = a->x;
    D_8019C05C = u - originX;
    v = a->y;
    az = g_FxCommonCameraPosition.z;
    D_8019C060 = v - ay;
    one = 1;
    D_8019C054 = one << bi;
    D_8019C06C = 0;
    w = a->z;
    D_8019C064 = w - az;
    D_8019C070 = 0;
    D_8019C074 = 0;
    D_8019C07C = originX;
    origin1->y = ay;
    origin1->z = az;
    D_8019C084 = (u16)a->x;
    ayCopy = a->y;
    ai &= 255;
    target1->y = ayCopy;
    azCopy = a->z;
    D_8019C052 = ai;
    D_8019C050 = one << ai;
    target1->z = azCopy;
}
