#ifndef PE1_PAD_STATE_H
#define PE1_PAD_STATE_H

#include "common.h"

/* Game button bits sampled once per frame by Field_HandleStateTransition
 * (boot/init.c). */
extern u32 g_FieldPadBits; /* held (g_FieldPadBits) */
extern u32 g_FieldPadLastFrame; /* held on the previous frame */
extern u32 g_FieldPadPressed; /* pressed this frame */
extern u32 g_FieldPadReleased; /* released this frame */

/* Raw controller packet that LIBPAD fills for one port (PadInitDirect):
 * the receive status byte and the type/length byte (type in the high
 * nibble, 4 = digital pad, 7 = analog pad), the active-low button bits,
 * then the analog sticks, centred at 0x80. */
typedef struct PadPacket {
    u16 header;
    u16 buttons;
    u8 rightX, rightY;
    u8 leftX, leftY;
    u8 reserved[0x1A];
} PadPacket;

PE1_STATIC_ASSERT(sizeof(PadPacket) == 0x22, pad_packet_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(PadPacket, leftX) == 6, pad_packet_left_x_offset);

/* The packets of controller ports 1 and 2. */
extern PadPacket g_PadPackets[2];
union SDL_Event;
/* Install game defaults once; subsequent pad resets preserve user bindings. */
void Boot_InitHostBindings(void);
void Boot_ResetPadInput(void);
void Boot_HandlePadEvent(const union SDL_Event *event);
void Boot_BuildRenderFlagTable(void);
void Field_HandleStateTransition(void);

#endif /* PE1_PAD_STATE_H */
