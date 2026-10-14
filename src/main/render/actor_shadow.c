#include "pe1/render_shadow.h"
#include "pe1/render_buffers.h"
#include "pe1/render_camera.h"
#include "pe1/room_floor.h"
#include "pe1/gte.h"

/* Builds the ground-aligned shadow transform for `actor` and links its
 * shadow quad, projected from a square of the model's shadow radius. */
int Render_DrawActorShadow(RenderShadowActor *actor)
{
    GteShortVector origin = {0};
    GteShortVector corner = {0};
    GteVector axis;
    GteVector normal;
    GteVector up;
    GteMatrixStorage local;
    GteMatrix world;
    GteMatrixStorage *source;
    u32 first, last;
    s32 sxy;
    s32 p;
    s32 flag;
    RenderShadowQuad *quad;
    int radius;
    int shade;
    int depth;
    int maxDepth;

    if (actor->flags & 0x400)
        return 0;
    if (!actor->render_object.variant_visible)
        return 0;

    radius = actor->render_object.header->shadow_radius;
    origin.x = Pe1Fixed_Integer(&actor->pos_x);
    origin.y = Pe1Fixed_Integer(&actor->pos_y);
    origin.z = Pe1Fixed_Integer(&actor->pos_z);
    /* The part matrix without its translation turns +Z into the facing. */
    source = (GteMatrixStorage *)&actor->render_object.matrices[actor->shadow_matrix_index];
    first = source->words[0];
    local.words[1] = source->words[1];
    local.words[2] = source->words[2];
    local.words[3] = source->words[3];
    local.words[4] = source->words[4];
    local.words[5] = source->words[5];
    local.words[6] = source->words[6];
    last = source->words[7];
    corner.z = 0x1000;
    local.words[0] = first;
    corner.x = 0;
    corner.y = 0;
    local.matrix.t[1] = 0;
    local.matrix.t[0] = 0;
    local.words[7] = last;
    local.matrix.t[2] = 0;
    gte_ldrotmatrix(&local);
    gte_ldtransmatrix(&local);
    gte_ldv0(&corner);
    gte_rt();
    gte_stlvnl(&axis);
    axis.y = 0;
    if (axis.x == 0 && axis.z == 0) {
        normal.x = 0;
        normal.y = 0;
        normal.z = 0x1000;
    } else {
        VectorNormal(&axis, &normal);
    }
    /* Shadow basis: X = up x facing, Y = up, Z = facing. */
    local.matrix.m[1][1] = 0x1000;
    up.y = 0x1000;
    local.matrix.m[0][1] = 0;
    local.matrix.m[2][1] = 0;
    up.x = 0;
    up.z = 0;
    local.matrix.m[0][2] = normal.x;
    local.matrix.m[1][2] = normal.y;
    local.matrix.m[2][2] = normal.z;
    gte_ldopv1_psyq(&normal);
    gte_ldopv2(&up);
    gte_op12_psyq();
    gte_stlvnl(&axis);
    local.matrix.m[0][0] = axis.x;
    local.matrix.m[1][0] = axis.y;
    local.matrix.m[2][0] = axis.z;
    if (actor->flags & 0x4000000)
        local.matrix.t[1] = g_RoomFloorY->y;
    else
        local.matrix.t[1] = origin.y;
    local.matrix.t[0] = actor->render_object.matrices[actor->shadow_matrix_index].translation[0];
    local.matrix.t[2] = actor->render_object.matrices[actor->shadow_matrix_index].translation[2];
    gte_CompMatrix((GteMatrixWords *)g_EntityRenderScratch, &local, &world);
    SetRotMatrix(&world);
    SetTransMatrix(&world);
    SetGeomScreen(g_EntityRenderScratch[8]);

    quad = &actor->shadow_quads[g_ActiveDrawSlot];
    quad->tag.length = 9;
    quad->code = 0x2C;
    if (actor->render_object.flags_9C & 6)
        shade = (actor->render_object.fade_red + actor->render_object.fade_green + actor->render_object.fade_blue) / 3;
    else
        shade = 0x80;
    shade = shade * actor->shadow_intensity / 128;
    quad->r0 = shade / 2;
    quad->g0 = shade / 2;
    quad->b0 = shade / 2;
    quad->u0 = 0;
    quad->v0 = 0x40;
    quad->u1 = 0x3F;
    quad->v1 = 0x40;
    quad->u2 = 0;
    quad->v2 = 0x7F;
    quad->u3 = 0x3F;
    quad->v3 = 0x7F;
    quad->clut = 0x7210;
    quad->tpage = 0xCB;
    quad->code |= 2;

    maxDepth = -1;
    corner.y = 0;
    corner.x = -radius;
    corner.z = radius;
    depth = RotTransPers(&corner, &sxy, &p, &flag);
    quad->xy0 = sxy;
    if (maxDepth < depth)
        maxDepth = depth;
    corner.x = radius;
    corner.z = radius;
    depth = RotTransPers(&corner, &sxy, &p, &flag);
    quad->xy1 = sxy;
    if (maxDepth < depth)
        maxDepth = depth;
    corner.x = -radius;
    corner.z = -radius;
    depth = RotTransPers(&corner, &sxy, &p, &flag);
    quad->xy2 = sxy;
    if (maxDepth < depth)
        maxDepth = depth;
    corner.x = radius;
    corner.z = -radius;
    depth = RotTransPers(&corner, &sxy, &p, &flag);
    quad->xy3 = sxy;
    if (maxDepth < depth)
        maxDepth = depth;

    if (maxDepth >= 0 && maxDepth < 0x1000) {
        RenderNativeTag *entry = &D_800B0E38.ordering[g_ActiveDrawSlot][maxDepth];
        quad->tag.address = entry->address;
        entry->address = (uintptr_t)&quad->tag;
    }
    return 0;
}

