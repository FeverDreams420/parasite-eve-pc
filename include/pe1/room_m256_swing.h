#ifndef PE1_ROOM_M256_SWING_H
#define PE1_ROOM_M256_SWING_H

#include "common.h"
#include "pe1/field_actor.h"

/* Room m256 swing module (class g_RoomM256SwingClass): once its actor enters mode
 * 0xB, the update callback eases the actor's first rotation override
 * through three cosine segments, angles[i] to angles[i + 1] over
 * durations[i] frames, then closes the object. Script command 0x100 sets
 * the two middle angles, 0x101 the three durations, 0x19 the completion
 * word. */
struct PmSlotHeader;
typedef struct RoomM256Swing RoomM256Swing;
typedef struct RoomM256SwingCycle {
    /* 0x00 */ void (*callback)(void *);
    /* 0x04 */ int *signal;            /* completion word */
    /* 0x08 */ u8 pad08[6];
    /* 0x0E */ u8 started;             /* the current segment is set up */
    /* 0x0F */ u8 pad0F;
    /* 0x10 */ s16 segment;
    /* 0x12 */ u8 pad12[2];
    /* 0x14 */ u16 angles[4];
    /* 0x1C */ u16 durations[3];
    /* 0x22 */ s16 startAngle;         /* angles[segment] */
    /* 0x24 */ s16 sweep;              /* angles[segment + 1] - startAngle */
    /* 0x26 */ s16 tick;
    /* 0x28 */ s16 length;             /* durations[segment] */
} RoomM256SwingCycle;

struct RoomM256Swing {
    /* 0x00 */ u8 status;              /* 4 = closed */
    /* 0x01 */ u8 pad01[7];
    /* 0x08 */ FieldActor *actor;
    /* 0x0C */ RoomM256SwingCycle cycle;
};

PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM256Swing, cycle.started) == 8 + 3*sizeof(void *) + 6,
                  room_m256_swing_started_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM256Swing, cycle.angles) == 8 + 3*sizeof(void *) + 12,
                  room_m256_swing_angles_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomM256Swing, cycle.length) == 8 + 3*sizeof(void *) + 32,
                  room_m256_swing_length_offset);

void RoomM256_SwingNop0(void);
void RoomM256_SwingInit(struct PmSlotHeader *);
int RoomM256_SwingConfigure(struct PmSlotHeader *,int,int,intptr_t,intptr_t,intptr_t);
int RoomM256_SwingNop3(struct PmSlotHeader *);
void RoomM256_SwingAwaitMode(void *);
void RoomM256_SwingStep(void *);
int RoomM256_SwingRelease(struct PmSlotHeader *);
void RoomM256_SwingNop6(void);
#endif
