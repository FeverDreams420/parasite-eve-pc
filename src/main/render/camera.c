#include "common.h"
#include "pe1/battle_runtime.h"
#include "pe1/field_movement.h"
#include "pe1/game_state.h"
#include "pe1/geom_state.h"
#include "pe1/gte.h"
#include "pe1/gte_types.h"
#include "pe1/menu_background.h"
#include "pe1/pad_state.h"
#include "pe1/projection_slot.h"
#include "pe1/render_camera.h"
#include "pe1/render_lighting.h"
#include "pe1/render_object.h"
#include "pe1/render_prim.h"
#include "pe1/render_tint.h"

/* Geometry animation control: reset, update and script setup commands. */

int Render_ResetAllEntries(void) {
    GeomState *state;
    GeomStateAddress base, controls, slots;
    GeomEntry *entries;
    unsigned int i;
    unsigned int count;

    state = g_GeomState;
    base.state = g_GeomState;
    controls.word = base.word + state->ctrl_offset;
    base.word += state->entry_offset;
    entries = base.entry;
    count = state->entry_count;
    for (i = 0; i < count; i++) {
        GeomCtrlEntry *control = &controls.ctrl[i];
        u8 group = control->position.parts.groupFraction;

        slots.ctrl = control;
        slots.word += control->slot_offset;
        control->step = 0x100;
        control->elapsed = 0;
        control->head.b.flags = 1;
        control->position.packed = group;
        entries[*slots.bytes].head.b.flags |= 2;
    }
    return 0;
}

/* Advances the geometry animations of the selected group. */
int Render_StepGroupAnimations(void)
{
    GeomStateAddress state, controls, entries;
    unsigned int i, count;

    if (!(g_GameStateFlags & 0x104) &&
        (g_GameState.flags & 0xC00000) != 0x800000) {
        state.state = g_GeomState;
        controls.word = state.word + state.state->ctrl_offset;
        entries.word = state.word + state.state->entry_offset;
        count = state.state->entry_count;
        for (i = 0; i < count; i++) {
            GeomCtrlEntry *control = &controls.ctrl[i];
            int position = control->position.bits.value;

            if ((control->head.b.flags & 2) &&
                (control->head.b.flags & 0x14) &&
                control->position.bits.group == g_GeomGroupSel) {
                unsigned int j;
                GeomStateAddress slotTable;
                GeomAnimationSlot *slots;
                unsigned int frames;
                GeomAnimationSlot *slot;

                slotTable.ctrl = control;
                slotTable.word += control->slot_offset;
                slots = slotTable.slots;
                frames = control->head.packed >> 8;
                for (j = 0; j < frames; j++)
                    entries.entry[slots[j].entry].head.b.flags &= ~2;
                /* Subtraction retains retail's address-add operand order. */
                slot = slots - (-(position >> 8));
                entries.entry[slot->entry].head.b.flags |= 2;
                if (slot->duration < 0) {
                    slot->duration = 0;
                    control->elapsed = 0;
                    control->head.b.flags &= ~4;
                    return 0;
                }
                {
                    unsigned short elapsed = control->elapsed + 1;
                    control->elapsed = elapsed;
                    if (elapsed >= slot->duration) {
                        int next;
                        unsigned int length;

                        control->elapsed = 0;
                        control->position.bits.value = position + control->step;
                        next = control->position.bits.value;
                        length = control->head.packed >> 8;
                        if ((next >> 8) >= (int)length) {
                            if (control->head.b.flags & 0x20)
                                control->position.bits.value = next % (int)(length << 8);
                            else
                                control->position.bits.value = 0;
                            control->head.b.flags &= ~4;
                        } else if ((next >> 8) < 0) {
                            if (control->head.b.flags & 0x20)
                                control->position.bits.value = (int)(length << 8) -
                                    (-next % (int)(length << 8));
                            else
                                control->position.bits.value = (length - 1) << 8;
                            control->head.b.flags &= ~4;
                        }
                    }
                }
            }
        }
    }
    return 0;
}

/* Contiguous commands update the shared geometry animation control table. */

#define GEOM_CTRL_ENTRY(address, base, index) \
    (GEOM_STATE_OFFSET(address, base, ctrl_offset, (index) << 4), (address).ctrl)

int Render_SetEntryLimit(int arg0, int arg1)
{
  GeomStateAddress address, base;
  GeomCtrlEntry *entry;
  u32 positionWord;
  u32 flags;
  u32 hi;
  int ret;
  entry = GEOM_CTRL_ENTRY(address, base, arg0);
  positionWord = entry->position.packed;
  positionWord &= 0xFF;
  ret = entry->head.b.flags;
  flags = ret;
  hi = arg1 << 16;
  entry->elapsed = 0;
  positionWord |= hi;
  flags |= 2;
  entry->head.b.flags = flags;
  ret = 0;
  entry->position.packed = positionWord;
  return ret;
}

int Render_SetFlagSlot(int arg0, int arg1) {
    GeomStateAddress table, base;
    GeomCtrlEntry *slot;

    GEOM_STATE_OFFSET(table, base, ctrl_offset, 0);
    slot = table.ctrl + arg0;
    if (arg1 != 0) {
        slot->head.b.flags |= 6;
    } else {
        slot->head.b.flags &= 0xF9;
    }

    return 0;
}

s16 Render_GetEntryField6(int index) {
    GeomStateAddress address, base;

    return GEOM_CTRL_ENTRY(address, base, index)->position.parts.frame;
}

int Render_SetEntryField8(int index, unsigned int value) {
    GeomStateAddress address, base;

    GEOM_CTRL_ENTRY(address, base, index)->step = value >> 8;
    return 0;
}

int Render_FillEntrySlotValues(int index, u8 value)
{
    GeomStateAddress address, base, slots;
    GeomCtrlEntry *entry;
    int count;
    int i;

    entry = GEOM_CTRL_ENTRY(address, base, index);
    slots.ctrl = entry;
    slots.word += entry->slot_offset;
    count = entry->head.packed >> 8;
    for (i = 0; i < count; i++) {
        slots.slots[i].duration = value;
    }
    return 0;
}

int Render_SetEntrySlotValue(int index, int slot, u8 value) {
    GeomStateAddress address, base;
    GeomCtrlEntry *entry;
    int ret;

    entry = GEOM_CTRL_ENTRY(address, base, index);
    address.word += entry->slot_offset;
    ret = 0;
    address.slots[slot].duration = value;
    return ret;
}

int Render_SetEntryFlags(int index, int bits) {
    GeomStateAddress address, base;

    GEOM_CTRL_ENTRY(address, base, index)->head.b.flags |= bits & 0x30;
    return 0;
}

/* The empty asm barrier fences the cc1 scheduler so the flag load/stores
 * keep the retail order. */
int Menu_SetSlotEntry(int index, unsigned int limit, unsigned int slot)
{
    GeomStateAddress table, base;
    GeomCtrlEntry *entry;
    u32 positionWord;
    u32 flags;
    u32 hi;
    GeomAnimationSlot *slots;
    s32 offset;
    int neg;
    s16 f8;

    GEOM_STATE_OFFSET(table, base, ctrl_offset, index << 4);
    entry = table.ctrl;
    positionWord = entry->position.packed;
    positionWord &= 0xFF;
    offset = entry->slot_offset;
    hi = limit << 16;

        entry->elapsed = 0;
        positionWord |= hi;
        {
            flags = entry->head.b.flags;
            table.word += offset;
            slots = table.slots;
            entry->position.packed = positionWord;
        }
        neg = -1;
        flags |= 6;
        entry->head.b.flags = flags;
        slots[slot].duration = neg;
        f8 = entry->step;
        if ((f8 > 0 && slot < limit) || (f8 < 0 && limit < slot)) {
            entry->step = -(u16)entry->step;
        }
        return 0;

}

void SetGeomScreen(int h);

int Render_LoadGeomState(int index) {
    GeomStateAddress table, base;
    CameraViewport *entry;
    u16 *dst;
    int value;

    GEOM_STATE_OFFSET(table, base, entry_offset_1C, index * 52);
    entry = table.viewport;
    *g_GeomScroll.state.position.projectionDistance = entry->prefix.gpu.geom_screen;
    SetGeomScreen(entry->prefix.gpu.geom_screen);

    dst = (u16 *)g_GeomScroll.words.viewMatrix;
    dst[0] = entry->prefix.gpu.half02;
    dst = (u16 *)g_GeomScroll.words.viewMatrix;
    dst[1] = entry->prefix.gpu.half04;
    dst = (u16 *)g_GeomScroll.words.viewMatrix;
    dst[2] = entry->prefix.gpu.half06;
    dst = (u16 *)g_GeomScroll.words.viewMatrix;
    dst[3] = entry->prefix.gpu.half08;
    dst = (u16 *)g_GeomScroll.words.viewMatrix;
    dst[4] = entry->prefix.gpu.half0A;
    dst = (u16 *)g_GeomScroll.words.viewMatrix;
    dst[5] = entry->prefix.gpu.half0C;
    dst = (u16 *)g_GeomScroll.words.viewMatrix;
    dst[6] = entry->prefix.gpu.half0E;
    dst = (u16 *)g_GeomScroll.words.viewMatrix;
    dst[7] = entry->prefix.gpu.half10;
    dst = (u16 *)g_GeomScroll.words.viewMatrix;
    dst[8] = entry->prefix.gpu.half12;
    dst = (u16 *)g_GeomScroll.words.viewMatrix;
    ((u32 *)dst)[5] = entry->prefix.gpu.word14;
    dst = (u16 *)g_GeomScroll.words.viewMatrix;
    ((u32 *)dst)[6] = entry->prefix.gpu.word18;
    dst = (u16 *)g_GeomScroll.words.viewMatrix;
    ((u32 *)dst)[7] = entry->prefix.gpu.word1C;

    g_GeomGroupSel = index;
    value = g_RenderStateFlags;
    g_RenderStateFlags = value | 0x80;
    return 0;
}

int CdRom_SetSeekPos(unsigned int arg0) {
    unsigned short old0;
    unsigned short old1;
    unsigned short old2;

    old0 = (u16)g_ScreenTint.targetR;
    old1 = g_ScreenTint.targetG;
    old2 = (u16)g_ScreenTint.targetB;

    g_ScreenTint.targetR = 0xFF;
    g_ScreenTint.targetG = 0xFF;
    g_ScreenTint.targetB = 0xFF;
    g_ScreenTint.fadeMode = 2;
    g_ScreenTint.blendMode = 2;
    g_ScreenTint.duration = arg0;
    g_ScreenTint.frame = 0;
    g_ScreenTint.startR = old0;
    g_ScreenTint.startG = old1;
    g_ScreenTint.startB = old2;

    return 0;
}

int CdRom_SetScreenPos(int arg0, int arg1, int arg2, int arg3, unsigned short arg4) {
    unsigned short old0;
    unsigned short old1;
    unsigned short old2;
    int mode;

    old0 = (u16)g_ScreenTint.targetR;
    old1 = g_ScreenTint.targetG;
    old2 = (u16)g_ScreenTint.targetB;

    g_ScreenTint.targetR = arg1;
    g_ScreenTint.targetG = arg2;
    g_ScreenTint.targetB = arg3;
    g_ScreenTint.fadeMode = 2;
    g_ScreenTint.duration = arg0;
    g_ScreenTint.frame = 0;
    g_ScreenTint.startR = old0;
    mode = arg4;
    g_ScreenTint.startG = old1;
    g_ScreenTint.startB = old2;

    if (mode >= 4) {
        g_ScreenTint.blendMode = 1;
        return 0;
    }
    if (mode < 0) {
        g_ScreenTint.blendMode = 1;
        return 0;
    }

    g_ScreenTint.blendMode = arg4;
    return 0;
}

int Render_SetFadeColour(unsigned int arg0) {
    unsigned short old0;
    unsigned short old1;
    unsigned short old2;

    old0 = (u16)g_ScreenTint.targetR;
    old1 = g_ScreenTint.targetG;
    old2 = (u16)g_ScreenTint.targetB;

    g_ScreenTint.fadeMode = 6;
    g_ScreenTint.targetR = 0;
    g_ScreenTint.targetG = 0;
    g_ScreenTint.targetB = 0;
    g_ScreenTint.duration = arg0;
    g_ScreenTint.frame = 0;
    g_ScreenTint.startR = old0;
    g_ScreenTint.startG = old1;
    g_ScreenTint.startB = old2;

    return 0;
}

/* Build the camera basis matrix from the camera yaw (plus the player's
 * facing quadrant while aiming): forward = R * Z, right = Y x forward,
 * up = forward x right. */
int Render_DrawSprite(void)
{
    GteMatrixStorage rotation;
    GteShortVector axis;
    GteVector up, right, newUp, forward;
    int angle, cosine, sine;
    if ((g_PadPackets[0].header & 0xF000) == 0x7000)
        angle = g_CameraBaseAngleAnalog;
    else
        angle = g_CameraBaseAngleDigital;
    {
        int masked = angle & 4095;
        if (D_8009D254 && (D_8009D2E8 & 16)) {
            angle += 2048;
            angle += (((unsigned int)((Combatant *)D_8009D254->core)->stateFlags) >> 7) & 0xC00;
            masked = angle & 4095;
        }
        cosine = rcos(masked);
        sine = rsin(masked);
    }
    rotation.matrix.m[0][2] = sine;
    rotation.matrix.m[2][0] = -sine;
    rotation.matrix.m[1][1] = 4096;
    axis.z = 4096;
    rotation.matrix.m[0][0] = cosine;
    rotation.matrix.m[2][2] = cosine;
    rotation.matrix.t[0] = rotation.matrix.t[1] = rotation.matrix.t[2] = 0;
    rotation.matrix.m[0][1] = rotation.matrix.m[1][0] = rotation.matrix.m[1][2] = rotation.matrix.m[2][1] = 0;
    axis.x = 0;
    axis.y = 0;
    gte_ldrotmatrix(&rotation);
    gte_ldtransmatrix(&rotation);
    gte_ldv0(&axis);
    gte_rt();
    gte_stlvnl(&forward);
    up.y = 4096;
    up.x = 0;
    up.z = 0;
    gte_ldopv1_psyq(&up);
    gte_ldopv2(&forward);
    gte_op12_psyq();
    gte_stlvnl(&right);
    gte_ldopv1_psyq(&forward);
    gte_ldopv2(&right);
    gte_op12_psyq();
    gte_stlvnl(&newUp);
    g_CameraBasisMatrix.t[0] = 0;
    g_CameraBasisMatrix.t[1] = 0;
    g_CameraBasisMatrix.t[2] = 0;
    g_CameraBasisMatrix.m[0][0] = right.x;
    g_CameraBasisMatrix.m[0][1] = newUp.x;
    g_CameraBasisMatrix.m[0][2] = forward.x;
    g_CameraBasisMatrix.m[1][0] = right.y;
    g_CameraBasisMatrix.m[1][1] = newUp.y;
    g_CameraBasisMatrix.m[1][2] = forward.y;
    g_CameraBasisMatrix.m[2][0] = right.z;
    g_CameraBasisMatrix.m[2][1] = newUp.z;
    g_CameraBasisMatrix.m[2][2] = forward.z;
    return 0;
}
