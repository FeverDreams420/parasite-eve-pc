/*
 * The floor walker actor class driven by RoomLib_HandlerG: a sixth class
 * that 8 rooms (room_m145, m152, m153, m154, m332, m399, m401, m402) link
 * directly after the room library (actor_classes.c), listed in the
 * same room class table with the same seven-method layout (no-op, Init,
 * Configure, no-op, Update, ..., Release, no-op).
 */

#include "common.h"
#include "pe1/field_actor.h"
#include "pe1/fixed_point.h"
#include "pe1/gte.h"
#include "pe1/room_floor_walker.h"

#include "room_lib.h"

PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomFloorWalkerObject,state.started)==PE1_OFFSETOF(RoomEnt,t1A),floor_walker_started_alias);
PE1_STATIC_ASSERT(PE1_OFFSETOF(RoomFloorWalkerObject,state.callback)==PE1_OFFSETOF(RoomEnt,sub.cb),floor_walker_callback_alias);
PE1_STATIC_ASSERT(sizeof(RoomFloorWalkerObject)<=sizeof(PmSecondarySlot),floor_walker_slot_capacity);

void RoomLib_HandlerGNop0(void) {
}
void RoomLib_InitHandlerG(PmSlotHeader *raw) {
    RoomEnt *o=(void *)raw;
    unsigned int m = 0x10002;
    FieldActor *l = raw->owner;
    o->t16 = -1;
    o->t17 = -1;
    o->t18 = -1;
    o->t19 = 3;
    o->sub.signal = 0;
    o->active = 0;
    o->t1A = 0;
    ((RoomFloorWalkerObject *)o)->state.rest = 0;
    ((RoomFloorWalkerObject *)o)->state.turning = 0;
    o->sub.cb = RoomLib_HandlerG;
    l->flags |= m;
    l->render_object.flags_9C |= 0x400;
}

int RoomLib_ConfigureHandlerG(PmSlotHeader *raw,int mode,int op,intptr_t first,intptr_t second,intptr_t third) {
    RoomEnt *o=(void *)raw;
    s32 arg0=(s32)first,arg1=(s32)second,arg2=(s32)third;
    RoomFloorWalkerState *p = &((RoomFloorWalkerObject *)o)->state;

    if (op != 4) {
    if (op < 5) {
        if (op == 0) {
            goto case0;
        }
        return 0;
    }
    if (op == 12) {
        goto case12;
    }
    if (op != 25) {
        return 0;
    }
    if (mode != 1) {
        return 0;
    }
    o->sub.signal = (int *)first;
    *(int *)first = mode;

    }
    else
    {
case4:
    ((RoomFloorWalkerObject *)o)->state.speed = arg0;
    return 0;

case0:
    ((RoomFloorWalkerObject *)o)->state.leader = g_FieldActorListHead;
    if (g_FieldActorListHead != 0) {
        do {
            FieldActor *cur=p->leader;
            if (cur->type_id == arg0 && cur->sub_id == arg1) {
                if ((cur->flags & 0x10) == 0) {
                    break;
                }
            }
            p->leader = p->leader->next;
        } while (p->leader != 0);
        if (p->leader != 0) {
            return 0;
        }
        RoomLib_ReleaseHandlerG((PmSlotHeader *)o);
    } else {
        RoomLib_ReleaseHandlerG((PmSlotHeader *)o);
    }
    return 0;

case12:
    ((RoomFloorWalkerObject *)o)->state.hopVelocity = arg0;
    ((RoomFloorWalkerObject *)o)->state.gravity = arg1;
    ((RoomFloorWalkerObject *)o)->state.restTime = arg2;

    }
ret:
    return 0;
}

int RoomLib_HandlerGNop3(PmSlotHeader *unused) {
    return 0;
}

int RoomLib_UpdateHandlerG(PmSlotHeader *raw) {
    RoomFloorWalkerObject *owner=(void *)raw;
    RoomFloorWalkerState *state=&owner->state;
    GteVector scaleStorage;
    FieldActor *actor;
    RenderMatrix *matrix;
    s32 *scale;
    s32 scaleValue;
    s32 curve;

    actor = owner->header.owner;
    scale = (s32 *)&scaleStorage;
    state->callback((RoomEnt *)owner);

    matrix = &actor->render_object.model_matrix;
    matrix->translation[0] = Pe1Fixed_Integer(&actor->pos_x);
    matrix->translation[1] = Pe1Fixed_Integer(&actor->pos_y);
    matrix->translation[2] = Pe1Fixed_Integer(&actor->pos_z);
    RotMatrixYXZ((void *)&actor->rot_x, (void *)matrix);

    scaleValue = actor->move_speed;
    scaleStorage.z = scaleValue;
    scaleStorage.y = scaleValue;
    scaleStorage.x = scaleValue;
    ScaleMatrix((void *)matrix, (void *)&scaleStorage);

    if (state->rest != 0) {
        curve = rsin(((s32)((u32)((s16)state->restTime-state->rest)<<11)/(s16)state->restTime)&0xFFF)>>1;
        scaleStorage.z = 0x1000 + curve;
        scale[0] = 0x1000 + curve;
        scaleStorage.y = 0x1000 - curve;
        ScaleMatrix((void *)matrix, (void *)scale);
        state->rest--;
    }

    return 0;
}

void RoomLib_HandlerG(RoomEnt *o) {
    RoomEnt *self = o;
    FieldActor *src=((RoomFloorWalkerObject *)self)->state.leader;
    FieldActor *dst=(FieldActor *)self->link;
    dst->pos_x=(s32)((u32)src->render_object.matrices[4].translation[0]<<16);
    dst->pos_y=(s32)((u32)src->render_object.matrices[4].translation[1]<<16);
    dst->pos_z=(s32)((u32)src->render_object.matrices[4].translation[2]<<16);
    memcpy(&dst->rot_x,&src->rot_x,8);
    if ((src->mode == 7) && (src->anim.parts.integer >= 3) && (((u32)src->anim_prev>>16) < 3)) {
        self->sub.cb = RoomLib_HandlerGFloorWalker;
    }
    if (src->state != 0) {
        if (Battle_GetEnemyContextField((struct FieldActor *)src, 0x2C) <= 0) {
            Entity_WriteFieldByCmd(dst, 0x2D, 0);
            Entity_WriteFieldByCmd(dst, 0x2C, 0);
            Entity_WriteFieldByCmd(dst, 0x5F, 0);
            RoomLib_ReleaseHandlerG((PmSlotHeader *)self);
        }
    } else {
        RoomLib_ReleaseHandlerG((PmSlotHeader *)self);
    }
}

/* Walks the creature forward along its heading.  A step that leaves the
 * walkable polygon reflects the heading off the crossed edge (one retry per
 * frame); a creature already outside turns away from the player instead.
 * The vertical hop rises with velocity and falls with gravity until it lands
 * on the floor, where it rests before the next hop. */
void RoomLib_HandlerGFloorWalker(RoomEnt *obj) {
    RoomFloorWalkerObject *object = (RoomFloorWalkerObject *)obj;
    RoomFloorWalkerState *state = &object->state;
    FieldActor *core=object->header.owner;
    RoomFloorWalkerScratch workspace={0},*scratch=&workspace;
    s16 retries;
    int i;
    int point;
    int previous;
    int current;

    if (state->started == 0) {
        state->started = 1;
        core->gravity_y = 0;
        core->motion_y = 0;
        core->flags &= ~0x40000;
        state->angle = core->rot_y;
        state->velocity = state->hopVelocity;
    }
    if (core->mode < 2) {
        return;
    }
    if (core->flags & 0x40000) {
        state->angle = (state->angle + 0x800) & 0xFFF;
        core->flags &= ~0x40000;
    }
    if (state->rest != 0) {
        return;
    }
    retries = 1;
    do {
        {
            int cosine = rcos((state->angle+0x800)&0xFFF);
            int sine;
            u16 c;
            u16 s;

            scratch->yaw.m[0][1] = 0;
            scratch->yaw.m[0][0] = cosine;
            sine=rsin((state->angle+0x800)&0xFFF);
            c = scratch->yaw.m[0][0];
            scratch->yaw.m[1][0] = 0;
            scratch->yaw.m[1][2] = 0;
            scratch->yaw.m[2][1] = 0;
            scratch->step.words.xy = 0;
            scratch->yaw.m[0][2] = sine;
            s = scratch->yaw.m[0][2];
            scratch->yaw.m[1][1] = 0x1000;
            scratch->yaw.m[2][2] = c;
            scratch->yaw.m[2][0] = -s;
        }
        scratch->step.words.z = state->speed >> 12;
        gte_ldrotmatrix(&scratch->yaw);
        gte_ldv0(&scratch->step);
        gte_mvmva();
        gte_stmac(&scratch->rotated);
        scratch->next.x = (s32)((u32)core->pos_x+((u32)scratch->rotated.x<<12));
        scratch->next.z = (s32)((u32)core->pos_z+((u32)scratch->rotated.z<<12));
        point = ((u32)scratch->next.x >> 16) | (scratch->next.z & 0xFFFF0000);
        previous = (u16)g_FloorPolygonVertices[(s16)g_FloorPolygonVertexCount - 1].x |
                   ((u32)(u16)g_FloorPolygonVertices[(s16)g_FloorPolygonVertexCount - 1].z << 16);
        i = 0;
        while (i < (s16)g_FloorPolygonVertexCount) {
            current = (u16)g_FloorPolygonVertices[i].x | ((u32)(u16)g_FloorPolygonVertices[i].z << 16);
            gte_ldsxy0(point);
            gte_ldsxy2(previous);
            gte_ldsxy1(current);
            gte_nclip();
            gte_stmac0(&scratch->area);
            if (scratch->area < 0) break;
            i++;
            previous = current;
        }
        if (i >= (s16)g_FloorPolygonVertexCount) {
            state->turning = 1;
            break;
        }
        if (state->turning == 0) {
            state->angle = (FieldFx_VecToAngle(&(Vec3Fixed){core->pos_x,core->pos_y,core->pos_z},
                                                &(Vec3Fixed){g_PlayerEntity->pos_x,g_PlayerEntity->pos_y,g_PlayerEntity->pos_z}) + 0x800) & 0xFFF;
            break;
        }
        {
            int edge = ratan2((s16)current - (s16)previous,
                                 (current >> 16) - (previous >> 16)) & 0xFFF;
            if (edge >= 0x800) edge -= 0x800;
            retries--;
            state->angle = (edge + (edge - state->angle)) & 0xFFF;
        }
    } while (retries != -1);
    if (retries < 0) {
        state->turning = 0;
    }
    core->pos_x = scratch->next.x;
    core->pos_z = scratch->next.z;
    core->pos_y=(s32)((u32)core->pos_y+(u32)state->velocity);
    if ((core->pos_y >> 16) >= g_RoomFloorY->y) {
        core->pos_y = (s32)((u32)(s32)g_RoomFloorY->y<<16);
        state->velocity=(s32)(0u-(u32)state->hopVelocity);
        state->rest = state->restTime;
    } else {
        state->velocity=(s32)((u32)state->velocity+(u32)state->gravity);
    }
}

int RoomLib_ReleaseHandlerG(PmSlotHeader *raw) {
    RoomEnt *o=(void *)raw;
    int *p = o->sub.signal;
    o->state = 4;
    if (p != 0) {
        *p = 0;
    }
    return 0;
}
void RoomLib_HandlerGNop6(void) {
}

PmCommand g_RoomFloorWalkerHandlers={RoomLib_HandlerGNop0,RoomLib_InitHandlerG,RoomLib_ConfigureHandlerG,RoomLib_HandlerGNop3,RoomLib_UpdateHandlerG,RoomLib_ReleaseHandlerG,RoomLib_HandlerGNop6};
