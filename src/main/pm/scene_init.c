#include "pe1/pm.h"
#include "pe1/scene_assets.h"

/* Native storage for the original shared PM table record. */
PmTables g_PmTables;

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
