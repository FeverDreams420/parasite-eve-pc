/* Shared room-script library. Every room links its own copy; instance
 * functions are stamped per room/offset with these body macros. */
#ifndef ROOM_LIB_H
#define ROOM_LIB_H

#include "common.h"
#include "pe1/battle_context.h"
#include "pe1/field_glow_sprite.h"
#include "pe1/field_owner_actor.h"
#include "pe1/field_script_context.h"
#include "pe1/field_sprite_state.h"
#include "pe1/field_vector_math.h"
#include "pe1/fieldfx_callback_list.h"
#include "pe1/fieldfx_calls.h"
#include "pe1/fieldfx_slot.h"
#include "pe1/fieldfx_state.h"
#include "pe1/fieldfx_task_pool.h"
#include "pe1/gte.h"
#include "pe1/gte_sine_table.h"
#include "pe1/gte_types.h"
#include "pe1/pe_image.h"
#include "pe1/render_object.h"
#include "pe1/room_floor.h"
#include "pe1/room_fx.h"
#include "pe1/room_particle_state.h"
#include "pe1/room_twelve_effect.h"

typedef struct RoomObj {
    char pad0[0xC];
    void (*callback)(void);
} RoomObj;

/* Parameters read by the matching room particle emitter handlers. */
typedef struct RoomParticleEmitter {
    short x;                      /* 0x00 */
    short y;                      /* 0x02 */
    short z;                      /* 0x04 */
    short soundId;                /* 0x06: -1 disables the completion sound */
    int radialMagnitude;          /* 0x08 */
    int spawnPeriod;              /* 0x0C */
    int endFrame;                 /* 0x10 */
} RoomParticleEmitter;

#define ROOMLIB_RETURN_ZERO(name) \
    int name(void) { \
        return 0; \
    }

#define ROOMLIB_RETURN_VOID(name) \
    void name(void) { \
    }

#define RW32(o, off) (*(int *)((char *)(o) + (off)))
#define RW16(o, off) (*(short *)((char *)(o) + (off)))
#define RW8(o, off)  (*(unsigned char *)((char *)(o) + (off)))
#define RWU16(o, off) (*(unsigned short *)((char *)(o) + (off)))
#define RVW32(o, off) (*(volatile int *)((char *)(o) + (off)))
#define RVW16(o, off) (*(volatile short *)((char *)(o) + (off)))
#define RVU16(o, off) (*(volatile unsigned short *)((char *)(o) + (off)))
#define RWPTR(o, off) ((void *)((char *)(o) + (off)))

/* Locals for the room rotation-table lookups. These used to carry * pins; stock allocation now places them where retail does. */
#define ROOMLIB_ROT_ENTRY_DECL \
    int *entry; \
    int *base

#define ROOMLIB_V0_PTR_DECL(name) void *name
#define ROOMLIB_V1_INT_DECL(name) int name
#define ROOMLIB_A0_INT_DECL(name) int name

#define ROOMLIB_TICK_12_COUNTERS(name) \
    void name(void *arg0, unsigned char *signal, RoomLibTick12Rec *rec) { \
        unsigned int i; \
        for (i = 0; i < 12; i++) { \
            int s = rec->state[i]; \
            if (s != -1) { \
                if (s == 1) { \
                    short limit; \
                    rec->phase[i] += rec->phaseStep; \
                    limit = rec->phaseLimit; \
                    if (limit < (short)rec->phase[i]) { \
                        rec->phase[i] = limit; \
                    } \
                } \
                rec->position[i].z -= rec->frameStep; \
                if ((short)rec->position[i].z < 10) { \
                    rec->state[i] = 1; \
                } \
            } \
            rec->timer[i]++; \
            if (rec->timer[i] == rec->timerLimit) { \
                rec->repeatCount--; \
                if ((short)rec->repeatCount == 0) { \
                    signal[1] = 2; \
                } \
            } \
        } \
    }

struct RoomEnt;
extern void RoomLib_HandlerA(struct RoomEnt *obj);
extern void RoomLib_HandlerB(struct RoomEnt *obj);
extern void RoomLib_HandlerC(struct RoomEnt *obj);
extern void RoomLib_HandlerD(struct RoomEnt *obj);
extern void RoomLib_HandlerE(struct RoomEnt *obj);
extern void RoomLib_HandlerF(struct RoomEnt *obj);
extern struct FieldActor *g_PlayerEntity; /* the player actor (0x8009D254) */
/* Field object status: 0 no model, 1 model without data, 2 inactive,
 * 3 active, 4 disabled object, 5 the player. */
extern int FieldFx_GetStatus(void *object);
extern int func_80192BFC();

typedef struct RoomRenderNode {
    int flags;                    /* 0x00: 0x3F000000 owner bits, 0xC0FFFFFF mask dance */
    char pad04[0x14];
    unsigned char *state;         /* 0x18: byte poked with entity state (4) */
    char pad1C[0x90];
    unsigned char startFlag;      /* 0xAC: opens the gated module start */
} RoomRenderNode;

/* The actor/object record shared with the field engine (>= 0x254 bytes).
 * Positions are 16.16 fixed point; offsets mined from 90 canonical shapes. */
typedef struct RoomLink {
    RoomRenderNode *target;       /* 0x00 */
    char pad04[0xA];
    unsigned char variant;        /* 0x0E: matched against RoomEnt.t16 */
    char pad0F[0x7];
    unsigned short winLo;         /* 0x16: t17 window upper bound */
    char pad18[0x2];
    unsigned short winHi;         /* 0x1A: t17 window lower bound */
    char pad1C[0xA];
    unsigned short h26;           /* 0x26 */
    int pos[4];                   /* 0x28: 16.16 x/y/z plus copied trailing word */
    short rotation[3];            /* 0x38: x/y/z angles, y is the heading */
    char pad3E[0x2];
    int posMirror[4];             /* 0x40: optional position snapshot */
    char pad50[0x18];
    int vel[3];                   /* 0x68 */
    char pad74[0x4];
    int move[3];                  /* 0x78 */
    char pad84[0x4];
    int accel[3];                 /* 0x88 */
    char pad94[0x4];
    unsigned int flags;           /* 0x98 */
    char pad9C[0xF0];
    struct RoomLink *link18C;     /* 0x18C: nested link record */
    char pad190[0x6C];
    int modelTranslation[3];      /* 0x1FC: integer position of the model matrix */
    char pad208[0x30];
    void *p238;                   /* 0x238 */
    char pad23C[0x14];
    unsigned short h250;          /* 0x250 */
} RoomLink;

typedef RoomRenderNode RoomLinkByte;  /* legacy alias */

/* Script-entity record (~0xC4 bytes), one per room script object. */
typedef struct RoomEnt {
    unsigned char state;          /* 0x00: 4 = closed */
    char pad1[0x2];
    unsigned char flag3;          /* 0x03 */
    int w04;                      /* 0x04 */
    RoomLink *link;               /* 0x08 */
    struct RoomSub {
        void (*cb)(struct RoomEnt *); /* 0x0C: armed FX handler */
        int *signal;              /* 0x10: completion word (0/1/2) */
    } sub;
    short active;                 /* 0x14 */
    signed char t16;              /* 0x16: variant gate */
    signed char t17;              /* 0x17: window gate */
    signed char t18;
    unsigned char t19;            /* 0x19: phase (3/7) */
    unsigned char t1A;
    char pad1B[0x1];
    short mat[9];                 /* 0x1C: s16 3x3 rotation (MATRIX.m) */
    short rot[3];                 /* 0x2E: euler angles */
    short h34;                    /* 0x34 */
    char pad36[0x4];
    short heading;                /* 0x3A */
    int pos[2];                   /* 0x3C, 0x40 */
    short h44;                    /* 0x44: word-view via RW32 in some shapes */
    short h46;                    /* 0x46 */
    short h48;
    char pad4A[0x12];
    int w5C;                      /* 0x5C */
    int w60;
    int w64;
    char pad68[0x14];
    int w7C;                      /* 0x7C */
    int w80;                      /* 0x80 */
    char pad84[0x10];
    int w94;                      /* 0x94 */
    char pad98[0x4];
    short h9C;                    /* 0x9C: also written as word with 0x9E */
    short h9E;
    short hA0;                    /* 0xA0 */
    char padA2[0x12];
    unsigned char bB4;            /* 0xB4: FX-notify gate */
    char padB5[0x1];
    unsigned char bB6;            /* 0xB6 */
    unsigned char bB7;
    unsigned char bB8;
    char padB9[0x3];
    unsigned short hBC[4];        /* 0xBC..0xC2 */
} RoomEnt;

/* Working records used while rotating scripted room motion vectors. */
typedef struct RoomLibMotionState {
    char pad00[0x40];
    int position[3];              /* 0x40 */
    union {
        int word;
        struct {
            short pad4C;
            short height;
        } half;
    } mode;                       /* 0x4C */
    int anchor[3];                /* 0x50 */
    char pad5C[0x14];
    short localStep[3];           /* 0x70: packed GTE V0, shared with rotation */
    char pad76[0xA];
    RoomLink *target;             /* 0x80 */
} RoomLibMotionState;

typedef struct RoomLibMotionWork {
    short input[3];               /* 0x00 */
    short pad06;
    int rotated[3];               /* 0x08 */
    char pad14[0x4];
    int displacement[3];          /* 0x18: second transform result */
    char pad24[0x4];
    short matrix[9];              /* 0x28 */
    short pad3A;
} RoomLibMotionWork;

/* Scratchpad layout of the arc handlers (B and C): the heading matrix, the
 * local vector it turns, the point and offset vectors and the landed flag. */
typedef struct RoomLibArcWork {
    short matrix[9];              /* 0x00 */
    char pad12[0xE];
    short input[3];               /* 0x20 */
    short pad26;
    int point[3];                 /* 0x28: target point or turned step */
    char pad34[0x4];
    int offset[3];                /* 0x38: turned local offset */
    char pad44[0x4];
    int landed;                   /* 0x48 */
} RoomLibArcWork;

/* Four lanes of three-component values with increments and a shared timer. */
typedef struct RoomLibStepRecords {
    char pad00[0x30];
    unsigned short value[4][4];  /* 0x30: 8-byte lane stride */
    unsigned short increment[4][4]; /* 0x50 */
    char pad70[0x8];
    unsigned short timer[4];     /* 0x78 */
} RoomLibStepRecords;

PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomLibStepRecords, value) == 0x30,
                  room_lib_step_values_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomLibStepRecords, increment) == 0x50,
                  room_lib_step_increments_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomLibStepRecords, timer) == 0x78,
                  room_lib_step_timer_offset);
PE1_STATIC_ASSERT(sizeof(RoomLibStepRecords) == 0x80,
                  room_lib_step_records_size);

/* Motion state addressed by HandlerE through RoomEnt + 0x0C. */
typedef struct RoomLibHandlerEState {
    void (*callback)(void);       /* 0x00 */
    int *signal;                  /* 0x04 */
    short active;                 /* 0x08 */
    signed char variant;          /* 0x0A */
    unsigned char optionB;        /* 0x0B */
    unsigned char optionC;        /* 0x0C */
    unsigned char flags;          /* 0x0D */
    char pad0E[0x2];
    int start[3];                 /* 0x10 */
    char pad1C[0x4];
    int target[3];                /* 0x20 */
    char pad2C[0x4];
    int delta[3];                 /* 0x30 */
    char pad3C[0x4];
    int localOffset[3];           /* 0x40 */
    char pad4C[0x4];
    int secondary[3];             /* 0x50 */
    char pad5C[0x4];
    RoomLink *targetLink;         /* 0x60 */
    RoomLink *secondaryLink;      /* 0x64 */
    int speed;                    /* 0x68 */
    short duration;               /* 0x6C */
    short secondaryHeading;       /* 0x6E */
    short eased;                  /* 0x70 */
    short heading;                /* 0x72 */
    short mirrorPosition;         /* 0x74 */
    short frameLimit;             /* 0x76 */
    short frame;                  /* 0x78 */
    short phase;                  /* 0x7A */
    short phaseFrame[4];          /* 0x7C */
    unsigned char lockY;          /* 0x84 */
    unsigned char copyPosition;  /* 0x85 */
} RoomLibHandlerEState;

/* Argument-controlled state used by HandlerD through RoomEnt + 0x0C. */
typedef struct RoomLibHandlerDState {
    void (*callback)(void);       /* 0x00 */
    int *signal;                  /* 0x04 */
    short active;                 /* 0x08 */
    signed char variant;          /* 0x0A */
    unsigned char optionB;        /* 0x0B */
    unsigned char optionC;        /* 0x0C */
    unsigned char flags;          /* 0x0D */
    char pad0E[0x2];
    short matrix[9];              /* 0x10: travel rotation */
    char pad22[0xE];
    int motion[3];                /* 0x30: local x/y/z travel */
    char pad3C[0x4];
    int target[4];                /* 0x40 */
    int anchor[3];                /* 0x50: start position */
    char pad5C[0x4];
    int positionX;                /* 0x60 */
    char pad64[0x4];
    int positionZ;                /* 0x68 */
    char pad6C[0x4];
    short rotation[3];            /* 0x70 */
    char pad76[0x2];
    short angles[3];              /* 0x78: travel direction */
    char pad7E[0x2];
    RoomLink *targetLink;         /* 0x80 */
    RoomLink *secondaryLink;      /* 0x84 */
    int value88;                  /* 0x88 */
    int value8C;                  /* 0x8C */
    int value90;                  /* 0x90 */
    int speed;                    /* 0x94 */
    union {
        int word;                 /* 0x98: 16.16 progress */
        struct {
            short fraction;
            short whole;          /* 0x9A: reported state value */
        } half;
    } progress;
    short range[3];               /* 0x9C */
    short heading;                /* 0xA2 */
    short duration;               /* 0xA4 */
    short rate;                   /* 0xA6 */
    unsigned char mirrorPosition; /* 0xA8 */
    unsigned char reverse;        /* 0xA9 */
    unsigned char lockY;          /* 0xAA */
    unsigned char copyPosition;   /* 0xAB */
    unsigned char copyRotation;   /* 0xAC */
} RoomLibHandlerDState;

/* Argument-controlled state used by HandlerB through RoomEnt + 0x0C. */
typedef struct RoomLibHandlerBState {
    void (*callback)(void);       /* 0x00 */
    int *signal;                  /* 0x04 */
    short active;                 /* 0x08 */
    signed char variant;          /* 0x0A */
    unsigned char optionB;        /* 0x0B */
    unsigned char optionC;        /* 0x0C */
    unsigned char flags;          /* 0x0D */
    unsigned char started;        /* 0x0E */
    char pad0F;
    short matrix[9];              /* 0x10: heading rotation */
    char pad22[0xE];
    int target[3];                /* 0x30 */
    char pad3C[0x4];
    int localOffset[3];           /* 0x40 */
    char pad4C[0x4];
    int secondaryX;               /* 0x50 */
    char pad54[0x4];
    int secondaryZ;               /* 0x58 */
    char pad5C[0x4];
    RoomLink *targetLink;         /* 0x60 */
    RoomLink *secondaryLink;      /* 0x64 */
    int speed;                    /* 0x68 */
    int acceleration;             /* 0x6C */
    int duration;                 /* 0x70 */
    int phaseValue;               /* 0x74 */
    int horizontalVelocity[2];   /* 0x78: X/Z velocity during the arc */
    int mode;                     /* 0x80 */
    short floorY;                 /* 0x84: landing height */
    short rate;                   /* 0x86 */
    short heading;                /* 0x88 */
    short secondaryHeading;       /* 0x8A */
    unsigned char finished;       /* 0x8C */
} RoomLibHandlerBState;

/* Argument-controlled state used by HandlerC through RoomEnt + 0x0C. */
typedef struct RoomLibHandlerCState {
    void (*callback)(void);       /* 0x00 */
    int *signal;                  /* 0x04 */
    short active;                 /* 0x08 */
    signed char variant;          /* 0x0A */
    unsigned char optionB;        /* 0x0B */
    unsigned char optionC;        /* 0x0C */
    unsigned char flags;          /* 0x0D */
    unsigned char started;        /* 0x0E */
    char pad0F[0x21];
    int target[3];                /* 0x30 */
    char pad3C[0x4];
    int arcTarget[3];             /* 0x40: transformed destination */
    char pad4C[0x4];
    int localOffset[3];           /* 0x50 */
    char pad5C[0x4];
    int secondaryX;               /* 0x60 */
    char pad64[0x4];
    int secondaryZ;               /* 0x68 */
    char pad6C[0x4];
    RoomLink *targetLink;         /* 0x70 */
    RoomLink *secondaryLink;      /* 0x74 */
    char pad78[0x4];
    int phaseValue;               /* 0x7C */
    int horizontalVelocity[2];   /* 0x80: X/Z velocity during the arc */
    int mode;                     /* 0x88 */
    char pad8C[0x4];
    short rate;                   /* 0x90 */
    short heading;                /* 0x92 */
    short secondaryHeading;       /* 0x94 */
    unsigned char finished;       /* 0x96 */
} RoomLibHandlerCState;

extern short D_800966EE[];
extern struct BattleEntity *D_8009D254;
struct FieldActorNode;
extern struct FieldActor *g_FieldActorListHead;
extern int RoomLib_ResetAndSignal_801914B0(RoomEnt *obj);
extern int RoomLib_Set4ClearSignal_80192510(RoomEnt *obj);
extern void RoomLib_ArmWindowA_80190D0C(RoomEnt *obj);
extern void RoomLib_ArmWindowB_80191824(RoomEnt *obj);
extern void RoomLib_NotifyArmB_8018F598(RoomEnt *obj);
extern void RoomLib_Notify2ArmB_801902D8(RoomEnt *obj);
extern void RoomLib_Notify2ArmB_801902C8(RoomEnt *obj);
extern void RoomLib_Notify2ArmB_801902D0(RoomEnt *obj);
extern void RoomLib_Notify2ArmB_801902D4(RoomEnt *obj);
extern void RoomLib_Notify2ArmB_801902DC(RoomEnt *obj);
extern void RoomLib_Notify2ArmB_801902E0(RoomEnt *obj);
extern void RoomLib_Notify2ArmB_801902E4(RoomEnt *obj);
extern void RoomLib_Notify2ArmB_801902E8(RoomEnt *obj);
extern void RoomLib_Notify2ArmB_801902EC(RoomEnt *obj);
extern void RoomLib_Notify2ArmB_801902FC(RoomEnt *obj);
extern void RoomLib_Notify2ArmB_8019031C(RoomEnt *obj);
extern void RoomLib_Notify2ArmB_80190314(RoomEnt *obj);
extern void RoomLib_Notify2ArmB_80190320(RoomEnt *obj);
extern void RoomLib_Notify2ArmB_80190328(RoomEnt *obj);
extern void RoomLib_Notify2ArmB_80190340(RoomEnt *obj);
extern void RoomLib_Notify2ArmB_8019035C(RoomEnt *obj);
extern void RoomLib_Notify2ArmB_80190F64(RoomEnt *obj);
extern void RoomLib_Notify2ArmB_801912CC(RoomEnt *obj);
extern void RoomLib_Notify2ArmB_801912D4(RoomEnt *obj);
extern void RoomLib_Notify2ArmB_801912E0(RoomEnt *obj);
extern void RoomLib_Notify2ArmB_80191368(RoomEnt *obj);
extern void RoomLib_Notify2ArmB_80191430(RoomEnt *obj);
extern void RoomLib_Notify2ArmB_801917E4(RoomEnt *obj);
extern void RoomLib_Notify2ArmB_80191D00(RoomEnt *obj);
extern void RoomLib_Notify2ArmB_80191F18(RoomEnt *obj);
extern void RoomLib_Notify2ArmB_80192420(RoomEnt *obj);
extern void RoomLib_Notify2ArmB_80192428(RoomEnt *obj);
extern void RoomLib_Notify2ArmB_80194A5C(RoomEnt *obj);
extern void RoomLib_NotifyArmB_8018F588(RoomEnt *obj);
extern void RoomLib_NotifyArmB_8018F590(RoomEnt *obj);
extern void RoomLib_NotifyArmB_8018F594(RoomEnt *obj);
extern void RoomLib_NotifyArmB_8018F59C(RoomEnt *obj);
extern void RoomLib_NotifyArmB_8018F5A0(RoomEnt *obj);
extern void RoomLib_NotifyArmB_8018F5A4(RoomEnt *obj);
extern void RoomLib_NotifyArmB_8018F5A8(RoomEnt *obj);
extern void RoomLib_NotifyArmB_8018F5AC(RoomEnt *obj);
extern void RoomLib_NotifyArmB_8018F5BC(RoomEnt *obj);
extern void RoomLib_NotifyArmB_8018F5DC(RoomEnt *obj);
extern void RoomLib_NotifyArmB_8018F5E0(RoomEnt *obj);
extern void RoomLib_NotifyArmB_8018F5E8(RoomEnt *obj);
extern void RoomLib_NotifyArmB_8018F600(RoomEnt *obj);
extern void RoomLib_NotifyArmB_8018F61C(RoomEnt *obj);
extern void RoomLib_NotifyArmB_80190224(RoomEnt *obj);
extern void RoomLib_NotifyArmB_8019058C(RoomEnt *obj);
extern void RoomLib_NotifyArmB_80190594(RoomEnt *obj);
extern void RoomLib_NotifyArmB_801905A0(RoomEnt *obj);
extern void RoomLib_NotifyArmB_80190628(RoomEnt *obj);
extern void RoomLib_NotifyArmB_801906F0(RoomEnt *obj);
extern void RoomLib_NotifyArmB_80190AA4(RoomEnt *obj);
extern void RoomLib_NotifyArmB_80190FC0(RoomEnt *obj);
extern void RoomLib_NotifyArmB_801911D8(RoomEnt *obj);
extern void RoomLib_NotifyArmB_801916E0(RoomEnt *obj);
extern void RoomLib_NotifyArmB_801916E8(RoomEnt *obj);
extern void RoomLib_NotifyArmB_80193D1C(RoomEnt *obj);
extern void RoomLib_ArmWindowA_80190CFC(RoomEnt *obj);
extern void RoomLib_ArmWindowA_80190D04(RoomEnt *obj);
extern void RoomLib_ArmWindowA_80190D08(RoomEnt *obj);
extern void RoomLib_ArmWindowA_80190D10(RoomEnt *obj);
extern void RoomLib_ArmWindowA_80190D14(RoomEnt *obj);
extern void RoomLib_ArmWindowA_80190D18(RoomEnt *obj);
extern void RoomLib_ArmWindowA_80190D1C(RoomEnt *obj);
extern void RoomLib_ArmWindowA_80190D20(RoomEnt *obj);
extern void RoomLib_ArmWindowA_80190D30(RoomEnt *obj);
extern void RoomLib_ArmWindowA_80190D48(RoomEnt *obj);
extern void RoomLib_ArmWindowA_80190D50(RoomEnt *obj);
extern void RoomLib_ArmWindowA_80190D54(RoomEnt *obj);
extern void RoomLib_ArmWindowA_80190D5C(RoomEnt *obj);
extern void RoomLib_ArmWindowA_80190D74(RoomEnt *obj);
extern void RoomLib_ArmWindowA_80190D90(RoomEnt *obj);
extern void RoomLib_ArmWindowA_80191998(RoomEnt *obj);
extern void RoomLib_ArmWindowA_80191D00(RoomEnt *obj);
extern void RoomLib_ArmWindowA_80191D08(RoomEnt *obj);
extern void RoomLib_ArmWindowA_80191D14(RoomEnt *obj);
extern void RoomLib_ArmWindowA_80191D9C(RoomEnt *obj);
extern void RoomLib_ArmWindowA_80191E64(RoomEnt *obj);
extern void RoomLib_ArmWindowA_80192218(RoomEnt *obj);
extern void RoomLib_ArmWindowA_80192734(RoomEnt *obj);
extern void RoomLib_ArmWindowA_8019294C(RoomEnt *obj);
extern void RoomLib_ArmWindowA_80192E54(RoomEnt *obj);
extern void RoomLib_ArmWindowA_80192E5C(RoomEnt *obj);
extern void RoomLib_ArmWindowA_80195490(RoomEnt *obj);
extern void func_80191100(RoomEnt *obj);
extern void func_80191108(RoomEnt *obj);
extern void func_8019110C(RoomEnt *obj);
extern void func_80191114(RoomEnt *obj);
extern void func_80191118(RoomEnt *obj);
extern void func_8019111C(RoomEnt *obj);
extern void func_80191120(RoomEnt *obj);
extern void func_80191124(RoomEnt *obj);
extern void func_80191134(RoomEnt *obj);
extern void func_80191154(RoomEnt *obj);
extern void func_80191158(RoomEnt *obj);
extern void func_80191160(RoomEnt *obj);
extern void func_80191178(RoomEnt *obj);
extern void func_80191194(RoomEnt *obj);
extern void func_80191D9C(RoomEnt *obj);
extern void func_80192104(void);
extern void func_8019210C(RoomEnt *obj);
extern void func_80192118(RoomEnt *obj);
extern void func_801921A0(RoomEnt *obj);
extern void func_80192268(RoomEnt *obj);
extern void func_8019261C(RoomEnt *obj);
extern void func_80192B38(RoomEnt *obj);
extern void func_80193258(RoomEnt *obj);
extern void func_80193260(RoomEnt *obj);
extern void func_80195894(RoomEnt *obj);
extern void RoomLib_ArmWindowB_80191814(RoomEnt *obj);
extern void RoomLib_ArmWindowB_80191860(RoomEnt *obj);
extern void func_801902C8(RoomEnt *obj);
extern void func_80191814(RoomEnt *obj);
extern void func_80191820(RoomEnt *obj);
extern void RoomLib_ArmWindowB_8019181C(RoomEnt *obj);
extern void RoomLib_ArmWindowB_80191820(RoomEnt *obj);
extern void RoomLib_ArmWindowB_80191828(RoomEnt *obj);
extern void RoomLib_ArmWindowB_8019182C(RoomEnt *obj);
extern void RoomLib_ArmWindowB_80191830(RoomEnt *obj);
extern void RoomLib_ArmWindowB_80191834(RoomEnt *obj);
extern void RoomLib_ArmWindowB_80191838(RoomEnt *obj);
extern void RoomLib_ArmWindowB_80191848(RoomEnt *obj);
extern void RoomLib_ArmWindowB_80191868(RoomEnt *obj);
extern void RoomLib_ArmWindowB_8019186C(RoomEnt *obj);
extern void RoomLib_ArmWindowB_80191874(RoomEnt *obj);
extern void RoomLib_ArmWindowB_8019188C(RoomEnt *obj);
extern void RoomLib_ArmWindowB_801918A8(RoomEnt *obj);
extern void RoomLib_ArmWindowB_801924B0(RoomEnt *obj);
extern void RoomLib_ArmWindowB_80192818(RoomEnt *obj);
extern void RoomLib_ArmWindowB_80192820(RoomEnt *obj);
extern void RoomLib_ArmWindowB_8019282C(RoomEnt *obj);
extern void RoomLib_ArmWindowB_801928B4(RoomEnt *obj);
extern void RoomLib_ArmWindowB_8019297C(RoomEnt *obj);
extern void RoomLib_ArmWindowB_80192D30(RoomEnt *obj);
extern void RoomLib_ArmWindowB_8019324C(RoomEnt *obj);
extern void RoomLib_ArmWindowB_80193464(RoomEnt *obj);
extern void RoomLib_ArmWindowB_8019396C(RoomEnt *obj);
extern void RoomLib_ArmWindowB_80193974(RoomEnt *obj);
extern void RoomLib_ArmWindowB_80195FA8(RoomEnt *obj);
extern void func_80191D08();
extern void func_80191D10(void);
extern void func_80191D54(void);
extern void func_8019114C(RoomEnt *obj);
extern void func_80191D14(void);
extern void func_80191D1C(void);
extern void func_80191D20(void);
extern void func_80191D24(void);
extern void func_80191D28(void);
extern void func_80191D2C(void);
extern void func_80191D3C(void);
extern void func_80191D5C(RoomEnt *obj);
extern void func_80191D60(void);
extern void func_80191D68(RoomEnt *obj);
extern void func_80191D80(void);
extern void func_801929A4(RoomEnt *obj);
extern void func_80192D0C(void);
extern void func_80192D14(void);
extern void func_80192D20(void);
extern void func_80192DA8(void);
extern void func_80192E70(void);
extern void func_80193224(void);
extern void func_80193740(void);
extern void func_80193958(void);
extern void func_80193E60(void);
extern void func_80193E68(void);
extern void func_8019649C(struct RoomEnt *obj);
extern int RoomLib_Set4ClearSignal_801924D4(RoomEnt *o);

/* A view matrix copied as eight words. */
typedef union RoomLibFxMatrixWords {
    int words[8];
    GteMatrix matrix;
} RoomLibFxMatrixWords;

/* Effect owner state set up by the room effect Init methods: the owner's
 * link, a copy of its view matrix (the translation is where the effect
 * stands), the sprite asset whose low byte selects the texture page, and
 * the reload timer and hit flag the owner tick polls. */
typedef struct RoomLibFxMatrixState {
    RoomLink *link;               /* 0x00 */
    RoomLibFxMatrixWords matrix;  /* 0x04 */
    union {
        int index;
        unsigned char page;
    } asset;                      /* 0x24 */
    unsigned short reload;        /* 0x28 */
    short cooldown;               /* 0x2A */
    short hit;                    /* 0x2C */
} RoomLibFxMatrixState;

typedef struct RoomLibPacked8 {
    int lo;
    int hi;
} __attribute__((packed)) RoomLibPacked8;


/* Each room that links room_lib/twelve_elements.c defines the two packet
 * templates and the sprite table in its data. */
extern RoomFxSpritePacket RoomLib_TwelveEffectPrimaryPacket;
extern RoomFxSpritePacket RoomLib_TwelveEffectSecondaryPacket;
extern unsigned char RoomLib_TwelveEffectTable[];

#define ROOMLIB_JOIN_RAW(a, b) a##b
#define ROOMLIB_JOIN(a, b) ROOMLIB_JOIN_RAW(a, b)

extern void RoomLib_FxNotify(RoomLink *l, struct RoomSub *s, int scratch);
extern void func_800DFB20(void *state);

/* state=4 and clear the signal word */
#define ROOMLIB_SET4_CLEAR_SIGNAL(name) \
    int name(RoomEnt *o) { \
        int *p = o->sub.signal; \
        o->state = 4; \
        if (p != 0) { \
            *p = 0; \
        } \
        return 0; \
    }

/* Move the field actor by a local, matrix-rotated X/Z step. */
#define ROOMLIB_MOVE_ACTOR_LOCAL(name, finish) \
    void name(RoomEnt *o) { \
        int height; \
        if (RW8(D_8009D254, 0xE) >= 4) { \
            finish(o); \
        } else if (RW32(D_8009D254, 0x98) & 0xC0000) { \
            RW32(D_8009D254, 0x98) &= 0xFFF3FFFF; \
            finish(o); \
        } else { \
            volatile short *scratch = (volatile short *)0x1F800000; \
            scratch[0] = 0; \
            scratch[1] = 0; \
            scratch[2] = o->pos[0] >> 12; \
            gte_ldrotmatrix(o->mat); \
            gte_ldv0((void *)scratch); \
            gte_mvmva(); \
            gte_stmac((void *)&scratch[4]); \
            RW32(D_8009D254, 0x28) += *(int *)&scratch[4] << 12; \
            RW32(D_8009D254, 0x30) += *(int *)&scratch[8] << 12; \
            height = o->pos[0] - o->pos[1]; \
            o->pos[0] = height; \
            if (height < 0) { \
                finish(o); \
            } \
            if (o->h46 != 0) { \
                RW16(D_8009D254, 0x3A) = FieldFx_TurnToward( \
                    RW16(D_8009D254, 0x3A), (short)o->h48, (short)o->h46); \
            } \
        } \
    }

/* six-argument passthrough to the field engine spawn */
#define ROOMLIB_SPAWN6(name) \
    int name(int a, int b, int c, int d, int e, int f) { \
        FieldFx_Spawn6(a, b, c, d, e, f); \
        return 0; \
    }

/* close entity: state 4 here and on the link target, clear high flags */
#define ROOMLIB_CLOSE_TARGET(name) \
    int name(RoomEnt *o) { \
        o->state = 4; \
        if ((unsigned int)FieldFx_GetStatus(o) >= 2) { \
            RoomLinkByte *tgt = o->link->target; \
            *(int *)tgt &= 0xC0FFFFFF; \
            *o->link->target->state = 4; \
        } \
        return 0; \
    }

/* this entity's table with the field engine when active */
#define ROOMLIB_REGISTER_TABLE(name, table) \
    int name(void *o) { \
        if ((unsigned int)FieldFx_GetStatus(o) >= 2) { \
            FieldFx_Register(o, table); \
        } \
        return 0; \
    }

/* registration method that registers the draw list whatever the field
 * engine's status */
#define ROOMLIB_REGISTER_TABLE_ANY(name, table) \
    int name(void *o) { \
        FieldFx_Register(o, table); \
        return 0; \
    }

/* start method that acts whatever the field engine's status: the
 * update handlers and spawn from the init list and the spawn layout,
 * closing the module with `close` when either call fails */
#define ROOMLIB_START_ANY(name, update, init, layout, close) \
    int name(void *o) { \
        if ((FieldFx_UpdateSlots(o, update) | FieldFx_RunScript(o, init, layout)) == -1) { \
            close(o); \
        } \
        return 0; \
    }

/* start method that acts only while the field engine runs the object
 * (status 3): the update handlers and spawn from the init list and
 * the spawn layout, closing the module with `close` when either call fails
 * or the object is not running */
#define ROOMLIB_START_AT3(name, update, init, layout, close) \
    int name(void *o) { \
        int result; \
        if (FieldFx_GetStatus(o) == 3) { \
            result = FieldFx_UpdateSlots(o, update); \
            result |= FieldFx_RunScript(o, init, layout); \
        } else { \
            result = -1; \
        } \
        if (result == -1) { \
            close(o); \
        } \
        return 0; \
    }

/* plant the room table pointer into the engine slot */
#define ROOMLIB_PLANT_TABLE(name, table) \
    int name(void *object) { \
        *FieldFx_GetSlot(object) = table; \
        return 0; \
    }

/* stash one arg into a room slot; returns the slot */
#define ROOMLIB_SET_ARG1(name, sA) \
    int *name(int a0, int a1) { \
        int *p = &sA; \
        *p = a1; \
        return p; \
    }

typedef struct FieldActorNode {
    int w00;
    struct FieldActorNode *next;  /* 0x04 */
    int w08;
    unsigned char b0C;            /* 0x0C: kind matched against arg */
    unsigned char b0D;            /* 0x0D: sub-kind */
    char pad0E[0x8A];
    int w98;                      /* 0x98: 0x10 = busy */
} FieldActorNode;

typedef struct RoomSlotRec {
    short h0;
    short h2;                     /* stamped with the global frame counter */
    short h4;
    short pad6;
    int w8;
    int wC;
} RoomSlotRec;

/* The room's slot table (script_slot.c), named in each room's symbol
 * file. */
extern RoomSlotRec RoomLib_SlotTable[];
RoomSlotRec *RoomLib_SlotSet(int mode, int idx, int a, int b);

typedef struct RoomBlob8 {
    char b[8];
} RoomBlob8;

typedef struct RoomMsgSub {
    short h0;
    short h2;
    short h4;
} RoomMsgSub;

typedef struct RoomMsg {
    short h0;
    short h2;
    short h4;
    short h6;
    RoomMsgSub sub;               /* 0x08 */
} RoomMsg;

/* Dropped flare particle state as seen by the dispatcher; the handler at
 * RoomLib_DlgBlob is RoomEffect_DroppedFlareParticle (room_flare.h).
 * Its first mode integrates the three velocity halfwords into x/y/z and
 * accelerates velocityY by two each tick. */
typedef struct RoomDlgAnimState {
    short x;                     /* 0x00 */
    short y;                     /* 0x02 */
    short z;                     /* 0x04 */
    short pad06;                 /* 0x06 */
    short velocityX;             /* 0x08 */
    short velocityY;             /* 0x0A */
    short velocityZ;             /* 0x0C */
    short pad0E;                 /* 0x0E */
    short state;                 /* 0x10 */
    short timer;                 /* 0x12 */
} RoomDlgAnimState;

typedef struct RoomDlgAnimParams {
    char pad00[0x4];
    int scale;                   /* 0x04 */
} RoomDlgAnimParams;

typedef int (*RoomDlgCallback)(int mode, RoomDlgAnimState *state,
                               RoomDlgAnimParams *params);

typedef struct RoomQRec {
    short h0;
    short h2;
    short h4;
    short h6;
    char sub[8];                  /* 0x08 */
    short h10;                    /* 0x10 */
    short h12;                    /* 0x12 */
} RoomQRec;

typedef struct RoomNodeB {
    char pad[0x18];
    unsigned char *state;         /* 0x18 */
} RoomNodeB;

typedef struct RoomChanCtx {
    int w0;
    int w4;
    RoomNodeB **w8;               /* 0x08 */
} RoomChanCtx;

typedef struct RoomDlgState {
    char pad[0xD];
    unsigned char bD;             /* 0x0D */
    char padE[0x4];
    short h12;                    /* 0x12 */
} RoomDlgState;


/* The colour ramp table of the timed spawns (timed_spawn.c), each
 * room's own data. */
extern char RoomLib_TimedRenderTable;

#define ROOMLIB_MSG_DISPATCH(name, rect, blob) \
    int name(int mode, RoomMsg *msg, short *pitch) { \
        RoomBlob8 tmp = rect; \
        switch (mode) { \
        case 0: \
            FieldFx_TransformMatrixPoint(ANIM_OWNER_AS(RoomChanCtx)->w8, 0x13, &tmp, msg); \
            if (FX_TASK_AS(RoomDlgState)->h12 == 0) goto send_pos; \
            if (FX_TASK_AS(RoomDlgState)->h12 == 1) goto at_actor; \
            goto after; \
        send_pos: \
            FieldFx_GetTransformLookAngles(ANIM_OWNER_AS(RoomChanCtx)->w8, 0x13, &msg->sub); \
            goto after; \
        at_actor: \
            { \
                short vec[4]; \
                FieldFx_GetActorPosition(g_PlayerEntity, 0, vec); \
                FieldFx_CalculateLookAngles(msg, vec, &msg->sub); \
                msg->sub.h0 = 0x180; \
            } \
        after: \
            if (FX_TASK_AS(RoomDlgState)->bD != 0) { \
                RoomNodeB **q = ANIM_OWNER_AS(RoomChanCtx)->w8; \
                if (q != 0 && *q != 0) { \
                    unsigned char *st = (*q)->state; \
                    if (*st == 1) { \
                        *st = 2; \
                    } \
                } \
            } \
            return FieldFx_InitCallbackList(ANIM_POOL_AS(RoomChanCtx)->w8, 0x14, 0x18, &blob); \
        case 1: \
            if (g_FieldFxCurrentAge == mode) { \
                RoomQRec *p = FieldFx_AcquireCallbackSlot(ANIM_POOL_AS(RoomChanCtx)->w8); \
                if (p != 0) { \
                    p->h0 = msg->h0; \
                    p->h2 = msg->h2; \
                    p->h4 = msg->h4; \
                    FieldFx_AnglesToVector(&msg->sub, *pitch, p->sub); \
                    p->h10 = 0; \
                    p->h12 = 0; \
                } \
                p = FieldFx_AcquireCallbackSlot(ANIM_POOL_AS(RoomChanCtx)->w8); \
                if (p != 0) { \
                    p->h0 = msg->h0; \
                    p->h2 = msg->h2; \
                    p->h4 = msg->h4; \
                    p->h10 = 3; \
                    p->h12 = 0; \
                } \
                FieldFx_PlaySoundAtOwner(0x586, FieldFx_GetOwnerProgress()); \
            } \
            if (g_FieldFxCurrentAge < 2) goto ret0; \
            return 2; \
        case 2: \
            g_FieldFxSpriteParams.parameter0A = 0; \
            g_FieldFxSpriteParams.depth = 8; \
            goto ret0; \
        default: \
        ret0: \
            return 0; \
        } \
    }

/* Field engine state query used by the actor class update handlers. */
extern int FieldFx_IsOwnerBusy();

typedef struct RoomTimer {
    char pad0[0x24];
    unsigned short h24;           /* 0x24: reload value */
    short h26;                    /* 0x26: countdown */
    short h28;                    /* 0x28: fire request */
} RoomTimer;

typedef struct RoomTimer0 {
    char pad0[0x4];
    unsigned short h4;            /* 0x04: reload value */
    short h6;                     /* 0x06: countdown */
    short h8;                     /* 0x08: fire request */
} RoomTimer0;

typedef struct RoomTimer2 {
    char pad0[0x28];
    unsigned short h28;           /* 0x28: reload value */
    short h2A;                    /* 0x2A: countdown */
    short h2C;                    /* 0x2C: fire request */
} RoomTimer2;


#define ROOMLIB_PARTICLE_TICK_A(name) \
    void name(RoomEnt *o, RoomFxControl *state, RoomFxPairedEmitterState *sys) { \
        struct { short a; short pad; short b; } saved; \
        RoomLibFxMatrixState *clock = FieldFx_GetScriptContext(); \
        int tmp = RW16(o->link, 0x2A); \
        saved.a = tmp; \
        tmp = RW16(o->link, 0x32); \
        saved.b = tmp; \
        if (state->frame < 0x20) { \
            sys->timer = (unsigned short)sys->timer + (unsigned short)sys->phase; \
            sys->phase = (unsigned short)sys->phase + 7; \
            if (state->frame < 0x20 && sys->intensity >= 5) { \
                sys->intensity = sys->intensity - 4; \
            } \
        } \
        if (state->frame == 0x10) { \
            clock->hit = 1; \
        } \
        if (state->frame == 0x20) { \
            state->state = 2; \
        } \
    }

typedef struct RoomFxParams {
    char pad0[0x8];
    short h8;
    short hA;
    short hC;
    char padE[0x2];
    short h10;
    short h12;
    char pad14[0x1];
    unsigned char b15;
    unsigned char b16;
} RoomFxParams;

#define ROOMLIB_SET3_RESET(name, v10, v12) \
    void name(int a, int b, RoomFxParams *c) { \
        c->h10 = v10; \
        c->h8 = 0; \
        c->hA = 0; \
        c->hC = 0; \
        c->h12 = v12; \
    }

typedef struct RoomStatePair {
    unsigned char b0;
    unsigned char b1;             /* 0x01: state byte, 2 = done */
    short h2;                     /* 0x02: threshold */
} RoomStatePair;

typedef struct RoomClock {
    char pad0[0x8];
    short h8;                     /* 0x08 */
    short hA;                     /* 0x0A */
    short hC;                     /* 0x0C: current tick */
    char padE[0x2];
    unsigned char renderOwner;   /* 0x10 */
} RoomClock;

extern int rand(void);

#define ROOMLIB_REGISTER_TABLE_AT3(name, table) \
    int name(void *o) { \
        if (FieldFx_GetStatus(o) == 3) { \
            FieldFx_Register(o, table); \
        } \
        return 0; \
    }

/* The room library's actor classes (actor_classes.c, and
 * floor_walker_class.c in the rooms that link the longer library).
 * A room's class table lists seven methods per class: three no-ops (slots
 * 0, 3 and 6), Init, Configure, Update and Release; the remaining functions
 * are the class's private states. */
struct RoomLibMotionState;
struct RoomLibMotionWork;
int RoomLib_HandlerDNop0(void);
int RoomLib_InitHandlerD(RoomEnt *o);
int RoomLib_ConfigureHandlerD(RoomEnt *o, int query, unsigned int op,
                              int arg0, int arg1, int arg2);
int RoomLib_HandlerDNop3(void);
int RoomLib_UpdateHandlerD(RoomEnt *o);
void RoomLib_ArmHandlerD(RoomEnt *o);
void RoomLib_HandlerDTransformTarget(struct RoomLibMotionState *state,
                                     struct RoomLibMotionWork *work);
int RoomLib_ReleaseHandlerD(RoomEnt *o);
int RoomLib_HandlerDNop6(void);
int RoomLib_HandlerENop0(void);
int RoomLib_InitHandlerE(RoomEnt *o);
int RoomLib_ConfigureHandlerE(RoomEnt *o, int query, unsigned int op,
                              int arg0, int arg1, int arg2);
int RoomLib_HandlerENop3(void);
int RoomLib_UpdateHandlerE(RoomEnt *o);
void RoomLib_ArmHandlerE(RoomEnt *o);
void RoomLib_HandlerESteerToward(char *entity, char *state);
int RoomLib_ReleaseHandlerE(RoomEnt *o);
int RoomLib_HandlerENop6(void);
int RoomLib_HandlerBNop0(void);
int RoomLib_InitHandlerB(RoomEnt *o);
int RoomLib_ConfigureHandlerB(RoomEnt *o, int query, unsigned int op,
                              int arg0, int arg1, int arg2);
int RoomLib_HandlerBNop3(void);
int RoomLib_UpdateHandlerB(RoomObj *obj);
void RoomLib_ArmHandlerB(RoomEnt *o);
void RoomLib_HandlerBPhase(RoomEnt *obj);
void RoomLib_AdvanceArcToTarget(RoomEnt *obj);
int RoomLib_ReleaseHandlerB(RoomEnt *o);
int RoomLib_HandlerBNop6(void);
int RoomLib_HandlerCNop0(void);
int RoomLib_InitHandlerC(RoomEnt *o);
int RoomLib_ConfigureHandlerC(RoomEnt *o, int query, unsigned int op,
                              int arg0, int arg1, int arg2);
int RoomLib_HandlerCNop3(void);
int RoomLib_UpdateHandlerC(RoomObj *obj);
void RoomLib_ArmHandlerC(RoomEnt *o);
void RoomLib_HandlerCPhase(void);
void RoomLib_AdvanceArcToTargetY(RoomEnt *obj);
int RoomLib_ReleaseHandlerC(RoomEnt *o);
int RoomLib_HandlerCNop6(void);
int RoomLib_HandlerANop0(void);
int RoomLib_InitHandlerA(RoomEnt *o);
int RoomLib_ConfigureHandlerA(RoomEnt *o, int arg1, unsigned int op,
                              int arg3, int sp10, int sp14);
int RoomLib_HandlerANop3(void);
int RoomLib_UpdateHandlerA(RoomObj *obj);
void RoomLib_RearmHandlerA(RoomEnt *o);
int RoomLib_ReleaseHandlerA(RoomEnt *o);
int RoomLib_HandlerANop6(void);
struct RoomFloorWalkerObject;
int RoomLib_HandlerGNop0(void);
int RoomLib_InitHandlerG(RoomEnt *o);
int RoomLib_ConfigureHandlerG(RoomEnt *o, int mode, unsigned int op,
                              int arg0, int arg1, int arg2);
int RoomLib_HandlerGNop3(void);
void RoomLib_HandlerG(RoomEnt *o);
void RoomLib_HandlerGFloorWalker(struct RoomEnt *obj);
int RoomLib_ReleaseHandlerG(RoomEnt *o);
int RoomLib_HandlerGNop6(void);

/* One row of a room's class table (g_RoomClassTable) per actor class. */
#define ROOMLIB_ACTOR_CLASS_D \
    { RoomLib_HandlerDNop0, RoomLib_InitHandlerD, RoomLib_ConfigureHandlerD, \
      RoomLib_HandlerDNop3, RoomLib_UpdateHandlerD, RoomLib_ReleaseHandlerD, \
      RoomLib_HandlerDNop6 }
#define ROOMLIB_ACTOR_CLASS_E \
    { RoomLib_HandlerENop0, RoomLib_InitHandlerE, RoomLib_ConfigureHandlerE, \
      RoomLib_HandlerENop3, RoomLib_UpdateHandlerE, RoomLib_ReleaseHandlerE, \
      RoomLib_HandlerENop6 }
#define ROOMLIB_ACTOR_CLASS_B \
    { RoomLib_HandlerBNop0, RoomLib_InitHandlerB, RoomLib_ConfigureHandlerB, \
      RoomLib_HandlerBNop3, RoomLib_UpdateHandlerB, RoomLib_ReleaseHandlerB, \
      RoomLib_HandlerBNop6 }
#define ROOMLIB_ACTOR_CLASS_C \
    { RoomLib_HandlerCNop0, RoomLib_InitHandlerC, RoomLib_ConfigureHandlerC, \
      RoomLib_HandlerCNop3, RoomLib_UpdateHandlerC, RoomLib_ReleaseHandlerC, \
      RoomLib_HandlerCNop6 }
#define ROOMLIB_ACTOR_CLASS_A \
    { RoomLib_HandlerANop0, RoomLib_InitHandlerA, RoomLib_ConfigureHandlerA, \
      RoomLib_HandlerANop3, RoomLib_UpdateHandlerA, RoomLib_ReleaseHandlerA, \
      RoomLib_HandlerANop6 }

#endif
