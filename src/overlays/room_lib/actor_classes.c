/*
 * The room library: five actor classes, driven by RoomLib_HandlerD,
 * RoomLib_HandlerE, RoomLib_HandlerB, RoomLib_HandlerC and RoomLib_HandlerA.
 *
 * 117 room and scene overlays (and 8 more with a longer variant) link the
 * same 53 functions in the same order with the same four jump tables; this
 * unit is that library, compiled into each of them. Each class occupies the
 * seven-method run its room class table lists (no-op, Init, Configure,
 * no-op, Update, ..., Release, no-op), with the class's private states
 * between Update and Release. The zero word between the HandlerE and
 * HandlerB jump tables is the 8-byte alignment of the third table inside
 * this one object.
 */

#include "pe1/battle.h"
#include "pe1/gte_types.h"

#include "room_lib.h"

void RoomLib_HandlerDNop0(void) {
}
void RoomLib_InitHandlerD(PmSlotHeader *slot) {
    RoomEnt *o = (RoomEnt *)slot;
    RoomLibHandlerDState *state = (RoomLibHandlerDState *)&o->sub;

    o->flag3 = 1;
    o->t16 = -1;
    o->t17 = -1;
    o->t18 = -1;
    o->t19 = 3;
    state->value90 = 0x400;
    state->duration = 0x100;
    o->sub.cb = RoomLib_HandlerD;
    o->sub.signal = 0;
    o->active = 0;
    o->t1A = 0;
    state->value88 = 0;
    state->value8C = 0;
    state->targetLink = 0;
    state->motion[0] = 0;
    state->motion[1] = 0;
    state->motion[2] = 0;
    state->progress.word = 0;
    state->range[0] = 0;
    state->range[1] = 0;
    state->range[2] = 0;
    state->rotation[0] = 0;
    state->rotation[1] = 0;
    state->rotation[2] = 0;
    state->secondaryLink = 0;
    state->heading = 0;
    state->target[3] = 0;
    state->rate = 0;
    state->lockY = 0;
    state->copyPosition = 0;
    state->copyRotation = 0;
}
int RoomLib_ConfigureHandlerD(PmSlotHeader *slot, int query, int op, intptr_t arg0, intptr_t arg1, intptr_t arg2) {
    RoomEnt *o = (RoomEnt *)slot;
    RoomLibHandlerDState *state = (RoomLibHandlerDState *)&o->sub;
    FieldActorNode *node;
    switch (op) {
    case 19:
        if (query == 0) {
            o->flag3 = arg0;
        } else {
            *(int *)arg0 = o->flag3;
            *(int *)arg1 = state->progress.half.whole;
        }
        break;
    case 25:
        if (query == 1) {
            state->signal = (int *)arg0;
            *(int *)arg0 = query;
        }
        break;
    case 0:
        state->targetLink = (RoomLink *)g_FieldActorListHead;
        while (state->targetLink != 0) {
            node = (FieldActorNode *)state->targetLink;
            if (node->b0C == arg0 && node->b0D == arg1 &&
                (node->w98 & 0x10) == 0) {
                break;
            }
            state->targetLink = (RoomLink *)
                ((FieldActorNode *)state->targetLink)->next;
        }
        break;
    case 17:
        state->target[0] = arg0;
        state->target[2] = arg2;
        if (arg1 == -1) {
            state->target[1] = D_8009D254->posY.fixed;
        } else {
            state->target[1] = arg1;
        }
        state->targetLink = 0;
        break;
    case 26:
        state->target[0] = arg0;
        state->target[1] = o->link->pos[1];
        state->target[2] = arg1;
        state->target[3] = arg2;
        state->targetLink = 0;
        break;
    case 4:
        state->value88 = arg0;
        break;
    case 2:
        state->value8C = arg0;
        break;
    case 3:
        state->value90 = arg0;
        state->rate = arg1;
        if (arg2 > 0) {
            state->duration = arg2;
        }
        break;
    case 23:
        arg0 = arg0 != 0;
        arg1 = (arg1 != 0) << 1;
        arg2 = (arg2 != 0) << 2;
        state->flags = arg0 | arg1 | arg2;
        break;
    case 10:
        state->variant = arg0;
        state->active = arg1;
        state->mirrorPosition = arg2;
        state->callback = RoomLib_ArmHandlerD;
        break;
    case 11:
        state->optionB = arg0;
        state->optionC = arg1;
        break;
    case 6:
        state->rotation[0] = arg0 >> 16;
        state->rotation[1] = arg1 >> 16;
        state->rotation[2] = arg2 >> 16;
        break;
    case 20:
        state->range[0] = arg0;
        state->range[1] = arg1;
        state->range[2] = arg2;
        break;
    case 21:
        state->secondaryLink = (RoomLink *)g_FieldActorListHead;
        while (state->secondaryLink != 0) {
            node = (FieldActorNode *)state->secondaryLink;
            if (node->b0C == arg0 && node->b0D == arg1 &&
                (node->w98 & 0x10) == 0) {
                break;
            }
            state->secondaryLink = (RoomLink *)
                ((FieldActorNode *)state->secondaryLink)->next;
        }
        state->heading = arg2 & 0xFFF;
        state->reverse = (unsigned int)arg2 >> 31;
        break;
    case 22:
        state->positionX = arg0;
        state->positionZ = arg1;
        state->heading = arg2 & 0xFFF;
        state->reverse = (unsigned int)arg2 >> 31;
        break;
    case 30:
        state->lockY = arg0;
        break;
    case 31:
        state->copyPosition = arg0;
        state->copyRotation = arg1;
        break;
    }
    return 0;
}
int RoomLib_HandlerDNop3(PmSlotHeader *slot) {
    (void)slot;
    return 0;
}
int RoomLib_UpdateHandlerD(PmSlotHeader *slot) {
    RoomEnt *o = (RoomEnt *)slot;
    switch (FieldFx_IsOwnerBusy((struct FieldFxTaskOwner *)o)) {
    case 0:
        if (o->link->variant < 2) {
            return 0;
        }
        o->sub.cb(o);
        return 0;
    case 1:
        RoomLib_ReleaseHandlerD((PmSlotHeader *)o);
    case 2:
        return 0;
    }
    return 0;
}
void RoomLib_ArmHandlerD(RoomEnt *o) {
    signed char c;
    int t;
    unsigned short lo;
    if (((RoomLibHandlerDState *)&o->sub)->mirrorPosition != 0) {
        RoomLib_FxNotify(o->link, &o->sub, 0);
    }
    c = o->t16;
    if (c >= 0) {
        if (c != o->link->variant) {
            return;
        }
    }
    t = o->t17;
    if (t >= 0) {
        RoomLink *l = o->link;
        lo = l->winLo;
        if (t >= l->winHi && t <= lo) {
            o->sub.cb = RoomLib_HandlerD;
        }
    } else {
        o->sub.cb = RoomLib_HandlerD;
    }
}

void RoomLib_HandlerD(RoomEnt *o) {
    RoomEnt *ent = o;
    RoomLibHandlerDState *state = (RoomLibHandlerDState *)&o->sub;
    _Alignas(4) short scratchMemory[128] = {0};
    short *scratch;
    RoomLink *link = o->link;
    char *left;
    int value;
    int product;
    int *rot;
    int result;
    int work;
    int rot_word;

    scratch = scratchMemory;

    if (ent->t1A == 0) {
        ent->t1A = 1;
        ent->w04 = 0;
        link->flags &= 0xFFF3FFFF;
        if (ent->active != 0 && link->target != 0) {
            link->target->flags |= 0x40000000;
        }
        state->anchor[0] = link->pos[0];
        state->anchor[1] = link->pos[1];
        state->anchor[2] = link->pos[2];
        RoomLib_HandlerDTransformTarget(state,
                                        (void *)scratch);
        {
            Vec3Fixed *targetPosition;
            int initial_speed;
            char *left_arg;
            int initial_velocity;

            left = (char *)state->anchor;
            left_arg = left;
            targetPosition = (Vec3Fixed *)state->target;
            initial_speed = FieldFx_GetFixedDistance3D((Vec3Fixed *)left_arg, targetPosition);
            initial_speed = (s32)((u32)initial_speed + (u32)((s32)((u32)state->value8C * 0x70u) >> 21));
            if (initial_speed < 0x148) {
                initial_speed = 0x148;
            }
            initial_velocity = (s32)((u32)(0x5160000 / initial_speed) << 8);
            state->speed = initial_velocity;
            state->progress.word = initial_velocity >> 3;
            FieldFx_FixedVecToLookAngles((Vec3Fixed *)left, targetPosition, state->angles);
            RotMatrixYXZ((GteShortVector *)state->angles, (GteMatrix *)state->matrix);
        }
    }

    {
        int bias = 0x8000;
        GteSineWord sample;

    scratch[0x34] = 0;
    {
        int angle;
        angle = ((s32)((u32)state->progress.word + (u32)bias) >> 16) - 0x400;
        scratch[0x35] = angle;
        sample.part.sin=rsin(angle & 0xFFF);sample.part.cos=rcos(angle & 0xFFF);
        rot = &sample.word;
    }
    result = ((short *)rot)[1];
    scratch[0x15] = 0;
    scratch[0x14] = result;
    rot_word = *rot;
    scratch[0x17] = 0;
    scratch[0x19] = 0;
    scratch[0x1B] = 0;
    scratch[0x00] = 0;
    scratch[0x01] = 0;
    scratch[0x16] = rot_word;
    {
        unsigned short negated;
        negated = *(volatile unsigned short *)&scratch[0x16];
        scratch[0x18] = 0x1000;
        scratch[0x1C] = (unsigned short)scratch[0x14];
        negated = -negated;
        scratch[0x1A] = negated;
    }
    scratch[0x02] = state->value88 >> 12;
    gte_ldrotmatrix((void *)((u8 *)scratch + 0x28));
    gte_ldv0((void *)scratch);
    gte_mvmva();
    gte_stmac(((RoomLibMotionWork *)scratch)->rotated);
    state->motion[0] = (s32)((u32)state->motion[0] + ((u32)((RoomLibMotionWork *)scratch)->rotated[0] << 12));
    state->motion[2] = (s32)((u32)state->motion[2] + ((u32)((RoomLibMotionWork *)scratch)->rotated[2] << 12));

    if (state->progress.word > 0x08000000) {
        ent->flag3 = 2;
    }
    {
        int projected;
        int progress;

        if (((RoomLibMotionWork *)scratch)->rotated[2] >= 0) {
            int position;
            position = state->value88;
            position = (s32)((u32)position + (u32)state->value8C);
            projected = (s32)((u32)position + (u32)bias) >> 16;
            work = projected;
            product = (s32)((u32)work * (u32)(state->speed >> 12));
            state->value88 = position;
            progress = state->progress.word;
        } else {
            int updated;
            projected = state->value88;
            projected = (s32)((u32)projected + (u32)bias) >> 16;
            product = (s32)((u32)projected * (u32)(state->speed >> 12));
            updated = state->value88;
            updated = (s32)((u32)updated - (u32)state->value8C);
            progress = work = state->progress.word;
            state->value88 = updated;
        }
        result = (s32)((u32)progress + ((u32)(product >> 8) << 12));
        state->progress.word = result;
    }
    }

    {
        int bias = 0x8000;
        if (state->progress.half.whole >= 0xC01 && state->motion[0] <= 0) {
            ((RoomLibMotionWork *)scratch)->displacement[0] = 0;
            ((RoomLibMotionWork *)scratch)->displacement[1] = 0;
            ((RoomLibMotionWork *)scratch)->displacement[2] = 0;
            RoomLib_ReleaseHandlerD((PmSlotHeader *)ent);
        } else {
            scratch[0] = (s32)((u32)((s32)((u32)state->motion[0] + (u32)bias) >> 16) *
                                  (u32)state->value90) >> 12;
            scratch[1] = 0;
            scratch[2] = (s32)((u32)state->motion[2] + (u32)bias) >> 16;
            gte_ldrotmatrix(state->matrix);
            gte_ldv0((void *)scratch);
            gte_mvmva();
            gte_stmac(((RoomLibMotionWork *)scratch)->displacement);
        }
    }

    if (state->rate != 0 && state->rate < state->progress.half.whole) {
        RoomLib_ReleaseHandlerD((PmSlotHeader *)ent);
    }
    link->pos[0] = (s32)((u32)state->anchor[0] +
                       ((u32)((RoomLibMotionWork *)scratch)->displacement[0] << 16));
    link->pos[2] = (s32)((u32)state->anchor[2] +
                       ((u32)((RoomLibMotionWork *)scratch)->displacement[2] << 16));
    {
        link->modelTranslation[0] = (s32)((u32)link->pos[0] + 0x8000u) >> 16;
        link->modelTranslation[2] = (s32)((u32)link->pos[2] + 0x8000u) >> 16;
        if (state->copyPosition == 0) {
            value = (s32)((u32)state->anchor[1] +
                    ((u32)((RoomLibMotionWork *)scratch)->displacement[1] << 16));
            link->pos[1] = value;
            link->modelTranslation[1] = (s32)((u32)value + 0x8000u) >> 16;
        }
    }
    if (state->range[0] != 0 &&
        ((RoomLibMotionWork *)scratch)->rotated[2] >= 0) {
        if (ent->w04 >= (unsigned int)state->range[1] &&
            (state->range[2] == 0 ||
             ent->w04 <= (unsigned int)state->range[2])) {
            RoomLib_HandlerDTransformTarget(state,
                                        (void *)scratch);
            scratch[1] = state->angles[1];
            FieldFx_FixedVecToLookAngles((Vec3Fixed *)state->anchor, (Vec3Fixed *)state->target, state->angles);
            state->angles[1] = FieldFx_TurnToward(
                (short)*(volatile unsigned short *)&scratch[1],
                state->angles[1], state->range[0]) & 0xFFF;
            RotMatrixYXZ((GteShortVector *)state->angles, (GteMatrix *)state->matrix);
        }
    }
    if (state->lockY == 0) {
        RoomLib_FxNotify(link, (struct RoomSub *)state, (intptr_t)scratch);
    }
    FieldFx_LoopLinkedAnimation(ent);
}

void RoomLib_FxNotify(RoomLink *arg0, struct RoomSub *arg1, intptr_t scratch) {
    RoomLink *ent = arg0;
    RoomLibHandlerDState *rec = (RoomLibHandlerDState *)arg1;
    if (rec->heading > 0) {
        int *dst = &rec->positionX;
        RoomLink *v = rec->secondaryLink;
        if (v != 0) {
            dst[0] = v->pos[0];
            dst[2] = v->pos[2];
        }
        ent->rotation[1] = FieldFx_TurnToward(
            ent->rotation[1],
            (short)FieldFx_VecToAngle((Vec3Fixed *)dst, (Vec3Fixed *)ent->pos),
            rec->heading) & 0xFFF;
        if ((rec->reverse != 0)
            && (0x07FFFFFF < rec->progress.word)) {
            rec->heading = 0;
            rec->duration = 0;
        }
    } else {
        FieldFx_FixedVecToLookAngles((Vec3Fixed *)ent->pos, (Vec3Fixed *)ent->posMirror, ent->rotation);
        ent->rotation[1] = FieldFx_TurnToward(
            ent->rotation[1],
            ent->rotation[1],
            rec->duration) & 0xFFF;
    }
}
void RoomLib_HandlerDTransformTarget(RoomLibHandlerDState *state, RoomLibMotionWork *work) {
    int angle;
    int height;
    int lo;
    int hi;
    if (state->targetLink != 0) {
        state->target[0] = state->targetLink->pos[0];
        state->target[1] = state->targetLink->pos[1];
        state->target[2] = state->targetLink->pos[2];
    }
    if (state->target[3] != 0) {
        angle = FieldFx_VecToAngle((Vec3Fixed *)state->anchor, (Vec3Fixed *)state->target);
        angle &= 0xFFF;
        hi = rcos(angle);
        work->matrix[1] = 0;
        work->matrix[0] = hi;
        lo = rsin(angle);
        work->matrix[3] = 0;
        work->matrix[5] = 0;
        work->matrix[7] = 0;
        work->input[0] = 0;
        work->input[1] = 0;
        work->matrix[2] = lo;
        work->matrix[4] = 0x1000;
        work->matrix[8] = work->matrix[0];
        work->matrix[6] = -(unsigned short)work->matrix[2];
        height = (s16)((u32)state->target[3] >> 16);
        work->input[2] = height;
        gte_ldrotmatrix(work->matrix);
        gte_ldv0(work->input);
        gte_mvmva();
        gte_stmac(work->rotated);
        state->target[0] = (s32)((u32)state->anchor[0] + ((u32)work->rotated[0] << 16));
        state->target[2] = (s32)((u32)state->anchor[2] + ((u32)work->rotated[2] << 16));
    }
    angle = FieldFx_VecToAngle((Vec3Fixed *)state->anchor, (Vec3Fixed *)state->target);
    angle &= 0xFFF;
    hi = rcos(angle);
    work->matrix[1] = 0;
    work->matrix[0] = hi;
    lo = rsin(angle);
    work->matrix[3] = 0;
    work->matrix[5] = 0;
    work->matrix[7] = 0;
    work->matrix[2] = lo;
    work->matrix[4] = 0x1000;
    work->matrix[8] = work->matrix[0];
    work->matrix[6] = -(unsigned short)work->matrix[2];
    gte_ldrotmatrix(work->matrix);
    gte_ldv0(state->rotation);
    gte_mvmva();
    gte_stmac(work->rotated);
    state->target[0] = (s32)((u32)state->target[0] + ((u32)work->rotated[0] << 16));
    state->target[1] = (s32)((u32)state->target[1] + ((u32)work->rotated[1] << 16));
    state->target[2] = (s32)((u32)state->target[2] + ((u32)work->rotated[2] << 16));
}
int RoomLib_ReleaseHandlerD(PmSlotHeader *slot) {
    RoomEnt *o = (RoomEnt *)slot;
    struct RoomSub *s = &o->sub;
    o->state = 4;
    o->flag3 = 0;
    if (((RoomLibHandlerDState *)&o->sub)->copyRotation == 0) {
        if (o->active != 0) {
            if (o->link->target != 0) {
                *o->link->target->state = 4;
            }
        }
    } else {
        if (o->link->target != 0) {
            unsigned char *state = o->link->target->state;
            if (*state == 1) {
                *state = 4;
            }
        }
    }
    if (s->signal != 0) {
        *s->signal = 0;
    }
    return 0;
}
void RoomLib_HandlerDNop6(void) {
}

void RoomLib_HandlerENop0(void) {
}
void RoomLib_InitHandlerE(PmSlotHeader *slot) {
    RoomEnt *o = (RoomEnt *)slot;
    RoomLibHandlerEState *state = (RoomLibHandlerEState *)&o->sub;

    o->flag3 = 1;
    o->t16 = -1;
    o->t17 = -1;
    o->t18 = -1;
    o->t19 = 3;
    state->speed = 0x10000;
    o->sub.cb = RoomLib_HandlerE;
    o->sub.signal = 0;
    o->active = 0;
    o->t1A = 0;
    state->localOffset[0] = 0;
    state->localOffset[1] = 0;
    state->localOffset[2] = 0;
    state->targetLink = 0;
    state->secondaryLink = 0;
    state->secondaryHeading = 0;
    state->mirrorPosition = 0;
    state->frameLimit = 0;
    state->frame = 0;
    state->phase = 0;
    state->phaseFrame[0] = 0;
    state->lockY = 0;
    state->copyPosition = 0;
}
int RoomLib_ConfigureHandlerE(PmSlotHeader *slot, int query, int op, intptr_t arg0, intptr_t arg1, intptr_t arg2) {
    RoomEnt *o = (RoomEnt *)slot;
    RoomLibHandlerEState *state = (RoomLibHandlerEState *)&o->sub;
    FieldActorNode *node;
    switch (op) {
    case 19:
        if (query == 0) {
            o->flag3 = arg0;
        } else {
            *(int *)arg0 = o->flag3;
        }
        break;
    case 25:
        if (query == 1) {
            state->signal = (int *)arg0;
            *(int *)arg0 = query;
        }
        break;
    case 0:
        state->targetLink = (RoomLink *)g_FieldActorListHead;
        while (state->targetLink != 0) {
            node = (FieldActorNode *)state->targetLink;
            if (node->b0C == arg0 && node->b0D == arg1 &&
                (node->w98 & 0x10) == 0) {
                break;
            }
            state->targetLink = (RoomLink *)
                ((FieldActorNode *)state->targetLink)->next;
        }
        break;
    case 17:
        state->target[0] = arg0;
        state->target[2] = arg2;
        if (arg1 == -1) {
            state->target[1] = D_8009D254->posY.fixed;
        } else {
            state->target[1] = arg1;
        }
        state->targetLink = 0;
        break;
    case 4:
        state->speed = arg0;
        state->eased = arg1;
        state->duration = 0;
        break;
    case 6:
        state->localOffset[0] = arg0;
        state->localOffset[1] = arg1;
        state->localOffset[2] = arg2;
        break;
    case 15:
        state->duration = arg0;
        state->eased = arg1;
        break;
    case 21:
        state->secondaryLink = (RoomLink *)g_FieldActorListHead;
        while (state->secondaryLink != 0) {
            node = (FieldActorNode *)state->secondaryLink;
            if (node->b0C == arg0 && node->b0D == arg1 &&
                (node->w98 & 0x10) == 0) {
                break;
            }
            state->secondaryLink = (RoomLink *)
                ((FieldActorNode *)state->secondaryLink)->next;
        }
        state->secondaryHeading = arg2;
        break;
    case 22:
        state->secondary[0] = arg0;
        state->secondary[2] = arg1;
        state->secondaryHeading = arg2;
        break;
    case 23:
        arg0 = arg0 != 0;
        arg1 = (arg1 != 0) << 1;
        arg2 = (arg2 != 0) << 2;
        state->flags = arg0 | arg1 | arg2;
        break;
    case 10:
        state->variant = arg0;
        state->active = arg1;
        state->mirrorPosition = arg2;
        state->callback = RoomLib_ArmHandlerE;
        break;
    case 11:
        state->optionB = arg0;
        state->optionC = arg1;
        break;
    case 24:
        if (query == 1) {
            *(int *)arg0 = state->heading;
        }
        break;
    case 27:
        state->phaseFrame[1] = arg0;
        state->phaseFrame[2] = arg1;
        state->phaseFrame[3] = arg2;
        if (arg0 != -1) {
            if (arg1 != -1) {
                if (arg2 == -1) {
                    state->frameLimit = 2;
                } else {
                    state->frameLimit = 3;
                }
            } else {
                state->frameLimit = 1;
            }
        } else {
            state->frameLimit = 0;
        }
        break;
    case 31:
        state->lockY = arg0;
        break;
    case 32:
        state->copyPosition = arg0;
        break;
    }
    return 0;
}
int RoomLib_HandlerENop3(PmSlotHeader *slot) {
    (void)slot;
    return 0;
}
int RoomLib_UpdateHandlerE(PmSlotHeader *slot) {
    RoomEnt *o = (RoomEnt *)slot;
    switch (FieldFx_IsOwnerBusy((struct FieldFxTaskOwner *)o)) {
    case 0:
        if (o->link->variant < 2) {
            return 0;
        }
        o->sub.cb(o);
        return 0;
    case 1:
        RoomLib_ReleaseHandlerE((PmSlotHeader *)o);
    case 2:
        return 0;
    }
    return 0;
}
void RoomLib_ArmHandlerE(RoomEnt *o) {
    struct RoomSub *s = &o->sub;
    signed char c;
    int t;
    unsigned short lo;
    if (((RoomLibHandlerEState *)&o->sub)->mirrorPosition != 0) {
        RoomLib_HandlerESteerToward((void *)o->link, (void *)s);
    }
    c = o->t16;
    if (c >= 0) {
        if (c != o->link->variant) {
            return;
        }
    }
    t = o->t17;
    if (t >= 0) {
        RoomLink *l = o->link;
        lo = l->winLo;
        if (t >= l->winHi && t <= lo) {
            o->sub.cb = RoomLib_HandlerE;
        }
    } else {
        o->sub.cb = RoomLib_HandlerE;
    }
}
void RoomLib_HandlerE(RoomEnt *o) {
    RoomLink *link = o->link;
    RoomLink *targetLink;
    RoomLibHandlerEState *state = (RoomLibHandlerEState *)&o->sub;
    _Alignas(4) short scratchMemory[128] = {0};
        volatile short *scratch = scratchMemory;
    int remain;
    int scale;
    int angle;
    int rotWord;
    int localX;
    int localZ;
    int rotY;
    short rotX;
    short negRotY;
    short *rotBase;
    short *rot;
    if (o->t1A == 0) {
        ((RoomLibHandlerEState *)&o->sub)->phase = 1;
        o->t1A = 1;
        link->flags &= 0xFFF3FFFF;
        if (o->active != 0 && link->target != 0) {
            link->target->flags |= 0x40000000;
        }
        state->start[0] = link->pos[0];
        state->start[1] = link->pos[1];
        state->start[2] = link->pos[2];
        targetLink = state->targetLink;
        if (targetLink != 0) {
            state->target[0] = targetLink->pos[0];
            state->target[1] = targetLink->pos[1];
            state->target[2] = targetLink->pos[2];
        }
        angle = FieldFx_VecToAngle((Vec3Fixed *)state->start, (Vec3Fixed *)state->target) & 0xFFF;
        {
            GteSineWord sample={.part={rsin(angle),rcos(angle)}};
            rot = sample.half;
            rotY = rot[1];
            scratch[21] = 0;
            scratch[20] = rotY;
            rotWord = *(int *)rot;
            rotX = scratch[20];
        }
        scratch[23] = 0;
        scratch[25] = 0;
        scratch[27] = 0;
        scratch[22] = rotWord;
        negRotY = scratch[22];
        scratch[24] = 0x1000;
        scratch[28] = rotX;
        scratch[26] = -negRotY;
        localX = state->localOffset[0];
        scratch[1] = 0;
        scratch[0] = localX >> 12;
        localZ = state->localOffset[2];
        do {
            scratch[2] = localZ >> 12;
            gte_ldrotmatrix((void *)((u8 *)scratch + 0x28));
        } while (0);
        gte_ldv0((void *)scratch);
        gte_mvmva();
        gte_stmac((void *)&scratch[4]);
        state->target[0] = (s32)((u32)state->target[0] + ((u32)*(int *)&scratch[4] << 12));
        state->target[1] = (s32)((u32)state->target[1] + ((u32)*(int *)&scratch[6] << 12));
        state->target[2] = (s32)((u32)state->target[2] + ((u32)*(int *)&scratch[8] << 12));
        if (state->duration == 0) {
            state->duration = (s32)((u32)((s32)((u32)FieldFx_GetFixedDistance3D((Vec3Fixed *)state->start, (Vec3Fixed *)state->target) << 16) / state->speed) + 1u);
        }
        state->delta[0] = (s32)((u32)state->target[0] - (u32)state->start[0] + 0x800u) >> 12;
        state->delta[1] = (s32)((u32)state->target[1] - (u32)state->start[1] + 0x800u) >> 12;
        state->delta[2] = (s32)((u32)state->target[2] - (u32)state->start[2] + 0x800u) >> 12;
        state->heading = ratan2(state->delta[0], state->delta[2]);
    }
    scratch[52] = 0;
    remain = state->phase - state->phaseFrame[state->frame];
    if (remain < 0) {
        remain = 0;
    }
    if (state->duration < remain) {
        remain = state->duration;
    }
    if (state->eased != 0) {
        scale = 0x1000 - rcos(((s32)((u32)remain << 11) / state->duration) & 0xFFF);
        link->pos[0] = (s32)((u32)state->start[0] + ((u32)((s32)((u32)state->delta[0] * (u32)scale + 0x1000u) >> 13) << 12));
        if (state->lockY == 0) {
            link->pos[1] = (s32)((u32)state->start[1] + ((u32)((s32)((u32)state->delta[1] * (u32)scale + 0x1000u) >> 13) << 12));
        }
        link->pos[2] = (s32)((u32)state->start[2] + ((u32)((s32)((u32)state->delta[2] * (u32)scale + 0x1000u) >> 13) << 12));
    } else {
        link->pos[0] = (s32)((u32)state->start[0] + ((u32)((s32)((u32)state->delta[0] * (u32)remain) / state->duration) << 12));
        if (state->lockY == 0) {
            link->pos[1] = (s32)((u32)state->start[1] + ((u32)((s32)((u32)state->delta[1] * (u32)remain) / state->duration) << 12));
        }
        link->pos[2] = (s32)((u32)state->start[2] + ((u32)((s32)((u32)state->delta[2] * (u32)remain) / state->duration) << 12));
    }
    if (state->copyPosition != 0) {
        link->posMirror[0] = link->pos[0];
        link->posMirror[1] = link->pos[1];
        link->posMirror[2] = link->pos[2];
        link->posMirror[3] = link->pos[3];
    }
    if (remain >= state->duration && state->frame >= state->frameLimit) {
        RoomLib_ReleaseHandlerE((PmSlotHeader *)o);
    } else {
        state->frame++;
        if (state->frameLimit < state->frame) {
            state->frame = 0;
            state->phase++;
        }
    }
    RoomLib_HandlerESteerToward(link, state);
    FieldFx_LoopLinkedAnimation(o);
}
void RoomLib_HandlerESteerToward(RoomLink *entArg, RoomLibHandlerEState *recArg) {
    RoomLink *ent = (RoomLink *)entArg;
    RoomLibHandlerEState *rec = (RoomLibHandlerEState *)recArg;
    if (rec->secondaryHeading > 0) {
        int *dst = rec->secondary;
        RoomLink *v = rec->secondaryLink;
        if (v != 0) {
            dst[0] = v->pos[0];
            dst[2] = v->pos[2];
        }
        ent->rotation[1] = FieldFx_TurnToward(
            ent->rotation[1],
            (short)FieldFx_VecToAngle((Vec3Fixed *)dst, (Vec3Fixed *)ent->pos),
            rec->secondaryHeading);
    }
}
int RoomLib_ReleaseHandlerE(PmSlotHeader *slot) {
    RoomEnt *o = (RoomEnt *)slot;
    struct RoomSub *s = &o->sub;
    o->state = 4;
    o->flag3 = 0;
    if (o->active != 0) {
        RoomLink *l = o->link;
        if (l->target != 0) {
            *l->target->state = 4;
        }
    }
    if (s->signal != 0) {
        *s->signal = 0;
    }
    return 0;
}
void RoomLib_HandlerENop6(void) {
}

void RoomLib_HandlerBNop0(void) {
}
void RoomLib_InitHandlerB(PmSlotHeader *slot) {
    RoomEnt *o = (RoomEnt *)slot;
    RoomLibHandlerBState *state = (RoomLibHandlerBState *)&o->sub;
    RoomLink *l;
    o->flag3 = 1;
    o->t17 = -1;
    o->t16 = -1;
    o->t19 = 7;
    state->mode = 0x10000;
    l = o->link;
    o->sub.signal = 0;
    o->active = 0;
    o->t1A = 0;
    state->speed = 0;
    state->acceleration = 0;
    state->duration = 0;
    state->phaseValue = 0;
    state->rate = 0;
    state->localOffset[0] = 0;
    state->localOffset[1] = 0;
    state->localOffset[2] = 0;
    state->heading = 0;
    state->targetLink = 0;
    o->sub.cb = RoomLib_HandlerB;
    state->secondaryLink = 0;
    state->secondaryHeading = 0;
    l->vel[0] = 0;
    l->vel[1] = 0;
    l->vel[2] = 0;
    l->accel[0] = 0;
    l->accel[1] = 0;
    l->accel[2] = 0;
    l->move[0] = 0;
    l->move[1] = 0;
    l->move[2] = 0;
}
int RoomLib_ConfigureHandlerB(PmSlotHeader *slot, int query, int op, intptr_t arg0, intptr_t arg1, intptr_t arg2) {
    RoomEnt *o = (RoomEnt *)slot;
    RoomLibHandlerBState *state = (RoomLibHandlerBState *)&o->sub;
    FieldActorNode *node;
    switch (op) {
    case 19:
        if (query == 0) {
            o->flag3 = arg0;
        } else {
            *(int *)arg0 = o->flag3;
        }
        break;
    case 25:
        if (query == 1) {
            state->signal = (int *)arg0;
            *(int *)arg0 = query;
        }
        break;
    case 4:
        state->speed = arg0;
        state->acceleration = arg1;
        break;
    case 2:
        state->duration = arg0;
        break;
    case 12:
        state->phaseValue = arg0;
        break;
    case 13:
        state->mode = arg0;
        break;
    case 14:
        state->rate = arg1;
        break;
    case 6:
        state->localOffset[0] = arg0;
        state->localOffset[1] = arg1;
        state->localOffset[2] = arg2;
        break;
    case 23:
        arg0 = arg0 != 0;
        arg1 = (arg1 != 0) << 1;
        arg2 = (arg2 != 0) << 2;
        state->flags = arg0 | arg1 | arg2;
        break;
    case 10:
        state->variant = arg0;
        state->active = arg1;
        state->callback = RoomLib_ArmHandlerB;
        break;
    case 11:
        state->optionB = arg0;
        state->optionC = arg1;
        break;
    case 16:
        state->heading = arg0;
        break;
    case 18:
        if (o->flag3 == 2) {
            state->callback = RoomLib_AdvanceArcToTarget;
            state->phaseValue = arg0;
            o->flag3 = 3;
        }
        break;
    case 0:
        state->targetLink = (RoomLink *)g_FieldActorListHead;
        while (state->targetLink != 0) {
            node = (FieldActorNode *)state->targetLink;
            if (node->b0C == arg0 && node->b0D == arg1 &&
                (node->w98 & 0x10) == 0) {
                break;
            }
            state->targetLink = (RoomLink *)
                ((FieldActorNode *)state->targetLink)->next;
        }
        break;
    case 17:
        state->target[0] = arg0;
        state->target[2] = arg2;
        if (arg1 != -1) {
            state->target[1] = arg1;
        } else {
            state->target[1] = D_8009D254->posY.fixed;
        }
        state->targetLink = 0;
        break;
    case 21:
        state->secondaryLink = (RoomLink *)g_FieldActorListHead;
        while (state->secondaryLink != 0) {
            node = (FieldActorNode *)state->secondaryLink;
            if (node->b0C == arg0 && node->b0D == arg1 &&
                (node->w98 & 0x10) == 0) {
                break;
            }
            state->secondaryLink = (RoomLink *)
                ((FieldActorNode *)state->secondaryLink)->next;
        }
        state->secondaryHeading = arg2;
        break;
    case 22:
        state->secondaryX = arg0;
        state->secondaryZ = arg1;
        state->secondaryHeading = arg2;
        break;
    }
    return 0;
}
int RoomLib_HandlerBNop3(PmSlotHeader *slot) {
    (void)slot;
    return 0;
}
int RoomLib_UpdateHandlerB(PmSlotHeader *slot) {
    RoomEnt *obj = (RoomEnt *)slot;
    obj->sub.cb(obj);
    return 0;
}
void RoomLib_ArmHandlerB(RoomEnt *o) {
    signed char c = o->t16;
    int t;
    unsigned short lo;
    if (c >= 0) {
        if (c != o->link->variant) {
            return;
        }
    }
    t = o->t17;
    if (t >= 0) {
        RoomLink *l = o->link;
        lo = l->winLo;
        if (t > l->winHi && t <= lo) {
            o->sub.cb = RoomLib_HandlerB;
        }
    } else {
        o->sub.cb = RoomLib_HandlerB;
    }
}

#define ROOMLIB_HANDLER_B_GTE_LOAD_VECTOR(base) \
    do { \
        gte_ldv0(((RoomLibArcWork *)(base))->input); \
    } while (0)

static inline int RoomLib_ArcPhaseSpeed(int phase) {
    return phase >> 12;
}

void RoomLib_HandlerB(RoomEnt *obj) {
    RoomEnt *self = obj;
    RoomLibHandlerBState *state = (RoomLibHandlerBState *)&self->sub;
    RoomLink *link = self->link;
    int previousValue;
    int a0;
    RoomLibArcWork nativeScratch = {0};
    RoomLibArcWork *scratch;
    int result;

    scratch = &nativeScratch;

    if (self->t1A == 0) {
        int v0;
        int v1;
        GteSineWord *entry;
        GteSineWord sample;
        unsigned short matrixComponent;
        BattleEntity *player;

        self->t1A = 1;
        state->finished = 0;
        link->flags = (link->flags & 0xFFF3FFFF) | 2;
        if (self->active != 0 && link->target != 0) {
            link->target->flags |= 0x40000000;
        }
        v1 = (unsigned short)state->rate;
        v0 = (unsigned short)link->rotation[1];
        v1 += 0x800;
        v0 += v1;
        v0 &= 0xFFF;
        sample.part.sin=rsin(v0);sample.part.cos=rcos(v0);
        entry = &sample;
        v1 = entry->part.cos;
        state->matrix[1] = 0;
        ((volatile RoomLibHandlerBState *)state)->matrix[0] = v1;
        a0 = entry->word;
        v1 = (unsigned short)state->matrix[0];
        v0 = 0x1000;
        ((volatile RoomLibHandlerBState *)state)->matrix[4] = 0x1000;
        ((volatile RoomLibHandlerBState *)state)->matrix[2] = a0;
        matrixComponent = state->matrix[2];
        state->matrix[3] = 0;
        *(volatile short *)&state->matrix[5] = 0;
        *(volatile short *)&state->matrix[7] = 0;
        state->matrix[8] = v1;
        player = D_8009D254;
        matrixComponent = -matrixComponent;
        state->matrix[6] = matrixComponent;
        v0 = player->posY.parts.integer;
        state->floorY = v0;
    }

    scratch->landed = 0;
    if (state->finished == 0) {
        switch (FieldFx_IsOwnerBusy((struct FieldFxTaskOwner *)self)) {
        case 0:
            break;
        case 1:
            state->finished = 1;
            state->horizontalVelocity[0] = 0;
            state->horizontalVelocity[1] = 0;
            if (state->signal != 0) {
                *state->signal = 2;
            }
            return;
        case 2:
            return;
        default:
            break;
        }
    }

    if (link->variant < 2) {
        return;
    }
    {
        int y = link->pos[1];
        int phase = state->phaseValue;
        int speedRaw;
        int speed;
        int phaseAgain;
        int nextY;
        scratch->input[0] = 0;
        scratch->input[1] = 0;
        speedRaw = state->speed;
        speed = speedRaw >> 12;
        previousValue = y;
        scratch->input[2] = speed;
        phaseAgain = state->phaseValue;
        nextY = previousValue - phase;
        if (phaseAgain < 0 && (nextY >> 16) >= state->floorY) {
            int numerator;
            int divisor;

            nextY = state->floorY << 16;
            numerator = (short)speed;
            divisor = (previousValue - nextY) >> 12;
            numerator *= divisor;
            divisor = RoomLib_ArcPhaseSpeed(phaseAgain);
            result = numerator;
            scratch->input[2] = result / divisor;
            scratch->landed = 1;
        }
        link->pos[1] = nextY;
    }

    gte_ldrotmatrix(state->matrix);
    ROOMLIB_HANDLER_B_GTE_LOAD_VECTOR(scratch);
    gte_mvmva();
    gte_stmac(scratch->point);
    link->pos[0] += scratch->point[0] << 12;
    link->pos[2] += scratch->point[2] << 12;

    previousValue = state->phaseValue;
    {
        int enabled = state->heading;
        a0 = previousValue - state->mode;
        state->phaseValue = a0;
        if (enabled != 0) {
            if ((previousValue ^ a0) < 0) {
                state->callback = RoomLib_HandlerBPhase;
                state->started = 0;
                self->flag3 = 2;
            }
        }
    }
    {
        int speed = state->speed + state->duration;
        int limit = state->acceleration;
        state->speed = speed;
        if (limit != 0 && speed > limit) {
            state->speed = limit;
        }
    }
    if (state->secondaryHeading > 0) {
        RoomLink *secondary = state->secondaryLink;
        if (secondary != 0) {
            state->secondaryX = secondary->pos[0];
            state->secondaryZ = secondary->pos[2];
        }
        result = FieldFx_VecToAngle((Vec3Fixed *)&state->secondaryX, (Vec3Fixed *)link->pos);
        link->rotation[1] = FieldFx_TurnToward(
            link->rotation[1], (short)result, state->secondaryHeading);
    }
    FieldFx_LoopLinkedAnimation(self);
    if (scratch->landed != 0) {
        RoomLib_ReleaseHandlerB((PmSlotHeader *)self);
    }
}

void RoomLib_HandlerBPhase(RoomEnt *obj) {
}

void RoomLib_AdvanceArcToTarget(RoomEnt *obj) {
    RoomEnt *self = obj;
    char *state = (char *)&self->sub;
    RoomLink *link = self->link;
    _Alignas(4) short scratchMemory[128] = {0};
    short *scratch = scratchMemory;
    int angle;
    int y;

    if (self->t1A == 0) {
        int count;
        int step;
        int targetY;
        int accel;
        self->t1A = 1;
        ((RoomLibHandlerBState *)state)->finished = 0;
        if (((RoomLibHandlerBState *)state)->targetLink != 0) {
            ((RoomLibArcWork *)scratch)->point[0] =
                ((RoomLibHandlerBState *)state)->targetLink->pos[0];
            ((RoomLibArcWork *)scratch)->point[1] =
                ((RoomLibHandlerBState *)state)->targetLink->pos[1];
            ((RoomLibArcWork *)scratch)->point[2] =
                ((RoomLibHandlerBState *)state)->targetLink->pos[2];
        } else {
            ((RoomLibArcWork *)scratch)->point[0] = ((RoomLibHandlerBState *)state)->target[0];
            ((RoomLibArcWork *)scratch)->point[1] = ((RoomLibHandlerBState *)state)->target[1];
            ((RoomLibArcWork *)scratch)->point[2] = ((RoomLibHandlerBState *)state)->target[2];
        }

        angle = FieldFx_VecToAngle((Vec3Fixed *)link->pos, (Vec3Fixed *)&scratch[20]) & 0xFFF;
        {
            GteSineWord sample={.part={rsin(angle),rcos(angle)}};
            int *entry = &sample.word;
            int hi = ((short *)entry)[1];
            int lo;
            int savedHi;
            scratch[1] = 0;
            *(volatile short *)&scratch[0] = hi;
            lo = *entry;
            savedHi = (unsigned short)((RoomLibArcWork *)scratch)->matrix[0];
            scratch[3] = 0;
            scratch[5] = 0;
            scratch[7] = 0;
            scratch[2] = lo;
            scratch[4] = 0x1000;
            scratch[8] = savedHi;
            scratch[6] = -(unsigned short)((RoomLibArcWork *)scratch)->matrix[2];
        }
        scratch[16] = ((RoomLibHandlerBState *)state)->localOffset[0] >> 12;
        scratch[17] = 0;
        scratch[18] = ((RoomLibHandlerBState *)state)->localOffset[2] >> 12;
        gte_ldrotmatrix((void *)scratch);
        gte_ldv0((void *)((u8 *)scratch + 0x20));
        gte_mvmva();
        gte_stmac((void *)((u8 *)scratch + 0x38));

        ((RoomLibHandlerBState *)state)->target[0] = (s32)((u32)((RoomLibArcWork *)scratch)->point[0] +
                            ((u32)((RoomLibArcWork *)scratch)->offset[0] << 12));
        ((RoomLibHandlerBState *)state)->target[1] = (s32)((u32)((RoomLibArcWork *)scratch)->point[1] +
                            ((u32)((RoomLibArcWork *)scratch)->offset[1] << 12));
        ((RoomLibHandlerBState *)state)->target[2] = (s32)((u32)((RoomLibArcWork *)scratch)->point[2] +
                            ((u32)((RoomLibArcWork *)scratch)->offset[2] << 12));

        count = 0;
        step = ((RoomLibHandlerBState *)state)->phaseValue;
        accel = ((RoomLibHandlerBState *)state)->mode;
        targetY = ((RoomLibHandlerBState *)state)->target[1];
        y = link->pos[1];
        do {
            y += step;
            step += accel;
            count++;
        } while (y < targetY);
        {
            int xVelocity = (s32)((u32)((RoomLibHandlerBState *)state)->target[0] - (u32)link->pos[0]) / count;
            int zVelocity;
            *(volatile int *)&((RoomLibHandlerBState *)state)->horizontalVelocity[0] = xVelocity;
            zVelocity = (s32)((u32)((RoomLibHandlerBState *)state)->target[2] - (u32)link->pos[2]) / count;
            *(volatile int *)&((RoomLibHandlerBState *)state)->horizontalVelocity[1] = zVelocity;
        }
    }

    ((RoomLibArcWork *)scratch)->landed = 0;
    if (((RoomLibHandlerBState *)state)->finished == 0) {
        switch (FieldFx_IsOwnerBusy((struct FieldFxTaskOwner *)self)) {
        case 0:
            break;
        case 1:
            ((RoomLibHandlerBState *)state)->finished = 1;
            ((RoomLibHandlerBState *)state)->horizontalVelocity[0] = 0;
            ((RoomLibHandlerBState *)state)->horizontalVelocity[1] = 0;
            if (((RoomLibHandlerBState *)state)->signal != 0) {
                *((RoomLibHandlerBState *)state)->signal = 2;
            }
            break;
        case 2:
            return;
        default:
            break;
        }
    }

    if (link->variant < 2) {
        return;
    }
    link->pos[0] += ((RoomLibHandlerBState *)state)->horizontalVelocity[0];
    link->pos[1] += ((RoomLibHandlerBState *)state)->phaseValue;
    link->pos[2] += ((RoomLibHandlerBState *)state)->horizontalVelocity[1];
    ((RoomLibHandlerBState *)state)->phaseValue += ((RoomLibHandlerBState *)state)->mode;
    if (Pe1Fixed_Integer(&link->pos[1]) >= Pe1Fixed_Integer(&((RoomLibHandlerBState *)state)->target[1])) {
        ((RoomLibArcWork *)scratch)->landed = 1;
        link->pos[0] = ((RoomLibHandlerBState *)state)->target[0];
        link->pos[1] = ((RoomLibHandlerBState *)state)->target[1];
        link->pos[2] = ((RoomLibHandlerBState *)state)->target[2];
    }
    FieldFx_LoopLinkedAnimation(self);
    if (((RoomLibArcWork *)scratch)->landed != 0) {
        RoomLib_ReleaseHandlerB((PmSlotHeader *)self);
    }
}

int RoomLib_ReleaseHandlerB(PmSlotHeader *slot) {
    RoomEnt *o = (RoomEnt *)slot;
    struct RoomSub *s = &o->sub;
    if (o->active != 0) {
        RoomLink *l = o->link;
        if (l->target != 0) {
            *l->target->state = 4;
        }
    }
    o->flag3 = 0;
    o->state = 4;
    if (s->signal != 0) {
        *s->signal = 0;
    }
    return 0;
}
void RoomLib_HandlerBNop6(void) {
}

void RoomLib_HandlerCNop0(void) {
}
void RoomLib_InitHandlerC(PmSlotHeader *slot) {
    RoomEnt *o = (RoomEnt *)slot;
    RoomLink *l;
    RoomLibHandlerCState *state=(RoomLibHandlerCState *)&o->sub;
    o->flag3 = 1;
    o->t17 = -1;
    o->t16 = -1;
    o->t19 = 7;
    state->mode = 0x10000;
    state->rate = 0x10;
    l = o->link;
    o->sub.signal = 0;
    o->active = 0;
    o->t1A = 0;
    state->targetLink = 0;
    state->localOffset[0] = 0;
    state->localOffset[1] = 0;
    state->localOffset[2] = 0;
    state->heading = 0;
    o->sub.cb = RoomLib_HandlerC;
    state->secondaryLink = 0;
    state->secondaryHeading = 0;
    l->vel[0] = 0;
    l->vel[1] = 0;
    l->vel[2] = 0;
    l->accel[0] = 0;
    l->accel[1] = 0;
    l->accel[2] = 0;
    l->move[0] = 0;
    l->move[1] = 0;
    l->move[2] = 0;
}
int RoomLib_ConfigureHandlerC(PmSlotHeader *slot, int query, int op, intptr_t arg0, intptr_t arg1, intptr_t arg2) {
    RoomEnt *o = (RoomEnt *)slot;
    RoomLibHandlerCState *state = (RoomLibHandlerCState *)&o->sub;
    FieldActorNode *node;
    switch (op) {
    case 19:
        if (query == 0) {
            o->flag3 = arg0;
        } else {
            *(int *)arg0 = o->flag3;
        }
        break;
    case 25:
        if (query == 1) {
            state->signal = (int *)arg0;
            *(int *)arg0 = query;
        }
        break;
    case 0:
        state->targetLink = (RoomLink *)g_FieldActorListHead;
        while (state->targetLink != 0) {
            node = (FieldActorNode *)state->targetLink;
            if (node->b0C == arg0 && node->b0D == arg1 &&
                (node->w98 & 0x10) == 0) {
                break;
            }
            state->targetLink = (RoomLink *)
                ((FieldActorNode *)state->targetLink)->next;
        }
        break;
    case 17:
        state->target[0] = arg0;
        state->target[2] = arg2;
        if (arg1 == -1) {
            state->target[1] = D_8009D254->posY.fixed;
        } else {
            state->target[1] = arg1;
        }
        state->targetLink = 0;
        break;
    case 15:
        state->rate = arg0;
        break;
    case 13:
        state->mode = arg0;
        break;
    case 6:
        state->localOffset[0] = arg0;
        state->localOffset[1] = arg1;
        state->localOffset[2] = arg2;
        break;
    case 23:
        arg0 = arg0 != 0;
        arg1 = (arg1 != 0) << 1;
        arg2 = (arg2 != 0) << 2;
        state->flags = arg0 | arg1 | arg2;
        break;
    case 10:
        state->variant = arg0;
        state->active = arg1;
        if (o->flag3 == 1) {
            state->callback = RoomLib_ArmHandlerC;
        }
        break;
    case 11:
        if (o->flag3 == 1) {
            state->optionB = arg0;
            state->optionC = arg1;
        }
        break;
    case 16:
        state->heading = arg0;
        break;
    case 18:
        if (o->flag3 == 2) {
            state->callback = RoomLib_AdvanceArcToTargetY;
            state->phaseValue = arg0;
            o->flag3 = 3;
        }
        break;
    case 21:
        state->secondaryLink = (RoomLink *)g_FieldActorListHead;
        while (state->secondaryLink != 0) {
            node = (FieldActorNode *)state->secondaryLink;
            if (node->b0C == arg0 && node->b0D == arg1 &&
                (node->w98 & 0x10) == 0) {
                break;
            }
            state->secondaryLink = (RoomLink *)
                ((FieldActorNode *)state->secondaryLink)->next;
        }
        state->secondaryHeading = arg2;
        break;
    case 22:
        state->secondaryX = arg0;
        state->secondaryZ = arg1;
        state->secondaryHeading = arg2;
        break;
    }
    return 0;
}
int RoomLib_HandlerCNop3(PmSlotHeader *slot) {
    (void)slot;
    return 0;
}
int RoomLib_UpdateHandlerC(PmSlotHeader *slot) {
    RoomEnt *obj = (RoomEnt *)slot;
    obj->sub.cb(obj);
    return 0;
}
void RoomLib_ArmHandlerC(RoomEnt *o) {
    signed char c = o->t16;
    int t;
    unsigned short lo;
    if (c >= 0) {
        if (c != o->link->variant) {
            return;
        }
    }
    t = o->t17;
    if (t >= 0) {
        RoomLink *l = o->link;
        lo = l->winLo;
        if (t >= l->winHi && t <= lo) {
            o->sub.cb = RoomLib_HandlerC;
        }
    } else {
        o->sub.cb = RoomLib_HandlerC;
    }
}

void RoomLib_HandlerC(RoomEnt *obj) {
    RoomEnt *self = obj;
    RoomLibHandlerCState *state =
        (RoomLibHandlerCState *)((char *)&self->sub);
    RoomLink *link = self->link;
    _Alignas(4) short scratchMemory[128] = {0};
    short *scratch;
    int angle;
    scratch = scratchMemory;
    if (self->t1A == 0) {
        self->t1A = 1;
        (state)->finished = 0;
        link->flags = (link->flags & 0xFFF3FFFF) | 2;
        if (self->active != 0 && link->target != 0) {
            link->target->flags |= 0x40000000;
        }
        state->phaseValue = (state->mode * (state->rate - 1)) >> 1;
        if (state->targetLink == 0) {
            ((RoomLibArcWork *)scratch)->point[0] = state->target[0];
            ((RoomLibArcWork *)scratch)->point[1] = state->target[1];
            ((RoomLibArcWork *)scratch)->point[2] = state->target[2];
            if (((RoomLibArcWork *)scratch)->point[1] == -1) {
                ((RoomLibArcWork *)scratch)->point[1] = D_8009D254->posY.fixed;
            }
        } else {
            ((RoomLibArcWork *)scratch)->point[0] = state->targetLink->pos[0];
            ((RoomLibArcWork *)scratch)->point[1] = state->targetLink->pos[1];
            ((RoomLibArcWork *)scratch)->point[2] = state->targetLink->pos[2];
        }

        angle = FieldFx_VecToAngle((Vec3Fixed *)link->pos, (Vec3Fixed *)((u8 *)scratch + 0x28)) &
                0xFFF;
        {
            GteSineWord sample={.part={rsin(angle),rcos(angle)}};
            int *entry = &sample.word;
            int hi = ((short *)entry)[1];
            int lo;
            int savedHi;
            scratch[1] = 0;
            *(volatile short *)&scratch[0] = hi;
            lo = *entry;
            savedHi = (unsigned short)((RoomLibArcWork *)scratch)->matrix[0];
            scratch[3] = 0;
            scratch[5] = 0;
            scratch[7] = 0;
            scratch[2] = lo;
            scratch[4] = 0x1000;
            scratch[8] = savedHi;
            scratch[6] = -(unsigned short)((RoomLibArcWork *)scratch)->matrix[2];
        }
        scratch[16] = state->localOffset[0] >> 12;
        scratch[17] = 0;
        scratch[18] = state->localOffset[2] >> 12;
        gte_ldrotmatrix((void *)scratch);
        gte_ldv0(((RoomLibArcWork *)scratch)->input);
        gte_mvmva();
        gte_stmac(((RoomLibArcWork *)scratch)->offset);

        /* GTE offsets and destinations are PS1 words: shift and add
         * with unsigned wrapping before interpreting the signed delta. */
        (state)->arcTarget[0] = (s32)((u32)((RoomLibArcWork *)scratch)->point[0] +
                            ((u32)((RoomLibArcWork *)scratch)->offset[0] << 12));
        (state)->arcTarget[1] = (s32)((u32)((RoomLibArcWork *)scratch)->point[1] +
                            ((u32)((RoomLibArcWork *)scratch)->offset[1] << 12));
        (state)->arcTarget[2] = (s32)((u32)((RoomLibArcWork *)scratch)->point[2] +
                            ((u32)((RoomLibArcWork *)scratch)->offset[2] << 12));
        (state)->horizontalVelocity[0] =
            (s32)((u32)state->arcTarget[0] - (u32)link->pos[0]) / state->rate;
        (state)->horizontalVelocity[1] =
            (s32)((u32)state->arcTarget[2] - (u32)link->pos[2]) / state->rate;
    }

    ((RoomLibArcWork *)scratch)->landed = 0;
    if ((state)->finished == 0) {
        switch (FieldFx_IsOwnerBusy((struct FieldFxTaskOwner *)self)) {
        case 0:
            break;
        case 1:
            (state)->finished = 1;
            (state)->horizontalVelocity[0] = 0;
            (state)->horizontalVelocity[1] = 0;
            if (state->signal != 0) {
                *state->signal = 2;
            }
            break;
        case 2:
            return;
        default:
            break;
        }
    }

    if (link->variant < 2) {
        return;
    }
    link->pos[0] += (state)->horizontalVelocity[0];
    link->pos[1] -= state->phaseValue;
    link->pos[2] += (state)->horizontalVelocity[1];
    {
        int previous = state->phaseValue;
        state->phaseValue = previous - state->mode;
        if (state->heading != 0 && (previous ^ state->phaseValue) < 0) {
            state->callback = RoomLib_HandlerCPhase;
            (state)->started = 0;
            state->localOffset[0] = 0;
            state->localOffset[1] = 0;
            state->localOffset[2] = 0;
            self->flag3 = 2;
        }
    }
    if (state->phaseValue < 0) {
        int y = link->pos[1];
        int targetY = (state)->arcTarget[1];
        int delta = y - targetY;
        if (delta >= 0) {
            link->pos[1] = targetY;
            ((RoomLibArcWork *)scratch)->landed = 1;
        }
    }
    if (state->secondaryHeading > 0) {
        RoomLink *secondary = state->secondaryLink;
        if (secondary != 0) {
            state->secondaryX = secondary->pos[0];
            state->secondaryZ = secondary->pos[2];
        }
        angle = FieldFx_VecToAngle((Vec3Fixed *)&state->secondaryX, (Vec3Fixed *)link->pos);
        link->rotation[1] = FieldFx_TurnToward(
            link->rotation[1], (short)angle, state->secondaryHeading);
    }
    FieldFx_LoopLinkedAnimation(self);
    if (((RoomLibArcWork *)scratch)->landed != 0) {
        RoomLib_ReleaseHandlerC((PmSlotHeader *)self);
    }
}

void RoomLib_HandlerCPhase(RoomEnt *unused) {
}

void RoomLib_AdvanceArcToTargetY(RoomEnt *obj) {
    RoomEnt *self = obj;
    char *state = (char *)&self->sub;
    RoomLink *link = self->link;
    _Alignas(4) short scratchMemory[128] = {0};
    short *scratch = scratchMemory;
    int angle;
    int y;

    if (self->t1A == 0) {
        int count;
        int step;
        int targetY;
        int accel;
        self->t1A = 1;
        ((RoomLibHandlerCState *)state)->finished = 0;
        if (((RoomLibHandlerCState *)state)->targetLink == 0) {
            ((RoomLibArcWork *)scratch)->point[0] = ((RoomLibHandlerCState *)state)->target[0];
            ((RoomLibArcWork *)scratch)->point[1] = ((RoomLibHandlerCState *)state)->target[1];
            ((RoomLibArcWork *)scratch)->point[2] = ((RoomLibHandlerCState *)state)->target[2];
        } else {
            ((RoomLibArcWork *)scratch)->point[0] =
                ((RoomLibHandlerCState *)state)->targetLink->pos[0];
            ((RoomLibArcWork *)scratch)->point[1] =
                ((RoomLibHandlerCState *)state)->targetLink->pos[1];
            ((RoomLibArcWork *)scratch)->point[2] =
                ((RoomLibHandlerCState *)state)->targetLink->pos[2];
        }

        angle = FieldFx_VecToAngle((Vec3Fixed *)link->pos, (Vec3Fixed *)&scratch[20]) & 0xFFF;
        {
            GteSineWord sample={.part={rsin(angle),rcos(angle)}};
            int *entry = &sample.word;
            int hi = ((short *)entry)[1];
            int lo;
            int savedHi;
            scratch[1] = 0;
            *(volatile short *)&scratch[0] = hi;
            lo = *entry;
            savedHi = (unsigned short)((RoomLibArcWork *)scratch)->matrix[0];
            scratch[3] = 0;
            scratch[5] = 0;
            scratch[7] = 0;
            scratch[2] = lo;
            scratch[4] = 0x1000;
            scratch[8] = savedHi;
            scratch[6] = -(unsigned short)((RoomLibArcWork *)scratch)->matrix[2];
        }
        scratch[16] = ((RoomLibHandlerCState *)state)->localOffset[0] >> 12;
        scratch[17] = 0;
        scratch[18] = ((RoomLibHandlerCState *)state)->localOffset[2] >> 12;
        gte_ldrotmatrix((void *)scratch);
        gte_ldv0((void *)((u8 *)scratch + 0x20));
        gte_mvmva();
        gte_stmac((void *)((u8 *)scratch + 0x38));

        ((RoomLibHandlerCState *)state)->arcTarget[0] = (s32)((u32)((RoomLibArcWork *)scratch)->point[0] +
                            ((u32)((RoomLibArcWork *)scratch)->offset[0] << 12));
        ((RoomLibHandlerCState *)state)->arcTarget[1] = (s32)((u32)((RoomLibArcWork *)scratch)->point[1] +
                            ((u32)((RoomLibArcWork *)scratch)->offset[1] << 12));
        ((RoomLibHandlerCState *)state)->arcTarget[2] = (s32)((u32)((RoomLibArcWork *)scratch)->point[2] +
                            ((u32)((RoomLibArcWork *)scratch)->offset[2] << 12));

        count = 0;
        step = ((RoomLibHandlerCState *)state)->phaseValue;
        accel = ((RoomLibHandlerCState *)state)->mode;
        targetY = ((RoomLibHandlerCState *)state)->arcTarget[1];
        y = link->pos[1];
        do {
            y += step;
            step += accel;
            count++;
        } while (y < targetY);
        {
            int xVelocity = (s32)((u32)((RoomLibHandlerCState *)state)->arcTarget[0] - (u32)link->pos[0]) / count;
            int zVelocity;
            *(volatile int *)&((RoomLibHandlerCState *)state)->horizontalVelocity[0] = xVelocity;
            zVelocity = (s32)((u32)((RoomLibHandlerCState *)state)->arcTarget[2] - (u32)link->pos[2]) / count;
            *(volatile int *)&((RoomLibHandlerCState *)state)->horizontalVelocity[1] = zVelocity;
        }
    }

    ((RoomLibArcWork *)scratch)->landed = 0;
    if (((RoomLibHandlerCState *)state)->finished == 0) {
        switch (FieldFx_IsOwnerBusy((struct FieldFxTaskOwner *)self)) {
        case 0:
            break;
        case 1:
            ((RoomLibHandlerCState *)state)->finished = 1;
            ((RoomLibHandlerCState *)state)->horizontalVelocity[0] = 0;
            ((RoomLibHandlerCState *)state)->horizontalVelocity[1] = 0;
            if (((RoomLibHandlerCState *)state)->signal != 0) {
                *((RoomLibHandlerCState *)state)->signal = 2;
            }
            break;
        case 2:
            return;
        default:
            break;
        }
    }

    if (link->variant < 2) {
        return;
    }
    link->pos[0] += ((RoomLibHandlerCState *)state)->horizontalVelocity[0];
    link->pos[1] += ((RoomLibHandlerCState *)state)->phaseValue;
    link->pos[2] += ((RoomLibHandlerCState *)state)->horizontalVelocity[1];
    ((RoomLibHandlerCState *)state)->phaseValue += ((RoomLibHandlerCState *)state)->mode;
    if (Pe1Fixed_Integer(&link->pos[1]) >= Pe1Fixed_Integer(&((RoomLibHandlerCState *)state)->arcTarget[1])) {
        ((RoomLibArcWork *)scratch)->landed = 1;
        link->pos[1] = ((RoomLibHandlerCState *)state)->arcTarget[1];
    }
    FieldFx_LoopLinkedAnimation(self);
    if (((RoomLibArcWork *)scratch)->landed != 0) {
        RoomLib_ReleaseHandlerC((PmSlotHeader *)self);
    }
}

int RoomLib_ReleaseHandlerC(PmSlotHeader *slot) {
    RoomEnt *o = (RoomEnt *)slot;
    struct RoomSub *s = &o->sub;
    if (o->active != 0) {
        RoomLink *l = o->link;
        if (l->target != 0) {
            *l->target->state = 4;
        }
    }
    o->flag3 = 0;
    o->state = 4;
    if (s->signal != 0) {
        *s->signal = 0;
    }
    return 0;
}
void RoomLib_HandlerCNop6(void) {
}

void RoomLib_HandlerANop0(void) {
}
void RoomLib_InitHandlerA(PmSlotHeader *slot) {
    RoomEnt *o = (RoomEnt *)slot;
    o->t16 = -1;
    o->t17 = -1;
    o->t18 = -1;
    o->t19 = 3;
    o->sub.cb = RoomLib_HandlerA;
    o->sub.signal = 0;
    o->active = 0;
    o->t1A = 0;
    o->h46 = 0;
}
int RoomLib_ConfigureHandlerA(PmSlotHeader *slot, int arg1, int op, intptr_t arg3, intptr_t sp10, intptr_t sp14) {
    RoomEnt *o = (RoomEnt *)slot;
    if (op != 0xA) {
        if (op < 0xB) {
        if (op != 4) {
            return 0;
        }
        } else {
            if (op != 0x19) {
                if (op == 0x1C) {
                    goto case28;
                }
                return 0;
            }
        case25:
            if (arg1 != 1) {
                return 0;
            }
            o->sub.signal = (int *)arg3;
            *(int *)arg3 = arg1;
            goto done;
        }
    case4:
        o->pos[0] = arg3;
        o->pos[1] = sp10;
        o->h44 = sp14;
        goto done;
    case28:
        o->h48 = arg3;
        o->h46 = sp10;
    }
    else
    {
case10:
    o->t16 = arg3;
    o->sub.cb = RoomLib_RearmHandlerA;
    }
done:
    return 0;
}
int RoomLib_HandlerANop3(PmSlotHeader *slot) {
    (void)slot;
    return 0;
}
int RoomLib_UpdateHandlerA(PmSlotHeader *slot) {
    RoomEnt *obj = (RoomEnt *)slot;
    obj->sub.cb(obj);
    return 0;
}
void RoomLib_RearmHandlerA(RoomEnt *o) {
    if (o->link->variant == o->t16) {
        o->sub.cb = RoomLib_HandlerA;
    }
}
void RoomLib_HandlerA(RoomEnt *o) {
    char *g = (char *)D_8009D254;
    if (((RoomLink *)g)->variant < 4) {
        int *sig;
        ((RoomLink *)g)->flags &= 0xFFF3FFFF;
        sig = o->sub.signal;
        o->sub.cb = RoomLib_HandlerF;
        if (sig != 0) {
            *sig = 2;
        }
        if (o->h44 == -1) {
            o->h44 = FieldFx_VecToAngle(
                (Vec3Fixed *)o->link->pos,
                (Vec3Fixed *)&D_8009D254->posX.fixed);
        }
        if (o->h48 == -1) {
            o->h48 = (unsigned short)o->h44 + 0x800;
        }
        {
            int angle=(unsigned short)o->h44 & 0xFFF;
            GteSineWord sample={.part={rsin(angle),rcos(angle)}};
            int *e = &sample.word;
            int hi = ((short *)e)[1];
            int lo;
            int savedHi;
            o->mat[1] = 0;
            o->mat[0] = hi;
            lo = *e;
            savedHi = (unsigned short)o->mat[0];
            o->mat[3] = 0;
            o->mat[5] = 0;
            o->mat[7] = 0;
            o->mat[2] = lo;
            o->mat[4] = 0x1000;
            o->mat[8] = savedHi;
            o->mat[6] = -(unsigned short)o->mat[2];
        }
    }
}
void RoomLib_HandlerF(RoomEnt *o) {
    int height;
    if (D_8009D254->actionMode >= 4) {
        RoomLib_ReleaseHandlerA((PmSlotHeader *)o);
    } else if (D_8009D254->entityFlags & 0xC0000) {
        D_8009D254->entityFlags &= 0xFFF3FFFF;
        RoomLib_ReleaseHandlerA((PmSlotHeader *)o);
    } else {
        _Alignas(4) short scratchMemory[128] = {0};
        volatile short *scratch = scratchMemory;
        scratch[0] = 0;
        scratch[1] = 0;
        scratch[2] = o->pos[0] >> 12;
        gte_ldrotmatrix(o->mat);
        gte_ldv0((void *)scratch);
        gte_mvmva();
        gte_stmac((void *)&scratch[4]);
        D_8009D254->posX.fixed += *(int *)&scratch[4] << 12;
        D_8009D254->posZ.fixed += *(int *)&scratch[8] << 12;
        height = o->pos[0] - o->pos[1];
        o->pos[0] = height;
        if (height < 0) {
            RoomLib_ReleaseHandlerA((PmSlotHeader *)o);
        }
        if (o->h46 != 0) {
            D_8009D254->facingAngle = FieldFx_TurnToward(
                D_8009D254->facingAngle, o->h48, o->h46);
        }
    }
}
int RoomLib_ReleaseHandlerA(PmSlotHeader *slot) {
    RoomEnt *o = (RoomEnt *)slot;
    int *p = o->sub.signal;
    o->state = 4;
    if (p != 0) {
        *p = 0;
    }
    return 0;
}
void RoomLib_HandlerANop6(void) {
}
