#include <SDL3/SDL.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <psyz/system.h>
#include "common.h"
#include "pe1/battle_runtime.h"
#include "pe1/battle_status.h"
#include "pe1/boot.h"
#include "pe1/boot_services.h"
#include "pe1/cdrom_buffers.h"
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
#include "pe1/player_entity.h"
#include "pe1/textbox.h"
#include "pe1/menu_widget.h"

/* Native backing for the original boot allocator. Overlay workspaces are
 * ordinary persistent memory; native OT/actor pointers need larger arenas. */
_Alignas(16) u8 g_BootWorkArea[0x20000];
_Alignas(16) u8 D_8010BD00[BOOT_TEXTURE_WORKSPACE_BYTES];
_Alignas(16) u8 D_80120D08[0x200000], g_SceneLoadScratchArea[0x20000];
static _Alignas(16) u8 g_BootLoadedSceneArea[0x80008];
u8 *g_StrFileDirBuffer = g_BootLoadedSceneArea + 8;
static _Alignas(16) u8 g_BootDisplayWorkArea[BOOT_DISPLAY_WORK_SIZE];
u8 *g_CdLoadAreaBase = g_BootDisplayWorkArea;

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

int g_BootHostActive;
u32 g_BootHostVBlankLimit,g_BootHostVBlankCount;
const char *g_BootHostVramPath;
BootHostPadEvent *g_BootHostPadEvents;
unsigned g_BootHostPadEventCount;
static unsigned hostPadEvent;
static u16 hostButtons;

static void Boot_PollHost(void) {
    ++g_BootHostVBlankCount;
    if(g_BootHostPadEventCount){
        while(hostPadEvent<g_BootHostPadEventCount &&
              g_BootHostPadEvents[hostPadEvent].vblank<=g_BootHostVBlankCount)
            hostButtons=g_BootHostPadEvents[hostPadEvent++].buttons;
    }
    if(Psyz_QuitRequested())exit(EXIT_SUCCESS);
    if(!g_BootHostVBlankLimit || g_BootHostVBlankCount<g_BootHostVBlankLimit)return;
    if(g_BootHostVramPath){
        static _Alignas(8) u16 vram[1024*512];RECT all={0,0,1024,512};
        StoreImage(&all,vram);DrawSync(0);
        FILE *out=fopen(g_BootHostVramPath,"wb");
        if(!out){fprintf(stderr,"Cannot write VRAM capture\n");fflush(stderr);_Exit(2);}
        int written=fwrite(vram,1,sizeof(vram),out)==sizeof(vram);
        if(fclose(out)!=0 || !written){fprintf(stderr,"Cannot finish VRAM capture\n");fflush(stderr);_Exit(2);}
    }
    DISPENV display;GetDispEnv(&display);
    printf("{\"event\":\"frame_limit\",\"vblanks\":%u,\"scene\":%u,\"flags\":%u,\"draw_slot\":%d,\"movie\":%d,\"pad\":%u,\"game_flags\":%u,\"video_state\":%u,\"game_frame\":%u,\"movie_frame\":%d,\"move_lock\":%d,\"display\":{\"x\":%d,\"y\":%d,\"w\":%d,\"h\":%d,\"rgb24\":%d},\"dialogue\":[",
        g_BootHostVBlankCount,(u32)g_SceneDispatchToken,g_GameStateFlags,g_ActiveDrawSlot,
        g_GameState.scene_audio.tracks.movie_index,g_FieldPadBits,g_GameState.flags,
        g_GameState.scene_audio.tracks.draw_enabled.bits,g_GlobalFrameCounter[0],
        (s16)g_GameState.scene_audio.tracks.movie_frame,g_FieldMoveLock,
        display.disp.x,display.disp.y,display.disp.w,display.disp.h,display.isrgb24);
    for(unsigned i=0;i<4;++i)
        printf("%s{\"state\":%u,\"page\":%d}",i?",":"",g_TextboxEntries[i].state,g_TextboxEntries[i].page_id);
    printf("],\"actors\":[");
    unsigned actor_index=0;
    for(FieldActor *actor=g_FieldActorListHead;actor;actor=actor->next){
        printf("%s{\"type\":%u,\"sub\":%u,\"player\":%s,\"flags\":%u,\"position\":[%d,%d,%d],\"yaw\":%d}",
            actor_index++?",":"",actor->type_id,actor->sub_id,actor==g_PlayerEntity?"true":"false",actor->flags,
            actor->pos_x,actor->pos_y,actor->pos_z,actor->rot_y);
    }
    printf("],\"menu\":");
    if(g_MenuWidgetCurrentNode){
        MenuWidgetNode *node=g_MenuWidgetCurrentNode;
        printf("{\"mode\":%d,\"selection\":%d,\"cursor\":[%d,%d]}",
            node->mode,node->selected_base,node->cursor_x,node->cursor_y);
    }else printf("null");
    printf(",\"combat\":{\"mode\":%d,\"live_enemies\":%d",g_BattleModeState,(int)g_BattleLiveEnemyCount);
    if(g_PlayerEntity && g_PlayerEntity->state){
        Combatant *player=(Combatant *)g_PlayerEntity->state;
        printf(",\"player_hp\":%u,\"player_max_hp\":%u",(unsigned)player->curHP,(unsigned)player->maxHP);
    }
    printf("}}\n");
    fflush(stdout);
    /* A diagnostic cap is a forced process boundary, even inside a menu's
     * VSync callback. Do not unwind through the locked SDK callback stack. */
    _Exit(EXIT_SUCCESS);
}

void Boot_InitSystem(void) {
    ResetCallback();
    if(g_BootHostActive)VSyncCallbacks(0,Boot_PollHost);
    Render_ResetScene(0x140, 0xE0);
    SpuInit();
    InitGeom();
    SetGeomOffset(0xA0, 0x70);
    SetGeomScreen(0xF0);
    MemCard_InitManager();
    Boot_InitMemCard();
}

u32 Draw_FlushPrimQueue(void);
void Save_InitSystem(void);
void Script_InitNodePool(void);
void Entity_ResetStateGlobals(void);
void Entity_ResetAllPools(void);
void SsInit(void);
void Menu_SetEquipSlotIndex(int index);
void Boot_BuildRenderFlagTable(void);
void Save_PostInitStub(void);
int CdRom_InitScreenState();

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
    RECT rect;

    ResetGraph(0);
    SetGraphDebug(0);
    SetDispMask(0);

    rect.w = 0x3FF;
    rect.h = 0x200;
    rect.x = 0;
    rect.y = 0;
    ClearImage(&rect, 0, 0, 1);

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


void Boot_VsyncCallback(void) {
    Draw_FlushPrimQueue();
    Scene_TickTimers();
}

void Boot_InitMemCard(void) {
    Boot_ResetPadInput();
}


u32 g_FieldPadReleased;
u32 g_FieldPadPressed;
u32 g_FieldPadLastFrame;
u32 g_FieldPadBits;
u32 g_FieldPadBitsPrevious;

void Scene_CheckFlagBits(void *ctx, SceneFlagRecord **table, int *index_ptr)
{
    SceneFlagRecord *record;
    u32 flags;
    u32 mask_copy;
    int triggered;

    record = table[*index_ptr];
    triggered = 0;
    if (record != 0) {
        flags = record->flags;
    } else {
        flags = 0;
    }

    if (flags != 0) {
        do {
            void (*callback)(struct BattleEntity *, int *);
            u32 mask;
            u32 value;

            callback = record->callback;
            if (callback == 0) {
                return;
            }

            if (flags & 0x20) {
                callback(ctx, index_ptr);
                triggered = 1;
            } else if (flags & 1) {
                value = g_FieldPadBits;
                mask = record->mask_all;
                if ((value & mask) == mask) {
                    if (flags & 4) {
                        if (g_FieldPadPressed & record->mask_any) {
                            callback(ctx, index_ptr);
                            triggered = 1;
                        }
                    } else if (flags & 8) {
                        if (g_FieldPadReleased & record->mask_any) {
                            callback(ctx, index_ptr);
                            triggered = 1;
                        }
                    } else if (flags & 2) {
                        if (value & record->mask_any) {
                            callback(ctx, index_ptr);
                            triggered = 1;
                        }
                    } else {
                        callback(ctx, index_ptr);
                        triggered = 1;
                    }
                }
            } else if (flags & 2) {
                u32 current;

                current = g_FieldPadBits;
                mask = record->mask_any;
                mask_copy = mask;
                if (current & mask_copy) {
                    if (flags & 4) {
                        if (g_FieldPadPressed & mask) {
                            callback(ctx, index_ptr);
                            triggered = 1;
                        }
                    } else if (flags & 8) {
                        if (g_FieldPadReleased & mask_copy) {
                            callback(ctx, index_ptr);
                            triggered = 1;
                        }
                    } else {
                        callback(ctx, index_ptr);
                        triggered = 1;
                    }
                }
            } else if (flags & 4) {
                if (g_FieldPadPressed & record->mask_any) {
                    callback(ctx, index_ptr);
                    triggered = 1;
                }
            } else if (flags & 8) {
                if (g_FieldPadReleased & record->mask_any) {
                    callback(ctx, index_ptr);
                    triggered = 1;
                }
            }

            if (((flags & 0x10) == 0) && (triggered != 0)) {
                return;
            }

            record++;
            flags = record->flags;
        } while (flags != 0);
    }

    if ((record != 0) && (record->callback != 0)) {
        record->callback(ctx, index_ptr);
    }
}

PadPacket g_PadPackets[2];
int g_SceneDispatchToken;
s32 g_FieldRenderFlagTable[32];
u32 g_FieldPadHeldFrameCounts[32];
u32 g_PadComboSequence[9] = {0x10000000,0x20000000,0x40000000,0x40,8,0x10,0x20,2,4};
extern u32 g_FieldPadBitsPrevious;

/* Field render-flag table setup and the helper that stores one table entry
 * at the bit index of a single-bit mask. The helper's only caller is the
 * table builder that precedes it (main_tu_evidence G0344). */

void Boot_StoreTableEntry(u32 mask, int value);

void Boot_BuildRenderFlagTable(void) {
    s32 *entry;
    u32 i;

    i = 0;
    entry = g_FieldRenderFlagTable;
    g_FieldPadReleased = 0;
    g_FieldPadPressed = 0;
    g_FieldPadBitsPrevious = 0;
    g_FieldPadLastFrame = 0;
    g_FieldPadBits = 0;
    do {
        *entry = 0;
        i += 1;
        entry += 1;
    } while (i < 0x20U);
    Boot_StoreTableEntry(1, 0x4000);
    Boot_StoreTableEntry(0x80, 0x1000);
    Boot_StoreTableEntry(0x100, 0x2000);
    Boot_StoreTableEntry(8, 0x10);
    Boot_StoreTableEntry(0x20, 0x40);
    Boot_StoreTableEntry(0x40, 0x80);
    Boot_StoreTableEntry(0x10, 0x20);
    Boot_StoreTableEntry(2, 1);
    Boot_StoreTableEntry(4, 8);
    Boot_StoreTableEntry(0x200, 0x2000);
    Boot_StoreTableEntry(0x400, 0x4000);
    Boot_StoreTableEntry(0x2000, 0x8000);
    Boot_StoreTableEntry(0x01000000, 0x100);
    Boot_StoreTableEntry(0x02000000, 0x200);
    Boot_StoreTableEntry(0x04000000, 0x400);
    Boot_StoreTableEntry(0x08000000, 0x800);
    Boot_StoreTableEntry(0x10000000, 0x1000);
    Boot_StoreTableEntry(0x20000000, 0x2000);
    Boot_StoreTableEntry(0x40000000, 0x4000);
    Boot_StoreTableEntry(0x80000000, 0x8000);
    g_GameStateFlags = g_GameStateFlags | 0x4000;
}

void Boot_StoreTableEntry(u32 mask, int value) {
    int index;

    gte_ldlzcs(mask);
    index = 31;
    if (mask != 0x80000000) {
        gte_stlzcr(&mask);
        index -= mask;
    }
    g_FieldRenderFlagTable[index] = value;
}

u32 g_PadComboProgress;

void Field_HandleStateTransition(void) {
    /* Apply scheduled host input after SDK polling, before the unchanged
     * original button mapping and repeat/combo handling below. */
    if(g_BootHostActive && g_BootHostPadEventCount){
        g_PadPackets[0].header=0x4000;
        g_PadPackets[0].buttons=(u16)~hostButtons;
    }
    u32 flags;
    u32 pad_bits;
    u32 base_bits;
    u32 old_pad_bits;
    u32 pad_mask;
    u32 active_pad;
    u32 save_pad;
    u32 final_pad;
    u32 final_old;
    u32 changed;
    u32 mask;
    u32 rawInverted;
    u32 sequence_mask;
    u32 menu_open;
    u32 pad;
    u32 inverted;
    int mode;
    u16 i;
    u8 analog;

    if ((g_PadPackets[0].header & 0xFF) != 0) {
        u32 initial_flags = g_GameStateFlags;
        if ((initial_flags & 0x4001) == 0) {
            g_FieldPadPressed = 4;
            g_FieldPadBits = 4;
            g_FieldPadReleased = 0;
            g_GameStateFlags = initial_flags | 0x4000;
            return;
        }
        g_GameStateFlags = initial_flags | 0x4000;
        g_FieldPadBits = g_FieldPadPressed = g_FieldPadReleased = 0;
        return;
    }

    mode = g_PadPackets[0].header & 0xF000;
    if (!((mode != 0x4000) && (mode != 0x7000))) {

    /* The host supplies a ready packet; PS1 actuator/mode negotiation is
     * unnecessary. A nonzero receive-status byte means disconnected. */
    if ((g_PadPackets[0].header & 0xFF) == 0)
        g_GameStateFlags &= ~0x4000;

    pad = g_PadPackets[0].buttons;
    rawInverted = ~pad;
    pad_mask = 0xFFFF9FFF;
    base_bits = rawInverted & pad_mask;
    old_pad_bits = g_FieldPadBits;
    g_FieldPadLastFrame = old_pad_bits;
    inverted = ~(u16)pad;
    pad_bits = base_bits;
    if (inverted & 0x2000) {
        pad_bits = base_bits | 0x4000;
    }
    if (inverted & 0x4000) {
        pad_bits |= 0x2000;
    }

    g_FieldPadBits = 0;
    /* Decoded input is saved in pad_bits; reuse the scratch for key indices. */
    for (i = 0; i < 0x20; i++) {
        base_bits = i;
        if (((u16)pad_bits & g_FieldRenderFlagTable[base_bits]) != 0) {
            g_FieldPadBits |= 1u << base_bits;
            g_FieldPadHeldFrameCounts[base_bits]++;
        } else {
            g_FieldPadHeldFrameCounts[base_bits] = 0;
        }
    }

    flags = g_GameStateFlags;
    if ((flags & 1) != 0) {
        active_pad = g_FieldPadBits;
        if ((int)active_pad < 0) {
            mask = ((active_pad ^ g_FieldPadBitsPrevious) & active_pad) & 0x7000007E;
            if (mask != 0) {
                sequence_mask = g_PadComboSequence[g_PadComboProgress];
                if ((mask & sequence_mask) == sequence_mask) {
                    g_PadComboProgress++;
                    if (g_PadComboProgress == 9) {
                        g_SceneDispatchToken = 0xAA108448;
                        g_GameStateFlags = flags | 0x12000;
                    }
                } else {
                    g_PadComboProgress = 0;
                }
            }
        } else {
            g_PadComboProgress = 0;
        }
        save_pad = g_FieldPadBits;
        g_FieldPadBitsPrevious = save_pad;
    }

    menu_open = g_GameState.flags;
    if (menu_open & 0x400) {
        g_FieldPadBits &= 0x40FFDC7F;
    }
    if (menu_open & 0x200) {
        g_FieldPadBits &= 0x60FFDB04;
        if (g_GameState.scene_audio.tracks.movie_index != 1) {
            g_FieldPadBits &= 0xDFFFFCFF;
        }
    }
    if (g_FieldPadBits & 0x10000080) {
        g_FieldPadBits &= 0x7FFFFFFF;
    }
    if (g_FieldPadBits & 0x40000401) {
        g_FieldPadBits &= 0xEFFFFF7F;
        g_FieldPadBits &= 0x7FFFFFFF;
    }
    if (g_FieldPadBits & 0x20000300) {
        g_FieldPadBits &= 0xBFFFFBFE;
        g_FieldPadBits &= 0xEFFFFF7F;
        g_FieldPadBits &= 0x7FFFFFFF;
    }

    if ((g_PadPackets[0].header & 0xF000) == 0x7000) {
        g_FieldPadBits &= ~0x79;
        menu_open = MenuWidget_FindByModeAndSelectedBase(1, 0) != 0;

        if (menu_open) {
            analog = g_PadPackets[0].leftY;
            if (analog < 0x14) {
                g_FieldPadBits |= 0x8;
            } else if (analog >= 0xE7) {
                g_FieldPadBits |= 0x20;
            }

            analog = g_PadPackets[0].leftX;
            if (analog < 0x14) {
                g_FieldPadBits |= 0x40;
            } else if (analog >= 0xE7) {
                g_FieldPadBits |= 0x10;
            }
        } else {
            analog = g_PadPackets[0].leftY;
            if (analog < 0x5A) {
                g_FieldPadBits |= 0x8;
                if (analog < 0x14) {
                    g_FieldPadBits |= 0x1;
                }
            } else if (analog >= 0xA1) {
                g_FieldPadBits |= 0x20;
                if (analog >= 0xE7) {
                    g_FieldPadBits |= 0x1;
                }
            }

            analog = g_PadPackets[0].leftX;
            if (analog < 0x5A) {
                g_FieldPadBits |= 0x40;
                if (analog < 0x14) {
                    g_FieldPadBits |= 0x1;
                }
            } else if (analog >= 0xA1) {
                g_FieldPadBits |= 0x10;
                if (analog >= 0xE7) {
                    g_FieldPadBits |= 0x1;
                }
            }
        }
    }

    final_pad = g_FieldPadBits;
    final_old = g_FieldPadLastFrame;
    changed = final_pad ^ final_old;
    g_FieldPadPressed = changed & final_pad;
    g_FieldPadReleased = changed & final_old;
    return;
    }

    g_FieldPadBits = 0;
    g_FieldPadPressed = 0;
    g_FieldPadReleased = 0;
}

/* Host keyboard input feeds the original controller packet decoder. */
static bool held[SDL_SCANCODE_COUNT];
static bool focused;
static void publish(void) {
    unsigned buttons=0;
    if (focused) {
        if(held[SDL_SCANCODE_UP] || held[SDL_SCANCODE_W])buttons|=0x10;
        if(held[SDL_SCANCODE_RIGHT] || held[SDL_SCANCODE_D])buttons|=0x20;
        if(held[SDL_SCANCODE_DOWN] || held[SDL_SCANCODE_S])buttons|=0x40;
        if(held[SDL_SCANCODE_LEFT] || held[SDL_SCANCODE_A])buttons|=0x80;
        if(held[SDL_SCANCODE_LSHIFT] || held[SDL_SCANCODE_RSHIFT] || held[SDL_SCANCODE_Z])buttons|=0x2000;
        if(held[SDL_SCANCODE_X] || held[SDL_SCANCODE_SPACE])buttons|=0x4000;
        if(held[SDL_SCANCODE_C])buttons|=0x1000;
        if(held[SDL_SCANCODE_V])buttons|=0x8000;
        if(held[SDL_SCANCODE_RETURN] || held[SDL_SCANCODE_KP_ENTER] || held[SDL_SCANCODE_ESCAPE])buttons|=8;
        if(held[SDL_SCANCODE_TAB])buttons|=1;
        if(held[SDL_SCANCODE_Q])buttons|=0x400;
        if(held[SDL_SCANCODE_E])buttons|=0x800;
    }
    g_PadPackets[0].header=0x4100;
    g_PadPackets[0].buttons=(u16)~buttons;
    g_PadPackets[0].leftX=g_PadPackets[0].leftY=128;
    g_PadPackets[0].rightX=g_PadPackets[0].rightY=128;
}
static bool Boot_WatchPadEvent(void *userdata, SDL_Event *event) {
    (void)userdata;Boot_HandlePadEvent(event);return true;
}
void Boot_ResetPadInput(void) {
    SDL_RemoveEventWatch(Boot_WatchPadEvent,NULL);
    SDL_AddEventWatch(Boot_WatchPadEvent,NULL);
    memset(held,0,sizeof(held));focused=true;
    memset(g_PadPackets,0,sizeof(g_PadPackets));
    g_PadPackets[1].header=0x00ff;
    publish();
}
void Boot_HandlePadEvent(const SDL_Event *event) {
    if(event->type==SDL_EVENT_WINDOW_FOCUS_LOST || event->type==SDL_EVENT_QUIT) {
        focused=false;memset(held,0,sizeof(held));
    } else if(event->type==SDL_EVENT_WINDOW_FOCUS_GAINED)focused=true;
    else if(focused && (event->type==SDL_EVENT_KEY_DOWN || event->type==SDL_EVENT_KEY_UP)) {
        SDL_Scancode key=event->key.scancode;
        if(key>SDL_SCANCODE_UNKNOWN && key<SDL_SCANCODE_COUNT)
            held[key]=event->type==SDL_EVENT_KEY_DOWN;
    }
    publish();
}
