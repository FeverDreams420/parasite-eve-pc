#include "common.h"
#include "pe1/boot.h"
#include "pe1/entity_allocation.h"
#include "pe1/game_state.h"
#include "pe1/psyq_gpu.h"
#include "pe1/render_camera.h"
#include "pe1/render_lighting.h"
#include "pe1/room_floor.h"
#include "pe1/save_tail_state.h"
#include "pe1/scene_dispatch.h"
#include "pe1/scene_entity_textures.h"
#include "pe1/scene_room_load.h"
#include "pe1/scene_transition.h"
#include "pe1/script_state.h"

/* GPU pipeline start-up: voice bank, first room, light matrix and colours,
 * entity textures and fog. Retail compiles it without small-data address
 * lowering, so the light-matrix element stores keep their own absolute
 * addresses; only its COMMON words below are gp-relative. */

u32 g_GlobalFrameCounter[1];
int g_SceneDispatchToken;
unsigned int g_GameStateFlags;

#define W(sym) (*(s32 *)(sym))
#define H(sym) (*(s16 *)(sym))
#define B(sym) (*(s8 *)(sym))
#define UB(sym) (*(u8 *)(sym))

void Scene_LoadRoom(int arg0);
s32 Scene_StreamTexturePage(void);
void Entity_InitFreePool(void);
void Script_InitNodeFreeList(void);
void Boot_SetBackColor(void *arg0, int arg1, int arg2, int arg3);
void Boot_SetLightColor(void *arg0, int arg1, int arg2, int arg3, int arg4);
int CdRom_DetectDiscChange(void);
int Scene_LoadEntityTexture(void);
void Entity_RelocateSceneData(void);
void Render_SetupFogLayer(void *source);
void Script_DrawSyncAndFlush(void);
void func_800E0060(void);
void SetDispMask(int arg0);

void Draw_InitPipeline(void) {
    int one;
    int v1;
    int t0;
    int t1;
    char *p;
    char *a0p;
    void *fogSource;
    int frame_pad[6];

    Akao_LoadVoiceBankAlt();
    Scene_LoadRoom(g_SceneDispatchToken);
    Scene_StreamTexturePage();
    g_EntitySerialCounter = 1;
    H(g_ScriptNodeSeqCounter) = 1;
    Entity_InitFreePool();
    Script_InitNodeFreeList();

    Boot_SetBackColor((char *)&D_800BEA42 - 2, 0x28, 0x28, 0x28);

    v1 = 0;
    t1 = 0;
    if (v1 < -0x7FFF) {
        v1 = -0x7FFF;
    }
    t0 = 0x7FFF;
    a0p = (char *)&g_RenderLightingState;
    *(s16 *)a0p = t0;
    D_800BEA42 = t1;
    D_800BEA44 = v1;
    Boot_SetLightColor(a0p, 0, 0xFF, 0xFF, 0xFF);

    v1 = 0;
    t0 = 0;
    t1 = 0x7FFF;
    if (v1 < -0x7FFF) {
        v1 = -0x7FFF;
    }
    p = (char *)&D_800BEA46;
    *(s16 *)p = t0;
    D_800BEA48 = t1;
    D_800BEA4A = v1;
    Boot_SetLightColor(p - 6, 1, 0x80, 0x80, 0x80);

    v1 = 0x10000;
    t0 = 0;
    {
        int cond;
        cond = 1;
        t1 = 0;
        if (cond != 0) {
            v1 = 0x7FFF;
        } else {
            if (v1 < -0x7FFF) {
                v1 = -0x7FFF;
            }
        }
    }
    p = (char *)&D_800BEA4C;
    *(s16 *)p = t0;
    D_800BEA4E = t1;
    D_800BEA50 = v1;
    Boot_SetLightColor(p - 0xC, 2, 0x60, 0x60, 0x60);

    one = 1;
    Render_PrepareFrame();
    SetGeomScreen(W(g_RenderProjectionDistance));
    UB(g_ScreenTransitionState) |= 0x40;
    CdRom_DetectDiscChange();
    while (Scene_LoadEntityTexture() == one) {
    }

    one = 1;
    Scene_SetStoryDay(B(D_800B0CE4));
    while (Scene_LoadEntityTextures() == one) {
    }

    Entity_RelocateSceneData();
    if (g_GameState.flags & 0x40000000) {
        fogSource = g_GameState.bank_reset_950[1];
    } else {
        fogSource = g_GameState.bank_reset_950[0];
    }
    Render_SetupFogLayer(fogSource);
    Script_DrawSyncAndFlush();
    func_800E0060();
    g_GlobalFrameCounter[0] = 0;
    g_RoomFloorY->y = 0;
    DrawSync(0);
    SetDispMask(1);
    {
        int tmp;
        v1 = ~0x40;
        tmp = g_GameStateFlags;
        a0p = (char *)&g_GameState;
        g_GameStateFlags = tmp & v1;
    }
    D_8009D2E8 &= ~0xCu;
    W(a0p) &= ~0x402;
}
