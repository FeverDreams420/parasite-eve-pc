#include "common.h"
#include "pe1/akao/bgm.h"
#include "pe1/akao/commands.h"
#include "pe1/akao/stream.h"
#include "pe1/akao_send.h"
#include "pe1/boot.h"
#include "pe1/cdrom.h"
#include "pe1/game_state.h"
#include "pe1/psyq_bios.h"
#include "pe1/scene_archive.h"
#include "pe1/scene_assets.h"

/* COMMON supplies small-data metadata to MASPSX: these words are reached
 * through $gp, every other extern stays absolute. */
int g_CdFadeStartTime;
int g_CdFadeFramesLeft;
int g_CdRangeIndex;
u32 g_CdReadChunkSectors;
u32 g_CdReadRemainingSectors;
u32 g_CdReadStartLba;
u32 g_CdReadTotalSectors;
int g_CdStreamBankIndex;
s16 g_CdTrackBank;
struct SceneTrackRecord *g_CdTrackRecord;

/* Matching debt: pins and empty barriers retain the index,
* zero extension and callback state. The duplicated ready branch preserves
* GCC's block ordering. No ASM construct emits an instruction. */
int CdRom_ReadAssetSectors(unsigned kind, unsigned int index, int channel, void *buffer, u32 maximum, int blocking)
{
    /* Transfer addresses and phase changes use this scratch in separate cases. */
    u32 workValue;
    Pe1GameState *state = &g_GameState;
    int result = -1;
    u32 *archiveLba;
    u32 transferValue;
    int busy = 0;
    unsigned int isSequence;
    int finished = 0;
    u32 scaledIndex = index * 2;
    u32 byteOffset;
    SceneSectorDirectory *root = (SceneSectorDirectory *)g_SceneSectorDirectory;
    u8 *entries;
    volatile u16 *entry;
    byteOffset = scaledIndex;
    entries = (u8 *)root->offsets;
    entry = (u16 *)(scaledIndex + entries);
    do
    {
        archiveLba = &((SceneSectorDirectory *)g_SceneSectorDirectory)->base_lba;
        switch (state->cd_read_phase)
        {
        case 0:
            {
                u8 *table = (u8 *)root;
                u32 first;
                entries = table + byteOffset;
                first = *(u16 *)entry;
                scaledIndex = ((volatile SceneSectorDirectory *)entries)->offsets[1];
                scaledIndex -= entry[0];
                transferValue = *archiveLba + first;
                workValue = state->pe_image_base_lba + transferValue;
                g_CdReadTotalSectors = scaledIndex;
                g_CdReadRemainingSectors = g_CdReadTotalSectors;
                isSequence = kind == 0;
                g_CdReadStartLba = workValue;
                if (isSequence)
                {
                    result = Spu_SetStreamModeA();
                    if (result == (-1))
                    {
                        busy = 1;
                        finished = (blocking ^ 1) & 1;
                    }
                }
                else
                if (kind == 3)
                {
                    result = Spu_SetStreamModeB();
                    if (result == (-1))
                    {
                        busy = 1;
                        finished = (blocking ^ 1) & 1;
                    }
                }
                transferValue = 7;
                state->cd_read_phase = transferValue;
                break;
            }

        case 7:
            if (g_CdReadRemainingSectors != 0)
            {
                u32 count = g_CdReadRemainingSectors;
                if (maximum < count)
                {
                    count = maximum;
                }
                g_CdReadChunkSectors = count;
                result = CdRom_ReadSectors(g_CdReadStartLba, g_CdReadTotalSectors - g_CdReadRemainingSectors, buffer, count);
                if (result != (-1))
                {
                    state->cd_read_phase = 8;
                }
                busy = 1;
                finished = (blocking ^ 1) & 1;
            }
            else
            {
                state->cd_read_phase = 0;
                busy = 0;
                finished = 1;
            }
            break;

        case 8:
            result = CdRom_PollReady();
            if (result == (-1))
            {
                transferValue = 7;
                state->cd_read_phase = transferValue;
            }
            else
            if (result == 0)
            {
                if (g_CdReadRemainingSectors)
                {
                    state->cd_read_phase = 9;
                    break;
                }
                else
                {
                    state->cd_read_phase = 9;
                    break;
                }
            }
            busy = 1;
            finished = (blocking ^ 1) & 1;
            break;

        case 9:
            switch (kind)
            {
            case 0:
                result = Akao_StepNoteSequencer(buffer, g_CdReadChunkSectors << 11);
                break;

            case 1:
                result = Spu_UploadSampleBlockBlocking(buffer, 0);
                break;

            case 2:
                result = Spu_UploadStreamBlockB(channel, buffer);
                break;

            case 3:
                result = Spu_UploadStreamBlockA(channel, buffer, g_CdReadChunkSectors << 11);
                break;

            }

            if (result == (-1))
            {
                state->cd_read_phase = 0;
                busy = 1;
                finished = (blocking ^ 1) & 1;
            }
            else
            {
                state->cd_read_phase = 10;
            }
            break;

        case 10:
            result = Spu_GetTransferStatus();
            if (result == -1) {
                state->cd_read_phase = 0;
                busy = 1;
                finished = (blocking ^ 1) & 1;
            } else if (result != 0) {
                busy = 1;
                finished = (blocking ^ 1) & 1;
            } else {
                u32 remaining, remainingSnapshot;
                u32 transferred;
                busy = 1;
                remainingSnapshot = g_CdReadRemainingSectors;
                finished = (blocking ^ 1) & 1;
                remaining = remainingSnapshot;
                transferred = g_CdReadChunkSectors;
                workValue = 7;
                state->cd_read_phase = workValue;
                g_CdReadRemainingSectors = remaining - transferred;
            }
            break;
        }
    } while (!finished);
    return busy;
}

int CdRom_FindNextDataSector(void)
{
    Pe1GameState *state = &g_GameState;
    u8 *base = (u8 *)state->loaded_scene_assets;
    CdArchiveHeader *header = (CdArchiveHeader *)(base + ((CdArchiveRoot *)base)->directoryOffset);
    CdRangeAddress ranges;

    ranges.range = (CdRange *)(base + (header->range_info & 0x3fffff));

    /* Each state falls back to this dispatch until a read is pending (1)
     * or the archive's ranges are exhausted (0). */
dispatch:
    switch (state->cd_range_state) {
    case 0:
        g_CdRangeIndex = 0;
        state->cd_range_state = 0x28;
        break;
    case 0x28:
        if (CdRom_ReadAssetSectors(1, 1, 0, state->scene_load_scratch, 0x21, 0) == 1) return 1;
        if (state->cd_range_read_mode >= 2) {
            state->cd_range_state = 0x29;
            return 1;
        }
        state->cd_range_state = 0x2a;
        break;
    case 0x29:
        if (CdRom_ReadAssetSectors(1, state->cd_range_read_mode, 0, state->scene_load_scratch, 0x21, 0) == 1) return 1;
        state->cd_range_state = 0x2a;
        break;
    case 0x2a:
    {
        int index = g_CdRangeIndex;
        CdRangeAddress entry;

        if (index < (int)(header->range_info >> 22)) {
            entry.word = index * sizeof(CdRange) + ranges.word;
            if (!(entry.range->flags & 0x10) || entry.range->first < 2) {
                g_CdRangeIndex = index + 1;
            } else {
                state->cd_range_state = 0x2b;
            }
        } else {
            state->cd_range_state = 0;
            return 0;
        }
        break;
    }
    case 0x2b:
    {
        CdRangeAddress entry;

        entry.word = g_CdRangeIndex * sizeof(CdRange) + ranges.word;
        if (CdRom_ReadAssetSectors(3, entry.range->first, entry.range->last, state->scene_load_scratch, 0x21, 0) == 1) return 1;
        state->cd_range_state = 0x2a;
        g_CdRangeIndex = g_CdRangeIndex + 1;
        break;
    }
    default:
        return 0;
    }
    goto dispatch;
}

void Akao_ResetPositionalState(void) {
    g_GameState.scene_audio.tracks.keys[0][1] = -1;
    g_GameState.scene_audio.tracks.keys[0][0] = -1;
    g_GameState.scene_audio.tracks.keys[1][1] = -1;
    g_GameState.scene_audio.tracks.keys[1][0] = -1;
    g_GameState.scene_audio.tracks.banks[1] = -1;
    g_GameState.scene_audio.tracks.banks[0] = -1;
    g_GameState.flags &= -0xF1;
    Akao_Cmd_F0();
}

/* Matching debt: finished is pinned to s1. The empty count barrier preserves
 * the signed loop test and its scheduling; volatile directory reads retain
 * the original pair of loads. The shifted state view retains the workspace
 * selection made before callbacks. Neither ASM construct emits instructions. */
int CdRom_SeekToTrack(int key, int mode, int group, int *slot, int blocking)
{
    int busy = 0;
    int finished = 0;
    Pe1GameState *state = &g_GameState;
    int bankIndex = group != 0;
    u8 *destination;
    volatile SceneAssetDirectory *directory;

    /* Group 1 copies into the second bank work bound. */
    destination = bankIndex ? state->bank_work_end : state->bank_work_base;
    directory = SceneAsset_ResolveOffset(*g_LoadedSceneAssetBlock, (*g_LoadedSceneAssetBlock)->directoryOffset);
    do {
        switch (state->cd_track_phase) {
        case 0: {
            SceneTrackRecord *record;
            int count;
            int i;
            g_CdTrackBank = -1;
            record = SceneAsset_ResolveOffset(state->loaded_scene_assets,
                                             directory->trackEntries & 0x3FFFFF);
            count = directory->trackEntries >> 22;
            g_CdTrackRecord = record;
            for (i = 0; i < count; i++, record++) {
                if (record->key == key) {
                    g_CdTrackBank = record->bank;
                    g_CdTrackRecord = record;
                    break;
                }
            }
            if (g_CdTrackBank == -1) {
                for (i = 0; i < 2; i++) {
                    if (state->scene_audio.tracks.keys[i][0] == key) {
                        g_CdTrackBank = state->scene_audio.tracks.banks[i];
                        g_CdTrackRecord = 0;
                        break;
                    }
                }
                if (g_CdTrackBank == -1) {
                    *slot = -1;
                    busy = 0;
                    finished = 1;
                    break;
                }
            }
            if (mode) {
                if (state->scene_audio.tracks.keys[0][0] == key) {
                    if (state->flags & 0x40)
                        *slot = -2;
                    else
                        *slot = 0;
                    busy = 0;
                    finished = 1;
                } else if (state->scene_audio.tracks.keys[1][0] == key) {
                    if (state->flags & 0x80)
                        *slot = -2;
                    else
                        *slot = 1;
                    busy = 0;
                    finished = 1;
                } else {
                    if (!group)
                        Akao_Cmd_F0();
                    state->cd_track_phase = 7;
                }
            } else {
                if (state->scene_audio.tracks.keys[0][0] == key) {
                    state->scene_audio.tracks.keys[0][0] = -1;
                    ((s8 *)state->scene_audio.tracks.banks)[0] = -1;
                    state->flags &= ~0x40;
                    *slot = 0;
                }
                if (state->scene_audio.tracks.keys[1][0] == key) {
                    state->scene_audio.tracks.keys[1][0] = -1;
                    ((s8 *)state->scene_audio.tracks.banks)[1] = -1;
                    state->flags &= ~0x80;
                    *slot = 1;
                }
                busy = 0;
                finished = 1;
            }
            break;
        }
        case 7:
            if (CdRom_ReadAssetSectors(0, g_CdTrackBank, 0, state->scene_load_scratch, 0x21, blocking) == 1) {
                busy = 1;
                finished = (blocking ^ 1) & 1;
            } else {
                state->cd_track_phase = 9;
            }
            break;
        case 9:
            *slot = bankIndex;
            if (g_CdTrackRecord) {
                memcpy(destination,
                       SceneAsset_ResolveOffset(state->loaded_scene_assets,
                                                g_CdTrackRecord->offset & 0xFFFFFF),
                       g_CdTrackRecord->size & 0xFFFFFF);
            }
            busy = 0;
            finished = 1;
            state->scene_audio.tracks.keys[*slot][0] = key;
            state->scene_audio.tracks.banks[*slot] = g_CdTrackBank;
            state->cd_track_phase = 0;
            break;
        }
    } while (!finished);
    return busy;
}

/* Matching debt: the retry jump. */
int CdRom_StepReadState(int active)
{
    Pe1GameState *state = &g_GameState;
    int slot;
    int voice;
    u32 flags;
    u32 changing;
retry:
    {
        switch (state->cd_transition_phase) {
        case 0:
            g_CdStreamBankIndex = 0;
            Akao_Cmd_F1();
            if (active) {
                if (!(g_GameState.flags & 0x400000)) {
                    if (g_AkaoBgmHandle[0]) {
                        Akao_SendTableCommand(g_AkaoBgmHandle[0], 0x451, 0, 0x80, 0x7F);
                        if (g_AkaoBgmHandle[0])
                            Akao_SendTableCommand(g_AkaoBgmHandle[0], 0x452, 0, 0x80, 0x7F);
                    }
                }
                state->cd_transition_phase = 44;
            } else {
                if (state->flags & 4) {
                    int zero = 0;
                    int duration = 60;
                    int now = *g_GlobalFrameCounter;
                    g_CdFadeStartTime = now;
                    Akao_Cmd_C1_WithSlot(zero, duration, 0);
                }
                state->cd_transition_phase = 45;
            }
            break;
        case 44:
            if (state->scene_audio.tracks.pending_key != state->scene_audio.tracks.keys[0][0]) {
                if (state->flags & 0x40) {
                    int zero = 0;
                    int duration = 60;
                    int now = *g_GlobalFrameCounter;
                    g_CdFadeStartTime = now;
                    Akao_Cmd_C1_WithSlot(zero, duration, 0);
                }
                state->flags |= 4;
            }
            state->cd_transition_phase = 46;
            break;
        case 46:
            if (CdRom_FindNextDataSector() == 1)
                return 1;
            flags = state->flags;
            changing = flags & 4;
            if (changing) {
                if (flags & 0x40) {
                    g_CdFadeFramesLeft = 60 - (*g_GlobalFrameCounter - g_CdFadeStartTime);
                    if (g_CdFadeFramesLeft > 8) {
                        g_CdFadeFramesLeft = 8;
                        Akao_Cmd_C1_WithSlot(0, 16, 0);
                    } else if (g_CdFadeFramesLeft < 0) {
                        g_CdFadeFramesLeft = 0;
                    }
                }
            }
            state->cd_transition_phase = 63;
            break;
        case 63:
            if (state->flags & 4) {
                if (state->flags & 0x40) {
                    if (g_CdFadeFramesLeft > 0) {
                        --g_CdFadeFramesLeft;
                        return 1;
                    }
                    Akao_Cmd_40();
                    Akao_Cmd_F0();
                }
                state->cd_transition_phase = 47;
            } else {
                state->cd_transition_phase = 48;
            }
            break;
        case 47:
            if (CdRom_ReadAssetSectors(0, state->scene_audio.tracks.pending_bank, 0,
                               state->scene_load_scratch, 0x21, 0) == 1)
                return 1;
            state->cd_transition_phase = 48;
            break;
        case 48:
            Akao_Cmd_10(state->bank_asset_table);
            Akao_Cmd_C0_WithSlot(0, 0x7F);
            state->cd_transition_phase = 0;
            return 0;
        case 45:
            if (g_CdStreamBankIndex < 2) {
                if (state->pending_stream_banks[g_CdStreamBankIndex]) {
                    state->cd_transition_phase = 49;
                    break;
                }
                ++g_CdStreamBankIndex;
            }
            state->cd_transition_phase = 50;
            break;
        case 49:
            if (CdRom_ReadAssetSectors(2, state->pending_stream_banks[g_CdStreamBankIndex], g_CdStreamBankIndex,
                               state->scene_load_scratch, 0x21, 0) == 1)
                return 1;
            ++g_CdStreamBankIndex;
            state->cd_transition_phase = 45;
            break;
        case 50:
            if (state->pending_sample_bank != -1 &&
                CdRom_ReadAssetSectors(1, state->pending_sample_bank, 0,
                               state->scene_load_scratch, 0x21, 0) == 1)
                return 1;
            if (state->flags & 4) {
                g_CdFadeFramesLeft = 60 - (*g_GlobalFrameCounter - g_CdFadeStartTime);
                if (g_CdFadeFramesLeft > 8) {
                    g_CdFadeFramesLeft = 8;
                    Akao_Cmd_C1_WithSlot(0, 16, 0);
                } else if (g_CdFadeFramesLeft < 0) {
                    g_CdFadeFramesLeft = 0;
                }
            }
            state->cd_transition_phase = 64;
            break;
        case 64:
            if (state->flags & 4) {
                if (g_CdFadeFramesLeft > 0) {
                    --g_CdFadeFramesLeft;
                    return 1;
                }
                Akao_Cmd_F0();
                if (state->flags & 0x40) {
                    state->flags &= ~0x40;
                    state->cd_transition_phase = 62;
                    break;
                }
            }
            state->cd_transition_phase = 0;
            state->flags &= ~4;
            return 0;
        case 62:
            if (CdRom_ReadAssetSectors(0, ((s8 *)state->scene_audio.tracks.banks)[0], 0,
                               state->scene_load_scratch, 0x21, 0) == 1)
                return 1;
            state->cd_transition_phase = 51;
            break;
        case 51:
            if (CdRom_SeekToTrack(state->scene_audio.tracks.keys[0][0], 1, 0, &slot, 0) == 1)
                return 1;
            voice = Akao_Cmd_10(state->bank_work_base);
            if (voice == -1) {
                state->cd_transition_phase = 62;
                return 1;
            }
            Akao_Cmd_C1_WithSlot(voice, 60, state->track_volumes[0]);
            if (voice)
                CdRom_RegisterAudioSlot(0, state->scene_audio.tracks.keys[0][0], voice,
                                          state->track_volumes[0]);
            state->cd_transition_phase = 0;
            state->flags = (state->flags & ~4) | 0x40;
            return 0;
        default:
            return 0;
        }
        goto retry;
    }
}

/* Scene audio slots 0 and 1: each caches a track key, its bank and a volume,
 * and registering one marks that slot's track as loaded in the state flags. */
int CdRom_RegisterAudioSlot(int slot, int key, int bank, int volume) {
    Pe1GameState *state = &g_GameState;

    state->scene_audio.tracks.keys[slot][0] = key;
    state->scene_audio.tracks.keys[slot][1] = bank;
    state->track_volumes[slot] = volume;

    if (slot == 0) {
        state->flags |= 0x40;
    } else if (slot == 1) {
        state->flags |= 0x80;
    }

    return 0;
}

int CdRom_GetAudioSlotByKey(int key) {
    Pe1GameState *state = &g_GameState;
    int i;

    for (i = 0; i < 2; i++) {
        if (state->scene_audio.tracks.keys[i][0] == key) {
            return state->scene_audio.tracks.keys[i][1];
        }
    }

    return -1;
}

int CdRom_FindAudioSlotIndex(int key) {
    Pe1GameState *state = &g_GameState;
    int i = 0;

    do {
        if (state->scene_audio.tracks.keys[i][0] == key) {
            return i;
        }
        i++;
    } while (i < 2);

    return -1;
}
