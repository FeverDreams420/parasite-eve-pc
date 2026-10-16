/*
 * The window actor class: a script object that flies a spinning frame from
 * its anchor actor towards a target actor (RoomLib_WindowHandler), in the
 * room library's seven-method layout (no-op, Init, Configure, no-op,
 * Update, Release, no-op) with its arm, flight and arrival callbacks.
 *
 * room_m063 and m083 link only this class; room_m358, m384 and m387 link
 * it directly after the room library (actor_classes.c). The ten
 * functions and the configure jump table are the same in all five rooms.
 */

#include "pe1/gte.h"
#include "pe1/room_window.h"

#include "room_lib.h"

void RoomLib_WindowClassNop0(void) {
}

void RoomLib_InitWindowClass(PmSlotHeader *slot) {
    RoomWindowObject *ctx = (RoomWindowObject *)slot;
    int value;

    ctx->enabled = 1;
    value = -1;
    ctx->state.variant = value;
    ctx->state.optionB = value;
    ctx->state.optionC = value;
    ctx->state.flags = 3;
    ctx->state.scale = 0x400;
    ctx->state.callback = RoomLib_WindowHandler;
    ctx->state.signal = 0;
    ctx->state.active = 0;
    ctx->state.started = 0;
    ctx->state.touched = 0;
    ctx->state.travel = 0;
    ctx->state.step = 0;
    ctx->state.targetNode = 0;
    ctx->state.pos[0].value = 0;
    ctx->state.pos[1].value = 0;
    ctx->state.pos[2].value = 0;
    ctx->state.offset.x = 0;
    ctx->state.offset.y = 0;
    ctx->state.offset.z = 0;
    ctx->state.flags = 0;
}

int RoomLib_ConfigureWindowClass(PmSlotHeader *slot, int query, int op,
                                  intptr_t a, intptr_t b, intptr_t c) {
    RoomWindowObject *obj = (RoomWindowObject *)slot;
    RoomWindowState *state = &obj->state;
    FieldActorNode *node;
    void (*callback)(RoomWindowObject *);
    int sentinel;

    switch (op) {
    case 19:
        if (query == 0) {
            obj->enabled = a;
        } else {
            *(int *)a = obj->enabled;
        }
        break;
    case 0:
        state->targetNode = (RoomWindowTarget *)g_FieldActorListHead;
        while (state->targetNode != 0) {
            node = (FieldActorNode *)state->targetNode;
            if (node->b0C == a && node->b0D == b && !(node->w98 & 0x10)) {
                break;
            }
            state->targetNode =
                (RoomWindowTarget *)((FieldActorNode *)state->targetNode)->next;
        }
        break;
    case 17:
        state->target[0] = a;
        state->target[2] = c;
        if (b == -1) {
            state->target[1] = g_PlayerEntity->pos_y;
        } else {
            state->target[1] = b;
        }
        state->targetNode = 0;
        break;
    case 4:
        state->travel = a;
        break;
    case 2:
        state->step = a;
        break;
    case 3:
        state->scale = a;
        break;
    case 23:
        a = a != 0;
        b = (b != 0) << 1;
        c = (c != 0) << 2;
        state->flags = a | b | c;
        break;
    case 10:
        callback = RoomLib_ArmWindowClass;
        sentinel = -1;
        state->variant = a;
        state->active = b;
        state->callback = callback;
        if (a != sentinel) {
            break;
        }
        return 0;
    case 11:
        state->optionB = a;
        break;
    case 6:
        state->offset.x = a >> 16;
        state->offset.y = b >> 16;
        state->offset.z = c >> 16;
        break;
    }
    return 0;
}

int RoomLib_WindowClassNop3(PmSlotHeader *slot) {
    (void)slot;
    return 0;
}

/* Runs the armed handler, or releases the class when the script ends. */
int RoomLib_UpdateWindowClass(PmSlotHeader *slot) {
    RoomWindowObject *o = (RoomWindowObject *)slot;
    switch (FieldFx_IsOwnerBusy((FieldFxTaskOwner *)o)) {
    case 0:
        o->state.callback(o);
        return 0;
    case 1:
        RoomLib_ReleaseWindowClass(slot);
    case 2:
        return 0;
    }
    return 0;
}

/* Arms the window when the actor's variant and window match the
 * configured ones. */
void RoomLib_ArmWindowClass(RoomWindowObject *o) {
    FieldActor *l = (FieldActor *)o->actor->anchor;
    if (o->state.variant >= 0) {
        if (o->state.variant != l->mode) return;
    }
    if (o->state.optionB >= 0) {
        int hi = (u32)l->anim_prev >> 16;
        int lo = l->anim.parts.integer;
        if (o->state.optionB < hi || lo < o->state.optionB) return;
    }
    o->state.callback = RoomLib_WindowHandler;
}

/* Window effect tick. On the first call it places the window in front of
 * its anchor actor, aims it at the target node and derives the spin speed
 * from the distance. Every call then advances the window along its path,
 * spins it, writes the actor position, centre and render matrix, flags the
 * player once it comes within 200 units, and hands over to the arrival
 * callback when it reaches the target.
 *
 * The original scratch workspace is local native memory; the GTE receives
 * the same matrices and vectors in the original order. */

void RoomLib_WindowHandler(RoomWindowObject *obj) {
    RoomWindowScratch scratch = {0};
    RoomWindowScratch *sp;
    RoomWindowState *state = &obj->state;
    RoomWindowActor *actor = obj->actor;
    RoomWindowAnchor *anchor = actor->anchor;
    RoomWindowTrig sample, *trig = &sample;
    int angle;
    int cosv;
    int dist;
    int quot;

    sp = &scratch;
    if (obj->state.started == 0) {
        obj->state.started = 1;
        obj->actor->flags |= 0x10000;
        obj->actor->drawFlags |= 0x400;
        obj->index = 0;
        anchor->marker->flags |= 0x40000000;
        sp->in.local.x = 0x80;
        sp->in.local.y = 0;
        sp->in.local.z = -350;
        angle = anchor->yaw & 0xFFF;
        sample.part.sin = rsin(angle); sample.part.cos = rcos(angle);
        {
            int c = trig->part.cos;
            int s;

            sp->yaw.m[0][1] = 0;
            sp->yaw.m[0][0] = c;
            s = trig->part.sin;
            sp->yaw.m[1][0] = 0;
            sp->yaw.m[1][1] = 0x1000;
            sp->yaw.m[1][2] = 0;
            sp->yaw.m[2][1] = 0;
            sp->yaw.m[2][2] = c;
            sp->yaw.m[0][2] = s;
            sp->yaw.m[2][0] = -s;
        }
        gte_ldrotmatrix(&sp->yaw);
        gte_ldv0(&sp->in.local);
        gte_mvmva();
        gte_stmac(&sp->out);
        obj->state.origin[0].value = (s32)((u32)anchor->pos[0] + ((u32)sp->out.x << 16));
        obj->state.origin[1].value = (s32)((u32)anchor->pos[1] + ((u32)sp->out.y << 16));
        obj->state.origin[2].value = (s32)((u32)anchor->pos[2] + ((u32)sp->out.z << 16));
        if (obj->state.targetNode != 0) {
            obj->state.target[0] = obj->state.targetNode->pos[0];
            obj->state.target[1] = obj->state.targetNode->pos[1];
            obj->state.target[2] = obj->state.targetNode->pos[2];
        }
        angle = FieldFx_VecToAngle((Vec3Fixed *)&obj->state.origin[0].value,
                                  (Vec3Fixed *)obj->state.target) & 0xFFF;
        sample.part.sin = rsin(angle); sample.part.cos = rcos(angle);
        {
            int c = trig->part.cos;
            int s;

            sp->yaw.m[0][1] = 0;
            sp->yaw.m[0][0] = c;
            s = trig->part.sin;
            sp->yaw.m[1][0] = 0;
            sp->yaw.m[1][1] = 0x1000;
            sp->yaw.m[1][2] = 0;
            sp->yaw.m[2][1] = 0;
            sp->yaw.m[2][2] = c;
            sp->yaw.m[0][2] = s;
            sp->yaw.m[2][0] = -s;
        }
        gte_ldrotmatrix(&sp->yaw);
        gte_ldv0(&obj->state.offset);
        gte_mvmva();
        gte_stmac(&sp->out);
        obj->state.target[0] = (s32)((u32)obj->state.target[0] + ((u32)sp->out.x << 16));
        obj->state.target[1] = (s32)((u32)obj->state.target[1] + ((u32)sp->out.y << 16));
        obj->state.target[2] = (s32)((u32)obj->state.target[2] + ((u32)sp->out.z << 16));
        dist = (s32)((u32)FieldFx_GetFixedDistance3D((Vec3Fixed *)&obj->state.origin[0].value, (Vec3Fixed *)obj->state.target) +
               (u32)((s32)((u32)obj->state.step * 0x70u) >> 21));
        if (dist < 0x148) {
            dist = 0x148;
        }
        quot = 0x5160000 / dist;
        obj->state.spinSpeed = quot << 8;
        obj->state.spin = (quot << 8) >> 3;
        FieldFx_FixedVecToLookAngles((Vec3Fixed *)&obj->state.origin[0].value, (Vec3Fixed *)obj->state.target,
                      (short *)&obj->state.angles);
        RotMatrixYXZ(&obj->state.angles, &obj->state.rotation);
    }

    sp->arrived = 0;
    angle = (((s32)((u32)state->spin + 0x8000u) >> 16) - 0x400) & 0xFFF;
    sample.part.sin = rsin(angle); sample.part.cos = rcos(angle);
    {
        u16 c;
        u16 s;
        int sn;

        cosv = trig->part.cos;
        sp->yaw.m[0][1] = 0;
        sp->yaw.m[0][0] = cosv;
        sn = trig->part.sin;
        c = sp->yaw.m[0][0];
        sp->yaw.m[1][0] = 0;
        sp->yaw.m[1][2] = 0;
        sp->yaw.m[2][1] = 0;
        sp->in.local.x = 0;
        sp->in.local.y = 0;
        sp->yaw.m[0][2] = sn;
        s = sp->yaw.m[0][2];
        sp->yaw.m[1][1] = 0x1000;
        sp->yaw.m[2][2] = c;
        sp->yaw.m[2][0] = -s;
    }
    sp->in.local.z = state->travel >> 12;
    gte_ldrotmatrix(&sp->yaw);
    gte_ldv0(&sp->in.local);
    gte_mvmva();
    gte_stmac(&sp->advance);
    state->pos[0].value = (s32)((u32)state->pos[0].value + ((u32)sp->advance.x << 12));
    state->pos[2].value = (s32)((u32)state->pos[2].value + ((u32)sp->advance.z << 12));
    if (sp->advance.z >= 0) {
        state->travel = (s32)((u32)state->travel + (u32)state->step);
        state->spin = (s32)((u32)state->spin + ((u32)((s32)((u32)((s32)((u32)state->travel + 0x8000u) >> 16) * (u32)(state->spinSpeed >> 12)) >> 8) << 12));
    } else {
        state->spin = (s32)((u32)state->spin + ((u32)((s32)((u32)((s32)((u32)state->travel + 0x8000u) >> 16) * (u32)(state->spinSpeed >> 12)) >> 8) << 12));
        state->travel = (s32)((u32)state->travel - (u32)state->step);
    }
    sp->spot.x = (s32)((u32)((s32)((u32)state->pos[0].value + 0x8000u) >> 16) * (u32)state->scale + 0x800u) >> 12;
    sp->spot.y = 0;
    sp->spot.z = (s32)((u32)state->pos[2].value + 0x8000u) >> 16;
    if (sp->advance.z < 0) {
        if (FieldFx_IntSqrt((s32)((u32)state->pos[0].part.integer * (u32)state->pos[0].part.integer +
                          (u32)state->pos[2].part.integer * (u32)state->pos[2].part.integer)) < 0x100) {
            sp->arrived = 1;
        }
    }
    if (sp->arrived != 0) {
        anchor->speed = 0x380000;
        obj->actor->flags &= ~0x10000;
        obj->actor->drawFlags &= ~0x400;
        state->callback = RoomLib_WindowArrived;
        return;
    }
    sp->in.angles.x = 0;
    sp->in.angles.y = (u32)obj->index * 0x280u + 0x640u;
    sp->in.angles.z = -0x400;
    RotMatrixYXZ(&sp->in.angles, (GteMatrix *)&sp->yaw);
    angle = sp->in.angles.y & 0xFFF;
    sample.part.sin = rsin(angle); sample.part.cos = rcos(angle);
    {
        u16 c;
        u16 s;
        int sn;

        cosv = trig->part.cos;
        sp->roll.m[0][1] = 0;
        sp->roll.m[0][0] = cosv;
        sn = trig->part.sin;
        c = sp->roll.m[0][0];
        sp->roll.m[1][1] = 0x1000;
        sp->roll.m[0][2] = sn;
        s = sp->roll.m[0][2];
        sp->roll.m[1][0] = 0;
        sp->roll.m[1][2] = 0;
        sp->roll.m[2][1] = 0;
        sp->in.local.x = 0x100;
        sp->in.local.y = 0;
        sp->in.local.z = 0;
        sp->roll.m[2][2] = c;
        sp->roll.m[2][0] = -s;
    }
    gte_ldrotmatrix(&sp->roll);
    gte_ldv0(&sp->in.local);
    gte_mvmva();
    gte_stmac(&sp->out);
    {
        int x = sp->out.x;
        int z;

        sp->edge.y = -200;
        sp->edge.x = sp->spot.x + x;
        z = sp->out.z;
        sp->edge.z = sp->spot.z + z;
    }
    gte_ldrotmatrix(&state->rotation);
    gte_ldv0(&sp->spot);
    gte_mvmva();
    gte_stmac(&sp->out);
    gte_ldv0(&sp->edge);
    gte_mvmva();
    gte_stmac(&sp->corner);
    gte_MulMatrix0(&state->rotation, &sp->yaw, &sp->roll);
    actor->pos[0] = (s32)((u32)state->origin[0].value + ((u32)sp->out.x << 16));
    actor->pos[1] = (s32)((u32)state->origin[1].value + ((u32)sp->out.y << 16));
    actor->pos[2] = (s32)((u32)state->origin[2].value + ((u32)sp->out.z << 16));
    actor->center[0] = state->origin[0].part.integer + sp->corner.x;
    actor->center[1] = state->origin[1].part.integer + sp->corner.y;
    actor->center[2] = state->origin[2].part.integer + sp->corner.z;
    actor->matrix[0] = sp->roll.m[0][0];
    actor->matrix[1] = sp->roll.m[0][1];
    actor->matrix[2] = sp->roll.m[0][2];
    actor->matrix[3] = sp->roll.m[1][0];
    actor->matrix[4] = sp->roll.m[1][1];
    actor->matrix[5] = sp->roll.m[1][2];
    actor->matrix[6] = sp->roll.m[2][0];
    actor->matrix[7] = sp->roll.m[2][1];
    actor->matrix[8] = sp->roll.m[2][2];
    if (state->touched == 0) {
        if (FieldFx_GetDistanceXZ((Vec3Fixed *)actor->pos, (Vec3Fixed *)&g_PlayerEntity->pos_x) < 200) {
            state->touched = 1;
            g_PlayerEntity->state->flags |= 0x4000;
            if (anchor->marker != 0) {
                anchor->marker->flags |= 0x80000000;
            }
        }
    }
    if (anchor->frame >= 0x2E) {
        anchor->speed = 0x180000;
    }
}

void RoomLib_WindowArrived(RoomWindowObject *ctx) {
    RoomWindowActor *obj = ((RoomWindowObject *)ctx)->actor;
    RoomWindowAnchor *window = obj->anchor;
    unsigned int limit = window->frameCount;

    limit--;
    if (window->frame >= limit) {
        RoomLib_ReleaseWindowClass((PmSlotHeader *)ctx);
    }
}

int RoomLib_ReleaseWindowClass(PmSlotHeader *slot) {
    RoomWindowObject *ctx = (RoomWindowObject *)slot;
    RoomWindowAnchor *anchor;
    RoomWindowFlags *marker;

    ctx->status = 4;
    ctx->enabled = 0;

    ctx->actor->flags &= 0xFFFEFFFF;
    ctx->actor->drawFlags &= 0xFBFF;

    anchor = ctx->actor->anchor;
    if (anchor != 0) {
        marker = anchor->marker;
        if (marker != 0) {
            marker->state[0] = 4;
        }
    }

    return 0;
}

void RoomLib_WindowClassNop6(void) {
}
