#ifndef ROOM_M273_H
#define ROOM_M273_H

#include "pe1/field_actor_script_state.h"
#include "pe1/field_texture_tables.h"
#include "pe1/fieldfx_callback_list.h"
#include "pe1/gte_sine_table.h"
#include "pe1/render_object.h"
#include "pe1/room_floor.h"

#include "room_m273_effects.h"

typedef GteShortVector RoomM273Vector;

typedef struct {
    signed int low : 16;
    signed int high : 16;
} RoomM273TrigEntry;

typedef struct RoomM273PaletteWord {
    u8 bytes[4];
} RoomM273PaletteWord;

typedef struct RoomM273PaletteEffect {
    RoomM273PaletteWord color;
    GteVector *source;
    s16 size;
    s16 depth;
    s16 x;
    s16 y;
} RoomM273PaletteEffect;

typedef struct RoomM273PaletteInput {
    GteVector *source;
    s16 size;
} RoomM273PaletteInput;

typedef struct RoomM273PulseInput {
    GteShortVector position;
    u16 velocity;
    u16 unknown_0A;
} RoomM273PulseInput;

typedef union RoomM273PulseRecord {
    RoomM273PulseInput callback;
    RoomM273PulseEmitterView emitter;
} RoomM273PulseRecord;

typedef union RoomM273DelayedBurstRecord {
    RoomM273TemplatePoint emitter;
    RoomM273RisingParticle callback;
} RoomM273DelayedBurstRecord;

typedef struct RoomM273SampledLayerEffect {
    GteShortVector position;
    u8 parameter[4];
} RoomM273SampledLayerEffect;

PE1_STATIC_ASSERT(sizeof(RoomM273PaletteWord) == 4, room_m273_palette_word_size);
PE1_STATIC_ASSERT(sizeof(RoomM273PaletteEffect) == 16, room_m273_palette_effect_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM273PaletteInput, size) == 4,
                  room_m273_palette_input_size_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM273PulseInput, velocity) == 8,
                  room_m273_pulse_velocity_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM273PulseInput, unknown_0A) == 0x0A,
                  room_m273_pulse_unknown_offset);
PE1_STATIC_ASSERT(sizeof(RoomM273PulseInput) == 0x0C,
                  room_m273_pulse_input_size);
PE1_STATIC_ASSERT(sizeof(RoomM273PulseRecord) == 0x0C,
                  room_m273_pulse_record_size);
PE1_STATIC_ASSERT(sizeof(RoomM273TemplatePoint) == 8,
                  room_m273_template_point_size);
PE1_STATIC_ASSERT(sizeof(RoomM273DelayedBurstRecord) == 8,
                  room_m273_delayed_burst_record_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM273TemplatePoint, flags) ==
                      PE1_OFFSETOF(RoomM273RisingParticle, speed),
                  room_m273_delayed_burst_tail_offset);

extern s16 g_RoomM273GrabColorIndex;
extern int g_RoomM273GrabColors[];
extern s16 g_RoomM273HandTrailLandingPulseScale[];
extern u8 g_RoomM273SwayHitFlags[];
extern void *g_RoomM273GrabSparkPool;
extern RoomM273PaletteWord g_RoomM273StarBurstColors[];
extern unsigned char g_RoomM273BlackColor[];
extern char g_RoomM273LandingFanColor[];
extern char g_RoomM273LandingFanRotations[];
int Inv_ScrambleGrid(void);
int RoomM273_SwayBurstCallback(int mode, RoomM273RisingParticle *particle);

typedef struct RoomPlacementMap {
    u8 pad_000[0x594];
    s32 x;
    s32 y;
    s32 z;
} RoomPlacementMap;

typedef struct RoomPlacementOwner {
    u8 pad_000[0x238];
    RoomPlacementMap *map;
} RoomPlacementOwner;

typedef struct RoomPlacementStateContext {
    u8 reserved[8];
    RoomPlacementOwner *owner;
} RoomPlacementStateContext;

typedef struct RoomPlacementState {
    u8 pad_00[4];
    int *signal;
    s16 phase_countdown;
    u8 pad_0A[0x26];
    s32 vertical_step;
    s32 fallback_x;
    s32 fallback_z;
    s16 vertical_ticks;
    u8 placement_checked;
    u8 active;
    u8 pending_action;
} RoomPlacementState;

typedef struct RoomSelectionState RoomSelectionState;
struct RoomSelectionState {
    u8 status;                  /* 4 = closed */
    u8 pad_01[2];
    u8 flag3;                   /* RoomEnt flag3, 0xFF while armed */
    u8 pad_04[4];
    struct FieldActor *actor;
    void (*callback)(RoomSelectionState *);
    int *signal;
    u8 pad_14[6];
    u8 started;                 /* 0x1A */
    u8 pad_1B;
    RenderMatrix matrix;        /* 0x1C: grab placement, translation at 0x30 */
    u8 pad_3c[4];
    int savedX;
    int savedZ;
    s16 verticalTicks;          /* 0x48: RoomPlacementState.vertical_ticks */
    u8 placementChecked;        /* 0x4A */
    u8 activated;
};

PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomSelectionState, callback) == 0x0C,
                  room_selection_callback_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomSelectionState, signal) == 0x10,
                  room_selection_signal_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomSelectionState, actor) == 0x08,
                  room_selection_actor_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomSelectionState, matrix) == 0x1c,
                  room_selection_matrix_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomSelectionState, activated) == 0x4b,
                  room_selection_activated_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomSelectionState, callback) +
                      PE1_OFFSETOF(RoomPlacementState, active) ==
                      PE1_OFFSETOF(RoomSelectionState, activated),
                  room_selection_placement_state_overlap);

/* View of the partner actor's script state: the byte record at 0x18 is the
 * substate the grab sequence flips to 2 and 4. */
typedef FieldActorScriptState RoomM273ActorState;

s32 RoomM273_GrabRelease(RoomSelectionState *selection);

PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomPlacementOwner, map) == 0x238,
                  room_placement_map_offset);
PE1_STATIC_ASSERT(sizeof(RoomM273SampledLayerEffect) == 12,
                  room_m273_sampled_layer_effect_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM273SampledLayerEffect, parameter) == 8,
                  room_m273_sampled_layer_effect_parameter_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomPlacementStateContext, owner) == 8,
                  room_placement_state_context_owner_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomPlacementMap, x) == 0x594,
                  room_placement_x_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomPlacementState, active) == 0x3F,
                  room_placement_active_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomPlacementState, signal) == 4,
                  room_placement_signal_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomPlacementState, phase_countdown) == 8,
                  room_placement_phase_countdown_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomPlacementState, vertical_step) == 0x30,
                  room_placement_vertical_step_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomPlacementState, vertical_ticks) == 0x3C,
                  room_placement_vertical_ticks_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomPlacementState, placement_checked) == 0x3E,
                  room_placement_checked_offset);

void RoomM273_ReturnPlayerToPlacement(RoomPlacementOwner *owner, RoomPlacementState *state);

/* The oscillation module object (class methods RoomM273_OscillationInit, the
 * command handler RoomM273_OscillationConfigure, RoomM273_OscillationUpdate, RoomM273_OscillationRelease): bobs the
 * actor up and down and reports the player angle and distance through the
 * two output words the script hands it (command 4). Its first two words
 * double as the choice inputs of RoomM273_ChooseWeightedAction (command 3). */
typedef struct RoomOscillationParams {
    int *angleOut;          /* 0x00 */
    int *distanceOut;       /* 0x04 */
    int *signal;            /* 0x08: completion word (command 5) */
    int frame;              /* 0x0C */
    int baseHeight;         /* 0x10 */
    s16 active;             /* 0x14 */
    s16 amplitude;          /* 0x16 */
    s16 period;             /* 0x18 */
} RoomOscillationParams;

typedef struct RoomOscillationEffect {
    u8 status;              /* 4 = closed */
    u8 pad_01[7];
    struct FieldActor *actor;
    RoomOscillationParams params;   /* 0x0C */
} RoomOscillationEffect;

PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomOscillationEffect, actor) == 0x08,
                  room_oscillation_actor_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomOscillationEffect, params.frame) == 0x18,
                  room_oscillation_frame_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomOscillationEffect, params.period) == 0x24,
                  room_oscillation_period_offset);

int RoomM273_OscillationUpdate(RoomOscillationEffect *effect);

#endif
