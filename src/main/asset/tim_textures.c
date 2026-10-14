#include "pe1/cdrom.h"
#include "pe1/game_state.h"
#include "pe1/pm.h"
#include "pe1/field_effect_modules.h"
#include "pe1/render_camera.h"
#include "pe1/psyq_tim.h"
#include "pe1/render_animation_frame.h"
#include "pe1/scene_assets.h"


/* Native storage for the original shared PM table record. */
PmCommand *g_PmCmdHandlerArray[86] = {
    [0]=&g_GlowEffectClass, [1]=&g_GlowEffectBClass,
    [2]=&g_GlowEffectCClass, [3]=&g_FourLayerGlowClass, [4]=&g_BurstEffectBClass,
    [5]=&g_BurstEffectClass, [6]=&g_BurstEffectCClass, [7]=&g_PointSpriteClass
};
PmTables g_PmTables = {.handlers=g_PmCmdHandlerArray};

static inline void InitHandler(int index)
{
    SceneAssetHandler *handler = g_PmCmdHandlerTable[index];
    if (handler && handler->init) handler->init();
}

/* Original initialization from Asset_LoadTimTextures. */
void Pm_InitScene(void)
{
    Pe1GameState *state = &g_GameState;
    int i;
    if (!(g_GameStateFlags & 0x80)) {
        g_PmTables.primary = state->scene_process_slots->primary;
        for (i = 0; i < 11; i++) {
            PmPrimarySlot *slot = &g_PmTables.primary[i];
            slot->header.state = 0;
            slot->header.owner = 0;
            slot->header.command = 255;
            slot->header.field02 = 255;
            slot->header.field03 = 255;
            slot->header.ticks = 0;
        }
        g_PmTables.secondary = state->scene_process_slots->secondary;
        for (i = 0; i < 11; i++) {
            PmSecondarySlot *slot = &g_PmTables.secondary[i];
            slot->header.state = 0;
            slot->header.owner = 0;
            slot->header.command = 255;
            slot->header.field02 = 255;
            slot->header.field03 = 255;
            slot->header.ticks = 0;
        }
        for (i = 0; i < 8; i++) InitHandler(i);
        InitHandler(85);
        {
            SceneAssetBlob *blob = &state->loaded_scene_assets->header;
            SceneAssetDirectory *directory = SceneAsset_ResolveOffset(blob, blob->directoryOffset);
            SceneAssetRecord *entry = SceneAsset_ResolveOffset(blob, directory->entries & 0x3FFFFF);
            for (i = 0; i < (int)(directory->entries >> 22); i++) {
                unsigned int id = entry[i].handlerId;
                if (id >= 8 && id < 85) InitHandler(id);
            }
        }
        state->flags = (state->flags & ~0x10000) | 8;
        g_GameStateFlags |= 0x80;
    }
}

int Asset_LoadTimTextures(int force)
{
    Pe1GameState *state = &g_GameState;
    int i;
    int status;
    RECT rect;

restart:
    switch (state->tim_load_state) {
    case 0:
        Pm_InitScene();
        if ((force || (g_GameStateFlags & 2)) && (state->flags & 8)) {
            state->tim_load_state = 0x34;
            goto restart;
        }
        state->tim_load_state = 0;
        return 0;
    case 0x34:
        {
            u32 first=g_PeImageSectorTable[5], end=g_PeImageSectorTable[6];
            u64 lba=(u64)state->pe_image_base_lba+first;
            if (!state->scene_load_scratch || end<=first ||
                lba+(end-first)>UINT64_C(0x100000000)) return -1;
            status=CdRom_ReadSectorsFromLba((u32)lba,state->scene_load_scratch,end-first);
            if (status<0) return -1;
            state->tim_load_state=0x35;
            return 1;
        }
    case 0x35:
        status = CdRom_PollReady();
        if (status < 0) {
            state->tim_load_state = 0x34;
            return -1;
        }
        if (status != 0) return 1;
        state->tim_load_state = 0x36;
        goto restart;
    case 0x36:
        {
            SceneAssetBlob *blob = (SceneAssetBlob *)state->scene_load_scratch;
            SceneAssetDirectory *directory = SceneAsset_ResolveOffset(blob, blob->directoryOffset);
            TimUploadRecord *entry = SceneAsset_ResolveOffset(blob, directory->timEntries & 0x3FFFFF);
            u32 key;
            TimPackedImage *image;
            for (i = 0; i < (int)(directory->timEntries >> 22); i++)
                Draw_LoadTimAsset(&entry[i], blob);
            blob = &state->loaded_scene_assets->header;
            for (key = 0x73DECD80; ; key += 4) {
                image = Asset_FindTable08ByU32Key(blob, key);
                if (!image) break;
                rect.x = (image->geometry >> 10) & 0x7FF;
                rect.y = image->geometry >> 21;
                rect.w = image->geometry & 0x3FF;
                rect.h = image->source.bytes.height ? image->source.bytes.height : 256;
                LoadImage(&rect, SceneAsset_ResolveOffset(image, image->source.offsetAndHeight & 0xFFFFFF));
            }
            state->tim_load_state = 0;
            state->flags &= ~8;
            return 0;
        }
    default:
        return 0;
    }
}


int Pm_RunPrimary(void) {
    int i;
    int slot;
    u32 command;

    if (g_GameStateFlags & 0x80) {
        i = 0;
        Render_SetGteScreenOffset();
        do {
            Pm_Start(i);
            i += 1;
        } while (i < 11);
        Render_ResetGteScreenOffset();
        if (!(g_GameStateFlags & 4)) {
            slot = 0;
            do {
                command = g_PmSlotTable[slot].header.command;
                if (!(g_GameStateFlags & 0x100) || command - 0x55 < 0x1E) {
                    Pm_Exec(slot);
                }
                slot += 1;
            } while (slot < 11);
        }
    }
    return 0;
}

int Pm_RunBatch(void) {
    int i;

    if (g_GameStateFlags & 0x80) {
        Render_SetGteScreenOffset();

        for (i = 0; i < 11; i++) {
            Pm_Start(i + 0xB);
        }

        Render_ResetGteScreenOffset();

        if (g_GameStateFlags & 0x104) {
            return 0;
        }

        for (i = 0; i < 11; i++) {
            Pm_Exec(i + 0xB);
        }
    }

    return 0;
}
