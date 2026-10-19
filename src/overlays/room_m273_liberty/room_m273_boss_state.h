#ifndef ROOM_M273_BOSS_STATE_H
#define ROOM_M273_BOSS_STATE_H

#include "common.h"
#include "pe1/gte_types.h"
#include "pe1/room_fx.h"

/* Boss controller state at 0x8019AE9C. */
typedef struct RoomM273BossState {
    GteShortVector hands[2];
    union {
        struct { s16 hit[3]; s16 hit_flag; };
        RoomFxSeed8 hit_seed;
    };
    s32 base_height;
    s16 landing_x[8]; /* 0x1C: trail landing positions */
    s16 landing_y[8];
    s16 landing_z[8];
    s16 landing_count;
    s16 repeat;
    s16 floor;
    s16 spin_target;
    s16 spin;
    s16 spin_step;
    s16 tick;
    s16 tick_limit;
    u8 done;
    u8 reserved_5D;
    u8 spinning;
} RoomM273BossState;

extern RoomM273BossState g_RoomM273HandTrailState;

#endif /* ROOM_M273_BOSS_STATE_H */
