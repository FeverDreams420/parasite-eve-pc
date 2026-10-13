#ifndef ROOM_M273_SWEEP_H
#define ROOM_M273_SWEEP_H

#include "common.h"
#include "pe1/gte_types.h"
#include "pe1/room_fx.h"

/* Transformed boss sweep point, shared by the sweep and rising-sprite emitters. */
typedef GteShortVector RoomM273SweepPoint;
extern RoomM273SweepPoint g_RoomM273SweepPoint;

/* Drop splash queue at 0x8019AF04: the contact position, then twelve
 * queued landing positions stored as separate x/y/z arrays. */
typedef struct RoomM273DropQueue {
    RoomFxSeed8 hit;  /* also the seed the rising sprite pool copies */
    s16 x[12];
    s16 y[12];
    s16 z[12];
    s16 count;
    /* Sweep attack state (animation 13). */
    s16 loops;        /* 0x52: loops left */
    s16 reserved_54;
    s16 side;         /* 0x56: sweep step table index */
    s16 divisor;      /* 0x58 */
    s16 step;         /* 0x5A */
    s16 target;       /* 0x5C: sweep target angle */
    s16 floor;        /* 0x5E */
    s16 contact;      /* 0x60: contact frame */
    s16 turn;         /* 0x62: turn step */
    u8 in_contact;    /* 0x64 */
    u8 done;
    u8 in_window;
    u8 started;
    u8 *model;        /* 0x68: ground ring model */
} RoomM273DropQueue;

extern RoomM273DropQueue g_RoomM273SweepState;

#endif
