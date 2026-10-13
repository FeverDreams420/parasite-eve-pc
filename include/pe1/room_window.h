#ifndef PE1_ROOM_WINDOW_H
#define PE1_ROOM_WINDOW_H

#include "common.h"
#include "pe1/field_actor.h"
#include "pe1/field_vector_math.h"
#include "pe1/gte_sine_table.h" /* packed PSY-Q sine table, one RoomWindowTrig per angle step */
#include "pe1/gte_types.h"

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
    u8 pad04[0x14];
    u8 *state;                    /* 0x18 */
} RoomWindowFlags;

/* The anchor record reached through RoomWindowActor.anchor. */
typedef struct RoomWindowAnchor {
    RoomWindowFlags *marker;      /* 0x00 */
    u8 pad04[0xB];
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
    u8 pad00[0x28];
    s32 pos[3];                   /* 0x28 */
    u8 pad34[0x64];
    s32 flags;                    /* 0x98 */
    u8 pad9C[0xF0];
    RoomWindowAnchor *anchor;     /* 0x18C */
    u8 pad190[0x58];
    s16 matrix[9];                /* 0x1E8 */
    u8 pad1FA[0x2];
    s32 center[3];                /* 0x1FC */
    u8 pad208[0x48];
    u16 drawFlags;                /* 0x250 */
} RoomWindowActor;

typedef struct RoomWindowTarget {
    u8 pad00[0x28];
    s32 pos[3];                   /* 0x28 */
} RoomWindowTarget;

typedef struct RoomWindowPlayerCore {
    u8 pad00[0x4C];
    s32 flags;                    /* 0x4C */
} RoomWindowPlayerCore;

typedef struct RoomWindowPlayer {
    RoomWindowPlayerCore *core;   /* 0x00 */
    u8 pad04[0x24];
    s32 pos[3];                   /* 0x28 */
} RoomWindowPlayer;

/* Handler state at RoomWindowObject + 0xC (see
 * RoomLib_ConfigureWindowClass for the configuration ops). */
typedef struct RoomWindowState {
    void (*callback)(void);       /* 0x00 */
    int *signal;                  /* 0x04 */
    s16 active;                   /* 0x08 */
    s8 variant;                   /* 0x0A */
    s8 optionB;                   /* 0x0B */
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

#define ROOM_WINDOW_SCRATCH ((RoomWindowScratch *)0x1F800000)

extern struct FieldActor *g_PlayerEntity;

extern int FieldFx_IntSqrt(int value);

struct RoomEnt;

/* The window actor class (src/overlays/room_lib/window_class.c). */
int RoomLib_WindowClassNop0(void);
int RoomLib_InitWindowClass(RoomWindowObject *ctx);
int RoomLib_ConfigureWindowClass(RoomWindowObject *obj, int query, unsigned int op,
                                 int a, int b, int c);
int RoomLib_WindowClassNop3(void);
int RoomLib_UpdateWindowClass(struct RoomEnt *o);
int RoomLib_WindowClassNop6(void);
void RoomLib_WindowHandler(RoomWindowObject *obj);
void RoomLib_WindowArrived(void *ctx);
void RoomLib_ArmWindowClass(struct RoomEnt *o);
int RoomLib_ReleaseWindowClass(char *ctx);

/* Its row of a room's class table (g_RoomClassTable, pe1/room_module.h). */
#define ROOMLIB_WINDOW_CLASS \
    { RoomLib_WindowClassNop0, RoomLib_InitWindowClass, RoomLib_ConfigureWindowClass, \
      RoomLib_WindowClassNop3, RoomLib_UpdateWindowClass, RoomLib_ReleaseWindowClass, \
      RoomLib_WindowClassNop6 }

#endif
