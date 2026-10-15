#include "fx_common_motion.h"
#include "fx_common_setup.h"

#define SET_RGB(color, r, g, b) \
    (color)[0] = (r);           \
    (color)[1] = (g);           \
    (color)[2] = (b)

/* Build both frame buffers' full-screen gradient quads (sky, shade, the
 * semi-transparent bottom and top fades) and their three draw modes. */
void FxCommon_InitFadeQuads(void)
{
    u32 i;

    for (i = 0; i < 2; i++) {
        SetPolyG4(&g_FxCommonSkyQuads[i]);
        SET_RGB(g_FxCommonSkyQuads[i].color0, 20, 68, 100);
        SET_RGB(g_FxCommonSkyQuads[i].color1, 20, 68, 100);
        SET_RGB(g_FxCommonSkyQuads[i].color2, 5, 17, 25);
        SET_RGB(g_FxCommonSkyQuads[i].color3, 5, 17, 25);
        g_FxCommonSkyQuads[i].x0 = 0;
        g_FxCommonSkyQuads[i].y0 = 0;
        g_FxCommonSkyQuads[i].x1 = 320;
        g_FxCommonSkyQuads[i].y1 = 0;
        g_FxCommonSkyQuads[i].x2 = 0;
        g_FxCommonSkyQuads[i].y2 = 240;
        g_FxCommonSkyQuads[i].x3 = 320;
        g_FxCommonSkyQuads[i].y3 = 240;

        SetPolyG4(&g_FxCommonShadeQuads[i]);
        SET_RGB(g_FxCommonShadeQuads[i].color0, 10, 24, 40);
        SET_RGB(g_FxCommonShadeQuads[i].color1, 10, 24, 40);
        SET_RGB(g_FxCommonShadeQuads[i].color2, 5, 7, 15);
        SET_RGB(g_FxCommonShadeQuads[i].color3, 5, 7, 15);
        g_FxCommonShadeQuads[i].x0 = 0;
        g_FxCommonShadeQuads[i].y0 = 0;
        g_FxCommonShadeQuads[i].x1 = 320;
        g_FxCommonShadeQuads[i].y1 = 0;
        g_FxCommonShadeQuads[i].x2 = 0;
        g_FxCommonShadeQuads[i].y2 = 240;
        g_FxCommonShadeQuads[i].x3 = 320;
        g_FxCommonShadeQuads[i].y3 = 240;

        SetPolyG4(&g_FxCommonFloorFadeQuads[i]);
        SetSemiTrans(&g_FxCommonFloorFadeQuads[i], 1);
        SET_RGB(g_FxCommonFloorFadeQuads[i].color0, 0, 0, 0);
        SET_RGB(g_FxCommonFloorFadeQuads[i].color1, 0, 0, 0);
        SET_RGB(g_FxCommonFloorFadeQuads[i].color2, 255, 255, 255);
        SET_RGB(g_FxCommonFloorFadeQuads[i].color3, 255, 255, 255);
        g_FxCommonFloorFadeQuads[i].x0 = 0;
        g_FxCommonFloorFadeQuads[i].y0 = 200;
        g_FxCommonFloorFadeQuads[i].x1 = 320;
        g_FxCommonFloorFadeQuads[i].y1 = 200;
        g_FxCommonFloorFadeQuads[i].x2 = 0;
        g_FxCommonFloorFadeQuads[i].y2 = 240;
        g_FxCommonFloorFadeQuads[i].x3 = 320;
        g_FxCommonFloorFadeQuads[i].y3 = 240;

        SetPolyG4(&g_FxCommonTopFadeQuads[i]);
        SetSemiTrans(&g_FxCommonTopFadeQuads[i], 1);
        SET_RGB(g_FxCommonTopFadeQuads[i].color0, 255, 255, 255);
        SET_RGB(g_FxCommonTopFadeQuads[i].color1, 255, 255, 255);
        SET_RGB(g_FxCommonTopFadeQuads[i].color2, 0, 0, 0);
        SET_RGB(g_FxCommonTopFadeQuads[i].color3, 0, 0, 0);
        g_FxCommonTopFadeQuads[i].x0 = 0;
        g_FxCommonTopFadeQuads[i].y0 = 0;
        g_FxCommonTopFadeQuads[i].x1 = 320;
        g_FxCommonTopFadeQuads[i].y1 = 0;
        g_FxCommonTopFadeQuads[i].x2 = 0;
        g_FxCommonTopFadeQuads[i].y2 = 40;
        g_FxCommonTopFadeQuads[i].x3 = 320;
        g_FxCommonTopFadeQuads[i].y3 = 40;

        SetDrawTPage(&g_FxCommonDrawModes0[i], 0, 0, 0);
        SetDrawTPage(&g_FxCommonDrawModes1[i], 0, 0, 0x20);
        SetDrawTPage(&g_FxCommonDrawModes2[i], 0, 0, 0x40);
    }
}

int FxCommon_InitScene(void)
{
    s32 firstPointOffset;
    s32 pointIndex;
    FxCommonPoint8 *pointList;
    FxCommonMotionRecord motion;
    GteShortVector shortMotion;
    s32 variantBase;
    s32 textureIndexOrHasPoints;
    u8 *resourceTableData;
    s32 typeFixed;
    s32 subtypeFixed;
    s32 groupVariant;
    s32 resourceOffset;
    FxCommonNode **secondInitialSlot;
    FxCommonEffectSetupRecord *setupCursor;
    FxCommonNode **initialSlots;
    FxCommonNode **secondSlot;
    FxCommonNode **firstSlot;
    FxCommonNode *secondNode;
    FxCommonNode *firstInitialNode;
    FxCommonNode *secondInitialNode;
    FxCommonNode *firstNode;
    FxCommonNode *initialNode;
    FxCommonNode *pairedInitialNode;
    FxCommonNode *pairedNode;
    s32 initialNodeSubtype;
    s32 initialNodeVariant;
    s32 initialNodeType;
    s32 variantOrMotionOffset;
    s16 motionBase;
    s32 pointCount;
    FxCommonOffsetTable *resourceTable;
    s32 nodeType;
    s32 nodeSubtype;
    s32 nextPointIndex;
    s32 pointOffset;
    s32 nodeIndex;
    FxCommonNode *yNode;
    s32 groupIndex;
    s32 tableIndex;
    u16 textureY;
    u16 pointZ;
    FxCommonNode *effectRoot;
    u32 motionIndex;
    s16 selectedMotion;
    void *nodeResource;
    void *initialResource;
    void *resourceData;
    FxCommonPoint8 *point;

    if(FxCommon_SelectRoomVariant()<0)return -1;
    /* Entering a statically linked scene must restore the state formerly
     * supplied by loading a fresh overlay image, including nonzero defaults. */
    D_8019C00D=0;D_8019C02E=1;g_FxCommonMarkerFade=1;
    g_FxCommonPageChanged=1;g_FxCommonPageOpen=0;g_FxCommonPageId=0;
    g_FxCommonExitRequested=g_FxCommonExitCountdown=0;
    g_FxCommonScreenFade=-1;g_FxCommonDrawTintedCopies=1;
    g_FxCommonDemoFrame=0;g_FxCommonDemoTicks=g_FxCommonDemoExiting=0;
    g_FxCommonDemoResource=-1;g_FxCommonDemoResourceTicks=0;
    g_FxCommonDemoShade=g_FxCommonDemoFadePhase=0;
    g_FxCommonTailPathPosition=g_FxCommonFlyerPathPosition=g_FxCommonCameraPathPosition=0;
    g_FxCommonInputLock=g_FxCommonTailViewEntered=0;
    g_FxCommonExitVariant=g_FxCommonHsyncCount=0;g_FxCommonResetRequested=0;
    g_FxCommonResourceState=(FxCommonResourceState){.active=-1,.current={.word=-1}};
    D_8019C0D0[0]=D_8019C0D0[1]=0;
    g_FxCommonViewMode=0;
    g_FxCommonPlaying=g_FxCommonPlayAdvance=g_FxCommonReplayCamera=0;
    g_FxCommonPlayTicks=g_FxCommonPlayFrame=0;
    g_FxCommonCameraPosition=(FxCommonVec3){0};
    g_FxCommonCameraTarget=(FxCommonVec3){0};
    g_FxCommonCameraPathRemaining=g_FxCommonCameraPathStep=g_FxCommonCameraPathIndex=0;
    memset(&g_FxCommonRuntime,0,sizeof(g_FxCommonRuntime));
    memset(g_FxCommonEffectSetup,0,sizeof(g_FxCommonEffectSetup));
    memset(g_FxCommonMotionPositions,0,sizeof(g_FxCommonMotionPositions));
    memset(g_FxCommonMotionDeltas,0,sizeof(g_FxCommonMotionDeltas));
    FxCommon_InitNodePools();
    /* Static linking replaces reloading the overlay's zeroed route counter. */
    g_FxCommonRouteNodeCount=0;
    FxCommon_InitFadeQuads();
    SetLightMatrix(&g_FxCommonLightMatrix);
    SetColorMatrix(&g_FxCommonColorMatrix);
    SetBackColor(0x40, 0x40, 0x40);
    SetFogNearFar(0x1000, 0x2000, 0x300);
    SetFarColor(0xFF, 0xFF, 0xFF);
    textureIndexOrHasPoints = 0;
    do {
        textureY = g_FieldBgTextureSlots[textureIndexOrHasPoints].y;
        g_FieldBgTextureSlots[textureIndexOrHasPoints].tpage =
            BG_SLOT_TPAGE(g_FieldBgTextureSlots[textureIndexOrHasPoints].x, textureY);
        g_FieldBgTextureSlots[textureIndexOrHasPoints].clut =
            BG_SLOT_CLUT(g_FieldBgTextureSlots[textureIndexOrHasPoints].clut_x,
                         g_FieldBgTextureSlots[textureIndexOrHasPoints].clut_y);
        textureIndexOrHasPoints++;
    } while (textureIndexOrHasPoints < 2);
    if ((g_FxCommonAltFogLayer != 0) || (Render_SetupFogLayer(Draw_GetTimTableEntry(g_FxCommonTimTable, 0)), (g_FxCommonAltFogLayer != 0))) {
        Render_SetupFogLayer(Draw_GetTimTableEntry(g_FxCommonTimTable, 1));
    }
    resourceTableData = g_FxCommonTimTable;
    resourceTable = Draw_GetTimTableEntry(resourceTableData, 2);
    pointIndex = 0;
    initialSlots = g_FxCommonRuntime.pairedNodes;
    secondInitialSlot = initialSlots + 10;
    setupCursor = g_FxCommonEffectSetup;
    firstPointOffset = resourceTable->offsets[0];
    g_FxCommonEffectSetup[0].subtype = 0x15;
    g_FxCommonEffectSetup[0].variant = 0x16;
    g_FxCommonEffectSetup[0].type = 0x17;
    g_FxCommonEffectSetup[0].motionIndices[0] = 0x1B;
    g_FxCommonEffectSetup[0].motionIndices[1] = 0x1A;
    g_FxCommonEffectSetup[0].motionIndices[2] = 0x2F;
    g_FxCommonEffectSetup[0].motionIndices[3] = 0x2E;
    pointList = (FxCommonPoint8 *)((u8 *)resourceTable + firstPointOffset + 8);

    g_FxCommonEffectSetup[0].value0c = 0x62;
    g_FxCommonEffectSetup[0].parameter3c = 0x7D0;
    g_FxCommonEffectSetup[0].value30 = 0x2E;
    g_FxCommonEffectSetup[0].value10 = 0x161;
    g_FxCommonEffectSetup[0].parameter38 = 0xBB8;
    g_FxCommonEffectSetup[0].value22 = 1;
    g_FxCommonEffectSetup[0].value23 = 4;
    g_FxCommonEffectSetup[1].subtype = 0x18;
    g_FxCommonEffectSetup[0].value28 = 0x20;
    g_FxCommonEffectSetup[0].value2c = 0x10;
    g_FxCommonEffectSetup[0].rotation = 0;
    g_FxCommonEffectSetup[0].value25 = 9;
    g_FxCommonEffectSetup[1].variant = 0x19;
    g_FxCommonEffectSetup[0].enabled = (g_ScriptVariableBank[SCRIPT_VAR_FLAGS] >> 0x16) & 1;
    g_FxCommonEffectSetup[1].type = 0x1A;
    g_FxCommonEffectSetup[1].motionIndices[0] = 0xB;
    g_FxCommonEffectSetup[1].motionIndices[2] = 0x1F;
    g_FxCommonEffectSetup[1].motionIndices[3] = 0x1E;
    g_FxCommonEffectSetup[1].motionIndices[1] = 0xA;
    g_FxCommonEffectSetup[1].value0c = 0x4F;
    g_FxCommonEffectSetup[1].value10 = 0x161;
    g_FxCommonEffectSetup[1].parameter38 = 0x1388;
    g_FxCommonEffectSetup[1].parameter3c = 0x9C4;
    g_FxCommonEffectSetup[1].value22 = 2;
    g_FxCommonEffectSetup[1].value23 = 0;
    g_FxCommonEffectSetup[1].enabled = (g_ScriptVariableBank[SCRIPT_VAR_FLAGS] >> 0x14) & 1;
    g_FxCommonEffectSetup[1].value28 = 0x10;
    g_FxCommonEffectSetup[2].subtype = 0xF;
    g_FxCommonEffectSetup[1].value2c = 0x20;
    g_FxCommonEffectSetup[1].rotation = 0;
    g_FxCommonEffectSetup[1].value25 = 7;
    g_FxCommonEffectSetup[1].value30 = 0x1C;
    g_FxCommonEffectSetup[2].variant = 0x10;
    g_FxCommonEffectSetup[2].type = 0x11;
    g_FxCommonEffectSetup[2].motionIndices[0] = 0x19;
    g_FxCommonEffectSetup[2].motionIndices[1] = 0x18;
    g_FxCommonEffectSetup[2].motionIndices[2] = 0x2D;
    g_FxCommonEffectSetup[2].value0c = 0x56;
    g_FxCommonEffectSetup[2].value10 = 0x161;
    g_FxCommonEffectSetup[2].parameter38 = 0x1388;
    g_FxCommonEffectSetup[2].parameter3c = 0x9C4;
    g_FxCommonEffectSetup[2].value22 = 3;
    g_FxCommonEffectSetup[2].value23 = 1;
    g_FxCommonEffectSetup[2].value28 = -0x30;
    g_FxCommonEffectSetup[2].motionIndices[3] = 0x2C;
    g_FxCommonEffectSetup[2].value2c = 0x20;
    g_FxCommonEffectSetup[2].value30 = 0x27;
    g_FxCommonEffectSetup[2].enabled = (g_ScriptVariableBank[SCRIPT_VAR_FLAGS] >> 0x12) & 1;
    g_FxCommonEffectSetup[2].rotation = 0;
    g_FxCommonEffectSetup[2].value25 = 5;
    g_FxCommonEffectSetup[3].type = 0x1D;
    g_FxCommonEffectSetup[3].variant = 0x1C;
    g_FxCommonEffectSetup[3].motionIndices[0] = 0x1D;
    g_FxCommonEffectSetup[3].motionIndices[1] = 0x1C;
    g_FxCommonEffectSetup[3].motionIndices[2] = 0x31;
    g_FxCommonEffectSetup[3].subtype = 0x1B;
    g_FxCommonEffectSetup[3].value0c = 0x56;
    g_FxCommonEffectSetup[3].motionIndices[3] = 0x30;
    g_FxCommonEffectSetup[3].value10 = 0x161;
    g_FxCommonEffectSetup[3].parameter3c = 0xDAC;
    g_FxCommonEffectSetup[3].parameter38 = 0x1388;
    g_FxCommonEffectSetup[3].value22 = 6;
    g_FxCommonEffectSetup[3].value23 = 2;
    g_FxCommonEffectSetup[3].value28 = -0x40;
    g_FxCommonEffectSetup[3].value30 = 0x1E;
    g_FxCommonEffectSetup[3].enabled = (g_ScriptVariableBank[SCRIPT_VAR_FLAGS] >> 0x15) & 1;
    g_FxCommonEffectSetup[3].value2c = 0x10;
    g_FxCommonEffectSetup[4].subtype = 9;
    g_FxCommonEffectSetup[3].rotation = 0;
    g_FxCommonEffectSetup[4].variant = 0xA;
    g_FxCommonEffectSetup[3].value25 = 8;
    g_FxCommonEffectSetup[4].motionIndices[0] = 0x13;
    g_FxCommonEffectSetup[4].motionIndices[1] = 0x12;
    g_FxCommonEffectSetup[4].motionIndices[2] = 0x27;
    g_FxCommonEffectSetup[4].motionIndices[3] = 0x26;
    g_FxCommonEffectSetup[4].type = 0xB;
    g_FxCommonEffectSetup[4].value0c = 0x54;
    g_FxCommonEffectSetup[4].value10 = 0x161;
    g_FxCommonEffectSetup[4].parameter38 = 0x1388;
    g_FxCommonEffectSetup[4].parameter3c = 0xBB8;
    g_FxCommonEffectSetup[4].value2c = -0x10;
    g_FxCommonEffectSetup[4].rotation = 0xC00;
    g_FxCommonEffectSetup[4].value30 = 0x24;
    g_FxCommonEffectSetup[4].enabled = (g_ScriptVariableBank[SCRIPT_VAR_FLAGS] >> 0x13) & 1;
    g_FxCommonEffectSetup[4].value22 = 0;
    g_FxCommonEffectSetup[4].value23 = 5;
    g_FxCommonEffectSetup[4].value28 = 0x10;
    g_FxCommonEffectSetup[5].variant = 1;
    g_FxCommonEffectSetup[4].value25 = 6;
    g_FxCommonEffectSetup[5].type = 2;
    g_FxCommonEffectSetup[5].motionIndices[0] = 0x11;
    g_FxCommonEffectSetup[5].subtype = 0;
    g_FxCommonEffectSetup[5].motionIndices[1] = 0x10;
    g_FxCommonEffectSetup[5].motionIndices[2] = 0x25;
    g_FxCommonEffectSetup[5].motionIndices[3] = 0x24;
    g_FxCommonEffectSetup[5].parameter38 = 0x1B58;
    g_FxCommonEffectSetup[5].parameter3c = 0xFA0;
    g_FxCommonEffectSetup[5].value28 = 0x30;
    g_FxCommonEffectSetup[5].value2c = -0x25;
    g_FxCommonEffectSetup[5].enabled = (g_ScriptVariableBank[SCRIPT_VAR_FLAGS] >> 0x17) & 1;
    g_FxCommonEffectSetup[5].value0c = 0x54;
    g_FxCommonEffectSetup[5].value10 = 0x161;
    g_FxCommonEffectSetup[5].value22 = 4;
    g_FxCommonEffectSetup[5].value23 = 7;
    g_FxCommonEffectSetup[5].rotation = 0;
    g_FxCommonEffectSetup[5].value25 = 0xA;
    g_FxCommonEffectSetup[6].subtype = 3;
    g_FxCommonEffectSetup[5].value30 = 0x1B;
    g_FxCommonEffectSetup[6].variant = 4;
    g_FxCommonEffectSetup[6].type = 5;
    g_FxCommonEffectSetup[6].motionIndices[0] = 0xD;
    g_FxCommonEffectSetup[6].motionIndices[1] = 0xC;
    g_FxCommonEffectSetup[6].motionIndices[2] = 0x21;
    g_FxCommonEffectSetup[6].motionIndices[3] = 0x20;
    g_FxCommonEffectSetup[6].value0c = 0x66;
    g_FxCommonEffectSetup[6].value28 = -0x60;
    g_FxCommonEffectSetup[6].value2c = 5;
    g_FxCommonEffectSetup[6].rotation = 0x800;
    g_FxCommonEffectSetup[6].value30 = 0x32;
    g_FxCommonEffectSetup[6].enabled = (g_ScriptVariableBank[SCRIPT_VAR_FLAGS] >> 0xE) & 1;
    g_FxCommonEffectSetup[6].value10 = 0x161;
    g_FxCommonEffectSetup[6].parameter38 = 0x1388;
    g_FxCommonEffectSetup[6].parameter3c = 0x9C4;
    g_FxCommonEffectSetup[6].value22 = 9;
    g_FxCommonEffectSetup[6].value23 = 3;
    g_FxCommonEffectSetup[6].value25 = 1;
    g_FxCommonEffectSetup[7].subtype = 0xC;
    g_FxCommonEffectSetup[7].variant = 0xD;
    g_FxCommonEffectSetup[7].type = 0xE;
    g_FxCommonEffectSetup[7].motionIndices[0] = 0x17;
    g_FxCommonEffectSetup[7].motionIndices[1] = 0x16;
    g_FxCommonEffectSetup[7].value10 = 0x161;
    g_FxCommonEffectSetup[7].motionIndices[2] = 0x2B;
    g_FxCommonEffectSetup[7].motionIndices[3] = 0x2A;
    g_FxCommonEffectSetup[7].value0c = 0x53;
    g_FxCommonEffectSetup[7].enabled = (g_ScriptVariableBank[SCRIPT_VAR_FLAGS] >> 0xF) & 1;
    g_FxCommonEffectSetup[7].parameter38 = 0x1388;
    g_FxCommonEffectSetup[7].parameter3c = 0x9C4;
    g_FxCommonEffectSetup[7].value22 = 5;
    g_FxCommonEffectSetup[7].value23 = 8;
    g_FxCommonEffectSetup[7].value28 = 0x20;
    g_FxCommonEffectSetup[7].value2c = -0x40;
    g_FxCommonEffectSetup[7].rotation = 0;
    g_FxCommonEffectSetup[7].value25 = 2;
    g_FxCommonEffectSetup[8].subtype = 0x12;
    g_FxCommonEffectSetup[7].value30 = 0xB;
    g_FxCommonEffectSetup[8].variant = 0x13;
    g_FxCommonEffectSetup[8].type = 0x14;
    g_FxCommonEffectSetup[8].motionIndices[0] = 0xF;
    g_FxCommonEffectSetup[8].motionIndices[1] = 0xE;
    g_FxCommonEffectSetup[8].motionIndices[2] = 0x23;
    g_FxCommonEffectSetup[8].motionIndices[3] = 0x22;
    g_FxCommonEffectSetup[8].value0c = 0x69;
    g_FxCommonEffectSetup[8].parameter38 = 0x4650;
    g_FxCommonEffectSetup[8].parameter3c = 0x2EE0;
    g_FxCommonEffectSetup[8].value10 = 0x161;
    g_FxCommonEffectSetup[8].value22 = 7;
    g_FxCommonEffectSetup[8].value23 = 9;
    g_FxCommonEffectSetup[8].value28 = -0x40;
    g_FxCommonEffectSetup[8].value2c = -0x30;
    g_FxCommonEffectSetup[8].rotation = 0;
    g_FxCommonEffectSetup[8].value30 = 0x35;
    g_FxCommonEffectSetup[8].enabled = (g_ScriptVariableBank[SCRIPT_VAR_FLAGS] >> 0x11) & 1;
    g_FxCommonEffectSetup[8].value25 = 4;
    g_FxCommonEffectSetup[9].subtype = 6;
    g_FxCommonEffectSetup[9].variant = 7;
    g_FxCommonEffectSetup[9].type = 8;
    g_FxCommonEffectSetup[9].value10 = 0x161;
    g_FxCommonEffectSetup[9].motionIndices[0] = 0x15;
    g_FxCommonEffectSetup[9].motionIndices[1] = 0x14;
    g_FxCommonEffectSetup[9].motionIndices[2] = 0x29;
    g_FxCommonEffectSetup[9].motionIndices[3] = 0x28;
    g_FxCommonEffectSetup[9].value0c = 0x59;
    g_FxCommonEffectSetup[9].value28 = -0x55;
    g_FxCommonEffectSetup[9].value2c = -0x15;
    g_FxCommonEffectSetup[9].rotation = 0x400;
    g_FxCommonEffectSetup[9].parameter38 = 0x1388;
    g_FxCommonEffectSetup[9].parameter3c = 0x9C4;
    g_FxCommonEffectSetup[9].value22 = 8;
    g_FxCommonEffectSetup[9].value23 = 6;
    g_FxCommonEffectSetup[9].value25 = 3;
    g_FxCommonEffectSetup[9].value30 = 0x5B;
    g_FxCommonEffectSetup[9].enabled = (g_ScriptVariableBank[SCRIPT_VAR_FLAGS] >> 0x10) & 1;
    for (; pointIndex < 10; ++pointIndex) {
        setupCursor->position[0] = pointList->x;
        setupCursor->position[1] = (s16) (u16)pointList->y;
        pointZ = (u16)pointList->z;
        setupCursor->position[2] = (s16) pointZ;
        firstInitialNode = FxCommon_CreateConfiguredNode(&g_FxCommonDefaultNode, setupCursor->subtype, setupCursor->variant, setupCursor->type, g_FxCommonEffectSetup[pointIndex].parameter38, g_FxCommonEffectSetup[pointIndex].parameter3c, Draw_GetTimTableEntry(g_FxCommonModelTable, 0));
        initialSlots[pointIndex] = firstInitialNode;
        firstInitialNode->lod.matrix.t[0] = setupCursor->position[0];
        (initialSlots[pointIndex])->lod.matrix.t[1] = (s32) setupCursor->position[1];
        (initialSlots[pointIndex])->lod.matrix.t[2] = (s32) setupCursor->position[2];
        initialNode = initialSlots[pointIndex];
        (initialNode)->lod.seed.component[1] = (u16) setupCursor->rotation;
        initialResource = Draw_GetTimTableEntry(g_FxCommonModelTable, 0);
        initialNodeType = setupCursor->type;
        initialNodeVariant = setupCursor->variant;
        initialNodeSubtype = setupCursor->subtype;
        secondInitialNode = FxCommon_CreateConfiguredNode(&g_FxCommonDefaultNode, initialNodeSubtype, initialNodeVariant, initialNodeType, 0x3E8, 0x1F4, initialResource);
        *secondInitialSlot = secondInitialNode;
        secondInitialNode->lod.matrix.t[0] = setupCursor->position[0];
        (*secondInitialSlot)->lod.matrix.t[1] = (s32) setupCursor->position[1];
        (*secondInitialSlot)->lod.matrix.t[2] = (s32) setupCursor->position[2];
        (*secondInitialSlot)->lod.seed.component[1] = (u16) setupCursor->rotation;
        pairedInitialNode = *secondInitialSlot;
        (pairedInitialNode)->lod.seed.component[0] = 0x800;
        secondInitialSlot++;
        pointList++;
        setupCursor++;
    }
    nodeResource = Draw_GetTimTableEntry(g_FxCommonModelTable, 0);
    g_FxCommonRuntime.node0 = FxCommon_CreateBareNode(&g_FxCommonDefaultNode, Draw_GetTimTableEntry(g_FxCommonModelTable, 1));
    g_FxCommonType51Node = FxCommon_CreateTypedNode(&g_FxCommonDefaultNode, 0x51, nodeResource);
    g_FxCommonSceneNodes[0] = FxCommon_CreateTypedNode(&g_FxCommonDefaultNode, 0x28, nodeResource);
    g_FxCommonSceneNodes[1] = FxCommon_CreateTypedNode(&g_FxCommonDefaultNode, 0x29, nodeResource);
    g_FxCommonSceneNodes[2] = FxCommon_CreateTypedNode(&g_FxCommonDefaultNode, 0x2A, nodeResource);
    g_FxCommonSceneNodes[3] = FxCommon_CreateTypedNode(&g_FxCommonDefaultNode, 0x2B, nodeResource);
    g_FxCommonSceneNodes[4] = FxCommon_CreateTypedNode(&g_FxCommonDefaultNode, 0x2C, nodeResource);
    g_FxCommonSceneNodes[5] = FxCommon_CreateTypedNode(&g_FxCommonDefaultNode, 0x2D, nodeResource);
    g_FxCommonSceneNodes[6] = FxCommon_CreateTypedNode(&g_FxCommonDefaultNode, 0x2E, nodeResource);
    g_FxCommonSceneNodes[7] = FxCommon_CreateTypedNode(&g_FxCommonDefaultNode, 0x2F, nodeResource);
    g_FxCommonSceneNodes[8] = FxCommon_CreateTypedNode(&g_FxCommonDefaultNode, 0x30, nodeResource);
    g_FxCommonSceneNodes[9] = FxCommon_CreateTypedNode(&g_FxCommonDefaultNode, 0x31, nodeResource);
    g_FxCommonSceneNodes[10] = FxCommon_CreateTypedNode(&g_FxCommonDefaultNode, 0x32, nodeResource);
    g_FxCommonRuntime.node25 = FxCommon_CreateTypedNode(&g_FxCommonDefaultNode, 0x25, nodeResource);
    g_FxCommonType54Node = FxCommon_CreateTypedNode(&g_FxCommonDefaultNode, 0x54, nodeResource);
    g_FxCommonPathTailNodes[0] = (FxCommonTransformNode *)FxCommon_CreateTypedNode(&g_FxCommonDefaultNode, 0x23, nodeResource);
    g_FxCommonPathTailNodes[1] = (FxCommonTransformNode *)FxCommon_CreateTypedNode(&g_FxCommonDefaultNode, 0x23, nodeResource);
    g_FxCommonPathNodes[0] = (FxCommonTransformNode *)FxCommon_CreateTypedNode(&g_FxCommonDefaultNode, 0x52, nodeResource);
    g_FxCommonPathNodes[2] = (FxCommonTransformNode *)FxCommon_CreateTypedNode(&g_FxCommonDefaultNode, 0x52, nodeResource);
    g_FxCommonPathNodes[1] = (FxCommonTransformNode *)FxCommon_CreateTypedNode(&g_FxCommonDefaultNode, 0x52, nodeResource);
    g_FxCommonPathNodes[3] = (FxCommonTransformNode *)FxCommon_CreateTypedNode(&g_FxCommonDefaultNode, 0x52, nodeResource);
    g_FxCommonEchoNodes[0] = (FxCommonTransformNode *)FxCommon_CreateTypedNode(&g_FxCommonDefaultNode, 0x53, nodeResource);
    g_FxCommonEchoNodes[2] = (FxCommonTransformNode *)FxCommon_CreateTypedNode(&g_FxCommonDefaultNode, 0x53, nodeResource);
    g_FxCommonEchoNodes[1] = (FxCommonTransformNode *)FxCommon_CreateTypedNode(&g_FxCommonDefaultNode, 0x53, nodeResource);
    g_FxCommonEchoNodes[3] = (FxCommonTransformNode *)FxCommon_CreateTypedNode(&g_FxCommonDefaultNode, 0x53, nodeResource);
    g_FxCommonType21Nodes[0] = FxCommon_CreateTypedNode(&g_FxCommonDefaultNode, 0x21, nodeResource);
    g_FxCommonType21Nodes[1] = FxCommon_CreateTypedNode(&g_FxCommonDefaultNode, 0x21, nodeResource);
    g_FxCommonType26Nodes[0] = FxCommon_CreateTypedNode(&g_FxCommonDefaultNode, 0x26, nodeResource);
    g_FxCommonType26Nodes[1] = FxCommon_CreateTypedNode(&g_FxCommonDefaultNode, 0x26, nodeResource);
    g_FxCommonType26Nodes[2] = FxCommon_CreateTypedNode(&g_FxCommonDefaultNode, 0x26, nodeResource);
    g_FxCommonType26Nodes[3] = FxCommon_CreateTypedNode(&g_FxCommonDefaultNode, 0x26, nodeResource);
    g_FxCommonLodNodes[0] = FxCommon_CreateConfiguredNode(&g_FxCommonDefaultNode, 0x22, 0x20, 0x20, 0x2710, 0x1F40, nodeResource);
    g_FxCommonLodNodes[1] = FxCommon_CreateConfiguredNode(&g_FxCommonDefaultNode, 0x22, 0x20, 0x20, 0x14, 0xA, nodeResource);
    g_FxCommonPairedNodes[0] = FxCommon_CreateConfiguredNode(&g_FxCommonDefaultNode, 0x22, 0x20, 0x20, 0x2710, 0x1F40, nodeResource);
    g_FxCommonPairedNodes[1] = FxCommon_CreateConfiguredNode(&g_FxCommonDefaultNode, 0x22, 0x20, 0x20, 0x14, 0xA, nodeResource);
    (g_FxCommonPairedNodes[0])->lod.seed.component[1] = 0xC8;
    (g_FxCommonPairedNodes[1])->lod.seed.component[1] = 0xC8;
    g_FxCommonPairedNodes[0]->lod.matrix.t[0] = 0x334;
    g_FxCommonPairedNodes[0]->lod.matrix.t[1] = 0x14;
    g_FxCommonPairedNodes[0]->lod.matrix.t[2] -= 0xFA0;
    {
        FxCommonNode *node = g_FxCommonPairedNodes[1];
        node->lod.matrix.t[0] = 0x334;
    }
    g_FxCommonPairedNodes[1]->lod.matrix.t[1] = -0x14;
    g_FxCommonPairedNodes[1]->lod.matrix.t[2] -= 0xFA0;
    g_FxCommonType26Nodes[0]->matrix.t[0] = -0x32C8;
    g_FxCommonType26Nodes[0]->matrix.t[1] = 0x1B58;
    g_FxCommonType26Nodes[0]->matrix.t[2] = -0x1770;
    g_FxCommonType26Nodes[1]->matrix.t[0] = -0x7D0;
    g_FxCommonType26Nodes[1]->matrix.t[1] = 0x2328;
    g_FxCommonType26Nodes[1]->matrix.t[2] = 0x7D0;
    g_FxCommonType26Nodes[2]->matrix.t[0] = 0x1388;
    g_FxCommonType26Nodes[2]->matrix.t[1] = 0x1770;
    g_FxCommonType26Nodes[2]->matrix.t[2] = 0xFA0;
    typeFixed = 0x350000;
    subtypeFixed = 0x330000;
    {
        FxCommonNode *node = g_FxCommonType26Nodes[3];
        /* The counter is initialized after this scheduling-only clobber. */
        groupIndex = 0;
        groupVariant = 0;
        node->matrix.t[0] = 0x2710;
    }
    {
        FxCommonNode *node = g_FxCommonType26Nodes[3];
        /* Keep the resource-offset constant after the final node loads. */
        resourceOffset = 4;
        node->matrix.t[1] = 0x1B58;
    }
    g_FxCommonType26Nodes[3]->matrix.t[2] = 0xBB8;
    for (; groupIndex < 9; ++groupIndex) {
        resourceData = Draw_GetTimTableEntry(resourceTableData, 2);
        resourceTable = (FxCommonOffsetTable *)((u8 *)resourceData + resourceOffset);
        pointOffset = resourceTable->offsets[0];
        pointIndex = 0;
        pointList = (FxCommonPoint8 *)((u8 *)resourceData + pointOffset);
        pointCount = ((FxCommonPointList *)pointList)->count;
        if(pointCount<0 || pointCount>(132-g_FxCommonRouteNodeCount)/2)return -3;
        ++pointList;
        textureIndexOrHasPoints = pointIndex < pointCount;
        if (textureIndexOrHasPoints) {
            tableIndex = groupIndex * 2;
            point = pointList;
            variantBase = groupVariant;
            nodeSubtype = subtypeFixed >> 0x10;
            nodeType = typeFixed >> 0x10;
            do {
                effectRoot = &g_FxCommonDefaultNode;
                variantOrMotionOffset = variantBase + 0x34;
                firstNode = FxCommon_CreateConfiguredNode(effectRoot, (s16) nodeSubtype, variantOrMotionOffset, (s16) nodeType, (u16) *(short *)((u8 *)g_FxCommonRouteFarDepths + tableIndex), (u16) *(short *)((u8 *)g_FxCommonRouteNearDepths + tableIndex), nodeResource);
                firstSlot = &g_FxCommonRouteNodes[g_FxCommonRouteNodeCount];
                g_FxCommonRouteNodes[g_FxCommonRouteNodeCount] = firstNode;
                firstNode->lod.matrix.t[0] = point->x;
                yNode = *firstSlot;
                yNode->lod.matrix.t[1] = point->y;
                g_FxCommonRouteNodeCount += 1;
                (*firstSlot)->lod.matrix.t[2] = point->z;
                secondNode = FxCommon_CreateConfiguredNode(effectRoot, (s16) nodeSubtype, variantOrMotionOffset, (s16) nodeType, 0x7D0, 0x1F4, nodeResource);
                nodeIndex = g_FxCommonRouteNodeCount;
                secondSlot = &g_FxCommonRouteNodes[nodeIndex];
                g_FxCommonRouteNodes[nodeIndex] = secondNode;
                secondNode->lod.matrix.t[0] = point->x;
                (*secondSlot)->lod.matrix.t[1] = point->y;
                nextPointIndex = ++pointIndex;
                (*secondSlot)->lod.matrix.t[2] = point->z;
                pairedNode = *secondSlot;
                g_FxCommonRouteNodeCount = nodeIndex + 1;
                /* Byte view retains the retail ordering after the pool-index store. */
                pairedNode->lod.seed.component[0] = (s16)((u16)pairedNode->lod.seed.component[0] + 0x800);
                point++;
            } while (nextPointIndex < pointCount);
        }
        typeFixed += 0x30000;
        subtypeFixed += 0x30000;
        groupVariant += 3;
        resourceOffset += 4;
    }
    g_FxCommonRuntime.node25->matrix.t[0] = 0;
    g_FxCommonRuntime.node25->matrix.t[1] = 0x758;
    g_FxCommonRuntime.node25->matrix.t[2] = 0;
    FxCommon_StartCameraMove(g_FxCommonRoomVariant, 1, 1, 0);
    motionIndex = 0;
    g_FxCommonMotionBlendFrames = 0;
    motionBase = (s16) (u16) g_FxCommonRoomVariant;
    for (; motionIndex < 10; ++motionIndex) {
        FxCommon_SamplePath(motionIndex << 8, motionBase + 0x40, Draw_GetTimTableEntry(resourceTableData, 2), &motion.vector, &shortMotion);
        g_FxCommonMotionPositions[motionIndex].vector.x = (s32)((u32)motion.vector.x << 16);
        g_FxCommonMotionPositions[motionIndex].vector.y = (s32)((u32)motion.vector.y << 16);
        g_FxCommonMotionPositions[motionIndex].vector.z = (s32)((u32)motion.vector.z << 16);
    }
    if (g_FxCommonAutoPlay == 1) {
        g_FxCommonMarkerFade = 0;
        g_FxCommonPlayAdvance = 1;
        g_FxCommonPlaying = 1;
        g_FxCommonPlayTicks = 1;
        g_FxCommonRoomVariant = (s16) g_FxCommonAutoVariant;
    }
    if (g_FxCommonStartPreset == 0) {
        g_FxCommonRoomVariant = 7;
        g_FxCommonPlayFrame = 0;
        g_FxCommonPlayAdvance = 0;
        g_FxCommonReplayCamera = 0;
        g_FxCommonPlayTicks = 0;
        g_FxCommonPlaying = 0;
        g_FxCommonMarkerFade = 0;
        D_8019C02E = 0;
        g_FxCommonViewMode = 0xA;
        FxCommon_StartCameraMove(7, 0, 0, 0);
        D_8019C00D = 1;
    }
    if (g_FxCommonStartPreset == 7) {
        selectedMotion = g_FxCommonStartCameraPath;
        g_FxCommonRoomVariant = 9;
        g_FxCommonPlayFrame = 0;
        g_FxCommonMarkerFade = 0;
        g_FxCommonPlayAdvance = 0;
        g_FxCommonPlaying = 1;
        g_FxCommonPlayTicks = 1;
        g_FxCommonAutoPlay = 1;
        if(FxCommon_InitCameraPath(selectedMotion,0,0,0)<0)return -2;
    }
    return 0;
}

/* Original double-buffered screen overlays and the scene's setup records. */
FxCommonGradientQuad g_FxCommonSkyQuads[2],g_FxCommonShadeQuads[2];
FxCommonGradientQuad g_FxCommonFloorFadeQuads[2],g_FxCommonTopFadeQuads[2];
FxCommonDrawModePacket g_FxCommonDrawModes0[2],g_FxCommonDrawModes1[2],g_FxCommonDrawModes2[2];
FxCommonEffectSetupRecord g_FxCommonEffectSetup[10];

/* Authored scene data at USA 8019C0D4..8019C13B. These are not identity
 * matrices: all three colour channels use the first light. */
s16 g_FxCommonRouteFarDepths[10]={12000,12000,12000,11000,12000,11000,12000,12000,16000,12000};
s16 g_FxCommonRouteNearDepths[10]={4500,5500,5500,4500,5500,4500,5500,5500,7500,5500};
GteMatrix g_FxCommonColorMatrix={.m={{4096,0,0},{4096,0,0},{4096,0,0}}};
GteMatrix g_FxCommonLightMatrix={.m={{4096,4096,4096},{0,0,0},{0,0,0}}};
/* InitScene writes the tail through slots 4/5; StepPathNodes reads the
 * same slots through the full six-node path. */
FxCommonTransformNode *g_FxCommonPathNodes[6];
FxCommonTransformNode *g_FxCommonEchoNodes[4];
FxCommonNode *g_FxCommonType26Nodes[4],*g_FxCommonSceneNodes[11];
FxCommonNode *g_FxCommonType21Nodes[2],*g_FxCommonLodNodes[2],*g_FxCommonPairedNodes[2];
FxCommonNode *g_FxCommonType51Node,*g_FxCommonType54Node;
FxCommonNode *g_FxCommonRouteNodes[132];
s32 g_FxCommonRouteNodeCount;
