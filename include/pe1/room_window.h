#ifndef PE1_ROOM_WINDOW_H
#define PE1_ROOM_WINDOW_H

#include "common.h"
#include "pe1/field_actor.h"
#include "pe1/field_vector_math.h"
#include "pe1/gte_sine_table.h" /* packed PSY-Q sine table, one RoomWindowTrig per angle step */
#include "pe1/gte_types.h"
#include "pe1/pm_types.h"

/* The window effect: a script object that flies a camera-facing frame from
 * its anchor actor towards a target actor, spinning it around the flight
 * axis, until it reaches the target or arrives near the player. */

/* 16.16 fixed-point coordinate with its integer half addressable. */
typedef union RoomWindowFixed {
    s32 value;
    struct {
        u16 fraction;
        s16 integer;
    } part;
} RoomWindowFixed;

/* One packed entry of the PSY-Q sine table: sine in the low half, cosine
 * in the high half. */
typedef union RoomWindowTrig {
    s32 word;
    struct {
        s16 sin;
        s16 cos;
    } part;
} RoomWindowTrig;

/* The anchor's render marker: flag word and the state byte the release
 * closes. */
typedef struct RoomWindowFlags {
    s32 flags;                    /* 0x00 */
    u8 pad04[PE1_OFFSETOF(FieldActorState, action_state) - sizeof(u32)];
    u8 *state;                    /* 0x18 */
} RoomWindowFlags;

/* The anchor record reached through RoomWindowActor.anchor. */
typedef struct RoomWindowAnchor {
    RoomWindowFlags *marker;      /* 0x00 */
    u8 pad04[PE1_OFFSETOF(FieldActor, action) - sizeof(void *)];
    u8 frameCount;                /* 0x0F */
    u8 pad10[0x4];
    s32 speed;                    /* 0x14 */
    u8 pad18[0x2];
    u16 frame;                    /* 0x1A */
    u8 pad1C[0xC];
    s32 pos[3];                   /* 0x28 */
    u8 pad34[0x6];
    u16 yaw;                      /* 0x3A */
} RoomWindowAnchor;

/* The field actor the script object is attached to. */
typedef struct RoomWindowActor {
    u8 pad00[PE1_OFFSETOF(FieldActor, pos_x)];
    s32 pos[3];                   /* 0x28 */
    u8 pad34[0x64];
    s32 flags;                    /* 0x98 */
    u8 pad9C[PE1_OFFSETOF(FieldActor, parent) - PE1_OFFSETOF(FieldActor, flags) - sizeof(u32)];
    RoomWindowAnchor *anchor;     /* 0x18C */
    u8 pad190[PE1_OFFSETOF(FieldActor, render_object) + PE1_OFFSETOF(RenderObjectEntity, model_matrix) - PE1_OFFSETOF(FieldActor, parent) - sizeof(void *)];
    s16 matrix[9];                /* 0x1E8 */
    u8 pad1FA[0x2];
    s32 center[3];                /* 0x1FC */
    u8 pad208[PE1_OFFSETOF(RenderObjectEntity, flags_9C) - PE1_OFFSETOF(RenderObjectEntity, model_matrix) - sizeof(RenderMatrix)];
    u16 drawFlags;                /* 0x250 */
} RoomWindowActor;

typedef struct RoomWindowTarget {
    u8 pad00[PE1_OFFSETOF(FieldActor, pos_x)];
    s32 pos[3];                   /* 0x28 */
} RoomWindowTarget;

/* Handler state at RoomWindowObject + 0xC (see
 * RoomLib_ConfigureWindowClass for the configuration ops). */
struct RoomWindowObject;
typedef struct RoomWindowState {
    void (*callback)(struct RoomWindowObject *);       /* 0x00 */
    int *signal;                  /* 0x04 */
    s16 active;                   /* 0x08 */
    s8 variant;                   /* 0x0A */
    s8 optionB;                   /* 0x0B */
#if INTPTR_MAX > INT32_MAX
    u8 argumentHigh[sizeof(intptr_t) - sizeof(s32)];
#endif
    s8 optionC;                   /* 0x0C */
    u8 flags;                     /* 0x0D */
    u8 started;                   /* 0x0E (object + 0x1A) */
    u8 pad0F;
    GteMatrix rotation;           /* 0x10 */
    RoomWindowFixed pos[3];       /* 0x30 */
    s32 pad3C;
    s32 target[3];                /* 0x40 */
    s32 pad4C;
    RoomWindowFixed origin[3];    /* 0x50 */
    s32 pad5C;
    GteShortVector offset;        /* 0x60 */
    GteShortVector angles;        /* 0x68 */
    RoomWindowTarget *targetNode; /* 0x70 */
    s32 travel;                   /* 0x74 */
    s32 step;                     /* 0x78 */
    s32 scale;                    /* 0x7C */
    s32 spinSpeed;                /* 0x80 */
    s32 spin;                     /* 0x84 */
    s16 touched;                  /* 0x88 */
} RoomWindowState;

typedef struct RoomWindowObject {
    u8 status;                    /* 0x00: 4 = closed */
    u8 pad01[0x2];
    u8 enabled;                   /* 0x03 */
    s32 index;                    /* 0x04 */
    RoomWindowActor *actor;       /* 0x08 */
    RoomWindowState state;        /* 0x0C */
} RoomWindowObject;

/* Scratchpad layout used while placing the window. */
typedef struct RoomWindowScratch {
    volatile GteMatrix yaw;       /* 0x00 */
    volatile GteMatrix roll;      /* 0x20 */
    GteVector out;                /* 0x40 */
    GteVector corner;             /* 0x50 */
    GteVector advance;            /* 0x60 */
    union {
        volatile GteShortVector local; /* 0x70: vector fed to the GTE */
        GteShortVector angles;         /* 0x70: RotMatrixYXZ input */
    } in;
    GteShortVector pad78;         /* 0x78 */
    GteShortVector spot;          /* 0x80 */
    GteShortVector edge;          /* 0x88 */
    s16 arrived;                  /* 0x90 */
} RoomWindowScratch;

PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomWindowFlags, state) == PE1_OFFSETOF(FieldActorState, action_state), window_marker_native_state);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomWindowAnchor, frameCount) == PE1_OFFSETOF(FieldActor, action), window_anchor_native_count);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomWindowAnchor, speed) == PE1_OFFSETOF(FieldActor, anim), window_anchor_native_speed);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomWindowAnchor, frame) == PE1_OFFSETOF(FieldActor, anim_prev) + sizeof(u16), window_anchor_native_frame);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomWindowAnchor, pos) == PE1_OFFSETOF(FieldActor, pos_x), window_anchor_native_pos);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomWindowAnchor, yaw) == PE1_OFFSETOF(FieldActor, rot_y), window_anchor_native_yaw);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomWindowActor, flags) == PE1_OFFSETOF(FieldActor, flags), window_actor_native_flags);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomWindowActor, anchor) == PE1_OFFSETOF(FieldActor, parent), window_actor_native_anchor);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomWindowActor, matrix) == PE1_OFFSETOF(FieldActor, render_object) + PE1_OFFSETOF(RenderObjectEntity, model_matrix), window_actor_native_matrix);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomWindowActor, center) == PE1_OFFSETOF(FieldActor, render_object) + PE1_OFFSETOF(RenderObjectEntity, model_matrix) + PE1_OFFSETOF(RenderMatrix, translation), window_actor_native_center);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomWindowActor, drawFlags) == PE1_OFFSETOF(FieldActor, render_object) + PE1_OFFSETOF(RenderObjectEntity, flags_9C), window_actor_native_draw_flags);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomWindowObject, actor) == PE1_OFFSETOF(PmSlotHeader, owner), window_pm_native_owner);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomWindowObject, state) + PE1_OFFSETOF(RoomWindowState, flags) == PE1_OFFSETOF(FieldFxTaskOwner, tasks) + PE1_OFFSETOF(FieldFxTaskContext, flags), window_pm_native_busy_flags);
PE1_STATIC_ASSERT(sizeof(RoomWindowObject) <= sizeof(PmSecondarySlot), window_pm_native_capacity);

extern struct FieldActor *g_PlayerEntity;

extern int FieldFx_IntSqrt(int value);

struct RoomEnt;

/* The window actor class (src/overlays/room_lib/window_class.c). */
void RoomLib_WindowClassNop0(void);
void RoomLib_InitWindowClass(PmSlotHeader *slot);
int RoomLib_ConfigureWindowClass(PmSlotHeader *slot, int query, int op,
                                 intptr_t a, intptr_t b, intptr_t c);
int RoomLib_WindowClassNop3(PmSlotHeader *slot);
int RoomLib_UpdateWindowClass(PmSlotHeader *slot);
void RoomLib_WindowClassNop6(void);
void RoomLib_WindowHandler(RoomWindowObject *obj);
void RoomLib_WindowArrived(RoomWindowObject *ctx);
void RoomLib_ArmWindowClass(RoomWindowObject *obj);
int RoomLib_ReleaseWindowClass(PmSlotHeader *slot);

/* Its row of a room's class table (g_RoomClassTable, pe1/room_module.h). */
#define ROOMLIB_WINDOW_CLASS \
    { RoomLib_WindowClassNop0, RoomLib_InitWindowClass, RoomLib_ConfigureWindowClass, \
      RoomLib_WindowClassNop3, RoomLib_UpdateWindowClass, RoomLib_ReleaseWindowClass, \
      RoomLib_WindowClassNop6 }

#endif
