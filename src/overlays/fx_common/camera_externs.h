#ifndef PE1_EXT_CAMERA
#define PE1_EXT_CAMERA

#include "common.h"

#include "fx_common.h"

extern int D_8019CBB4, D_8019CBB8;
extern int D_8019CBD4, D_8019CBD8;
extern int g_FxCommonFrustumDistance0, g_FxCommonFrustumDistance1;
extern int g_FxCommonFrustumSide0, g_FxCommonFrustumSide1;
extern int g_FxCommonFrustumLength0;
extern int g_FxCommonFrustumLength1;
extern int g_FxCommonFrustumDistance3;
extern int D_8019CB54;
extern int D_8019CB58;
extern int g_FxCommonFrustumDistance2;
extern int g_FxCommonFrustumLength3;
extern int g_FxCommonFrustumSide3;
extern int D_8019CBF4;
extern int D_8019CBF8;
extern int g_FxCommonFrustumLength2;
extern int g_FxCommonFrustumSide2;
extern char D_801E4E6C[];
extern FxCommonNode D_801EA5E8[];
extern FxCommonNode D_8019CC58[];
extern short D_8019C39C;
extern short g_FxCommonIndexFreeNext[];
extern short g_FxCommonIndexLast;
extern short D_801E4DA8;
extern short D_801E4DAA;
extern short D_801E4DAC;
extern short D_801E4DAE;
void FxCommon_ResetTransformNode(FxCommonTransformNode *node);
extern u16 D_80093170[];
extern u16 D_80093174[];
extern u16 D_80093172[];
#endif /* PE1_EXT_CAMERA */
