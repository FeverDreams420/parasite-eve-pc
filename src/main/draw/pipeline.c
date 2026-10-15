#include "common.h"

#include "pe1/boot.h"
#include "pe1/entity_allocation.h"
#include "pe1/game_state.h"
#include "pe1/geom_state.h"
#include "pe1/psyq_gpu.h"
#include "pe1/render_camera.h"
#include "pe1/render_lighting.h"
#include "pe1/room_floor.h"
#include "pe1/save_tail_state.h"
#include "pe1/scene_dispatch.h"
#include "pe1/scene_room_load.h"
#include "pe1/scene_transition.h"
#include "pe1/script_state.h"

RoomFloorY g_RoomFloorY[1];

/* GPU pipeline start-up: voice bank, first room, light matrix and colours,
 * entity textures and fog. Retail compiles it without small-data address
 * lowering, so the light-matrix element stores keep their own absolute
 * addresses; only its COMMON words below are gp-relative. */

extern u32 g_GlobalFrameCounter[1];


#define W(sym) (*(s32 *)(sym))
#define H(sym) (*(s16 *)(sym))
#define B(sym) (*(s8 *)(sym))
#define UB(sym) (*(u8 *)(sym))

s32 Scene_StreamTexturePage(void);
void Entity_InitFreePool(void);
void Script_InitNodeFreeList(void);
int CdRom_DetectDiscChange(void);
int Scene_LoadEntityTexture(void);
void Entity_RelocateSceneData(void);
void Render_SetupFogLayer(void *source);
void Script_DrawSyncAndFlush(void);
void FieldFx_ClearScriptSprites(void);
void SetDispMask(int arg0);

int Draw_InitPipeline(void) {
    int one;
    int v1;
    int t0;
    int t1;
    char *p;
    char *a0p;
    void *fogSource;
    int room_status;

    Akao_LoadVoiceBankAlt();
    room_status = Scene_LoadRoom(g_SceneDispatchToken);
    if (room_status < 0) return room_status;
    Scene_StreamTexturePage();
    g_EntitySerialCounter = 1;
    H(g_ScriptNodeSeqCounter) = 1;
    Entity_InitFreePool();
    Script_InitNodeFreeList();

    Boot_SetBackColor((u16 *)&g_RenderLightingBlock.light.matrix.m[0][0], 0x28, 0x28, 0x28);

    v1 = 0;
    t1 = 0;
    if (v1 < -0x7FFF) {
        v1 = -0x7FFF;
    }
    t0 = 0x7FFF;
    a0p = (char *)&g_RenderLightingState;
    *(s16 *)a0p = t0;
    g_RenderLightingState.matrix.m[0][1] = t1;
    g_RenderLightingState.matrix.m[0][2] = v1;
    Boot_SetLightColor(&g_RenderLightingBlock, 0, 0xFF, 0xFF, 0xFF);

    v1 = 0;
    t0 = 0;
    t1 = 0x7FFF;
    if (v1 < -0x7FFF) {
        v1 = -0x7FFF;
    }
    p = (char *)&g_RenderLight1DirX;
    *(s16 *)p = t0;
    g_RenderLightingState.matrix.m[1][1] = t1;
    g_RenderLightingState.matrix.m[1][2] = v1;
    Boot_SetLightColor(&g_RenderLightingBlock, 1, 0x80, 0x80, 0x80);

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
    p = (char *)&g_RenderLightingState.matrix.m[2][0];
    *(s16 *)p = t0;
    g_RenderLightingState.matrix.m[2][1] = t1;
    g_RenderLightingState.matrix.m[2][2] = v1;
    Boot_SetLightColor(&g_RenderLightingBlock, 2, 0x60, 0x60, 0x60);

    one = 1;
    Render_PrepareFrame();
    SetGeomScreen(W(g_RenderProjectionDistance));
    g_ScreenTint.fadeMode |= 0x40;
    CdRom_DetectDiscChange();
    while ((room_status = Scene_LoadEntityTexture()) == one) {
    }
    if (room_status < 0) return room_status;

    one = 1;
    Scene_SetStoryDay(g_GameState.current_story_day);
    while ((room_status = Scene_LoadEntityTextures()) == one) {
    }
    if (room_status < 0) return room_status;

    Entity_RelocateSceneData();
    if (g_GameState.flags & 0x40000000) {
        fogSource = g_GameState.bank_reset_950[1];
    } else {
        fogSource = g_GameState.bank_reset_950[0];
    }
    Render_SetupFogLayer(fogSource);
    Script_DrawSyncAndFlush();
    FieldFx_ClearScriptSprites();
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
    return 0;
}
