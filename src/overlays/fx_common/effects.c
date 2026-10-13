#include "common.h"
#include "pe1/script_queue.h"

#include "fx_common.h"
#include "fx_common_motion.h"
#include "fx_common_render.h"

void func_80191FE8(void)
{
    Akao_Cmd_A9(1, 0);
}

void func_8019200C(void)
{
    Akao_Cmd_A9(30, 0);
}

/* Choose the room to return to once the effect sequence ends, clear the
 * screen, fade the music out and hand control back to the scene loader. */
void FxCommon_EndSequence(void)
{
    FxCommonRect area;
    s32 room;
    u32 night;
    u32 band;
    u8 fade;
    int i;

    fade = 1;
    night = g_ScriptVariableBank[SCRIPT_VAR_FLAGS] & 0x2000;
    g_ScriptVariableBank[SCRIPT_VAR_ROOM] = 999;
    D_8009D280 = 0xA9400048;
    room = g_ScriptVariableBank[SCRIPT_VAR_AREA];
    if (room == 0x80) {
        D_8009D280 = 0xA80650C8;
    } else if (room == 0x78) {
        D_8009D280 = 0xA8003348;
        fade = 0;
    } else if (room == 0x208) {
        D_8009D280 = 0xA80032C8;
        fade = 0;
    } else if (room == 0x210) {
        if (night)
            D_8009D280 = 0xA8029148;
        else
            D_8009D280 = 0xA80290C8;
    } else {
        switch (D_8019CA68) {
        case 0:
            D_8009D280 = 0xA80281C8;
            break;
        case 1:
            D_8009D280 = 0xA80260C8;
            break;
        case 2:
            D_8009D280 = 0xA8009348;
            break;
        case 3:
            D_8009D280 = 0xA8046048;
            break;
        case 4:
            if (room == 0x1C0) {
                D_8009D280 = 0xA80034C8;
                fade = 0;
            } else if (night) {
                D_8009D280 = 0xA80222C8;
            } else {
                D_8009D280 = 0xA80630C8;
            }
            break;
        case 5:
            D_8009D280 = 0xA8049048;
            break;
        case 6:
            D_8009D280 = 0xA8002248;
            break;
        case 7:
            if (room == 0xD0) {
                D_8009D280 = 0xA80033C8;
                fade = 0;
            } else if (room == 0x178) {
                D_8009D280 = 0xA80033C8;
                fade = 0;
            } else {
                band = room - 0x180;
                if (band < 0x88)
                    D_8009D280 = 0xA80201C8;
                else
                    D_8009D280 = 0xA8004348;
            }
            break;
        case 8:
            if (room == 0xE0) {
                D_8009D280 = 0xA80034C8;
                fade = 0;
            } else if (room >= 0x128) {
                D_8009D280 = 0xA8005448;
            } else {
                D_8009D280 = 0xA8067248;
            }
            break;
        case 9:
            if (room == 0xB8 || room == 0x148) {
                D_8009D280 = 0xA8003448;
                fade = 0;
            } else if (night) {
                D_8009D280 = 0xA8029148;
            } else {
                D_8009D280 = 0xA80290C8;
            }
            break;
        }
    }

    SetDispMask(0);
    ResetGraph(1);
    area.x = 0;
    area.y = 0;
    area.w = 0x140;
    area.h = 0x1C0;
    ClearImage(&area, 0, 0, 0);
    DrawSync(0);
    PutDispEnv(g_RenderDispEnvArray);
    if (fade == 1)
        Akao_Cmd_C1_WithSlot(g_SceneAudioSlot0Handle, 0x1E, 0);
    Akao_Cmd_A9(0x1E, 0);
    for (i = 0; i < 0x1E; i++)
        VSync(0);
    if (fade == 1) {
        g_SceneAudioSlot0Key = -1;
        D_800B0DB2 = -1;
        D_800B0CD8 &= ~0x40;
        Akao_Cmd_F0();
    }
    Akao_Cmd_F1();
    Render_ConsumeFontShuffleSeed();
}

/* The effect overlay's own frame loop: run every effect system, compact the
 * ordering table, present the frame and flip the double buffer until the
 * scene asks to leave. */
void func_8019234C(void)
{
    FxCommonBuffer *next;
    int last;
    int handle;
    u8 started;
    int scanning;
    int i;
    FxCommonAddress current;

    g_ActiveDrawSlot = 0;
    D_8019C00E = 0;
    last = 0;
    func_80196498();
    FxCommon_SetAkao3dVolumeRange(D_8019C034 != 10);
    srand(VSync(-1));
    started = 0;
    D_8019C03C = rand() % 3000000;
    handle = 0;
    if (D_8019C034 == 10)
        D_8019C03C = 120000;

    while (D_8019C024 == 0) {
        Field_HandleStateTransition();
        if ((g_FieldPadBits & 0x0F000006) == 0x0F000006) {
            VSync(0);
            SetDispMask(0);
            D_8019C00E = 1;
            D_8019C024 = 1;
            Boot_Shutdown();
        }
        FxCommon_StepEffects();
        if (D_8019C00E != 0)
            continue;

        FxCommon_UpdateCameraMatrices();
        FxCommon_UpdateFrustumPlanes((void *)&g_FxCommonCameraPosition);
        if (D_8019C02A != 0)
            D_8019C02A = FxCommon_StepFadeIntensity(D_8019C02A);
        func_80192740();
        FxCommon_DrawSceneNodes();
        FxCommon_StepPathNodes();
        if (!started) {
            handle = FxCommon_SendTableCommand3d(0xABE, D_801EA578->state);
            started = 1;
        } else if (D_8019C0C0 == 0) {
            FxCommon_Apply3dPan(handle, D_801EA578->state);
        }
        if (D_8019C034 == 10 || (D_8019C044 == 1 && D_8019C045 == 0))
            Menu_DrawTextboxEntries();

        scanning = 1;
        for (i = 0xFFF; i >= 0; i--) {
            if (scanning) {
                if ((FxCommonPacketTag *)(g_FxCommonActiveBuffer->allocation[i].packed |
                                          0x80000000) ==
                    &g_FxCommonActiveBuffer->allocation[i - 1]) {
                    last = i;
                    scanning = 0;
                }
            } else if ((FxCommonPacketTag *)(g_FxCommonActiveBuffer->allocation[i].packed |
                                             0x80000000) !=
                       &g_FxCommonActiveBuffer->allocation[i - 1]) {
                current.pointer = &g_FxCommonActiveBuffer->allocation[i];
                g_FxCommonActiveBuffer->allocation[last].packed = current.word & 0xFFFFFF;
                scanning = 1;
            }
        }

        D_8019CC14 = VSync(1);
        DrawSync(0);
        VSync(2);
        FxCommon_StepMotionBlend();
        ResetGraph(1);
        PutDrawEnv(((FxCommonFrame *)g_FxCommonActiveBuffer)->drawEnv);
        PutDispEnv(((FxCommonFrame *)g_FxCommonActiveBuffer)->dispEnv);
        DrawOTag(&g_FxCommonActiveBuffer->allocation[0xFFF]);
        next = &g_FxCommonFrames[0].buffer;
        if (g_FxCommonActiveBuffer == next)
            next = &g_FxCommonFrames[1].buffer;
        g_FxCommonActiveBuffer = next;
        g_ActiveDrawSlot ^= 1;
        ClearOTagR(next->allocation, 0x1000);
        FxCommon_SelectBufferAllocation((void **)g_FxCommonActiveBuffer);
    }
    if (D_8019C00E == 0)
        FxCommon_EndSequence();
}

void func_80192740(void)
{
    if (D_8019C1F0 == 1) {
        u32 *dst = D_801EA598;
        FxCommonBuffer *owner = g_FxCommonActiveBuffer;
        FxCommonBuffer *sentinel = (FxCommonBuffer *)&D_8019C1F8;
        u32 lowMask;
        u32 *base;
        u32 highMask;
        u32 *src1;
        u32 *src2;
        u32 destWord;
        u32 lowWord;
        FxCommonAddress address;

        if (owner != sentinel)
            dst += 9;
        lowMask = 0xFFFFFF;
        base = D_801EA598;
        highMask = 0xFF000000;
        src1 = (u32 *)(owner)->allocation;
        destWord = *dst;
        lowWord = src1[0xFFF];
        destWord &= highMask;
        lowWord &= lowMask;
        destWord |= lowWord;
        *dst = destWord;
        src2 = (u32 *)(owner)->allocation;
        if (owner != sentinel)
            base += 9;
        address.pointer = base;
        lowWord = address.word & lowMask;
        src2[0xFFF] = (src2[0xFFF] & highMask) | lowWord;
    }
    if (g_FxCommonMarkerFade != 0)
        g_FxCommonMarkerFade = FxCommon_DrawEffectMarkers(g_FxCommonMarkerFade);
}

#define DRAW(node, pass, force, mirrored) \
    FxCommon_DrawTransformNodeCulled((node), g_FxCommonActiveBuffer, (pass), (force), (mirrored))
#define DRAW_PAIR(node, pass, mode) \
    FxCommon_DrawLodNode((node), g_FxCommonActiveBuffer, (pass), (mode))

/* Draw every scene node for this frame, each with its depth bias: the
 * backdrop, the turning fans, the static props, the camera path nodes and
 * their echoes, the effect node pairs and the overlay nodes. */
void FxCommon_DrawSceneNodes(void)
{
    int i;

    if (D_8019C034 != 0) {
        g_FxCommonDepthBias = 0xFA1;
        DRAW(D_8019CC28, 1, 1, 0);
    }
    g_FxCommonDepthBias = 0xFA0;
    DRAW(g_FxCommonRuntime.node25, 1, 1, 0);
    g_FxCommonDepthBias = 0xF9F;
    DRAW(g_FxCommonType26Nodes[0], 1, 1, 0);
    DRAW(g_FxCommonType26Nodes[1], 1, 1, 0);
    DRAW(g_FxCommonType26Nodes[2], 1, 1, 0);
    DRAW(g_FxCommonType26Nodes[3], 1, 1, 0);
    g_FxCommonType26Nodes[0]->state[0] += 10;
    g_FxCommonType26Nodes[1]->state[0] += 2;
    g_FxCommonType26Nodes[2]->state[0] += 40;
    g_FxCommonType26Nodes[3]->state[0] += 30;
    if (g_FxCommonType26Nodes[0]->state[0] > 0x6EE0)
        g_FxCommonType26Nodes[0]->state[0] = -0x6328;
    if (g_FxCommonType26Nodes[1]->state[0] > 0x6EE0)
        g_FxCommonType26Nodes[1]->state[0] = -0x6328;
    if (g_FxCommonType26Nodes[2]->state[0] > 0x6EE0)
        g_FxCommonType26Nodes[2]->state[0] = -0x6328;
    if (g_FxCommonType26Nodes[3]->state[0] > 0x6EE0)
        g_FxCommonType26Nodes[3]->state[0] = -0x6328;

    g_FxCommonDepthBias = 1000;
    DRAW(D_8019CDA4, 0, 1, 0);
    DRAW(D_8019CDAC, 0, 1, 0);
    g_FxCommonDepthBias = 0xF3B;
    DRAW(D_8019CDA0, 1, 1, 0);
    DRAW(D_8019CDA8, 1, 1, 0);
    g_FxCommonDepthBias = 0xF3A;
    DRAW(D_8019CDB0, 1, 1, 0);
    g_FxCommonDepthBias = 0xF39;
    DRAW(D_8019CDB4, 1, 1, 0);
    g_FxCommonDepthBias = 0;
    DRAW(D_8019CDB8, 0, 1, 0);
    DRAW(D_8019CDBC, 0, 1, 0);
    DRAW(D_8019CDC0, 0, 1, 0);
    DRAW(D_8019CDC4, 0, 1, 0);
    DRAW(D_8019CDC8, 0, 1, 0);
    DRAW(D_8019C180, 0, 1, 0);

    if (g_FxCommonPathNodes[0]->kind == 0x24) {
        g_FxCommonPathNodes[0]->kind = 0x52;
        g_FxCommonPathNodes[1]->kind = 0x52;
        g_FxCommonPathNodes[2]->kind = 0x52;
        g_FxCommonPathNodes[3]->kind = 0x52;
    } else {
        g_FxCommonPathNodes[0]->kind = 0x24;
        g_FxCommonPathNodes[1]->kind = 0x24;
        g_FxCommonPathNodes[2]->kind = 0x24;
        g_FxCommonPathNodes[3]->kind = 0x24;
    }
    g_FxCommonDepthBias = 0;
    DRAW(g_FxCommonPathNodes[0], 0, 0, 0);
    if (D_8019C034 != 2)
        DRAW(g_FxCommonPathNodes[2], 0, 0, 0);
    g_FxCommonDepthBias = 1000;
    DRAW(g_FxCommonPathNodes[1], 0, 0, 1);
    if (D_8019C034 != 2)
        DRAW(g_FxCommonPathNodes[3], 0, 0, 1);

    g_FxCommonDepthBias = 0;
    DRAW_PAIR(D_8019C820, 1, 0);
    DRAW_PAIR(g_FxCommonPairedNodes[0], 1, 0);
    g_FxCommonDepthBias = 1000;
    DRAW_PAIR(D_8019C824, 1, 2);
    DRAW_PAIR(g_FxCommonPairedNodes[1], 1, 2);

    g_FxCommonDepthBias = 0;
    DRAW(g_FxCommonPathTailNodes[0], 0, 0, 0);
    DRAW(g_FxCommonPathTailNodes[1], 0, 0, 0);
    g_FxCommonDepthBias = 10;
    DRAW(g_FxCommonPathTailNodes[0], 0, 0, 1);
    DRAW(g_FxCommonPathTailNodes[1], 0, 0, 1);
    g_FxCommonDepthBias = 0;
    DRAW(D_8019CC20, 0, 1, 0);
    g_FxCommonDepthBias = 800;
    DRAW(D_8019CC24, 0, 1, 1);

    if (D_8019C1F0 == 1) {
        D_8019C0D0[0] += (s16)(rand() % 40 + 20);
        D_8019C0D0[1] += (s16)(rand() % 52 + 26);
        g_FxCommonEchoNodes[0]->matrix = g_FxCommonPathNodes[0]->matrix;
        g_FxCommonEchoNodes[0]->seed = g_FxCommonPathNodes[0]->seed;
        g_FxCommonEchoNodes[0]->kind = 0x53;
        g_FxCommonEchoNodes[2]->matrix = g_FxCommonPathNodes[2]->matrix;
        g_FxCommonEchoNodes[2]->seed = g_FxCommonPathNodes[2]->seed;
        g_FxCommonEchoNodes[2]->kind = 0x53;
        g_FxCommonEchoNodes[1]->matrix = g_FxCommonPathNodes[0]->matrix;
        g_FxCommonEchoNodes[1]->seed = g_FxCommonPathNodes[0]->seed;
        g_FxCommonEchoNodes[1]->kind = 0x53;
        g_FxCommonEchoNodes[3]->matrix = g_FxCommonPathNodes[2]->matrix;
        g_FxCommonEchoNodes[3]->seed = g_FxCommonPathNodes[2]->seed;
        g_FxCommonEchoNodes[3]->kind = 0x53;
        g_FxCommonDepthBias = 0;
        DRAW(g_FxCommonEchoNodes[0], 0, 0, 0);
        g_FxCommonDepthBias = 800;
        DRAW(g_FxCommonEchoNodes[1], 0, 0, 1);
    }

    for (i = 0; i < D_8019C020 >> 1; i++) {
        g_FxCommonDepthBias = 0;
        if (D_800A77FC & 0x2000) {
            D_801EA264[0] = 60;
            D_801EA264[1] = 60;
            D_801EA264[2] = 40;
        } else {
            D_801EA264[0] = 20;
            D_801EA264[1] = 40;
            D_801EA264[2] = 40;
        }
        DRAW_PAIR(D_8019C3B0[i * 2], 0, 0);
        if (D_8019C032 == 1 && (i & 3)) {
            g_FxCommonDepthBias = 800;
            DRAW_PAIR(D_8019C3B0[i * 2 + 1], 0, 1);
        }
    }

    g_FxCommonDepthBias = 0;
    for (i = 0; i < 8; i++)
        DRAW_PAIR(g_FxCommonRuntime.pairedNodes[i], 0, 0);
    g_FxCommonDepthBias = 0x23A;
    DRAW_PAIR(g_FxCommonRuntime.pairedNodes[8], 0, 0);
    g_FxCommonDepthBias = 0;
    DRAW_PAIR(g_FxCommonRuntime.pairedNodes[9], 0, 0);
    if (D_8019C032 == 1) {
        g_FxCommonDepthBias = 800;
        if (D_8019C044) {
            DRAW_PAIR(g_FxCommonRuntime.pairedNodes[11], 0, 1);
            DRAW_PAIR(g_FxCommonRuntime.pairedNodes[12], 0, 1);
            DRAW_PAIR(g_FxCommonRuntime.pairedNodes[13], 0, 1);
            DRAW_PAIR(g_FxCommonRuntime.pairedNodes[14], 0, 1);
            DRAW_PAIR(g_FxCommonRuntime.pairedNodes[16], 0, 1);
            DRAW_PAIR(g_FxCommonRuntime.pairedNodes[17], 0, 1);
            DRAW_PAIR(g_FxCommonRuntime.pairedNodes[18], 0, 1);
            DRAW_PAIR(g_FxCommonRuntime.pairedNodes[19], 0, 1);
        }
        DRAW_PAIR(g_FxCommonRuntime.pairedNodes[15], 0, 1);
    }
    D_801EA5E0 = 500;
    FxCommon_DrawTransformNode(D_801EA260, g_FxCommonActiveBuffer);
}

#undef DRAW
#undef DRAW_PAIR

/* Move the path-following nodes one step along their recorded paths, bank
 * the two flyers into their turns, and copy each flyer onto its shadow. */
void FxCommon_StepPathNodes(void)
{
    GteShortVector bank0;
    GteShortVector bank1;

    FxCommon_SamplePath(D_8019C038, 0x4A, Draw_GetTimTableEntry(&g_FxCommonTimTable, 2),
                  (FxCommonVec3 *)g_FxCommonPathNodes[4]->matrix.t,
                  &g_FxCommonPathNodes[4]->seed.angles);
    FxCommon_SamplePath(D_8019C038, 0x4B, Draw_GetTimTableEntry(&g_FxCommonTimTable, 2),
                  (FxCommonVec3 *)g_FxCommonPathNodes[5]->matrix.t,
                  &g_FxCommonPathNodes[5]->seed.angles);
    FxCommon_SamplePath(D_8019C03C, 0x4C, Draw_GetTimTableEntry(&g_FxCommonTimTable, 2),
                  (FxCommonVec3 *)g_FxCommonPathNodes[0]->matrix.t, &bank0);
    FxCommon_SamplePath(D_8019C03C, 0x4D, Draw_GetTimTableEntry(&g_FxCommonTimTable, 2),
                  (FxCommonVec3 *)g_FxCommonPathNodes[2]->matrix.t, &bank1);

    g_FxCommonPathNodes[0]->seed.component[0] = bank0.x;
    g_FxCommonPathNodes[0]->seed.component[1] = bank0.y;
    g_FxCommonPathNodes[0]->seed.component[2] += bank0.z;
    if (g_FxCommonPathNodes[0]->seed.component[2] > 0x200)
        g_FxCommonPathNodes[0]->seed.component[2] = 0x200;
    if (g_FxCommonPathNodes[0]->seed.component[2] < -0x200)
        g_FxCommonPathNodes[0]->seed.component[2] = -0x200;
    if (g_FxCommonPathNodes[0]->seed.component[2] > 0)
        g_FxCommonPathNodes[0]->seed.component[2] -= 8;
    if (g_FxCommonPathNodes[0]->seed.component[2] < 0)
        g_FxCommonPathNodes[0]->seed.component[2] += 8;

    g_FxCommonPathNodes[2]->seed.component[0] = bank1.x;
    g_FxCommonPathNodes[2]->seed.component[1] = bank1.y;
    g_FxCommonPathNodes[2]->seed.component[2] += bank1.z;
    if (g_FxCommonPathNodes[2]->seed.component[2] > 0x180)
        g_FxCommonPathNodes[2]->seed.component[2] = 0x180;
    if (g_FxCommonPathNodes[2]->seed.component[2] < -0x180)
        g_FxCommonPathNodes[2]->seed.component[2] = -0x180;
    if (g_FxCommonPathNodes[2]->seed.component[2] > 0)
        g_FxCommonPathNodes[0]->seed.component[2] -= 8;
    if (g_FxCommonPathNodes[2]->seed.component[2] < 0)
        g_FxCommonPathNodes[0]->seed.component[2] += 8;

    g_FxCommonPathNodes[4]->seed.component[1] -= 0x400;
    g_FxCommonPathNodes[5]->seed.component[1] -= 0x400;
    g_FxCommonPathNodes[0]->seed.component[1] -= 0x400;
    g_FxCommonPathNodes[2]->seed.component[1] -= 0x400;

    g_FxCommonPathNodes[1]->matrix = g_FxCommonPathNodes[0]->matrix;
    g_FxCommonPathNodes[1]->seed = g_FxCommonPathNodes[0]->seed;
    g_FxCommonPathNodes[3]->matrix = g_FxCommonPathNodes[2]->matrix;
    g_FxCommonPathNodes[3]->seed = g_FxCommonPathNodes[2]->seed;
    D_8019C038 += 4;
    D_8019C03C += 0x10;
}

void FxCommon_SnapMotionWhole(s16 base)
{
    FxCommonMotionRecord sample;
    GteShortVector extra;
    unsigned int i;

    g_FxCommonMotionBlendFrames = 0;
    for (i = 0; i < 10; i++) {
        FxCommon_SamplePath(i << 8, base + 0x40,
                      Draw_GetTimTableEntry(&g_FxCommonTimTable, 2), &sample.vector, &extra);
        D_8019CAA8[i].vector.x = sample.vector.x << 16;
        D_8019CAA8[i].vector.y = sample.vector.y << 16;
        D_8019CAA8[i].vector.z = sample.vector.z << 16;
    }
}

void FxCommon_BeginMotionBlend(s16 base)
{
    FxCommonMotionRecord sample;
    GteShortVector extra;
    unsigned int i;

    g_FxCommonMotionBlendFrames = 0x20;
    for (i = 0; i < 10; i++) {
        FxCommon_SamplePath(i << 8, base + 0x40,
                      Draw_GetTimTableEntry(&g_FxCommonTimTable, 2), &sample.vector, &extra);
        g_FxCommonMotionDeltas[i].vector.x =
            ((sample.vector.x << 16) - D_8019CAA8[i].vector.x) >> 5;
        g_FxCommonMotionDeltas[i].vector.y =
            ((sample.vector.y << 16) - D_8019CAA8[i].vector.y) >> 5;
        g_FxCommonMotionDeltas[i].vector.z =
            ((sample.vector.z << 16) - D_8019CAA8[i].vector.z) >> 5;
    }
}

void FxCommon_StepMotionBlend(void)
{
    FxCommonMotionLane *x;
    FxCommonMotionLane *y;
    FxCommonMotionLane *z;
    FxCommonMotionLane *end;
    FxCommonMotionByte *base;
    int sourceOffset;

    if (g_FxCommonMotionBlendFrames <= 0) {
        return;
    }

    g_FxCommonMotionBlendFrames--;
    base = (FxCommonMotionByte *)D_8019CAB0;
    z = D_8019CAB0;
    y = (FxCommonMotionLane *)(base - 4);
    x = (FxCommonMotionLane *)(base - 8);
    sourceOffset = 0;
    end = (FxCommonMotionLane *)(base + 0xA0);
    do {
        x->value += *(s32 *)(D_801EA268 + sourceOffset);
        y->value += *(s32 *)(D_801EA26C + sourceOffset);
        z->value += *(s32 *)(D_801EA270 + sourceOffset);
        x++;
        y++;
        z++;
        sourceOffset += 0x10;
    } while (z < end);
}

#define FX_MODE_PACKET                                          \
    (g_FxCommonActiveBuffer == &g_FxCommonFrames[0].buffer ? &g_FxCommonDrawModes1[1] \
                                               : &g_FxCommonDrawModes1[0])

/* Fade the effect markers in or out by eight steps and draw, for every
 * enabled effect, a line from its projected position to a two-part label
 * (the selected effect uses the brighter palette). */
s16 FxCommon_DrawEffectMarkers(s16 value)
{
    s16 level;
    FxCommonDrawModePacket *mode;
    FxCommonBuffer *buffer;
    FxCommonBuffer *leftBuffer;
    FxCommonBuffer *rightBuffer;
    FxCommonPacketTag *ot;
    FxCommonAddress address;
    FxCommonAddress lineLink;
    FxCommonAddress leftLink;
    FxCommonAddress rightLink;
    FxCommonLinePacket *line;
    FxCommonMarkerQuad *quad;
    GteShortVector rotation;
    GteShortVector position;
    GteShortVector unused;
    s32 screen0;
    s32 screen1;
    s32 screen2;
    s32 depth;
    s32 flag;
    u32 xy;
    u32 *vertex;
    u8 u;
    s16 x;
    s16 y;
    int i;

    level = 0;
    if (value > 0) {
        if (value < 0xFF)
            value += 8;
        if (value > 0xFF)
            value = 0xFF;
        level = value;
    }
    if (value < 0) {
        if (value >= -0xFE)
            value -= 8;
        if (value < -0xFF)
            value = -0xFF;
        level = value + 0xFF;
    }

    mode = FX_MODE_PACKET;
    mode->tag.bits.address = g_FxCommonActiveBuffer->allocation[10].bits.address;
    ot = &g_FxCommonActiveBuffer->allocation[10];
    address.pointer = FX_MODE_PACKET;
    ot->bits.address = address.word;

    for (i = 0; i < 10; i++) {
        position.x = g_FxCommonMotionWhole[i].x.whole;
        position.y = g_FxCommonMotionWhole[i].y.whole;
        position.z = g_FxCommonMotionWhole[i].z.whole;
        if (g_FxCommonEffectSetup[i].enabled != 1 || level == 0)
            continue;

        rotation.x = g_FxCommonEffectSetup[i].position[0];
        rotation.y = g_FxCommonEffectSetup[i].position[1];
        rotation.z = g_FxCommonEffectSetup[i].position[2];
        RotTransPers3(&rotation, &position, &unused, &screen0, &screen1,
                      &screen2, &depth, &flag);

        line = (FxCommonLinePacket *)g_FxCommonActiveBuffer->data;
        line->tag.bytes.length = 3;
        line->code = 0x40;
        SetSemiTrans(line, 1);
        vertex = &line->xy0;
        *vertex++ = screen0;
        *vertex = screen1;
        if (g_FxCommonRoomVariant == i) {
            line->r = level >> 1;
            line->g = level >> 1;
            line->b = level >> 1;
        } else {
            line->r = level >> 3;
            line->g = level >> 3;
            line->b = level >> 3;
        }
        buffer = g_FxCommonActiveBuffer;
        lineLink.pointer = line;
        line->tag.bits.address = buffer->allocation[10].bits.address;
        buffer->allocation[10].bits.address = lineLink.word;
        line++;
        buffer->data = (u8 *)line;
        quad = (FxCommonMarkerQuad *)buffer->data;

        xy = screen1;
        x = xy;
        y = xy >> 16;
        u = 0;
        SetPolyGT4(quad);
        SetSemiTrans(quad, 1);
        quad->r0 = level >> 1;
        quad->g0 = level >> 1;
        quad->b0 = level >> 1;
        quad->r1 = level >> 1;
        quad->g1 = level >> 1;
        quad->b1 = level >> 1;
        quad->r2 = level >> 1;
        quad->g2 = level >> 1;
        quad->b2 = level >> 1;
        quad->r3 = level >> 1;
        quad->g3 = level >> 1;
        quad->b3 = level >> 1;
        quad->x0 = x;
        quad->y0 = y;
        quad->x1 = x + 0x7F;
        quad->y1 = y;
        quad->x2 = x;
        quad->y2 = y + 12;
        quad->x3 = x + 0x7F;
        quad->y3 = y + 13;
        quad->u0 = u;
        quad->v0 = i * 13;
        quad->u1 = u + 0x7F;
        quad->v1 = i * 13;
        quad->u2 = u;
        quad->v2 = i * 13 + 12;
        quad->u3 = u + 0x7F;
        quad->v3 = i * 13 + 12;
        if (g_FxCommonRoomVariant == i)
            quad->clut = GetClut(0x3C0, 0x90);
        else
            quad->clut = GetClut(0x3C0, 0x8F);
        quad->tpage = GetTPage(0, 1, 0x3C0, 0);
        leftBuffer = g_FxCommonActiveBuffer;
        leftLink.pointer = quad;
        quad->tag.bits.address = leftBuffer->allocation[9].bits.address;
        leftBuffer->allocation[9].bits.address = leftLink.word;
        quad++;

        u = 0x80;
        SetPolyGT4(quad);
        SetSemiTrans(quad, 1);
        quad->r0 = u;
        quad->g0 = u;
        quad->b0 = u;
        quad->r1 = u;
        quad->g1 = u;
        quad->b1 = u;
        quad->r2 = u;
        quad->g2 = u;
        quad->b2 = u;
        quad->r3 = u;
        quad->g3 = u;
        quad->b3 = u;
        quad->x0 = x;
        quad->y0 = y;
        quad->x1 = x + 0x7F;
        quad->y1 = y;
        quad->x2 = x;
        quad->y2 = y + 12;
        quad->x3 = x + 0x7F;
        quad->y3 = y + 13;
        quad->u0 = u;
        quad->v0 = i * 13;
        quad->u1 = u + 0x7F;
        quad->v1 = i * 13;
        quad->u2 = u;
        quad->v2 = i * 13 + 12;
        quad->u3 = u + 0x7F;
        quad->v3 = i * 13 + 12;
        quad->clut = GetClut(0x3C0, 0x91);
        quad->tpage = GetTPage(0, 2, 0x3C0, 0);
        rightBuffer = g_FxCommonActiveBuffer;
        rightLink.pointer = quad;
        quad->tag.bits.address = rightBuffer->allocation[9].bits.address;
        rightBuffer->allocation[9].bits.address = rightLink.word;
        quad++;
        rightBuffer->data = (u8 *)quad;
    }
    return value;
}

#undef FX_MODE_PACKET

s16 FxCommon_StepFadeIntensity(s16 value)
{
    s16 result;

    result = value;
    if (value > 0) {
        if (value < 0xFF) {
            FxCommon_DrawFullScreenFade((u8)result);
            result = value + 0x10;
        } else {
            FxCommon_DrawFullScreenFade(0xFF);
        }
    }

    if (result < 0) {
        if (result >= -0xFE) {
            FxCommon_DrawFullScreenFade((u8)(result - 1));
            result -= 0x10;
        } else {
            FxCommon_DrawFullScreenFade(0);
            result = 0;
        }
    }

    return result;
}

void FxCommon_DrawFullScreenFade(int intensity)
{
    FxCommonPacketCursor *packet;
    FxCommonBuffer *quadBuffer;
    FxCommonBuffer *modeBuffer;
    FxCommonAddress packetAddress;
    u32 addressMask = 0xFFFFFF;
    u32 commandMask;
    u32 tag;

    packet = g_FxCommonActiveBuffer->data;
    SetPolyG4(&packet->quad);
    SetSemiTrans(&packet->quad, 1);

    packet->quad.color0[0] = intensity;
    packet->quad.color0[1] = intensity;
    packet->quad.color0[2] = intensity;
    packet->quad.color1[0] = intensity;
    packet->quad.color1[1] = intensity;
    packet->quad.color1[2] = intensity;
    packet->quad.color2[0] = intensity;
    packet->quad.color2[1] = intensity;
    packet->quad.color2[2] = intensity;
    packet->quad.color3[0] = intensity;
    packet->quad.color3[1] = intensity;
    packet->quad.color3[2] = intensity;
    packet->quad.x0 = 0;
    packet->quad.y0 = 0;
    packet->quad.x1 = 0x140;
    packet->quad.y1 = 0;
    packet->quad.x2 = 0;
    packet->quad.y2 = 0xF0;
    packet->quad.x3 = 0x140;
    packet->quad.y3 = 0xF0;

    commandMask = 0xFF000000;
    packetAddress.pointer = packet;
    quadBuffer = g_FxCommonActiveBuffer;
    tag = packet->quad.tag;
    tag &= commandMask;
    tag |= *(u32 *)quadBuffer->allocation & addressMask;
    packet->quad.tag = tag;
    *(u32 *)quadBuffer->allocation =
        (*(u32 *)quadBuffer->allocation & commandMask) |
        (packetAddress.word & addressMask);
    packet++;
    quadBuffer->data = packet;

    SetDrawTPage(&packet->mode.packet, 0, 0, 0x40);
    modeBuffer = g_FxCommonActiveBuffer;
    packet->mode.packet.tag.packed = (packet->mode.packet.tag.packed & commandMask) |
                              (*(u32 *)modeBuffer->allocation & addressMask);
    packetAddress.pointer = packet;
    addressMask = packetAddress.word & addressMask;
    *(u32 *)modeBuffer->allocation =
        (*(u32 *)modeBuffer->allocation & commandMask) | addressMask;
    modeBuffer->data = &packet->mode.packet + 1;
}

/* Advance effect selection, resource transitions and the two motion blends. */
void FxCommon_StepEffects(void)
{
    FxCommonMotionRecord mode1Sample;
    FxCommonMotionRecord mode1NextSample;
    FxCommonMotionRecord mode2Sample;
    FxCommonMotionRecord mode2NextSample;
    GteShortVector shortA;
    GteShortVector shortB;
    GteShortVector sampleExtra;
    FxCommonMotionRecord motionSample;
    GteShortVector motionExtra;
    s32 copyX;
    s32 copyY;
    s32 copyZ;
    s32 currentX;
    s32 currentY;
    s32 currentZ;
    s16 resourceResult;
    s32 rowIndex;
    /* Raw halfword is loaded before the blend duration, then sign-extended. */
    u16 phase0;
    u16 phase1;
    s16 nextMode;
    s16 remaining0;
    s16 remaining1;
    s16 motionBase0;
    s16 motionBase1;
    s16 countdown;
    s16 mode;
    s16 transitionMode;
    s32 accumX0;
    s32 accumX1;
    s32 accumZ0;
    s32 accumZ1;
    s32 nextFrame;
    s32 elapsed;
    s32 resourceTicks;
    s32 accumY1;
    s32 accumY0;
    s32 motionSlot0;
    s32 motionSlot1;
    u32 motionIndex0;
    u32 motionIndex1;
    s16 nextIndex0;
    s16 nextIndex1;

    resourceResult = 0;
    if (D_8019C026 != 0) {
        countdown = D_8019C026 - 1;
        D_8019C026 = countdown;
        if (countdown == 1) {
            D_8019C024 = 1;
        }
    }
    if (D_8019C045 == 1) {
        func_80195994(g_FxCommonRoomVariant, 0, 0, D_8019CC1C << 8);
        rowIndex = g_FxCommonRoomVariant;
        if (D_8019CC1C >= g_FxCommonEffectSetup[rowIndex].value30) {
            D_8019C032 = 0;
        }
        if (D_8019CC1C < (g_FxCommonEffectSetup[rowIndex].value0c - 1)) {
            D_8019CC1C += 1;
        } else {
            D_8019C045 = 0;
        }
    }
    if ((D_8019C026 != 0) && (D_8019C045 == 0) && (D_8019C034 != 0xA) && (D_8019C046 == 0) && (D_8019BFF8 == -1)) {
        nextFrame = D_8019CC1C + 1;
        D_8019CC1C = nextFrame;
        func_80195994(g_FxCommonRoomVariant, 0, 0, nextFrame << 8);
    }
    if ((D_8019C044 == 1) && (elapsed = D_8019C048 + 1, D_8019C048 = elapsed, ((elapsed < 0xC9) == 0)) && (D_8019C040 == 0)) {
        if (D_8019C046 == 0) {
            D_8019C04C = 0;
        }
        D_8019C046 = 1;
    } else {
        D_8019C046 = 0;
    }
    mode = D_8019C034;
    if (mode != 0xA) {
        if ((D_8019C026 == 0) && (g_FieldPadBits != 0)) {
            if ((D_8009D1F4 & 2) && (D_8019BFF8 == -1)) {
                nextMode = (s16)(mode + 1) % 3;
                D_8019C034 = nextMode;
                if ((nextMode << 0x10) == 0) {
                    g_FxCommonMarkerFade = 1;
                    D_8019C045 = 0;
                    func_80195994(g_FxCommonRoomVariant, 4, 5, 0);
                    if ((D_8019C026 == 0) && (g_AkaoBgmHandle[0] != 0)) {
                        Akao_SendTableCommand(g_AkaoBgmHandle[0], 0x44D, 0, 0x80, 0x7F);
                    }
                }
                transitionMode = D_8019C034;
                if (transitionMode == 1) {
                    if (g_FieldPadBits & 0x20) {
                        if ((D_8019C026 == 0) && (g_AkaoBgmHandle[0] != 0)) {
                            Akao_SendTableCommand(g_AkaoBgmHandle[0], 0x44C, 0, 0x80, 0x7F);
                        }
                        g_FxCommonMarkerFade = -1;
                        D_8019C032 = 1;
                        D_8019C048 = 0;
                        D_8019C044 = 0;
                        D_8019C045 = 0;
                        D_8019C13C = 0;
                        FxCommon_SamplePath(D_8019C038 + 0x20, 0x4A, Draw_GetTimTableEntry((&g_FxCommonTimTable), 2), &mode1Sample.vector, &sampleExtra);
                        FxCommon_SamplePath(D_8019C038 + 0x20, 0x4B, Draw_GetTimTableEntry((&g_FxCommonTimTable), 2), &mode1NextSample.vector, &sampleExtra);
                        copyX = mode1Sample.vector.x;
                        copyY = mode1Sample.vector.y;
                        copyZ = mode1Sample.vector.z;
                        currentX = g_FxCommonCameraTarget.x;
                        currentY = g_FxCommonCameraTarget.y;
                        currentZ = g_FxCommonCameraTarget.z;
                        shortA.x = (s16)copyX;
                        shortA.y = copyY - 0x64;
                        shortA.z = (s16)copyZ;
                        shortB.x = (s16)currentX;
                        shortB.y = (s16)currentY;
                        shortB.z = (s16)currentZ;
                        FxCommon_SetMotionEndpoints((FxCommonShortVec3 *)&shortA, (FxCommonShortVec3 *)&shortB, 4, 5);
                        D_8019C13E = 1;
                    } else {
                        D_8019C034 = 2;
                        D_8019C13E = 0;
                    }
                }
                if (D_8019C034 == 2) {
                    if ((D_8019C026 == 0) && (g_AkaoBgmHandle[0] != 0)) {
                        Akao_SendTableCommand(g_AkaoBgmHandle[0], 0x44C, 0, 0x80, 0x7F);
                    }
                    if (D_8019C13E == 0) {
                        g_FxCommonMarkerFade = -1;
                    }
                    if (D_8019C13E == 1) {
                        g_FxCommonMarkerFade = 0;
                    }
                    D_8019C032 = 1;
                    D_8019C048 = 0;
                    D_8019C044 = 0;
                    D_8019C045 = 0;
                    D_8019C13C = 0;
                    FxCommon_SamplePath(D_8019C03C + 0x100, 0x4C, Draw_GetTimTableEntry((&g_FxCommonTimTable), 2), &mode2Sample.vector, &sampleExtra);
                    FxCommon_SamplePath(D_8019C03C + 0x100, 0x4D, Draw_GetTimTableEntry((&g_FxCommonTimTable), 2), &mode2NextSample.vector, &sampleExtra);
                    shortA.x = (s16) mode2NextSample.vector.x;
                    shortA.y = (s16) mode2NextSample.vector.y;
                    shortA.z = (s16) mode2NextSample.vector.z;
                    shortB.x = (s16) mode2Sample.vector.x;
                    shortB.y = (s16) mode2Sample.vector.y;
                    shortB.z = (s16) mode2Sample.vector.z;
                    FxCommon_SetMotionEndpoints((FxCommonShortVec3 *)&shortA, (FxCommonShortVec3 *)&shortB, 4, 5);
                }
            }
            if ((D_8009D1F4 & 0x50) && (D_8019C034 != 0)) {
                D_8019C034 = 0;
                g_FxCommonMarkerFade = 1;
                D_8019C045 = 0;
                func_80195994(g_FxCommonRoomVariant, 4, 5, 0);
                if ((D_8019C026 == 0) && (g_AkaoBgmHandle[0] != 0)) {
                    Akao_SendTableCommand(g_AkaoBgmHandle[0], 0x44D, 0, 0x80, 0x7F);
                }
            }
            if ((D_8019C040 < 2U) && (D_8019C034 == 0) && (D_8019C13C == 0)) {
                if ((D_8009D1F4 & 0x20000000) && (D_8019C026 == 0)) {
                    if (D_8019C040 == 1) {
                        D_8019C02A = 1;
                        D_8019C026 = 0x10;
                        D_8019CA68 = g_FxCommonRoomVariant;
                    }
                    if (D_8019C044 == 0) {
                        if (g_AkaoBgmHandle[0] != 0) {
                            Akao_SendTableCommand(g_AkaoBgmHandle[0], 0x44C, 0, 0x80, 0x7F);
                        }
                        g_FxCommonMarkerFade = -1;
                        D_8019C045 = 1;
                        D_8019C044 = 1;
                        D_8019CC1C = 0;
                        D_8019C048 = 1;
                    } else {
                        if (g_AkaoBgmHandle[0] != 0) {
                            Akao_SendTableCommand(g_AkaoBgmHandle[0], 0x44C, 0, 0x80, 0x7F);
                        }
                        D_8019C02A = 1;
                        D_8019C026 = 0x10;
                        D_8019CA68 = g_FxCommonRoomVariant;
                    }
                    if ((D_8019C000 == 1) && (g_FxCommonRoomVariant == D_8019C004)) {
                        g_FxCommonMarkerFade = -1;
                        D_8019C02A = 1;
                        D_8019C045 = 0;
                        D_8019C044 = 0;
                        D_8019CC1C = 0;
                        D_8019C048 = 0;
                        D_8019C026 = 0x10;
                        D_8019CA68 = g_FxCommonRoomVariant;
                    }
                }
                if ((D_8009D1F4 & 0x40000000) && (D_8019BFF8 == -1) && (D_8019C026 == 0)) {
                    D_8019C032 = 1;
                    if (D_8019C044 == 1) {
                        if (g_AkaoBgmHandle[0] != 0) {
                            Akao_SendTableCommand(g_AkaoBgmHandle[0], 0x44D, 0, 0x80, 0x7F);
                        }
                        func_80195994(g_FxCommonRoomVariant, 4, 5, 0);
                        g_FxCommonMarkerFade = 1;
                        D_8019C044 = 0;
                        D_8019C045 = 0;
                    }
                }
                if ((D_8009D1F4 & 0x40) && (D_8019BFF8 == -1)) {
                    if (D_8019C044 == 1) {
                        g_FxCommonMarkerFade = 1;
                    }
                    D_8019C045 = 0;
                    D_8019C044 = 0;
                    D_8019CC1C = 0;
                    D_8019C032 = 1;
                    if (g_FxCommonEffectSetup[g_FxCommonRoomVariant].value22 != 0xFF) {
                        do {
                            nextIndex0 = g_FxCommonEffectSetup[g_FxCommonRoomVariant].value22;
                            g_FxCommonRoomVariant = nextIndex0;
                        } while (g_FxCommonEffectSetup[nextIndex0].enabled == 0);
                        D_8019C018 = 1;
                        D_8019CC1C = g_FxCommonEffectSetup[nextIndex0].value0c;
                        func_80195994(nextIndex0, 4, 5, 0);
                        motionIndex0 = 0;
                        motionSlot0 = 0;
                        phase0 = g_FxCommonRoomVariant;
                        g_FxCommonMotionBlendFrames = 0x20;
                        motionBase0 = (s16)phase0;
                        do {
                            FxCommon_SamplePath(motionIndex0 << 8, motionBase0 + 0x40, Draw_GetTimTableEntry((&g_FxCommonTimTable), 2), &motionSample.vector, &motionExtra);
                            motionIndex0 += 1;
                            g_FxCommonMotionDeltas[motionSlot0].vector.x = ((motionSample.vector.x << 0x10) - D_8019CAA8[motionSlot0].vector.x) >> 5;
                            g_FxCommonMotionDeltas[motionSlot0].vector.y = ((motionSample.vector.y << 0x10) - D_8019CAA8[motionSlot0].vector.y) >> 5;
                            g_FxCommonMotionDeltas[motionSlot0].vector.z = ((motionSample.vector.z << 0x10) - D_8019CAA8[motionSlot0].vector.z) >> 5;
                            motionSlot0 += 1;
                        } while (motionIndex0 < 0xAU);
                    }
                }
                if ((D_8009D1F4 & 0x10) && (D_8019BFF8 == -1)) {
                    if (D_8019C044 == 1) {
                        g_FxCommonMarkerFade = 1;
                    }
                    D_8019C045 = 0;
                    D_8019C044 = 0;
                    D_8019CC1C = 0;
                    D_8019C032 = 1;
                    if (g_FxCommonEffectSetup[g_FxCommonRoomVariant].value23 != 0xFF) {
                        do {
                            nextIndex1 = g_FxCommonEffectSetup[g_FxCommonRoomVariant].value23;
                            g_FxCommonRoomVariant = nextIndex1;
                        } while (g_FxCommonEffectSetup[nextIndex1].enabled == 0);
                        D_8019C018 = 1;
                        D_8019CC1C = g_FxCommonEffectSetup[nextIndex1].value0c;
                        func_80195994(nextIndex1, 4, 5, 0);
                        motionIndex1 = 0;
                        motionSlot1 = 0;
                        phase1 = g_FxCommonRoomVariant;
                        g_FxCommonMotionBlendFrames = 0x20;
                        motionBase1 = (s16)phase1;
                        do {
                            FxCommon_SamplePath(motionIndex1 << 8, motionBase1 + 0x40, Draw_GetTimTableEntry((&g_FxCommonTimTable), 2), &motionSample.vector, &motionExtra);
                            motionIndex1 += 1;
                            g_FxCommonMotionDeltas[motionSlot1].vector.x = ((motionSample.vector.x << 0x10) - D_8019CAA8[motionSlot1].vector.x) >> 5;
                            g_FxCommonMotionDeltas[motionSlot1].vector.y = ((motionSample.vector.y << 0x10) - D_8019CAA8[motionSlot1].vector.y) >> 5;
                            g_FxCommonMotionDeltas[motionSlot1].vector.z = ((motionSample.vector.z << 0x10) - D_8019CAA8[motionSlot1].vector.z) >> 5;
                            motionSlot1 += 1;
                        } while (motionIndex1 < 0xAU);
                    }
                }
            }
        }
    }
    if ((D_8019C034 == 1) && (g_FxCommonMotionRemaining0 == 0) && (g_FxCommonMotionRemaining1 == 0)) {
        ((FxCommonVec3 *)&g_FxCommonCameraPosition)->x = D_801EA58C->state[0];
        g_FxCommonCameraPosition.y = D_801EA58C->state[1] - 0x12C;
        g_FxCommonCameraPosition.z = D_801EA58C->state[2];
        ((FxCommonVec3 *)&g_FxCommonCameraTarget)->x = D_801EA578->state[0];
        g_FxCommonCameraTarget.y = D_801EA578->state[1];
        g_FxCommonCameraTarget.z = D_801EA578->state[2];
    }
    if ((D_8019C034 == 2) && (g_FxCommonMotionRemaining0 == 0) && (g_FxCommonMotionRemaining1 == 0)) {
        ((FxCommonVec3 *)&g_FxCommonCameraPosition)->x = D_801EA580->state[0];
        g_FxCommonCameraPosition.y = D_801EA580->state[1];
        g_FxCommonCameraPosition.z = D_801EA580->state[2];
        ((FxCommonVec3 *)&g_FxCommonCameraTarget)->x = D_801EA578->state[0];
        g_FxCommonCameraTarget.y = D_801EA578->state[1];
        g_FxCommonCameraTarget.z = D_801EA578->state[2];
    }
    if (D_8019C046 == 1) {
        FxCommon_SamplePath(D_8019C04C, g_FxCommonEffectSetup[g_FxCommonRoomVariant].motionIndices[3], Draw_GetTimTableEntry((&g_FxCommonTimTable), 2), &g_FxCommonCameraPosition, &sampleExtra);
        FxCommon_SamplePath(D_8019C04C, g_FxCommonEffectSetup[g_FxCommonRoomVariant].motionIndices[2], Draw_GetTimTableEntry((&g_FxCommonTimTable), 2), &g_FxCommonCameraTarget, &sampleExtra);
        D_8019C04C += 0x80;
    }
    if (D_8019C034 == 0xA) {
        D_8019C030 += 1;
        FxCommon_SamplePath(D_8019C04C, 0x4E, Draw_GetTimTableEntry((&g_FxCommonTimTable), 2), &g_FxCommonCameraPosition, &sampleExtra);
        FxCommon_SamplePath(D_8019C04C, 0x4F, Draw_GetTimTableEntry((&g_FxCommonTimTable), 2), &g_FxCommonCameraTarget, &sampleExtra);
        D_8019C04C += 0x10;
        if ((s16) D_8019C030 == 0x4B) {
            D_8019C0C4 = FxCommon_SwitchResource(0xB, 0U);
            D_8019C0C8 = 0;
        }
        if ((s16)D_8019C030 == 0x11D) {
            FxCommon_SwitchResource(0xE, 1U);
        }
        if ((s16)D_8019C030 == 0x13B) {
            D_8019C0C4 = FxCommon_SwitchResource(0xC, 0U);
            D_8019C0C8 = 0;
        }
        if ((s16)D_8019C030 == 0x20D) {
            FxCommon_SwitchResource(0xE, 1U);
        }
        if ((s16)D_8019C030 == 0x22B) {
            D_8019C0C4 = FxCommon_SwitchResource(0xD, 0U);
            D_8019C0C8 = 0;
        }
        if ((s16)D_8019C030 == 0x2FD) {
            FxCommon_SwitchResource(0xE, 1U);
        }
        if ((s16)D_8019C030 == 0x31B) {
            D_8019C0C4 = FxCommon_SwitchResource(0xE, 0U);
            D_8019C0C8 = 0;
        }
        if ((s16)D_8019C030 == 0x3ED) {
            FxCommon_SwitchResource(0xE, 1U);
        }
        if ((s16)D_8019C030 == 0x40B) {
            D_8019C0C4 = FxCommon_SwitchResource(0xE, 1U);
        }
        if (D_8019C0C8 == 0) {
            D_8019C0CD = 0;
            D_8019C0CC = 0;
        }
        if (D_8019C0C8 == 0x18) {
            D_8019C0CD = 1;
        }
        if (D_8019C0C8 == 0xBA) {
            D_8019C0CD = 2;
        }
        D_8019C0C8 += 1;
        if ((D_8019C0CD == 0) && (D_8019C0CC < 0x78U)) {
            D_8019C0CC += 5;
        }
        if ((D_8019C0CD == 2) && (D_8019C0CC >= 0xBU)) {
            D_8019C0CC -= 5;
        }
        Menu_SetHighlightColor(D_8019C0C4, D_8019C0CC, D_8019C0CC, D_8019C0CC);
        resourceTicks = D_8019C0BC + 1;
        D_8019C0BC = resourceTicks;
        if (resourceTicks == 0x4B1) {
            D_8019C02A = 1;
            D_8019C026 = 0x10;
            D_8019CA68 = g_FxCommonRoomVariant;
            Akao_Cmd_A9(0x12C, 0);
            D_8019C0C0 = 1;
        }
        if (D_8019C0BC == 0x41A) {
            Akao_Cmd_C1_WithSlot(g_SceneAudioSlot0Handle, 0xDC, 0);
        }
    }
    FxCommon_StepCameraPath();
    if (D_8019C018 != 0) {
        D_8019C018 = 0;
        if (D_8019C01C == 1) {
            Draw_CloseTextboxPage(D_8019C028);
        }
        Menu_SetHighlightColor(g_FxCommonEffectSetup[g_FxCommonRoomVariant].value25, 0x80, 0x80, 0x80);
        if (D_8019C034 != 0xA) {
            Render_SetupColorTable(g_FxCommonEffectSetup[g_FxCommonRoomVariant].value25, 3, &resourceResult);
        }
        D_8019C028 = (s16) g_FxCommonEffectSetup[g_FxCommonRoomVariant].value25;
        D_8019C01C = 1;
    }
    if (g_FxCommonMotionRemaining0 != 0) {
        remaining0 = g_FxCommonMotionRemaining0 - 1;
        g_FxCommonMotionRemaining0 = remaining0;
        accumX0 = g_FxCommonMotionAccum0.x + D_8019C08C;
        g_FxCommonMotionAccum0.x = accumX0;
        accumY0 = g_FxCommonMotionAccum0.y + D_8019C090;
        accumZ0 = g_FxCommonMotionAccum0.z + D_8019C094;
        g_FxCommonMotionAccum0.y = accumY0;
        g_FxCommonMotionAccum0.z = accumZ0;
        g_FxCommonCameraTarget.x = g_FxCommonMotionOrigin0.x + (accumX0 >> D_8019C056);
        g_FxCommonCameraTarget.y = g_FxCommonMotionOrigin0.y + (accumY0 >> D_8019C056);
        g_FxCommonCameraTarget.z = g_FxCommonMotionOrigin0.z + (accumZ0 >> D_8019C056);
        if ((remaining0 << 0x10) == 0) {
            g_FxCommonCameraTarget.x = g_FxCommonMotionTarget0.x;
            g_FxCommonCameraTarget.y = g_FxCommonMotionTarget0.y;
            g_FxCommonCameraTarget.z = g_FxCommonMotionTarget0.z;
        }
    }
    if (g_FxCommonMotionRemaining1 != 0) {
        remaining1 = g_FxCommonMotionRemaining1 - 1;
        g_FxCommonMotionRemaining1 = remaining1;
        accumX1 = g_FxCommonMotionAccum1.x + D_8019C05C;
        g_FxCommonMotionAccum1.x = accumX1;
        accumY1 = g_FxCommonMotionAccum1.y + D_8019C060;
        accumZ1 = g_FxCommonMotionAccum1.z + D_8019C064;
        g_FxCommonMotionAccum1.y = accumY1;
        g_FxCommonMotionAccum1.z = accumZ1;
        g_FxCommonCameraPosition.x = g_FxCommonMotionOrigin1.x + (accumX1 >> D_8019C052);
        g_FxCommonCameraPosition.y = g_FxCommonMotionOrigin1.y + (accumY1 >> D_8019C052);
        g_FxCommonCameraPosition.z = g_FxCommonMotionOrigin1.z + (accumZ1 >> D_8019C052);
        if ((remaining1 << 0x10) == 0) {
            g_FxCommonCameraPosition.x = g_FxCommonMotionTarget1.x;
            g_FxCommonCameraPosition.y = g_FxCommonMotionTarget1.y;
            g_FxCommonCameraPosition.z = g_FxCommonMotionTarget1.z;
        }
    }
}

#undef DRAW
#undef DRAW_PAIR
#undef FX_MODE_PACKET

s16 FxCommon_SwitchResource(s16 resourceId, u8 action)
{
    s16 result;
    s16 id;

    result = 0;
    id = resourceId;
    if (action == 0) {
        if (id != g_FxCommonResourceState.active) {
            Draw_CloseTextboxPage(g_FxCommonResourceState.current.half);
            Menu_SetHighlightColor(id, 0x80, 0x80, 0x80);
            Render_SetupColorTable(id, 3, &result);
            g_FxCommonResourceState.current.word = id;
        }
    }

    if (action == 1) {
        Draw_CloseTextboxPage(g_FxCommonResourceState.current.half);
    }

    return resourceId;
}

/* Pins and empty scheduling barriers preserve the retail argument setup,
 * interleaved vector reads, and snapshot stores; tracked in crutch debt. */
void func_80195994(s32 index, s32 first, s32 second, u32 source) {
    FxCommonMotionRecord point;
    GteShortVector extra;
    s32 secondMask;
    s16 setupIndex;
    s32 firstMask;
    s32 firstValue;
    s32 secondValue;
    TimRelativeTable *table = &g_FxCommonTimTable;
    GteShortVector *extraPtr;
    volatile FxCommonShortVec3 *origin0 = &g_FxCommonMotionOrigin0;
    volatile FxCommonShortVec3 *target0 = &g_FxCommonMotionTarget0;
    volatile FxCommonShortVec3 *origin1 = &g_FxCommonMotionOrigin1;
    volatile FxCommonShortVec3 *target1 = &g_FxCommonMotionTarget1;
    u32 sourceValue;
    TimRelativeTable *lookupTable;
    s32 lookupCount;
    void *base;
    s32 bx;
    s32 by;
    s32 bz;
    s32 ax;
    s32 ay;
    s32 az;
    s32 px;
    s32 py;
    s32 pz;
    s32 qx;
    s32 qy;
    s32 qz;

    lookupTable = table;
    firstValue = first;
    lookupCount = 2;
    sourceValue = source;
    /* Capture the second lane index after preparing the lookup arguments. */
    secondValue = second;
    base = Draw_GetTimTableEntry(lookupTable, lookupCount);
    setupIndex = (s16)index;
    extraPtr = &extra;
    FxCommon_SamplePath(sourceValue, g_FxCommonEffectSetup[setupIndex].motionIndices[0], base, &point.vector, extraPtr);

    lookupTable = table;
    firstValue &= 0xff;
    px = point.vector.x;
    bx = g_FxCommonCameraTarget.x;
    py = point.vector.y;
    by = g_FxCommonCameraTarget.y;
    pz = point.vector.z;
    bz = g_FxCommonCameraTarget.z;
    D_8019C056 = firstValue;
    firstMask = 1 << firstValue;
    D_8019C09C = 0;
    D_8019C0A0 = 0;
    D_8019C0A4 = 0;
    g_FxCommonMotionRemaining0 = firstMask;
    D_8019C08C = px - bx;
    D_8019C090 = py - by;
    D_8019C094 = pz - bz;
    D_8019C0AC = bx;
    origin0->y = by;
    origin0->z = bz;
    D_8019C0B4 = px;
    target0->y = py;
    target0->z = pz;

    base = Draw_GetTimTableEntry(lookupTable, 2);
    FxCommon_SamplePath(sourceValue, g_FxCommonEffectSetup[setupIndex].motionIndices[1], base, &point.vector, extraPtr);

    secondValue &= 0xff;
    qx = point.vector.x;
    ax = g_FxCommonCameraPosition.x;
    qy = point.vector.y;
    ay = g_FxCommonCameraPosition.y;
    qz = point.vector.z;
    az = g_FxCommonCameraPosition.z;
    secondMask = 1 << secondValue;
    D_8019C06C = 0;
    D_8019C070 = 0;
    D_8019C074 = 0;
    D_8019C052 = secondValue;
    g_FxCommonMotionRemaining1 = secondMask;
    D_8019C05C = qx - ax;
    D_8019C060 = qy - ay;
    D_8019C064 = qz - az;
    D_8019C07C = ax;
    origin1->y = ay;
    origin1->z = az;
    D_8019C084 = qx;
    target1->y = qy;
    target1->z = qz;
}
