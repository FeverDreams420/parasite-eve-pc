#ifndef PE1_RENDER_ANIMATION_FRAME_H
#define PE1_RENDER_ANIMATION_FRAME_H

#include "common.h"
#include "pe1/asset_find.h"
#include "pe1/render_anim_position.h"
#include "pe1/render_buffers.h"

typedef struct RenderAnimFrameData {
    u8 reserved00[2];
    s16 frameCount;
} RenderAnimFrameData;

typedef struct RenderAnimActor {
    u8 reserved00[0x4C];
    int modeFlags;
    u8 reserved50[0x18];
    RenderAnimFrameData *frameData;
} RenderAnimActor;

typedef struct RenderAnimPlayer {
    u8 reserved00[0x28];
    u8 decoderData[2];
    s16 x;
    u8 reserved2C[2];
    s16 y;
    u8 reserved30[2];
    s16 z;
} RenderAnimPlayer;

PE1_STATIC_ASSERT(PE1_OFFSETOF(RenderAnimActor, frameData) == 0x68,
                  render_anim_actor_frame_data_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RenderAnimFrameData, frameCount) == 2,
                  render_anim_frame_count_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RenderAnimPlayer, z) == 0x32,
                  render_anim_player_z_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RenderAnimPlayer, x) - 0x28 ==
                      PE1_OFFSETOF(RenderAnimPositionInput, x),
                  render_anim_player_position_x_view);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RenderAnimPlayer, y) - 0x28 ==
                      PE1_OFFSETOF(RenderAnimPositionInput, y),
                  render_anim_player_position_y_view);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RenderAnimPlayer, z) - 0x28 ==
                      PE1_OFFSETOF(RenderAnimPositionInput, z),
                  render_anim_player_position_z_view);

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
int Render_DrawWireframeModel(struct RenderAnimFrame *frame, void *position, int scale,
                              int yaw, void *ot, u8 *buffer);
int Render_ResetGteScreenOffset(void);
void Render_AnimationFrame(void);

#endif
