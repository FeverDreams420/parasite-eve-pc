#ifndef PE1_RENDER_ANIMATION_FRAME_H
#define PE1_RENDER_ANIMATION_FRAME_H

#include "common.h"
#include "pe1/asset_find.h"
#include "pe1/render_anim_position.h"
#include "pe1/render_buffers.h"

/* Preserve the original packet capacity after widening GPU headers. */
enum { RENDER_ANIM_BUFFER_BYTES = 0x3800 * (sizeof(RenderNativeTag) / sizeof(u32)) };

extern int g_BattleAttackAnimFrame;
extern struct Combatant *g_ActiveActor;
extern struct FieldActor *g_PlayerEntity;
/* Byte offset of the next free packet in the current draw buffer. */
extern int g_DrawPacketOffset;

/* Scratchpad buffers for the wireframe pass: projected screen XY per
 * vertex and its SZ depth. */
typedef struct RenderWireScratch {
    u32 *screenXY;
    s16 *depth;
} RenderWireScratch;
extern RenderWireScratch g_WireScratch;

int Render_SetGteScreenOffset(void);
struct RenderAnimFrame;
/* capacity bounds writes to the caller-owned packet buffer. */
int Render_DrawWireframeModel(struct RenderAnimFrame *frame, void *position, int scale,
                              int yaw, void *ot, u8 *buffer, size_t capacity);
int Render_ResetGteScreenOffset(void);
void Render_AnimationFrame(void);

#endif
