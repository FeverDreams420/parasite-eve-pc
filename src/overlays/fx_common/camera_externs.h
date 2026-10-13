#ifndef PE1_EXT_CAMERA
#define PE1_EXT_CAMERA

#include "common.h"

#include "fx_common.h"

extern int g_FxCommonFrustumDistance0, g_FxCommonFrustumDistance1;
extern int g_FxCommonFrustumSide0, g_FxCommonFrustumSide1;
extern int g_FxCommonFrustumLength0;
extern int g_FxCommonFrustumLength1;
extern int g_FxCommonFrustumDistance3;
extern int g_FxCommonFrustumDistance2;
extern int g_FxCommonFrustumLength3;
extern int g_FxCommonFrustumSide3;
extern int g_FxCommonFrustumLength2;
extern int g_FxCommonFrustumSide2;
extern FxCommonNode g_FxCommonListHeadNodes[];
extern FxCommonNode g_FxCommonListTailNodes[];
extern short g_FxCommonIndexFreeNext[];
extern short g_FxCommonIndexLast;
void FxCommon_ResetTransformNode(FxCommonTransformNode *node);
extern u16 g_PeImageSectorTable[];
#endif /* PE1_EXT_CAMERA */
