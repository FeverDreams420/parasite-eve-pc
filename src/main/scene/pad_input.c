#include "pe1/boot.h"
#include "pe1/game_state.h"
#include "pe1/gte.h"
#include "pe1/menu_widget.h"
#include "pe1/pad_state.h"
#include "pe1/scene_dispatch.h"

PadPacket g_PadPackets[2];
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
