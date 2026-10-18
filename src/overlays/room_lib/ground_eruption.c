/*
 * The ground eruption: its module class and the eruption it runs.
 *
 * The class is the seven methods the class table lists in front of the
 * eruption. The registration and the start method act only while the
 * field engine runs the object (status 3), and the close method clears the
 * target's owner bits without marking the target closed (compare
 * RoomLib_CloseTarget). In room_m146 the class comes before the falling
 * sprite burst's module, which uses the shared class methods. The script
 * and the lists are each overlay's own data (pe1/room_ground_eruption.h).
 *
 * room_m141, m146, m153, m154, m328 and scene_e02, e04 and e05 link the
 * class and the eruption directly after each other, with the eruption's
 * seeds and scales as the unit's only rodata, so they are one unit.
 */

#include "pe1/pe_image.h"
#include "pe1/battle.h"
#include "pe1/geom_state.h"
#include "pe1/random.h"
#include "pe1/room_ground_eruption.h"

#include "render_layouts.h"
#include "room_lib.h"

void RoomEffect_GroundEruptionNop0(void) {}
void RoomEffect_GroundEruptionPlantScript(PmSlotHeader *object) {
    *FieldFx_GetSlot((FieldFxObject *)object)=g_RoomEruptionScript;
}
int RoomEffect_GroundEruptionSpawn6(PmSlotHeader *object,int b,int c,intptr_t d,intptr_t e,intptr_t f) {
    FieldFx_Spawn6((intptr_t)object,b,c,d,(int)e,(int)f);return 0;
}
int RoomEffect_GroundEruptionRegister(PmSlotHeader *object) {
    if(FieldFx_GetStatus(object)==3)FieldFx_Register((FieldFxObject *)object,g_RoomEruptionDrawList);
    return 0;
}
int RoomEffect_GroundEruptionStart(PmSlotHeader *object) {
    int result=-1;
    if(FieldFx_GetStatus(object)==3) {
        result=FieldFx_UpdateSlots((FieldFxObject *)object,g_RoomEruptionUpdateList);
        result|=FieldFx_RunScript((FieldFxObject *)object,g_RoomEruptionInitList,(u16 *)g_RoomEruptionSpawnLayout);
    }
    if(result==-1)RoomEffect_GroundEruptionRelease(object);
    return 0;
}
int RoomEffect_GroundEruptionRelease(PmSlotHeader *object) {
    object->state=4;
    if((unsigned)FieldFx_GetStatus(object)>=2)
        ((FieldFxObjectPrefix *)object)->actor->state->core_flags&=0xC0FFFFFFu;
    return 0;
}
void RoomEffect_GroundEruptionNop6(void) {}

/*
 * The ground eruption: a column that rises from the floor under its owner,
 * with a flash at the owner's feet, a sixteen-point shock ring that spreads
 * along the floor, a floor shadow, and six debris particles flung out and
 * falling back with trails. While it runs, a hit on the player sets the
 * player's 0x4000 status flag.
 *
 * room_m141, m146, m153, m154, m328 and scene_e02, e04 and e05 link these
 * eighteen functions in this order right after the module's class methods,
 * with the same 0x50 bytes of seeds and scales after the 0xC-byte room
 * header; this unit is that object, compiled into each of them. The four
 * sprite packets are room data (pe1/room_ground_eruption.h); in scene_e04
 * and scene_e05 they lie past the end of the extracted image and are named
 * at their addresses.
 */

static const RoomFxSeed8 s_EruptionColumnSeed = {
    { 0x00, 0x04, 0, 0, 0, 0, 0, 0 }
};
static const RoomFxVec4 s_EruptionGlowScale = { 0x400, 0x400, 0x1000, 0 };
static const RoomFxVec4 s_EruptionRingScale = { 0x6D4, 0x6D4, 0x6D4, 0 };
static const RoomFxVec4 s_EruptionRingPointScale = { 0x338, 0x338, 0x1000, 0 };
static const RoomFxSeed8 s_EruptionShadowSeed = {
    { 0, 0, 0, 0, 0x00, 0x04, 0, 0 }
};
static const RoomFxVec4 s_EruptionDebrisScale = { 0x226, 0x226, 0x226, 0 };

/* Sets up the four packets and the column's work area, then hands the work
 * area to the field engine's effect handler 0x587. */
int RoomEffect_GroundEruptionInit(void *object, void *slot, void *data) {
    FieldFxObjectPrefix *owner = object;
    void *arg1 = slot;
    RoomEruptionActor *state = data;
    FieldActor *entity;
    int asset;
    void *callOwner;
    int depth;
    int color;
    int offset;
    int modeB;
    RoomEruptionActor *workState;
    RoomSpriteMatrix *workMatrix;

    (workState = state);
    FieldFx_SetScriptContext(workState);
    workState->cooldown = 0;
    workState->collisionState = 0;
    workState->reload = *FieldFx_VarPtr(5);
    workState->collisionRadius = *FieldFx_VarPtr(4);
    entity = owner->actor;
    workState->entity = entity;
    memcpy(&workState->matrix,entity->render_object.matrices,sizeof(workState->matrix));
    workState->matrix.t[0] = *FieldFx_VarPtr(1);
    workState->matrix.t[1] = *FieldFx_VarPtr(2);
    workState->matrix.t[2] = *FieldFx_VarPtr(3);
    asset = Asset_SearchByKeyType(0xA6);
    callOwner = owner;
    workState->asset.handle = asset;

    g_RoomEruptionGlowPacket.code = 0x42;
    g_RoomEruptionGlowPacket.mode = 3;
    {
        int minus50;
        (minus50 = -50, depth = 0x80, color = 0x80);
        g_RoomEruptionGlowPacket.offset = minus50;
    }
    g_RoomEruptionRingPacket.code = 0x20;
    g_RoomEruptionRingPacket.mode = 1;
    g_RoomEruptionRingPacket.offset = -51;
    g_RoomEruptionShadowPacket.code = 0x40;
    {
        int two;
        (two = 2, offset = -41);
        g_RoomEruptionShadowPacket.mode = two;
    }
    g_RoomEruptionDebrisPacket.code = 0x47;
    modeB = 5;
    workMatrix = &workState->matrix;
    g_RoomEruptionGlowPacket.depth = depth;
    g_RoomEruptionRingPacket.depth = depth;
    g_RoomEruptionShadowPacket.depth = depth;
    g_RoomEruptionDebrisPacket.depth = depth;
    g_RoomEruptionShadowPacket.offset = offset;
    g_RoomEruptionDebrisPacket.offset = offset;
    g_RoomEruptionGlowPacket.r = color;
    g_RoomEruptionGlowPacket.g = color;
    g_RoomEruptionGlowPacket.b = color;
    g_RoomEruptionGlowPacket.zero = 0;
    g_RoomEruptionRingPacket.r = color;
    g_RoomEruptionRingPacket.g = color;
    g_RoomEruptionRingPacket.b = color;
    g_RoomEruptionRingPacket.zero = 0;
    g_RoomEruptionShadowPacket.r = color;
    g_RoomEruptionShadowPacket.g = color;
    g_RoomEruptionShadowPacket.b = color;
    g_RoomEruptionShadowPacket.zero = 0;
    g_RoomEruptionDebrisPacket.mode = modeB;
    g_RoomEruptionDebrisPacket.r = color;
    g_RoomEruptionDebrisPacket.g = color;
    g_RoomEruptionDebrisPacket.b = color;
    g_RoomEruptionDebrisPacket.zero = 0;
    g_RoomFloorY->y = 0;
    FieldFx_PlaySoundAtMatrixPosition(callOwner, 0x587, workMatrix);
    return 0;
}

int RoomEffect_GroundEruptionNop(void *object, void *slot, void *data) {

    return 0;
}

/* Fires once the owner's target reaches state 2 while armed: sets the
 * player's 0x4000 flag and marks the target, then waits the reload time. */
int RoomEffect_GroundEruptionTick(void *object,void *slot,void *data) {
    RoomEruptionActor *state=data;RoomFxControl *signal=slot;
    FieldActor *actor=((FieldFxObjectPrefix *)object)->actor;
    if(!state->cooldown) {
        if(state->collisionState==1 && FieldFx_GetStatus(object)==3 && *actor->state->action_state==2) {
            state->collisionState=0;state->cooldown=state->reload;
            ((Combatant *)g_PlayerEntity->state)->stateFlags|=0x4000;
            actor->state->core_flags|=0x80000000u;
        }
    } else {
        --state->cooldown;state->collisionState=0;
    }
    if(FieldFx_IsLastSlotHalted()==1)signal->state=2;
    return 0;
}

/* Emitter parameters of the rising column. */
int RoomEffect_GroundEruptionInitColumn(void *object, void *slot, void *data) {
    void *ent = object;
    void *unused = slot;
    RoomEruptionColumn *state = data;
    state->scale.signedValue = 0x5DC;
    state->depth = 0xFF;
    state->emitter.mode = 0x10;
    state->emitter.color0[0] = 0x40;
    state->emitter.color0[1] = 0x20;
    state->emitter.color0[2] = 0x10;
    state->emitter.color1[0] = 0x80;
    state->emitter.color1[1] = 0x80;
    state->emitter.color1[2] = 0x80;
    state->emitter.extent0 = 0xF0;
    state->emitter.extent1 = 0xA;
    state->speed.signedValue = 0;
    state->emitter.offset = 0;
    state->emitter.source = state->sourceData;
    FieldFx_BuildRingGeometry(&state->emitter);
    return 0;
}

/* The retail function is marked handwritten and uses four explicit GTE windows. */
int RoomEffect_GroundEruptionTransformColumn(void *object, void *slot, void *data) {
    void *arg0 = object;
    void *arg1 = slot;
    RoomEruptionColumn *state = data;
    RoomLibSpriteTransformStack stack;
    RoomSpriteMatrix *matrix;
    RoomSpriteMatrix *scaleMatrix;
    RoomEruptionColumn *workState;
    RoomEruptionActor *owner;

    workState = state;
    owner = FieldFx_GetScriptContext();
    stack.seed = s_EruptionColumnSeed;

    stack.scaleMatrix.m[2][2] = 0x1000;
    stack.scaleMatrix.m[1][1] = 0x1000;
    stack.scaleMatrix.m[0][0] = 0x1000;
    stack.scaleMatrix.t[2] = 0;
    stack.scaleMatrix.t[1] = 0;
    stack.scaleMatrix.t[0] = 0;
    stack.scaleMatrix.m[2][1] = 0;
    stack.scaleMatrix.m[2][0] = 0;
    stack.scaleMatrix.m[1][2] = 0;
    stack.scaleMatrix.m[1][0] = 0;
    stack.scaleMatrix.m[0][2] = 0;
    stack.scaleMatrix.m[0][1] = 0;
    memset(&stack.sourceScale, 0, 0x10);

    stack.sourceScale.x = workState->scale.signedValue;
    stack.sourceScale.y = workState->scale.signedValue;
    stack.sourceScale.z = workState->scale.signedValue;
    stack.scale = stack.sourceScale;
    scaleMatrix = &stack.scaleMatrix;
    ScaleMatrix(scaleMatrix, &stack.scale);
    RotMatrix(&stack.seed.vector, &stack.matrix);
    matrix = &stack.matrix;

    gte_ldrotmatrix(scaleMatrix);
    gte_ldrtir12_matrix_column(&stack.matrix.m[0][0]);
    gte_stir123_column(matrix);
    gte_ldrtir12_matrix_column(&stack.matrix.m[0][1]);
    gte_stir123_column_at(&stack.matrix.m[0][1]);
    gte_ldrtir12_matrix_column(&stack.matrix.m[0][2]);
    gte_stir123_column_at(&stack.matrix.m[0][2]);
    gte_ldtransmatrix(scaleMatrix);
    {
        s32 *translation = stack.matrix.t;
        gte_ldv0_word3(translation);
        gte_cop2_hazard_slot();
        gte_cop2_hazard_slot();
        gte_rtv0tr_sf0();
        gte_swc2_9_0(translation);
        gte_swc2_10_4(translation);
        gte_swc2_11_8(translation);
    }

    FieldFx_SetSpriteBlendMode(2);
    workState->emitter.intensity = (u16)workState->depth;
    stack.matrix.t[0] = owner->matrix.t[0];
    stack.matrix.t[1] = owner->matrix.t[1];
    stack.matrix.t[2] = owner->matrix.t[2];
    FieldFx_DrawShadedRing(&workState->emitter, matrix, 0);
    return 0;
}

typedef struct RoomEruptionRiseScript {
    u8 state;
    u8 result;
    s16 frame;
} RoomEruptionRiseScript;

/* Grows the column and fades it after eight frames; the column's base hits
 * the owner when the owner stands inside its radius. */
int RoomEffect_GroundEruptionUpdateColumn(void *object, void *slot, void *data) {
    RoomEruptionRiseScript *script = slot;
    RoomEruptionColumn *effect = data;
    s16 position[3];
    RoomEruptionActor *actor;
    s16 value;

    actor = FieldFx_GetScriptContext();
    if (script->frame >= 9) {
        value = effect->depth;
        if (value >= 0x21) {
            effect->depth = value - 0x20;
        }
    }

    effect->scale.signedValue = effect->scale.unsignedValue +
        (effect->speed.signedValue >> 4);
    effect->speed.signedValue = effect->speed.unsignedValue + 0x320;
    if (script->frame == 0x10) {
        script->result = 2;
    }

    position[0] = actor->matrix.t[0];
    position[1] = g_RoomFloorY->y;
    position[2] = actor->matrix.t[2];
    if (FieldFx_IsPlayerWithinRadius(position, actor->collisionRadius) != 0) {
        actor->collisionState = 1;
    }
    return 0;
}

int RoomEffect_GroundEruptionInitGlow(void *object, void *slot, void *data) {
    void *unused0 = object;
    void *unused1 = slot;
    RoomEruptionGlow *state = data;
    RoomEruptionActor *obj = FieldFx_GetScriptContext();
    int tmp;
    int cst;

    tmp = obj->matrix.t[0];
    state->x = tmp;
    tmp = obj->matrix.t[1];
    state->y = tmp;
    tmp = obj->matrix.t[2];
    cst = 0x400;
    state->phase = 0;
    state->scale = cst;
    state->z = tmp;
    return 0;
}

/* The flash at the owner's feet. */
int RoomEffect_GroundEruptionDrawGlow(void *object, void *slot, void *data) {

    RoomSpriteMatrix matrix;
    RoomFxVec4 scale;
    RoomEruptionActor *owner;

    owner = FieldFx_GetScriptContext();
    FieldFx_SetSpriteTexturePreset(owner->asset.page);
    FieldFx_SetSpriteCellSize(0x20, 0x20);
    FieldFx_SetSpriteColorMode(0x10);
    FieldFx_SetSpriteBlendMode(2);

    matrix.m[2][2] = 0x1000;
    matrix.m[1][1] = 0x1000;
    matrix.m[0][0] = 0x1000;
    matrix.t[2] = 0;
    matrix.t[1] = 0;
    matrix.t[0] = 0;
    matrix.m[2][1] = 0;
    matrix.m[2][0] = 0;
    matrix.m[1][2] = 0;
    matrix.m[1][0] = 0;
    matrix.m[0][2] = 0;
    matrix.m[0][1] = 0;
    scale = s_EruptionGlowScale;
    ScaleMatrix(&matrix, &scale);

    matrix.t[0] = owner->matrix.t[0];
    matrix.t[1] = g_RoomFloorY->y;
    matrix.t[2] = owner->matrix.t[2];
    FieldFx_DrawGlowSprite((FieldGlowSprite *)&g_RoomEruptionGlowPacket, &matrix, 1);
    return 0;
}

int RoomEffect_GroundEruptionUpdateGlow(void *object, void *slot, void *data) {
    void *unused = object;
    RoomFxControl *state = slot;
    void *obj = data;
    if (state->frame == 0x3A) {
        state->state = 2;
    }
    return 0;
}

int RoomEffect_GroundEruptionInitRing(void *object, void *slot, void *data) {
    void *arg0 = object;
    void *arg1 = slot;
    RoomEruptionRing *ring = data;
    RoomEruptionActor *owner;
    unsigned int i;
    int angle;

    owner = FieldFx_GetScriptContext();
    for (i = 0; i < 0x10; i++) {
        angle = i << 9;
        ring->point[i].x = owner->matrix.t[0];
        ring->point[i].y = owner->matrix.t[1] - 0x64;
        ring->point[i].z = owner->matrix.t[2];
        ring->direction[i].x = rsin(angle);
        ring->direction[i].pad02 = 0;
        ring->direction[i].z = rcos(angle);
    }

    ring->depth = 0x80;
    ring->radius = 0x400;
    ring->acceleration = -0x3C;
    ring->phase = 0;
    ring->speed = 0x320;
    return 0;
}

/* Draws the ring's centre sprite, then each point twice: in the air and
 * as its shadow on the floor. */
int RoomEffect_GroundEruptionDrawRing(void *object, void *slot, void *data) {
    void *arg0 = object;
    void *arg1 = slot;
    RoomEruptionRing *state = data;
    RoomSpriteMatrix matrix;
    RoomFxVec4 scale1;
    RoomFxVec4 scale2;
    RoomEruptionActor *rawOwner;
    RoomEruptionActor *owner;
    unsigned int i;
    s16 *packetDepth;
    RoomFxSpritePacket *packet;
    RoomEruptionRing *workState;
    char *depthAddress;
    char *packetAddress;
    int ownerZ;
    u8 code;

    workState = state;
    rawOwner = FieldFx_GetScriptContext();
    i = 0;
    depthAddress = (char *)&g_RoomEruptionRingPacket.depth;
    packetDepth = (s16 *)depthAddress;
    packetAddress = (char *)packetDepth - 10;
    packet = (RoomFxSpritePacket *)packetAddress;
    owner = rawOwner;
    FieldFx_SetSpriteTexturePreset((s8)owner->asset.page);
    FieldFx_SetSpriteCellSize(0x20, 0x20);
    FieldFx_SetSpriteColorMode(0x10);
    FieldFx_SetSpriteBlendMode(2);

    matrix.m[2][2] = 0x1000;
    matrix.m[1][1] = 0x1000;
    matrix.m[0][0] = 0x1000;
    matrix.t[2] = 0;
    matrix.t[1] = 0;
    matrix.t[0] = 0;
    matrix.m[2][1] = 0;
    matrix.m[2][0] = 0;
    matrix.m[1][2] = 0;
    matrix.m[1][0] = 0;
    matrix.m[0][2] = 0;
    matrix.m[0][1] = 0;
    scale1 = s_EruptionRingScale;
    ScaleMatrix(&matrix, &scale1);
    matrix.t[0] = owner->matrix.t[0];
    matrix.t[1] = owner->matrix.t[1];
    ownerZ = owner->matrix.t[2];
    packet->depth = 0x40;
    matrix.t[2] = ownerZ;
    g_RoomEruptionRingPacket.code = workState->frame * 2 + 0x20;
    FieldFx_DrawGlowSprite((FieldGlowSprite *)packet, &matrix, 1);

    matrix.m[2][2] = 0x1000;
    matrix.m[1][1] = 0x1000;
    matrix.m[0][0] = 0x1000;
    matrix.t[2] = 0;
    matrix.t[1] = 0;
    matrix.t[0] = 0;
    matrix.m[2][1] = 0;
    matrix.m[2][0] = 0;
    matrix.m[1][2] = 0;
    matrix.m[1][0] = 0;
    matrix.m[0][2] = 0;
    matrix.m[0][1] = 0;
    scale2 = s_EruptionRingPointScale;
    ScaleMatrix(&matrix, &scale2);

    for (; i < 16; i++) {
        GteShortVector *point = &workState->point[i];
        matrix.t[0] = point->x;
        matrix.t[1] = point->y;
        matrix.t[2] = point->z;
        *packetDepth = (u16)workState->depth;
        code = workState->frame;
        RoomEffect_SpriteControlFromDepth(packetDepth)->code = code * 2 + 0x20;
        FieldFx_DrawGlowSprite((FieldGlowSprite *)packet, &matrix, 1);
        matrix.t[0] = point->x;
        matrix.t[1] = g_RoomFloorY->y;
        matrix.t[2] = point->z;
        *packetDepth = workState->depth >> 2;
        code = workState->frame;
        RoomEffect_SpriteControlFromDepth(packetDepth)->code = code * 2 + 0x20;
        FieldFx_DrawGlowSprite((FieldGlowSprite *)packet, &matrix, 1);
    }
    return 0;
}

/* Spreads the ring and fades it out over seven steps. */
int RoomEffect_GroundEruptionUpdateRing(void *object, void *slot, void *data) {
    void *arg0 = object;
    unsigned char *signal = slot;
    RoomEruptionRing *rec = data;
    unsigned int i;
    GteShortVector *cur;
    unsigned char phase;
    int delta;

    rec->speed += rec->acceleration;
    if (rec->depth >= 0x11) {
        rec->depth -= 0x10;
    }

    for (i = 0; i < 0x10; i++) {
        cur = &rec->point[i];
        delta = (rec->direction[i].x * rec->speed) >> 16;
        cur->y += 8;
        cur->x += delta;
        delta = (rec->direction[i].z * rec->speed) >> 16;
        cur->z += delta;
    }

    phase = rec->phase + 6;
    rec->phase = phase;
    rec->frame = (phase >> 4) + 1;
    if ((phase >> 4) == 7) {
        rec->frame = 0;
        signal[1] = 2;
    }
    return 0;
}

int RoomEffect_GroundEruptionInitShadow(void *object, void *slot, void *data) {
    void *unused0 = object;
    void *unused1 = slot;
    RoomEruptionShadow *state = data;
    RoomEruptionActor *obj = FieldFx_GetScriptContext();
    unsigned int i;
    GteShortVector *entries = state->point;

    for (i = 0; i < 0x10; i++) {
        entries[i].x = obj->matrix.t[0];
        entries[i].y = obj->matrix.t[1];
        entries[i].z = obj->matrix.t[2];
    }

    state->depth = 0x80;
    state->scale = 0x800;
    return 0;
}

typedef struct RoomEruptionShadowStack {
    RoomFxSeed8 seed;
    RoomSpriteMatrix matrix;
    RoomSpriteMatrix generatedMatrix;
    RoomFxVec4 scale;
} RoomEruptionShadowStack;

int RoomEffect_GroundEruptionDrawShadow(void *object, void *slot, void *data) {
    void *unused0 = object;
    void *unused1 = slot;
    RoomFxGroundSpriteParams *fx = data;
    RoomEruptionShadowStack stack;
    RoomEruptionActor *owner;
    s16 *depthSlot;
    RenderMatrixSlot *matrixSlot;

    owner = FieldFx_GetScriptContext();
    stack.seed = s_EruptionShadowSeed;
    RotMatrix(&stack.seed.vector, &stack.generatedMatrix);
    FieldFx_SetSpriteTexturePreset(owner->asset.page);
    FieldFx_SetSpriteCellSize(0x20, 0x20);
    FieldFx_SetSpriteColorMode(0x10);
    FieldFx_SetSpriteBlendMode(2);

    stack.matrix.m[2][2] = 0x1000;
    stack.matrix.m[1][1] = 0x1000;
    stack.matrix.m[0][0] = 0x1000;
    stack.matrix.t[2] = 0;
    stack.matrix.t[1] = 0;
    stack.matrix.t[0] = 0;
    stack.matrix.m[2][1] = 0;
    stack.matrix.m[2][0] = 0;
    stack.matrix.m[1][2] = 0;
    stack.matrix.m[1][0] = 0;
    stack.matrix.m[0][2] = 0;
    stack.matrix.m[0][1] = 0;

    stack.scale = s_EruptionGlowScale;
    ScaleMatrix(&stack.matrix, &stack.scale);
    stack.matrix.t[0] = fx->x;
    stack.matrix.t[1] = g_RoomFloorY->y;
    stack.matrix.t[2] = fx->z;
    depthSlot = &g_RoomEruptionShadowPacket.depth;
    *depthSlot = fx->depth;
    matrixSlot = &g_ViewMatrixSlot;
    gte_ldrotmatrix(matrixSlot->value);
    gte_ldtransmatrix(matrixSlot->value);
    FieldFx_DrawGlowSprite((FieldGlowSprite *)((char *)depthSlot - 0xA), &stack.matrix, 1);
    return 0;
}

int RoomEffect_GroundEruptionNop2(void *object, void *slot, void *data) {

    return 0;
}

/* Flings the six debris particles out at random angles. */
int RoomEffect_GroundEruptionInitDebris(void *object, void *slot, void *data) {
    void *arg0 = object;
    void *arg1 = slot;
    RoomEruptionDebrisState *state = data;
    RoomEruptionActor *owner;
    unsigned int i;
    int angle;
    int wave;

    owner = FieldFx_GetScriptContext();
    FieldFx_VarPtr(0);
    angle = rand() % 0x1000;
    for (i = 0; i < 6; i++) {
        state->position[i].x = owner->matrix.t[0];
        state->position[i].y = owner->matrix.t[1];
        state->position[i].z = owner->matrix.t[2];

        wave = rsin(angle);
        state->velocity[i].x =
            (wave * (rand() % 0x800 + 0x800)) >> 12;
        state->velocity[i].y = -(rand() % 6096 + 3000);
        state->velocity[i].z =
            (rcos(angle) * (rand() % 0x800 + 0x800)) >> 12;

        state->gravityStep[i] = 500;
        state->visible[i] = 1;
    }

    state->depth = 0x80;
    state->scale = 0x400;
    state->speed = 0x320;
    return 0;
}

/* Draws the live debris; the first particle also leaves a floor shadow. */
int RoomEffect_GroundEruptionDrawDebris(void *object, void *slot, void *data) {
    void *arg0 = object;
    void *arg1 = slot;
    RoomEruptionDebrisState *state = data;
    RoomSpriteMatrix matrix;
    RoomFxVec4 scale;
    RoomEruptionActor *owner;
    int depth;
    unsigned int i;
    GteShortVector *particle;
    RoomFxSpriteControl *packetFields;
    RoomFxSpritePacket *packet;
    u8 code;
    u8 mode;

    owner = FieldFx_GetScriptContext();
    depth = 0x80;
    i = 0;
    code = 0x42;
    mode = 3;
    packetFields = (RoomFxSpriteControl *)&g_RoomEruptionDebrisPacket.code;
    packet = (RoomFxSpritePacket *)((char *)packetFields - PE1_OFFSETOF(RoomFxSpritePacket, code));
    FieldFx_SetSpriteTexturePreset(owner->asset.page);
    FieldFx_SetSpriteCellSize(0x20, 0x20);
    FieldFx_SetSpriteColorMode(0x10);
    FieldFx_SetSpriteBlendMode(2);
    matrix.m[2][2] = 0x1000;
    matrix.m[1][1] = 0x1000;
    matrix.m[0][0] = 0x1000;
    matrix.t[2] = 0;
    matrix.t[1] = 0;
    matrix.t[0] = 0;
    matrix.m[2][1] = 0;
    matrix.m[2][0] = 0;
    matrix.m[1][2] = 0;
    matrix.m[1][0] = 0;
    matrix.m[0][2] = 0;
    matrix.m[0][1] = 0;
    scale = s_EruptionDebrisScale;
    ScaleMatrix(&matrix, &scale);

    for (; i < 6; i++) {
        particle = &state->position[i];
        depth -= 20;
        if (state->visible[i] == 1) {
            matrix.t[0] = particle->x;
            matrix.t[1] = particle->y;
            matrix.t[2] = particle->z;
            if (i == 0) {
                FieldFx_SetSpriteBlendMode(2);
                packetFields->code = code;
                packetFields->mode = mode;
                packetFields->depth = 0xFF;
            } else {
                FieldFx_SetSpriteBlendMode(3);
                packetFields->code = 0x47;
                packetFields->mode = 5;
                packetFields->depth = depth;
            }
            FieldFx_DrawGlowSprite((FieldGlowSprite *)packet, &matrix, 1);
            if (i == 0) {
                matrix.t[0] = state->position[0].x;
                matrix.t[1] = g_RoomFloorY->y;
                matrix.t[2] = state->position[0].z;
                g_RoomEruptionDebrisDepth = 0x40;
                g_RoomEruptionDebrisPacket.code = code;
                g_RoomEruptionDebrisPacket.mode = mode;
                FieldFx_SetSpriteBlendMode(2);
                FieldFx_DrawGlowSprite((FieldGlowSprite *)packet, &matrix, 1);
            }
        }
    }
    return 0;
}

typedef struct RoomEruptionTrailEntry {
    u32 word0;
    u32 word4;
} RoomEruptionTrailEntry;

typedef struct RoomEruptionTrailTrigger {
    u8 pad00;
    u8 state;
    s16 timer;
} RoomEruptionTrailTrigger;

typedef union RoomEruptionDebris {
    RoomEruptionDebrisState state;
    RoomEruptionTrailEntry history[6];
} RoomEruptionDebris;

/* Moves the lead debris particle under gravity and shifts its trail. */
int RoomEffect_GroundEruptionUpdateDebris(void *owner, void *slot, void *data) {
    void *unused = owner;
    RoomEruptionTrailTrigger *trigger = slot;
    RoomEruptionDebris *object = data;
    int count;

    count = 5;
    while (count != 0) {
        object->history[count] = object->history[count - 1];
        object->state.visible[count] = object->state.visible[count - 1];
        count--;
    }

    object->state.position[0].x += object->state.velocity[0].x >> 8;
    object->state.position[0].y += object->state.velocity[0].y >> 8;
    object->state.position[0].z += object->state.velocity[0].z >> 8;
    object->state.velocity[0].y += object->state.gravityStep[0];

    if (object->state.position[0].y > g_RoomFloorY->y) {
        object->state.visible[0] = 0;
    }
    if (trigger->timer == 60) {
        trigger->state = 2;
    }
    return 0;
}

/* Complete USA M141/M328 tables. Original bytecode and callback ordering;
 * only pointer-bearing allocations grow for the native ABI. */
RoomFxSpritePacket g_RoomEruptionGlowPacket,g_RoomEruptionDebrisPacket,
    g_RoomEruptionShadowPacket,g_RoomEruptionRingPacket;
FieldFxHandler g_RoomEruptionInitList[]={RoomEffect_GroundEruptionInit,RoomEffect_GroundEruptionInitColumn,
    RoomEffect_GroundEruptionInitGlow,RoomEffect_GroundEruptionInitRing,
    RoomEffect_GroundEruptionInitShadow,RoomEffect_GroundEruptionInitDebris,(FieldFxHandler)(intptr_t)-1};
FieldFxHandler g_RoomEruptionDrawList[]={RoomEffect_GroundEruptionNop,RoomEffect_GroundEruptionTransformColumn,
    RoomEffect_GroundEruptionDrawGlow,RoomEffect_GroundEruptionDrawRing,
    RoomEffect_GroundEruptionDrawShadow,RoomEffect_GroundEruptionDrawDebris,(FieldFxHandler)(intptr_t)-1};
FieldFxHandler g_RoomEruptionUpdateList[]={RoomEffect_GroundEruptionTick,RoomEffect_GroundEruptionUpdateColumn,
    RoomEffect_GroundEruptionUpdateGlow,RoomEffect_GroundEruptionUpdateRing,
    RoomEffect_GroundEruptionNop2,RoomEffect_GroundEruptionUpdateDebris,(FieldFxHandler)(intptr_t)-1};
#define ERUPTION_SIZE(bytes) (((bytes)+sizeof(void *)-1)&~(sizeof(void *)-1))
int g_RoomEruptionSpawnLayout[]={ERUPTION_SIZE(sizeof(RoomEruptionActor))|(ERUPTION_SIZE(sizeof(RoomEruptionColumn))<<16),
    ERUPTION_SIZE(sizeof(RoomEruptionGlow))|(ERUPTION_SIZE(sizeof(RoomEruptionRing))<<16),
    ERUPTION_SIZE(0x88)|(ERUPTION_SIZE(sizeof(RoomEruptionDebrisState))<<16),0xffff};
#undef ERUPTION_SIZE
int g_RoomEruptionScript[]={0x00010000u,0x00100000u,0x10040000u,0x100d0001u,0x00200001u,0x00020000u,0x30fb0000u,0x00010001u,0x00010003u,0x00010005u,0x00010005u,0x00010005u,0x00010005u,0x00010005u,0x00010005u,0x00010005u,0x30f30000u,0xffffffffu};
PmCommand g_RoomGroundEruptionHandlers={RoomEffect_GroundEruptionNop0,RoomEffect_GroundEruptionPlantScript,
    RoomEffect_GroundEruptionSpawn6,RoomEffect_GroundEruptionRegister,RoomEffect_GroundEruptionStart,
    RoomEffect_GroundEruptionRelease,RoomEffect_GroundEruptionNop6};
