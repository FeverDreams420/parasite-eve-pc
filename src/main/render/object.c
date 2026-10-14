#include "common.h"
#include "pe1/entity_frame_update.h"
#include "pe1/field_actor.h"
#include "pe1/gte.h"
#include "pe1/render_buffers.h"
#include "pe1/render_lighting.h"
#include "pe1/render_matrix_copy.h"
#include "pe1/render_object.h"
#include "pe1/render_setup.h"
#include "pe1/render_tint.h"

/* COMMON supplies small-data metadata to MASPSX: these words are reached
 * through $gp, every other extern stays absolute. */


/* Per-frame model transforms: bone matrices from the matrix command list,
 * the skinned hit volumes and the projected vertex buffers. */

/* Builds the object's bone matrices from its matrix command list (-1 pushes
 * the current GTE matrix, -2 pops it, any other value is a part index) and
 * transforms the hit-sphere centres of the visible parts. */


/* Places a skinned object's hit volumes and projects its origin and target
 * point; headerless objects take their transforms from their owner. */


/* Projects every vertex triple of the object's visible parts through the
 * view-composed part matrix into the screen and depth buffers. */


void Render_DrawEntity(RenderObjectEntity *object, void *viewMatrix) {
    u16 flags;
    if (object->header == 0 || object->draw_count == 0) return;
    if (object->flags_9C & 0x800) {
        Render_SetObjectTexturePage(object, 0);
        Render_SetEntityBlendMode(object, 0);
        object->flags_9C &= ~0x800;
    }
    if (object->flags_9C & 0x10)
        Render_DrawObjectVariant(object, object->script_value9a, viewMatrix);
    if (object->variant_visible == 1)
        Render_DrawTexturedQuads(object);
    flags = object->flags_9C;
    if (flags & 0x20) {
        Render_BuildEntityMatrix(object, g_ActiveDrawSlot);
        object->reserved9f = g_ActiveDrawSlot == 0;
        object->flags_9C = (object->flags_9C & ~0x20) | 0x40;
    } else if (flags & 0x40) {
        Render_BuildEntityMatrix(object, object->reserved9f);
        object->flags_9C &= ~0x40;
    }
    flags = object->flags_9C;
    if (flags & 2) {
        if ((s16)Render_ColorEntity(object))
            object->flags_9C = (object->flags_9C & ~2) | 0x200;
        if (object->flags_9C & 8)
            Render_DrawObjectAlt(object, object->script_value9a, object->script_param97,
                                 object->script_param98, object->script_param99);
    } else if (flags & 4) {
        if ((s16)Render_TickObject(object))
            object->flags_9C &= ~4;
        if (object->flags_9C & 8)
            Render_DrawObjectAlt(object, object->script_value9a, object->script_param97,
                                 object->script_param98, object->script_param99);
    } else if (flags & 8) {
        Render_DrawObject(object, &g_RenderLightingState);
        Render_DrawObjectAlt(object, object->script_value9a, object->script_param97,
                             object->script_param98, object->script_param99);
    } else if (flags & 1) {
        Render_DrawObject(object, &g_RenderLightingState);
        Render_UpdateClutTable(object, 0, g_ActiveDrawSlot);
    }
}

/* A GTE matrix copied as its eight 32-bit words. */
typedef union RenderMatrixWords {
    RenderMatrix matrix;
    s32 words[8];
} RenderMatrixWords;

void Render_BuildEntityMatrix(RenderObjectEntity *object, s16 bufferIndex)
{
    s32 saved[8];
    RenderMatrixWords *model;
    FieldActor *actor;
    s32 first;
    void *animation;
    RenderMatrixWords *player;

    if (object != &g_PlayerBattleEntity) {
        model = (RenderMatrixWords *)&object->model_matrix;
        first = model->words[0];
        saved[1] = model->words[1];
        saved[2] = model->words[2];
        saved[3] = model->words[3];
        saved[4] = model->words[4];
        saved[5] = model->words[5];
        saved[6] = model->words[6];
        saved[7] = model->words[7];
        animation = object->animation_data;
        saved[0] = first;
        model->words[0] = g_IdentityMatrixSource[0];
        model->words[1] = g_IdentityMatrixSource[1];
        model->words[2] = g_IdentityMatrixSource[2];
        model->words[3] = g_IdentityMatrixSource[3];
        model->words[4] = g_IdentityMatrixSource[4];
        model->words[5] = g_IdentityMatrixSource[5];
        model->words[6] = g_IdentityMatrixSource[6];
        model->words[7] = g_IdentityMatrixSource[7];
        Anim_BuildRotationMatrices(object, animation, 0);
        Render_TransformVertices(object);
        Render_InitRoomPrimState(object);
        Render_DrawObject(object, &g_RenderLightingState);
        Render_UpdateClutTable(object, 1, bufferIndex);
        model->words[0] = saved[0];
        model->words[1] = saved[1];
        model->words[2] = saved[2];
        model->words[3] = saved[3];
        model->words[4] = saved[4];
        model->words[5] = saved[5];
        model->words[6] = saved[6];
        model->words[7] = saved[7];
        actor = (FieldActor *)((char *)object - PE1_OFFSETOF(FieldActor, render_object));
        Anim_BuildRotationMatrices(object, actor->action_data, (s16)actor->anim.parts.integer);
        Render_TransformVertices(object);
        return;
    }
    player = (RenderMatrixWords *)g_PlayerBattleEntity.matrices;
    player->words[0] = g_IdentityMatrixSource[0];
    player->words[1] = g_IdentityMatrixSource[1];
    player->words[2] = g_IdentityMatrixSource[2];
    player->words[3] = g_IdentityMatrixSource[3];
    player->words[4] = g_IdentityMatrixSource[4];
    player->words[5] = g_IdentityMatrixSource[5];
    player->words[6] = g_IdentityMatrixSource[6];
    player->words[7] = g_IdentityMatrixSource[7];
    Render_InitRoomPrimState(object);
    Render_DrawObject(object, &g_RenderLightingState);
    Render_UpdateClutTable(object, 1, bufferIndex);
}

/* Colours the vertices of an object's visible parts that lie below a height
 * limit. GTE operations and their explicit nop hazard slots use individual
 * macros. */

#define Render_LoadObjectMatrix(matrix) \
    { \
        gte_ldrotmatrix((const GteMatrixWords *)(matrix)); \
        gte_ldtransmatrix((const GteMatrixWords *)(matrix)); \
    }

#define Render_TransformVertex(src, dst)                                                           \
    {                                                                                              \
        gte_ldv0(src);                                                                             \
        gte_rt();                                                                                  \
        gte_stsv(dst);                                                                             \
    }

void Render_DrawObjectAlt(RenderObjectEntity *entity, s16 limit, u8 red, u8 green, u8 blue) {
    RenderVec3s scratchStorage;
    RenderVec3s *scratch = &scratchStorage;
    RenderMatrix *matrix;
    int i, j;

    if (!entity->header || !entity->draw_count)
        return;
    matrix = entity->matrices;
    for (i = 0; i < entity->header->part_count; i++, matrix++) {
        RenderObjectPart *part;
        s32 y;

        if (entity->parts[i].visible != 1)
            continue;
        Render_LoadObjectMatrix(matrix);
        Render_TransformVertex(&entity->bounds_vertices[i * 2], scratch);
        part = &entity->parts[i];
        y = scratch->y;
        if (limit < y || limit < y - entity->bounds_vertices[i * 2].pad ||
            limit < y + entity->bounds_vertices[i * 2].pad) {
            RenderVec3s *vertex = &entity->vertices[part->vertex_start];
            RenderColor *colour =
                (RenderColor *)&g_RenderClutLookupTable[part->vertex_start];

            for (j = 0; j < part->vertex_count; colour++) {
                /* Retail advances the counters while the GTE works. */
                gte_ldv0(vertex);
                gte_rt();
                j++;
                vertex++;
                gte_stsv(scratch);
                if (limit < scratch->y) {
                    colour->r = red;
                    colour->g = green;
                    colour->b = blue;
                }
            }
        }
    }
    Render_UpdateClutTable(entity, 0, g_ActiveDrawSlot);
}
#undef Render_LoadObjectMatrix
#undef Render_TransformVertex

#define Render_LoadObjectMatrix(matrix) \
    { \
        gte_ldrotmatrix((const GteMatrixWords *)(matrix)); \
        gte_ldtransmatrix((const GteMatrixWords *)(matrix)); \
    }

#define Render_TransformVertex(src, dst)                                                           \
    {                                                                                              \
        gte_ldv0(src);                                                                             \
        gte_rt();                                                                                  \
        gte_stsv(dst);                                                                             \
    }

void Render_DrawObjectVariant(RenderObjectEntity *input, s16 limit, s32 *projectionMatrix) {
    RenderObjectEntity *entity = input;
    s32 *projectMatrix = projectionMatrix;
    s16 savedLimit = limit;
    s32 changed;
    RenderVec3s scratchStorage;
    RenderVec3s *scratch = &scratchStorage;
    u32 *projected;
    s32 *matrix;
    s32 i;
    s32 offset;
    s32 threshold;
    RenderObjectPart *part;
    RenderObjectPart *probe;
    s32 boundsOffset;
    RenderVec3s *bounds;
    s32 y;
    s32 radius;
    s32 sum;
    RenderVec3s *vertices;
    u32 *clut;
    s32 vertexIndex;
    s32 limitShift;
    changed = 0;
    u32 projectedStorage;
    projected = &projectedStorage;
    if (!entity->header || !entity->draw_count)
        return;
    matrix = (s32 *)entity->matrices;
    i = 0;
    if (changed < entity->header->part_count) {
        limitShift = (s32)((u32)(s32)limit << 16);
        threshold = limitShift >> 16;
        offset = 0;
        do {
            {
                RenderObjectPart *base = entity->parts;
                probe = (RenderObjectPart *)((u8 *)base + offset);
            }
            if (probe->visible == 1) {
                part = probe;
                Render_LoadObjectMatrix(matrix);
                boundsOffset = i * 16;
                bounds = (RenderVec3s *)((u8 *)entity->bounds_vertices + boundsOffset);
                Render_TransformVertex(bounds, scratch);
                {
                    u16 yy = (u16)scratch->y;
                    y = (s16)yy;
                }
                if (threshold < y ||
                    (bounds = (RenderVec3s *)((u8 *)entity->bounds_vertices + boundsOffset),
                     radius = bounds->pad,
                     threshold < y - radius || threshold < (sum = y + radius))) {
                    scratch->y = savedLimit;
                    Render_LoadObjectMatrix(projectMatrix);
                    gte_lwc2_0_0(scratch);
                    gte_lwc2_1_4(scratch);
                    gte_cop2_hazard_slot();
                    gte_cop2_hazard_slot();
                    gte_rtps_command();
                    gte_stsxy2(projected);
                    vertexIndex = 0;
                    {
                        s32 first = part->vertex_start;
                        RenderVec3s *base = entity->vertices;
                        s32 vertexOffset = first * 8;
                        vertices = (RenderVec3s *)((u8 *)base + vertexOffset);
                        first <<= 2;
                        clut = (u32 *)((u8 *)D_800B1640 + first);
                    }
                    Render_LoadObjectMatrix(matrix);
                    for (; vertexIndex < part->vertex_count; vertices++) {
                        gte_lwc2_0_0(vertices);
                        gte_lwc2_1_4(vertices);
                        gte_cop2_hazard_slot();
                        gte_cop2_hazard_slot();
                        gte_mvmva_rotation_v0_translation_sf12();
                        vertexIndex++;
                        clut++;
                        gte_stsv(scratch);
                        if (threshold < scratch->y) {
                            *clut = *projected;
                            changed++;
                        }
                    }
                }
            }
            offset += sizeof(RenderObjectPart);
            i++;
            matrix += 8;
        } while (i < entity->header->part_count);
    }
    if (changed == entity->header->visible_part_count || (entity->flags_9C & 0x200))
        entity->variant_visible = 0;
    else
        entity->variant_visible = 1;
}



/* Adjacent object updates use the same animation and colour state. */


int Render_TickObject(RenderObjectEntity *object) {
    int state;
    u8 r, g, b;
    if (!object->header || !object->draw_count) return 1;
    state = object->fade_remaining;
    if (!state) {
        object->fade_remaining = -1;
        Render_SetObjectTexturePage(object, 0);
        Render_InitRoomPrimState(object);
        Render_SetEntityBlendMode(object, 0);
        object->flags_9C |= 0x820;
        return 1;
    }
    if (state < 0) {
        object->variant_visible = 1;
        object->flags_9C &= ~0x200;
        Render_SetObjectTexturePage(object, 1);
        Render_SetEntityBlendMode(object, 1);
        r = object->primitive_red;
        g = object->primitive_green;
        b = object->primitive_blue;
        object->fade_red = 0;
        object->fade_green = 0;
        object->fade_blue = 0;
        Render_FadeEntityColor(object, 0, 0, 0);
        object->primitive_red = r;
        object->primitive_green = g;
        object->primitive_blue = b;
        object->fade_remaining = object->fade_duration;
    } else if (state == object->fade_duration - 1) {
        Render_SetObjectTexturePage(object, 1);
        Render_SetEntityBlendMode(object, 1);
    }
    g_RenderFadeColour = (object->fade_blue << 16) | (object->fade_green << 8) | object->fade_red;
    Render_DrawObject(object, &g_RenderLightingState);
    if (!(object->flags_9C & 8)) Render_UpdateClutTable(object, 0, (s16)g_ActiveDrawSlot);
    g_RenderFadeColour = 0x808080;
    object->fade_red += object->fade_red_step;
    object->fade_green += object->fade_green_step;
    object->fade_blue += object->fade_blue_step;
    object->fade_remaining--;
    return 0;
}

int Render_ColorEntity(RenderObjectEntity *object) {
    int state;
    if (!object->header || !object->draw_count) return 1;
    state = object->fade_remaining;
    if (!state) {
        object->fade_remaining = -1;
        g_RenderFadeColour = 0x808080;
        return 1;
    }
    if (state == 1) {
        object->variant_visible = 0;
    } else if (state < 0) {
        object->fade_remaining = object->fade_duration;
        Render_SetEntityBlendMode(object, 1);
        Render_SetObjectTexturePage(object, 1);
        if (object->primitive_red > 128) object->primitive_red = 128;
        if (object->primitive_green > 128) object->primitive_green = 128;
        if (object->primitive_blue > 128) object->primitive_blue = 128;
        object->fade_red_step = object->primitive_red / object->fade_duration;
        object->fade_green_step = object->primitive_green / object->fade_duration;
        object->fade_blue_step = object->primitive_blue / object->fade_duration;
        object->fade_red = object->primitive_red;
        object->fade_green = object->primitive_green;
        object->fade_blue = object->primitive_blue;
    } else if (state == object->fade_duration - 1) {
        Render_SetEntityBlendMode(object, 1);
        Render_SetObjectTexturePage(object, 1);
    } else {
        object->fade_red -= object->fade_red_step;
        if (object->fade_red > 128) object->fade_red = 0;
        object->fade_green -= object->fade_green_step;
        if (object->fade_green > 128) object->fade_green = 0;
        object->fade_blue -= object->fade_blue_step;
        if (object->fade_blue > 128) object->fade_blue = 0;
    }
    g_RenderFadeColour = (object->fade_blue << 16) | (object->fade_green << 8) | object->fade_red;
    Render_DrawObject(object, &g_RenderLightingState);
    if (!(object->flags_9C & 8)) Render_UpdateClutTable(object, 0, (s16)g_ActiveDrawSlot);
    g_RenderFadeColour = 0x808080;
    object->fade_remaining--;
    return 0;
}

/* Object fade and blend commands update the same colour packet state. */

void Render_FadeEntityColor(RenderObjectEntity *object, int r, int g, int b) {
    u8 *prim;
    u32 color;
    int i;
    int slot = g_ActiveDrawSlot;
    u8 opcode;

    if (!object->header || !object->draw_count) {
        return;
    }
    color = ((b & 255) << 16) | ((g & 255) << 8) | (r & 255);
    object->primitive_red = r;
    object->primitive_green = g;
    object->primitive_blue = b;
    prim = object->primitive_buffer;
    for (i = -1; ++i < object->header->packet34_count;
         prim += 2 * sizeof(RenderPacket34)) {
        RenderPacket34 *packet = (RenderPacket34 *)prim + slot;
        opcode = packet->values0.bytes.command;
        packet->values0.value = color;
        packet->value1 = color;
        packet->value2 = color;
        packet->value3 = color;
        packet->values0.bytes.command = opcode;
    }
    for (i = -1; ++i < object->header->packet28_count;
         prim += 2 * sizeof(RenderPacket28)) {
        RenderPacket28 *packet = (RenderPacket28 *)prim + slot;
        opcode = packet->values0.bytes.command;
        packet->values0.value = color;
        packet->value1 = color;
        packet->value2 = color;
        packet->values0.bytes.command = opcode;
    }
    for (i = -1; ++i < object->header->packet24_count;
         prim += 2 * sizeof(RenderPacket24)) {
        RenderPacket24 *packet = (RenderPacket24 *)prim + slot;
        opcode = packet->values0.bytes.command;
        packet->values0.value = color;
        packet->value1 = color;
        packet->value2 = color;
        packet->value3 = color;
        packet->values0.bytes.command = opcode;
    }
    for (i = -1; ++i < object->header->packet1c_count;
         prim += 2 * sizeof(RenderPacket1C)) {
        RenderPacket1C *packet = (RenderPacket1C *)prim + slot;
        opcode = packet->values0.bytes.command;
        packet->values0.value = color;
        packet->value1 = color;
        packet->value2 = color;
        packet->values0.bytes.command = opcode;
    }
}

static inline int primitiveBlendMode(int kind, int forcedKind, s16 mode) {
    if (kind == forcedKind) {
        return 1;
    }
    return mode;
}

static inline void setPrimitiveBlend(u8 *command, int enabled) {
    if (enabled) {
        *command |= 2;
    } else {
        *command &= ~2;
    }
}

void Render_SetEntityBlendMode(RenderObjectEntity *object, int mode) {
    int slot = g_ActiveDrawSlot;
    s16 savedMode = mode;
    int i;
    u8 *prim;
    RenderPrimitiveDescriptor *descriptor;

    if (!object->header || !object->draw_count) {
        return;
    }
    descriptor = object->primitive_descriptors;
    prim = object->primitive_buffer;
    for (i = 0; i < object->header->packet34_count;
         prim += 2 * sizeof(RenderPacket34), ++i, ++descriptor) {
        RenderPacket34 *packet;
        int enabled = primitiveBlendMode(descriptor->kind, 11, savedMode);
        packet = (RenderPacket34 *)prim + slot;
        setPrimitiveBlend(&packet->values0.bytes.command, enabled);
    }
    for (i = 0; i < object->header->packet28_count;
         prim += 2 * sizeof(RenderPacket28), ++i, ++descriptor) {
        RenderPacket28 *packet;
        int enabled = primitiveBlendMode(descriptor->kind, 16, savedMode);
        packet = (RenderPacket28 *)prim + slot;
        setPrimitiveBlend(&packet->values0.bytes.command, enabled);
    }
    for (i = 0; i < object->header->packet24_count;
         prim += 2 * sizeof(RenderPacket24), ++i, ++descriptor) {
        RenderPacket24 *packet;
        int enabled = primitiveBlendMode(descriptor->kind, 21, savedMode);
        packet = (RenderPacket24 *)prim + slot;
        setPrimitiveBlend(&packet->values0.bytes.command, enabled);
    }
    for (i = 0; i < object->header->packet1c_count;
         prim += 2 * sizeof(RenderPacket1C), ++i, ++descriptor) {
        RenderPacket1C *packet;
        int enabled = primitiveBlendMode(descriptor->kind, 26, savedMode);
        packet = (RenderPacket1C *)prim + slot;
        setPrimitiveBlend(&packet->values0.bytes.command, enabled);
    }
}


/* Texture-page transition deltas, indexed by old and requested page. */

/* Moves every packet of the object to texture page `page`: the current page
 * is read from the first packet and the per-page delta added to all. */
void Render_SetObjectTexturePage(RenderObjectEntity *obj, s32 page) {
    int slot = g_ActiveDrawSlot;
    int current = 0;
    RenderPrimCursor prim;
    int delta;
    int i;

    if (obj->header == NULL || obj->draw_count == 0)
        return;
    prim.bytes = obj->primitive_buffer;
    if (obj->header->packet34_count != 0)
        current = (prim.p34[slot].page_bits & 0x7F) >> 5;
    else if (obj->header->packet28_count != 0)
        current = (prim.p28[slot].page_bits & 0x7F) >> 5;
    if ((s16)page == current)
        return;
    delta = g_RenderTexturePageDeltas[current][(s16)page];
    for (i = 0; i < obj->header->packet34_count; i++) {
        RenderPacket34 *packet = &prim.p34[slot];

        packet->page_bits += delta;
        prim.p34 += 2;
    }
    for (i = 0; i < obj->header->packet28_count; i++) {
        RenderPacket28 *packet = &prim.p28[slot];

        packet->page_bits += delta;
        prim.p28 += 2;
    }
}

/* Parses a model block into the render object: section pointers, the
 * double-buffered GPU packets (command codes and texture coordinates), the
 * animated part lookup entries and one identity matrix per part. */


int Render_ReturnTrue(void *unused);

int Render_ReturnTrue(void *unused) {
    return 1;
}

/* Draws an object with an optional animation frame applied on top of its
 * saved model matrix, then refreshes the packets of both draw buffers. */
void Render_DrawWithAnim(RenderObjectEntity *obj, void *animation, s16 frame,
                         union RenderLightingMatrix *light) {
    if (animation != 0) {
        obj->animation_data = animation;
        Render_CopyMatrixBlock(&obj->model_matrix, (RenderMatrix *)g_RenderClutLookupTable, 1);
        Render_CopyMatrixBlock((RenderMatrix *)g_IdentityMatrixSource, &obj->model_matrix, 1);
        Anim_BuildRotationMatrices(obj, animation, frame);
    }
    Render_TransformVertices(obj);
    Render_CopyMatrixBlock((RenderMatrix *)g_RenderClutLookupTable, &obj->model_matrix, 1);
    Render_DrawObject(obj, light);
    Render_UpdateClutTable(obj, 1, g_ActiveDrawSlot);
    g_ActiveDrawSlot ^= 1;
    Render_UpdateClutTable(obj, 1, g_ActiveDrawSlot);
    g_ActiveDrawSlot ^= 1;
}

/* Equivalent branches preserve the stock compiler\'s allocation.
 * No packet operation depends on which of these identical branches is taken. */







/* Bounds vectors come in pairs per table entry; the first one stores the
 * hit radius in its pad word. */
void Render_InitObjectFromTable(RenderObjectEntity *obj, RenderObjectEntity *owner, int index) {
    obj->table_index = index;
    obj->header = 0;
    obj->animation_source = owner;
    obj->hit_cylinder.radius = owner->bounds_vertices[(short)index * 2].pad;
    obj->table_value2c = owner->bounds_vertices[(short)index * 2].x;
    obj->table_value2e = owner->bounds_vertices[(short)index * 2].y;
    obj->table_value30 = owner->bounds_vertices[(short)index * 2].z + obj->hit_cylinder.radius;
}

void Render_Noop(void) {
}

int Render_ReturnZero(void) {
    return 0;
}

void Render_CopyMatrixBlock(RenderMatrix *src, RenderMatrix *dst, s16 count) {
    int i;

    for (i = 0; i < count; i++, src++, dst++) {
        dst->rotation[0][0] = src->rotation[0][0];
        dst->rotation[0][1] = src->rotation[0][1];
        dst->rotation[0][2] = src->rotation[0][2];
        dst->rotation[1][0] = src->rotation[1][0];
        dst->rotation[1][1] = src->rotation[1][1];
        dst->rotation[1][2] = src->rotation[1][2];
        dst->rotation[2][0] = src->rotation[2][0];
        dst->rotation[2][1] = src->rotation[2][1];
        dst->rotation[2][2] = src->rotation[2][2];
        dst->translation[0] = src->translation[0];
        dst->translation[1] = src->translation[1];
        dst->translation[2] = src->translation[2];
    }
}

void Render_SetObjectAnim(RenderObjectEntity *arg0, RenderObjectEntity *arg1, short arg2) {
    if (arg0->header->part_count == 2) {
        arg0->animation_source = arg1;
        arg0->animation_state = 3;
    } else {
        arg0->animation_source = arg1;
        arg0->animation_state = 1;
    }
    arg0->animation_id = arg2;
}

void Render_ClearObjectAnim(RenderObjectEntity *arg0) {
    arg0->animation_source = 0;
    if (arg0->header->part_count == 2) {
        arg0->animation_state = 2;
    } else {
        arg0->animation_state = 0;
    }
}

int Render_FindAnimEntry(RenderObjectEntity *obj, int id, s32 *out) {
    RenderAnimationLookupEntry *entry = obj->animation_entries;
    int i;

    for (i = 0; i < obj->header->animation_entry_count; i++, entry++) {
        if (entry->animation_id == (s16)id) {
            out[0] = entry->value0;
            out[1] = entry->value1;
            out[2] = entry->value2;
            return 1;
        }
    }
    return 0;
}

void Render_OffsetObjectTextureCoordinates(RenderObjectEntity *object, int du, int dv, int clutOffset) {
    u8 savedClut = clutOffset;
    RenderPacket34 *quad = (RenderPacket34 *)object->primitive_buffer;
    RenderPacket28 *tri;
    int i, j;
    u8 triClut;
    for (i = 0; i < object->header->packet34_count; i++) {
        for (j = 0; j < 2; j++, quad++) {
            quad->v0 += dv;
            quad->u0 += du;
            quad->v1 += dv;
            quad->u1 += du;
            quad->v2 += dv;
            quad->u2 += du;
            /* Retail leaves u3 unchanged. */
            quad->v3 += dv;
            quad->clut += (s8)clutOffset;
        }
    }
    triClut = savedClut;
    /* Byte-sized copies preserve the two signed CLUT conversions in plain C. */
    tri = (RenderPacket28 *)quad;
    for (i = 0; i < object->header->packet28_count; i++) {
        for (j = 0; j < 2; j++, tri++) {
            tri->v0 += dv;
            tri->u0 += du;
            tri->v1 += dv;
            tri->u1 += du;
            tri->v2 += dv;
            tri->u2 += du;
            tri->clut += (s8)triClut;
        }
    }
}

/* The counter word sits in .text right after this routine (main.yaml
 * render/Render_Counter) and the routine works in $a0/$a1, both signs of a
 * hand-written assembler original; the two pins reproduce those registers. */
void Render_IncrementCounter(void) {
    int *ptr;
    int value;

    ptr = g_RenderCounter;
    value = *ptr;
    value++;
    *ptr = value;
}
