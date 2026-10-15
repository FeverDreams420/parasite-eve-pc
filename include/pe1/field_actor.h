#ifndef PE1_FIELD_ACTOR_H
#define PE1_FIELD_ACTOR_H

#include "common.h"
#include "pe1/fixed_point.h"
#include "pe1/render_object.h"
#include "pe1/script_node.h"

typedef struct FieldActorState FieldActorState;

/* Runtime actor: pointers use host width; old offsets below are documentation. */
typedef struct FieldActor {
    /* 0x000 */ FieldActorState *state;  /* script/battle state written by Entity_WriteFieldByCmd */
    /* 0x004 */ struct FieldActor *next;   /* entity list link (g_FieldActorListHead walks this) */
    /* 0x008 */ struct FieldActor *prev;   /* previous entity list link */
    /* 0x00C */ unsigned char type_id;     /* matched against arg id in attachment lookup */
    /* 0x00D */ unsigned char sub_id;
    /* 0x00E */ unsigned char mode;
    /* 0x00F */ unsigned char action;
    /* 0x010 */ unsigned short pad_010; /* 0xC8 when the actor is created */
    /* 0x012 */ unsigned short anim_frame_target; /* clamped target anim frame (Script_SetEntityAnimFrame) */
    /* 0x014 */ union {
        int fixed;                                /* current animation position, 16.16 */
        struct { unsigned short fraction; unsigned short integer; } parts;
    } anim;
    /* 0x018 */ int anim_prev;                    /* previous animation position, 16.16 */
    /* 0x01C */ int anim_step;                    /* signed animation increment, 16.16 */
    /* 0x020 */ int move_factor;          /* fixed movement magnitude, scaled for the player */
    /* 0x024 */ unsigned short field_sfx_id; /* entity value forwarded to Script_QueueFieldSfx */
    /* 0x026 */ unsigned short move_speed;  /* movement-speed multiplier, written by Script_SetEntityMoveSpeed */
    /* Signed 16.16 coordinates; Pe1Fixed_Integer reads their high halfwords. */
    /* 0x028 */ int pos_x;
    /* 0x02C */ int pos_y;
    /* 0x030 */ int pos_z;
    /* 0x034 */ unsigned char pad_034[0x04];
    /* 0x038 */ short rot_x;
    /* 0x03A */ short rot_y;   /* facing angle (yaw); signed 12-bit, 0x1000 = 360deg */
    /* 0x03C */ short rot_z;
    /* 0x03E */ unsigned char pad_03E[0x02];
    /* 0x040 */ int base_x;             /* pre-move position (rollback target) */
    /* 0x044 */ int base_y;
    /* 0x048 */ int base_z;
    /* 0x04C */ int field_4c;           /* actor status bits (player's 0xC0 mask gates a scene flag check) */
    /* 0x050 */ short saved_rot_x;
    /* 0x052 */ short saved_rot_y;
    /* 0x054 */ short saved_rot_z;
    /* 0x056 */ unsigned char pad_056[0x02];
    /* 0x058 */ int delta_x;            /* extra per-frame displacement added after motion */
    /* 0x05C */ int delta_y;
    /* 0x060 */ int delta_z;
    /* 0x064 */ unsigned char pad_064[0x04];
    /* 0x068 */ int motion_x;
    /* 0x06C */ int motion_y;
    /* 0x070 */ int motion_z;
    /* 0x074 */ unsigned char pad_074[0x04];
    /* 0x078 */ int accel_x;   /* constant per-frame accel added to motion (Entity_IntegratePositionFull) */
    /* 0x07C */ int accel_y;
    /* 0x080 */ int accel_z;
    /* 0x084 */ unsigned char pad_084[0x04];
    /* 0x088 */ int gravity_x; /* gravity accel added to motion when flags & 2 */
    /* 0x08C */ int gravity_y;
    /* 0x090 */ int gravity_z;
    /* 0x094 */ unsigned char pad_094[0x04];
    /* 0x098 */ unsigned int flags;
    /* 0x09C */ unsigned char *script_base; /* base of the entity's 16-bit scene script */
    /* 0x0A0 */ ScriptNode *script_node_lists[3]; /* 3 script-node list heads (Entity_MarkNodeFree) */
    /* 0x0AC */ unsigned int pad_0AC[0x38]; /* cleared when the actor is created */
    /* 0x18C */ struct FieldActor *parent; /* parent actor; child copies its pos/rot when flags & 0x400000 */
    /* 0x190 */ void (*frame_callback)(struct FieldActor *actor); /* run first every frame */
    /* 0x194 */ void (*contact_callback)(struct FieldActor *self, int self_part,
                                         struct FieldActor *other, int other_part);
                /* run for each overlapping hit-sphere pair (Scene_UpdateEntityPositions) */
    /* 0x198 */ unsigned int pad_198;
    /* 0x19C */ unsigned int *script_cursor_19c; /* entry for queued field-sfx tasks */
    /* 0x1A0 */ unsigned int *script_cursor_1a0; /* entry for actor-contact tasks */
    /* 0x1A4 */ void *collision_face;        /* floor face under the actor; rolled back with pos */
    /* 0x1A8 */ void *collision_face_mirror; /* face recorded before the move */
    /* A model-header address; the integer view is also tested before using
     * or releasing the actor's render allocation. */
    /* 0x1AC */ union {
        uintptr_t allocation_active;
        RenderObjectHeader *model_header;
    } model_state;
    /* 0x1B0 */ void *action_data;      /* current animation/action record */
    /* 0x1B4 */ RenderObjectEntity render_object;
    /* 0x270 */ union {
        unsigned char pad_270[0x08];
        struct { u16 x0, z0, x1, z1; } mirror_line;
    };
    /* 0x278 */ union {
        uintptr_t allocation_block;
        struct RenderShadowQuad *shadow_quads;
    };
    u8 shadow_matrix_index;
    u8 shadow_intensity;
} FieldActor;













/* Per-type frame and contact callbacks copied into a new actor
 * (Scene_LoadMap), indexed by the scene entry's type byte. */
typedef struct FieldActorCallbacks {
    void (*frame)(struct FieldActor *actor);
    void (*contact)(struct FieldActor *self, int self_part,
                    struct FieldActor *other, int other_part);
} FieldActorCallbacks;

extern FieldActorCallbacks g_FieldActorTypeCallbacks[];

/* Packed status values addressed by script selectors 0x64..0x6E. */
typedef union FieldActorPackedStatus {
    u32 raw;
    struct {
        u32 field64 : 2;
        u32 field65 : 2;
        u32 field66 : 2;
        u32 field67 : 2;
        u32 field68 : 2;
        u32 field69 : 2;
        u32 field6a : 2;
        u32 field6b : 2;
        u32 field6c : 2;
        u32 field6d : 1;
        u32 field6e : 5;
        u32 reserved : 8;
    } bits;
} FieldActorPackedStatus;

struct FieldActorState {
    /* 0x00 */ unsigned int core_flags;
    /* 0x04 */ union {
        short value;
        struct { signed char low; signed char high; } bytes;
    } id04;
    /* 0x06 */ union {
        short value;
        struct { signed char low; signed char high; } bytes;
    } id06;
    /* 0x08 */ int progress;
    /* 0x0C */ short amount;
    /* 0x0E */ unsigned short amount_mirror;
    /* 0x10 */ union {
        int command_value;
        struct {
            short actor_threshold;
            unsigned char saved_actor_mode;
            unsigned char reserved13;
        } parts;
    } control10;
    /* 0x14 */ int action_value14;
    /* 0x18 */ u8 *action_state;
    /* 0x1C */ int count_limit;
    /* 0x20 */ int divisor_basis;
    /* 0x24 */ short actor_threshold_delta;
    /* 0x26 */ unsigned char pad_26[0x02];
    /* 0x28 */ int progress_target;
    /* 0x2C */ int progress_step;
    /* 0x30 */ int countdown;
    /* 0x34 */ int terminal_delay;
    /* 0x38 */ unsigned short sub_action_step;
    /* 0x3A */ unsigned char sub_action_counter;
    /* 0x3B */ unsigned char sub_action_period;
    /* 0x3C */ unsigned char transition_timers[0x0C];
    /* 0x48 */ signed char recoil_steps;
    /* 0x49 */ unsigned char recoil_magnitude;
    /* 0x4A */ short recoil_angle;
    /* 0x4C */ unsigned int flags;
    /* 0x50 */ unsigned char number_desc_0[0x08];
    /* 0x58 */ unsigned char number_desc_1[0x08];
    /* 0x60 */ unsigned char coord_desc[0x08];
    /* Scalar slots overlap enemy animation-event records. Native pointers
     * are stored after the shared scalar state instead of widening these. */
    /* 0x68 */ u32 reserved_action_slot;
    /* 0x6C */ u32 reserved_substate_slot;
    /* 0x70 */ u32 reserved_aux_slot;
    /* 0x74 */ unsigned char reserved74[0x14];
    /* 0x88 */ int script_value88;
    /* 0x8C */ short script_value8c;
    /* 0x8E */ short script_value8e;
    /* 0x90 */ unsigned char effect_chance;
    /* 0x91 */ unsigned char effect_param91;
    /* 0x92 */ unsigned char effect_param92;
    /* 0x93 */ unsigned char effect_param93;
    /* 0x94 */ unsigned char effect_level;
    /* 0x95 */ unsigned char effect_duration;
    /* 0x96 */ short script_value96;
    /* 0x98 */ short script_value98;
    /* 0x9A */ short script_value9a;
    /* 0x9C */ unsigned char reserved9c[2];
    /* 0x9E */ signed char script_value9e;
    /* 0x9F */ signed char script_value9f;
    /* 0xA0 */ short loot_item_id;
    /* 0xA2 */ short loot_item_aux;
    /* 0xA4 */ signed char behavior_mode;
    /* 0xA5 */ unsigned char reserveda5;
    /* 0xA6 */ short behavior_timer;
    /* 0xA8 */ unsigned char reserveda8[6];
    /* 0xAE */ signed char script_valueae;
    /* 0xAF */ signed char script_valueaf;
    /* 0xB0 */ short script_values_b0[6];
    /* 0xBC */ signed char script_valuebc;
    /* 0xBD */ unsigned char reservedbd[0x0F];
    /* 0xCC */ FieldActorPackedStatus status_flags2;
    /* 0xD0 */ short panel_c_value;
    /* 0xD2 */ unsigned short panel_c_x;
    /* 0xD4 */ unsigned short panel_c_y;
    /* 0xD6 */ unsigned char panel_c_timer;
    /* 0xD7 */ unsigned char panel_c_mode;
    void *action;
    void *substate;
    void *aux_state;
};





PE1_STATIC_ASSERT(sizeof(FieldActorPackedStatus) == 4, field_actor_packed_status_size);



/* Scene movement target record; shared by the E02/E03/E06...E27 overlays. */
typedef struct FieldMoveTarget {
    unsigned char pad00[0x60];
    int target_pos[3];           /* 0x60: x/y/z passed to FieldFx_VecToAngle */
    unsigned char pad6C[0x18];
    FieldActor *target_actor;    /* 0x84: copies x/z while target tracking is enabled */
    unsigned char pad88[0x10];
    int completion_value;        /* 0x98: compared against 0x07FFFFFF */
    unsigned char pad9C[0x06];
    short turn_rate;             /* 0xA2 */
    short settle_rate;           /* 0xA4 */
    unsigned char padA6[0x03];
    unsigned char enabled;       /* 0xA9 */
} FieldMoveTarget;
#define FIELD_ACTOR_POS_X 0x28
#define FIELD_ACTOR_POS_Y 0x2C
#define FIELD_ACTOR_POS_Z 0x30
#define FIELD_ACTOR_BASE_X 0x40
#define FIELD_ACTOR_BASE_Y 0x44
#define FIELD_ACTOR_BASE_Z 0x48
#define FIELD_ACTOR_MODE 0x0E
#define FIELD_ACTOR_ACTION 0x0F
#define FIELD_ACTOR_ACTION_DATA 0x14
#define FIELD_ACTOR_MOTION_X 0x68
#define FIELD_ACTOR_MOTION_Y 0x6C
#define FIELD_ACTOR_MOTION_Z 0x70
#define FIELD_ACTOR_FLAGS 0x98
#define FIELD_ACTOR_DESCRIPTOR_A 0x210
#define FIELD_ACTOR_DESCRIPTOR_B 0x212

#define FIELD_ACTOR_STATE_PROGRESS 0x08
#define FIELD_ACTOR_STATE_AMOUNT 0x0C
#define FIELD_ACTOR_STATE_AMOUNT_MIRROR 0x0E
#define FIELD_ACTOR_STATE_ACTOR_THRESHOLD 0x10
#define FIELD_ACTOR_STATE_SAVED_ACTOR_MODE 0x12
#define FIELD_ACTOR_STATE_COUNT_LIMIT 0x1C
#define FIELD_ACTOR_STATE_DIVISOR_BASIS 0x20
#define FIELD_ACTOR_STATE_ACTOR_THRESHOLD_DELTA 0x24
#define FIELD_ACTOR_STATE_PROGRESS_TARGET 0x28
#define FIELD_ACTOR_STATE_PROGRESS_STEP 0x2C
#define FIELD_ACTOR_STATE_COUNTDOWN 0x30
#define FIELD_ACTOR_STATE_TERMINAL_DELAY 0x34
#define FIELD_ACTOR_STATE_FLAGS 0x4C
#define FIELD_ACTOR_STATE_SUBSTATE 0x6C

void Entity_WriteFieldByCmd(FieldActor *actor, int command, int value);

/* The actor whose script-node lists are being run. */
extern FieldActor *g_CurrentEntity;
/* Head of the active actor list (entity/pool.c). */
extern FieldActor *g_FieldActorListHead;

/* Spawns the actors of a scene record after an existing actor. */
FieldActor *Scene_LoadMap(u8 *scene, FieldActor *after, int allocateFull);

#endif
