/* Process manager: allocates the 22 process slots, dispatches the per-command
 * handler callbacks, and loads the task programs of commands 108..114. */

#include "pe1/cdrom.h"
#include "pe1/disc_load_state.h"
#include "pe1/fieldfx_task.h"
#include "pe1/game_state.h"
#include "pe1/player_entity.h"
#include "pe1/pm.h"
#include "pe1/psyq_bios.h"
#include "pe1/scene_assets.h"
#include "pe1/scene_room_load.h"

static inline int FindSlot(u32 command)
{
    int slot = -1;
    int i;
    if (command >= 192)
        return -1;
    if (command - 70 < 15) {
        PmSecondarySlot *entry = g_PmSlotTable2;
        for (i = 0; i < 11; ++i, ++entry)
            if (!entry->header.state) {
                slot = i + 11;
                break;
            }
    } else {
        PmPrimarySlot *entry = g_PmSlotTable;
        for (i = 0; i < 11; ++i, ++entry)
            if (!entry->header.state) {
                slot = i;
                break;
            }
    }
    return slot;
}

int Pm_AllocSlot(unsigned int cmd) {
    return FindSlot(cmd);
}

static inline PmSlotHeader *SlotHeader(int slot)
{
    if ((unsigned int)slot >= 11) {
        int idx = slot - 11;
        return &g_PmSlotTable2[idx].header;
    }
    return &g_PmSlotTable[slot].header;
}

int Pm_FreeSlot(int slot) {
    PmSlotHeader *entry;
    unsigned int i;

    if ((unsigned int)slot >= 22) {
        return -1;
    }
    entry = SlotHeader(slot);
    if (entry->command == 0x72) {
        for (i = 0x6C; i < 0x73; i++) {
            g_FieldFxTaskPrograms[i - 85] = 0;
        }
        g_GameState.flags &= ~0x10000;
    }
    entry->state = 0;
    entry->command = -1;
    entry->field02 = -1;
    entry->field03 = -1;
    entry->ticks = 0;
    entry->owner = 0;
    return 0;
}

int Scene_LoadRoomAssets(u32 command, void *owner)
{
    u32 original;
    int subcommand = 0;
    int slot, ready;
    PmSlotHeader *entry;
    if (command >= 192)
        return -7;
    if (command - 108 < 7 && !(g_GameState.flags & 0x10000)) {
retry:
        while (CdRom_ReadSectorsFromLba(g_DiscLoadState.baseLba + g_SceneAssetLbaTbl[0],
                g_PmTaskProgramBuffer, g_SceneAssetLbaTbl[1] - g_SceneAssetLbaTbl[0]) == -1) {}
        while ((ready = CdRom_PollReady()) != 0) {
            if (ready == -1)
                goto retry;
        }
        EnterCriticalSection();
        FlushCache();
        ExitCriticalSection();
        g_FieldFxTaskPrograms[108 - 85] = g_FieldFxTaskProgramCmd108;
        g_FieldFxTaskPrograms[109 - 85] = g_FieldFxTaskProgramCmd109;
        g_FieldFxTaskPrograms[110 - 85] = g_FieldFxTaskProgramCmd110;
        g_FieldFxTaskPrograms[111 - 85] = g_FieldFxTaskProgramCmd111;
        g_FieldFxTaskPrograms[112 - 85] = g_FieldFxTaskProgramCmd112;
        g_FieldFxTaskPrograms[113 - 85] = g_FieldFxTaskProgramCmd113;
        g_FieldFxTaskPrograms[114 - 85] = g_FieldFxTaskProgramCmd114;
        g_GameState.flags |= 0x10000;
    }
    Asset_LoadTimTextures(0);
    original = command;
    if (command >= 85) {
        subcommand = command - 85;
        command = 85;
    }
    if (!g_PmCmdHandlerTable[command])
        return -8;
    if (!g_PmCmdHandlerTable[command]->initialize)
        return -1;
    slot = FindSlot(command);
    if (slot == -1)
        return -3;
    if ((unsigned int)slot >= 22)
        return -1;
    if (slot >= 11) {
        int idx = slot - 11;
        entry = &g_PmSlotTable2[idx].header;
    } else
        entry = &g_PmSlotTable[slot].header;
    entry->state = 1;
    entry->command = original;
    entry->field02 = 0;
    entry->field03 = 0;
    entry->ticks = 0;
    entry->owner = owner;
    if (command == 85)
        FieldFx_BindTaskProgram((FieldFxTaskOwner *)entry, subcommand);
    g_PmCmdHandlerTable[command]->initialize(entry);
    return slot;
}

int Pm_SendCmd(int slot, int arg1, int arg2, int arg3, int arg4, int arg5) {
    PmSlotHeader *entry;
    int cmd;
    PmCommand *handler;

    if ((unsigned int)slot >= 22) {
        return -10;
    }
    entry = SlotHeader(slot);
    cmd = entry->command;
    if ((unsigned int)cmd >= 0xC0) {
        return -11;
    }
    if ((unsigned int)cmd >= 0x55) {
        cmd = 0x55;
    }
    handler = g_PmCmdHandlerTable[cmd];
    if (handler == 0) {
        return -12;
    }
    if (handler->send == 0) {
        return -1;
    }
    if (arg1 == 1 && arg2 == 0) {
        *(int *)arg3 = entry->field02;
        *(int *)arg4 = entry->field03;
        *(int *)arg5 = entry->ticks;
    }
    return g_PmCmdHandlerTable[cmd]->send(entry, arg1, arg2, arg3, arg4, arg5);
}

int Pm_SetGetState(int slot, int set, int value) {
    PmSlotHeader *entry;
    int cmd;

    if ((unsigned int)slot >= 22) {
        return -13;
    }
    entry = SlotHeader(slot);
    cmd = entry->command;
    if ((unsigned int)cmd >= 0x55) {
        cmd = 0x55;
    }
    if (g_PmCmdHandlerTable[cmd] == 0) {
        return -15;
    }
    if (set == 0) {
        if ((unsigned int)value < 6) {
            entry->state = value;
        }
    } else {
        *(int *)value = entry->state;
    }
    return entry->state;
}

int Pm_Start(int slot) {
    PmSlotHeader *entry;
    int cmd;
    PmCommand *handler;

    if ((unsigned int)slot >= 22) {
        return -16;
    }
    entry = SlotHeader(slot);
    if (entry->state - 1U >= 2U) {
        return 0;
    }
    cmd = entry->command;
    if ((unsigned int)cmd >= 0xC0) {
        return -17;
    }
    if ((unsigned int)cmd >= 0x55) {
        cmd = 0x55;
    }
    handler = g_PmCmdHandlerTable[cmd];
    if (handler == 0) {
        return -18;
    }
    if (handler->start != 0) {
        return handler->start(entry);
    }
    return -1;
}

static inline void ClearSlot(int slot)
{
    PmSlotHeader *entry;
    unsigned int i;

    if ((unsigned int)slot < 22) {
        entry = SlotHeader(slot);
        if (entry->command == 0x72) {
            for (i = 0x6C; i < 0x73; i++) {
                g_FieldFxTaskPrograms[i - 85] = 0;
            }
            g_GameState.flags &= ~0x10000;
        }
        entry->state = 0;
        entry->command = 0xFF;
        entry->field02 = 0xFF;
        entry->field03 = 0xFF;
        entry->ticks = 0;
        entry->owner = 0;
    }
}

int Pm_Exec(int slot) {
    PmSlotHeader *entry;
    s16 state;
    int cmd;
    PmCommand *handler;
    int ret;

    if ((unsigned int)slot >= 22) {
        return -19;
    }
    entry = SlotHeader(slot);
    state = entry->state;
    if (state - 1U >= 2U && entry->state - 4U >= 2U) {
        return 0;
    }
    if (entry->state == 4) {
        entry->state = 5;
        return 0;
    }
    if (entry->state == 5) {
        ClearSlot(slot);
        return 0;
    }
    if (entry->state == 1) {
        entry->state = 2;
    }
    cmd = entry->command;
    if ((unsigned int)cmd >= 0xC0) {
        return -20;
    }
    if ((unsigned int)cmd >= 0x55) {
        cmd = 0x55;
    }
    handler = g_PmCmdHandlerTable[cmd];
    if (handler == 0) {
        return -21;
    }
    if (handler->execute != 0) {
        ret = handler->execute(entry);
        entry->ticks++;
        return ret;
    }
    return -1;
}

int Pm_Stop(int slot, void *owner, int force) {
    PmSlotHeader *entry;
    int state;
    int cmd;
    PmCommand *handler;
    int result;

    if ((unsigned int)slot >= 22) {
        return -22;
    }
    entry = SlotHeader(slot);
    state = entry->state;
    if (state == 0 || state == 6) {
        return 0;
    }
    if (force == 0 && entry->owner != owner) {
        return 0;
    }
    cmd = entry->command;
    if ((unsigned int)cmd >= 0xC0) {
        return -23;
    }
    if ((unsigned int)cmd >= 0x55) {
        cmd = 0x55;
    }
    handler = g_PmCmdHandlerTable[cmd];
    if (handler == 0) {
        return -24;
    }
    if (handler->stop == 0) {
        return -1;
    }
    result = handler->stop(entry);
    ClearSlot(slot);
    return result;
}

int Scene_FreeEntityTable(void *owner)
{
    int result;
    int i;
    if (!owner)
        return -25;
    result = 0;
    for (i = 0; i < 11; ++i) {
        PmSlotHeader *entry = &g_PmSlotTable[i].header;
        if (entry->owner == owner) {
            result = Pm_Stop(i, owner, 1);
            if (result)
                return result;
            ClearSlot(i);
        }
    }
    for (i = 0; i < 11; ++i) {
        PmSlotHeader *entry = &g_PmSlotTable2[i].header;
        if (entry->owner == owner) {
            result = Pm_Stop(i + 11, owner, 1);
            if (result)
                return result;
            ClearSlot(i + 11);
        }
    }
    return result;
}

s32 Pm_StopAll(void) {
    int i;
    int result;
    u32 command;

    result = 0;
    for (i = 0; i < 11; i++) {
        PmSlotHeader *entry = &g_PmSlotTable[i].header;
        command = entry->command;
        if (command < 8 || command - 0x55 < 0x1E) {
            result = Pm_Stop(i, g_PlayerEntity, 1);
            if (result != 0) {
                return result;
            }
            ClearSlot(i);
        }
    }
    return result;
}

s32 Pm_StopUpperHalf(void) {
    int i;
    int result;

    for (i = 0; i < 11; i++) {
        result = Pm_Stop(i + 11, 0, 1);
        if (result != 0) {
            return result;
        }
        ClearSlot(i + 11);
    }
    return result;
}

s32 Pm_StopLowerHalf(void) {
    int i;
    int result;

    for (i = 0; i < 11; i++) {
        result = Pm_Stop(i, 0, 1);
        if (result != 0) {
            return result;
        }
        ClearSlot(i);
    }
    return result;
}

void Pm_StopAllBoth(void) {
    Pm_StopLowerHalf();
    Pm_StopUpperHalf();
}
