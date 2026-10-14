#include "common.h"
#include "pe1/battle_runtime.h"
#include "pe1/battle_status.h"
#include "pe1/boot.h"
#include "pe1/card_obj.h"
#include "pe1/display_env.h"
#include "pe1/draw_buffers.h"
#include "pe1/field_sfx.h"
#include "pe1/font.h"
#include "pe1/game_state.h"
#include "pe1/game_state_types.h"
#include "pe1/gte.h"
#include "pe1/menu_inventory.h"
#include "pe1/pad_state.h"
#include "pe1/psyq_callbacks.h"
#include "pe1/psyq_cd.h"
#include "pe1/psyq_gpu.h"
#include "pe1/psyq_spu_api.h"
#include "pe1/scene_dispatch.h"
#include "pe1/scene_flags.h"

/* COMMON supplies small-data metadata to MASPSX: these words are reached
 * through $gp, every other extern stays absolute. */

/* Boot-time system initialization: Boot_InitSystem, the subsystem and global
 * reset it is followed by, the start-up display reset, the vsync callback and
 * the pad start-up (G0341/G0342). */

void Render_ResetScene(int arg0, int arg1);
void InitGeom(void);
void SetGeomOffset(int x, int y);
void SetGeomScreen(int h);
void MemCard_InitManager(void);
void Boot_InitMemCard(void);
void DsInit(void);

void Boot_InitSystem(void) {
    ResetCallback();
    Render_ResetScene(0x140, 0xE0);
    SpuInit();
    InitGeom();
    SetGeomOffset(0xA0, 0x70);
    SetGeomScreen(0xF0);
    MemCard_InitManager();
    Boot_InitMemCard();
    DsInit();
    DsSetDebug(0);
}

#define NULL ((void *)0)
u32 Draw_FlushPrimQueue(void);
int Save_InitSystem();
void Script_InitNodePool(void);
int Entity_ResetStateGlobals();
void Entity_ResetAllPools(void);
int SsInit();
void Menu_SetEquipSlotIndex(int index);
void Boot_BuildRenderFlagTable(void);
int Save_PostInitStub();
int CdRom_InitScreenState();
int Draw_InitGpuHwRegs();

void Boot_InitSubsystems(void) {
    u32 var_s0;

    g_SceneDispatchCur = 0;
    g_SceneDispatchToken = 0;
    g_GameStateFlags = 0;
    g_FrameRngCounter = 0;
    g_ActiveDrawSlot = 0;
    Draw_InitGpuHwRegs();
    for (var_s0 = 0; var_s0 < 0x7D0U; var_s0++) {
        Draw_FlushPrimQueue();
    }
    Boot_BuildRenderFlagTable();
    SsInit();
    VSyncCallback(NULL);
    VSyncCallback(&Boot_VsyncCallback);
    Menu_SetEquipSlotIndex(0);
    Save_InitSystem();
    Save_PostInitStub();
    CdRom_InitScreenState();
    Script_InitNodePool();
    Entity_ResetStateGlobals();
    Entity_ResetAllPools();
    Script_ClearSfxTable();
    Menu_ConsumeEquipSlotFlag();
}

void ResetGraph(int arg0);
void SetGraphDebug(int level);
void ClearImage(RECT *rect, int r, int g, int b);

void Render_ResetScene(int width, int height)
{
    s16 rect[4];

    ResetGraph(0);
    SetGraphDebug(0);
    SetDispMask(0);

    rect[2] = 0x3FF;
    rect[3] = 0x200;
    rect[0] = 0;
    rect[1] = 0;
    ClearImage(rect, 0, 0, 1);

    g_RenderDispEnvArray[0].isrgb24 = 1;
    g_RenderDispEnvArray[1].isrgb24 = 1;
    SetDefDispEnv(&g_RenderDispEnvArray[0], 0, height, width, height);
    SetDefDispEnv(&g_RenderDispEnvArray[1], 0, 0, width, height);

    g_RenderDispEnvArray[1].screen.y = 8;
    g_RenderDispEnvArray[0].screen.y = 8;
    g_RenderDispEnvArray[1].screen.x = 0;
    g_RenderDispEnvArray[0].screen.x = 0;
    g_RenderDispEnvArray[0].screen.h = g_RenderDispEnvArray[1].screen.h = height;
    SetDefDrawEnv(&g_RenderDrawEnvArray[0], 0, 0, width, height);
    SetDefDrawEnv(&g_RenderDrawEnvArray[1], 0, height, width, height);

    g_RenderDrawEnvArray[1].isbg = 1;
    g_RenderDrawEnvArray[0].isbg = 1;
    g_RenderDrawEnvArray[1].dtd = 1;
    g_RenderDrawEnvArray[0].dtd = 1;
    g_RenderDrawEnvArray[1].dfe = 0;
    g_RenderDrawEnvArray[0].dfe = 0;
    g_RenderDrawEnvArray[1].tpage = 0;
    g_RenderDrawEnvArray[0].tpage = 0;
    g_RenderDrawEnvArray[1].r0 = 0;
    g_RenderDrawEnvArray[0].r0 = 0;
    g_RenderDrawEnvArray[1].g0 = 0;
    g_RenderDrawEnvArray[0].g0 = 0;
    g_RenderDrawEnvArray[1].b0 = 0;
    g_RenderDrawEnvArray[0].b0 = 0;
    g_ActiveDrawSlot = 0;

    PutDispEnv(&g_RenderDispEnvArray[0]);
}

void Scene_TickTimers(void);

void PadInitDirect(char *arg0, char *arg1);
void PadStartCom(void);

void Boot_VsyncCallback(void) {
    Draw_FlushPrimQueue();
    Scene_TickTimers();
}

void Boot_InitMemCard(void) {
    PadInitDirect((u8 *)&g_PadPackets[0], (u8 *)&g_PadPackets[1]);
    PadStartCom();
}

