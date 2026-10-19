/*
 * The orbit effect set: its module class and the effects it runs.
 *
 * The class is the seven methods the class table lists in front of the
 * set. The registration and the start method act only while the field
 * engine runs the object (status 3). room_m187 links the class as its only
 * module; room_m186, m385, m388 and m389 link it after the twelve-element
 * effect's module. The script and the lists are each room's own data
 * (pe1/room_orbit_set.h).
 *
 * The class and the set follow each other in all five rooms, with the
 * set's seeds as the unit's only rodata, so they are one unit.
 */

#include "pe1/field_actor.h"
#include "pe1/room_orbit_set.h"

#include "pe1/gte.h"
#include <string.h>
#include "room_lib.h"

void RoomEffect_OrbitSetNop0(void) {}
void RoomEffect_OrbitSetPlantScript(PmSlotHeader *object) {
    *FieldFx_GetSlot((FieldFxObject *)object) = g_RoomOrbitSetScript;
}
int RoomEffect_OrbitSetSpawn6(PmSlotHeader *object, int b, int c, intptr_t d, intptr_t e, intptr_t f) {
    FieldFx_Spawn6((intptr_t)object,b,c,d,(int)e,(int)f);
    return 0;
}
int RoomEffect_OrbitSetRegister(PmSlotHeader *object) {
    if(FieldFx_GetStatus(object)==3)
        FieldFx_Register((FieldFxObject *)object,g_RoomOrbitSetDrawList);
    return 0;
}
int RoomEffect_OrbitSetStart(PmSlotHeader *object) {
    int result=-1;
    if(FieldFx_GetStatus(object)==3) {
        result=FieldFx_UpdateSlots((FieldFxObject *)object,g_RoomOrbitSetUpdateList);
        result|=FieldFx_RunScript((FieldFxObject *)object,g_RoomOrbitSetInitList,g_RoomOrbitSetSpawnLayout);
    }
    if(result==-1)RoomEffect_OrbitSetClose(object);
    return 0;
}
int RoomEffect_OrbitSetClose(PmSlotHeader *object) {
    object->state=4;
    if((unsigned)FieldFx_GetStatus(object)>=2) {
        FieldActor *actor=((FieldFxObjectPrefix *)object)->actor;
        actor->state->core_flags &= 0xC0FFFFFFu;
        /* Animation events may finish before their effect slots expire. */
        if(actor->state->action_state) *actor->state->action_state=4;
    }
    return 0;
}
void RoomEffect_OrbitSetNop6(void) {}

/*
 * The orbit effect set: eight particles that orbit outwards from the actor,
 * a burst of eight sprites that fall to the floor over their shadows, and a
 * sprite that drifts away from the actor. Each effect is an init, draw and
 * update handler; the set's own init fills the four sprite records.
 *
 * room_m186, m187, m385, m388 and m389 link these twelve functions in this
 * order, with the same 0x20 bytes of read-only seeds; this unit is that
 * object, compiled into each of them. It starts after the no-op that ends
 * the room prefix. The four sprite records live in each room's own data.
 */

/* Copies the owner's matrix into the state, selects the texture page
 * and fills the four sprite records. */
/* The script object the set belongs to and its owner actor. */
int RoomEffect_OrbitSetInit(void *data0, void *data1, void *data2) {
    FieldFxObject * object = data0;
    void * unused = data1;
    RoomOrbitSetState * state = data2;

    FieldActor *owner;
    int allocation;
    RenderMatrix *source;
    RenderMatrix *matrixArg;
    void *callObject;
    int value;
    u16 value128a;
    u8 value128b;
    int value32;
    int packetValue;
    int valueNeg42;

    FieldFx_SetScriptContext(state);
    owner = object->prefix.actor;
    state->owner = owner;
    source = owner->render_object.matrices;
    state->matrix = *source;
    allocation = Asset_SearchByKeyType(0x24);

    callObject = object;
    state->asset.index = allocation;
    value = -0x29;
    value128a = 0x80;
    value128b = 0x80;
    value32 = 0x20;
    packetValue = 1;
    valueNeg42 = -0x2A;
    g_RoomOrbitParticlePacket.offset = value;
    value = 0x60;
    g_RoomOrbitBurstPacket.r = value;
    value = 0x40;
    matrixArg = &state->matrix;
    g_RoomOrbitBurstPacket.offset = valueNeg42;
    g_RoomOrbitShadowPacket.offset = valueNeg42;
    g_RoomOrbitSpritePacket.offset = valueNeg42;
    g_RoomOrbitParticlePacket.depth = value128a;
    g_RoomOrbitBurstPacket.depth = value128a;
    g_RoomOrbitSpritePacket.depth = value128a;
    g_RoomOrbitParticlePacket.code = 0;
    g_RoomOrbitParticlePacket.mode = 0;
    g_RoomOrbitParticlePacket.r = value128b;
    g_RoomOrbitParticlePacket.g = value128b;
    g_RoomOrbitParticlePacket.b = value128b;
    g_RoomOrbitParticlePacket.zero = 0;
    g_RoomOrbitBurstPacket.code = value32;
    g_RoomOrbitBurstPacket.mode = packetValue;
    g_RoomOrbitBurstPacket.g = value;
    g_RoomOrbitBurstPacket.b = value;
    g_RoomOrbitBurstPacket.zero = 0;
    g_RoomOrbitShadowPacket.code = value32;
    g_RoomOrbitShadowPacket.mode = packetValue;
    g_RoomOrbitShadowPacket.depth = 0;
    g_RoomOrbitShadowPacket.r = 0;
    g_RoomOrbitShadowPacket.g = 0;
    g_RoomOrbitShadowPacket.b = 0;
    g_RoomOrbitShadowPacket.zero = 0;
    packetValue = value128b;
    g_RoomOrbitSpritePacket.code = 0;
    g_RoomOrbitSpritePacket.mode = 0;
    g_RoomOrbitSpritePacket.r = packetValue;
    g_RoomOrbitSpritePacket.g = value128b;
    g_RoomOrbitSpritePacket.b = value128b;
    g_RoomOrbitSpritePacket.zero = 0;
    FieldFx_PlaySoundAtMatrixPosition(callObject, 0x592, matrixArg);
    return 0;
}

/* The set itself draws nothing; each effect draws its own sprites. */
int RoomEffect_OrbitSetDraw(void *data0, void *data1, void *data2) {


    return 0;
}

int RoomEffect_OrbitSetUpdate(void *data0, void *data1, void *data2) {
    void * arg0 = data0;
    char * event = data1;

    if (FieldFx_IsLastSlotHalted() == 1) {
        event[1] = 2;
    }
    return 0;
}

int RoomEffect_InitOrbitParticles(void *data0, void *data1, void *data2) {
    void * unused = data0;
    void * unused2 = data1;
    RoomOrbitParticleState * state = data2;

    RoomOrbitSetState *clock = FieldFx_GetScriptContext();
    unsigned int i;
    for (i=0;i<8;++i) {
        int angle;
        state->position[i].x = clock->matrix.translation[0];
        state->position[i].y = g_RoomFloorY->y;
        state->position[i].z = clock->matrix.translation[2];
        angle=rand()%0x1000;
        state->velocity[i].x=rsin(angle);
        angle=rand()%0x1000;
        state->velocity[i].z=rcos(angle);
        state->velocity[i].angle=-(rand()%8000+4000);
    }

    state->decay = 0x80;
    state->height = 0x400;
    state->radiusStep = -30;
    state->intensity = 0;
    state->radius = 0x258;
    return 0;
}

int RoomEffect_DrawOrbitParticles(void *data0, void *data1, void *data2) {
    void * unused0 = data0;
    void * unused1 = data1;
    RoomOrbitParticleState * fx = data2;

    RoomSpriteMatrix matrix;
    RoomFxVec4 scratchScale;
    RoomFxVec4 scale;
    RoomOrbitSetState *owner;
    short *depthSlot;
    unsigned int i;

    owner = FieldFx_GetScriptContext();

    i = 0;
    depthSlot = &g_RoomOrbitParticlePacket.depth;
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

    memset(&scale, 0, 0x10);
    scale.x = (s16)fx->height;
    scale.y = (s16)fx->height;
    scale.z = 0x400;
    scratchScale = scale;
    ScaleMatrix(&matrix, &scratchScale);

    while (i < 8) {
        matrix.t[0] = (s16)fx->position[i].x;
        matrix.t[1] = (s16)fx->position[i].y;
        matrix.t[2] = (s16)fx->position[i].z;
        *depthSlot = fx->decay >> 1;
        *((unsigned char *)depthSlot - 6) = fx->frame << 1;
        FieldFx_DrawGlowSprite((FieldGlowSprite *)((char *)depthSlot - 0xA), &matrix, 1);
        i++;
    }
    return 0;
}

int RoomEffect_UpdateOrbitParticles(void *data0, void *data1, void *data2) {
    void * unused = data0;
    u8 * control = data1;
    RoomOrbitParticleState * state = data2;

    unsigned int i;
    u8 intensity;
    u16 height;

    state->radius += state->radiusStep;
    if (state->decay >= 9) {
        state->decay -= 8;
    }

    for(i=0;i<8;++i) {
        state->position[i].y += (s8)(state->velocity[i].angle >> 8);
        state->position[i].x += (state->velocity[i].x * state->radius) >> 16;
        state->position[i].z += (state->velocity[i].z * state->radius) >> 16;
        state->velocity[i].angle += 0x258;
    }

    height = state->height + 0x46;
    intensity = state->intensity + 6;
    state->intensity = intensity;
    intensity >>= 4;
    state->height = height;
    state->frame = intensity + 1;
    if (intensity == 7) {
        state->frame = 0;
        control[1] = 2;
    }
    return 0;
}

int RoomEffect_InitOrbitBurst(void *data0, void *data1, void *data2) {
    void * unused0 = data0;
    void * unused1 = data1;
    RoomOrbitBurstState * state = data2;

    RoomOrbitSetState *view;
    unsigned int i;

    view = FieldFx_GetScriptContext();
    for(i=0;i<8;++i) {
        state->position[i].x=view->matrix.translation[0];
        state->position[i].y=g_RoomFloorY->y;
        state->position[i].z=view->matrix.translation[2];
        state->velocity[i].x=rand()%0x2000-0x1000;
        state->velocity[i].z=rand()%0x2000-0x1000;
        state->velocity[i].y=-(rand()%8000+8000);
        state->secondary[i].x=0;
        state->secondary[i].y=0;
        state->secondary[i].z=rand()%0x1000;
        state->active[i]=1;
        state->frame[i]=rand()%3;
    }

    state->depth = 0x80;
    state->scale = 0x200;
    state->phaseStep = 0xC8;
    state->count = 8;
    return 0;
}

int RoomEffect_DrawOrbitBurst(void *data0, void *data1, void *data2) {
    void * unused0 = data0;
    void * unused1 = data1;
    RoomOrbitBurstState * state = data2;

    RoomSpriteMatrix matrix;
    RoomFxVec4 scratchScale;
    RoomFxVec4 scale;
    RoomOrbitSetState *owner;
    RoomOrbitBurstVector *position;
    unsigned char *configA;
    unsigned char *configB;
    unsigned int i;

    owner = FieldFx_GetScriptContext();

    i = 0;
    configA = &g_RoomOrbitBurstPacket.code;
    configB = &g_RoomOrbitShadowPacket.code;
    position = state->position;
    FieldFx_SetSpriteTexturePreset(owner->asset.page);
    FieldFx_SetSpriteCellSize(0x20, 0x20);
    FieldFx_SetSpriteColorMode(0x10);
    FieldFx_SetSpriteBlendMode(2);

    while (i < 8) {
        if (state->active[i] == 1) {
            RotMatrix(&state->secondary[i], &matrix);
            memset(&scale, 0, 0x10);
            scale.x = state->scale;
            scale.y = state->scale;
            scale.z = 0x400;
            scratchScale = scale;
            ScaleMatrix(&matrix, &scratchScale);

            matrix.t[0] = position->x;
            matrix.t[1] = position->y;
            matrix.t[2] = position->z;
            *configA = state->frame[i] * 2 + 0x20;
            FieldFx_DrawGlowSprite((FieldGlowSprite *)(configA - 4), &matrix, 1);

            matrix.t[0] = position->x;
            matrix.t[1] = g_RoomFloorY->y;
            matrix.t[2] = position->z;
            *configB = state->frame[i] * 2 + 0x20;
            FieldFx_DrawGlowSprite((FieldGlowSprite *)(configB - 4), &matrix, 0);
        }
        i++;
        position++;
    }
    return 0;
}

int RoomEffect_UpdateOrbitBurst(void *data0, void *data1, void *data2) {
    void * unused = data0;
    unsigned char * control = data1;
    RoomOrbitBurstState * state = data2;

    unsigned int i;

    if ((short)state->depth >= 9) {
        state->depth -= 8;
    }

    for(i=0;i<8;++i) {
        if(state->active[i]==1) {
            state->position[i].x += (s8)((u16)state->velocity[i].x >> 8);
            state->position[i].y += (s8)((u16)state->velocity[i].y >> 8);
            state->position[i].z += (s8)((u16)state->velocity[i].z >> 8);
            state->velocity[i].y += 0x190;
            state->secondary[i].z += rand()%0x200;
            if(state->position[i].y > g_RoomFloorY->y) {
                state->count--;
                state->active[i]=0;
            }
        }
    }

    if (state->count == 0) {
        control[1] = 2;
    }
    return 0;
}

/* Sprite and floor-point offsets from the actor, for the two sides. */
static const RoomFxSeed8 s_OrbitSpriteSeed0 = {
    { 0xB8, 0x0B, 0x00, 0x00, 0x70, 0x17, 0x00, 0x00 }
};
static const RoomFxSeed8 s_OrbitSpriteSeed1 = {
    { 0xC8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
};
static const RoomFxSeed8 s_OrbitSpriteSeed2 = {
    { 0x48, 0xF4, 0x00, 0x00, 0x70, 0x17, 0x00, 0x00 }
};
static const RoomFxSeed8 s_OrbitSpriteSeed3 = {
    { 0x38, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
};

int RoomEffect_InitOrbitSprite(void *data0, void *data1, void *data2) {
    void * unused0 = data0;
    void * unused1 = data1;
    RoomOrbitSpriteFxParams * state = data2;

    RoomFxSeed8 seed0;
    RoomFxSeed8 seed1;
    RoomFxSeed8 seed2;
    RoomFxSeed8 seed3;
    unsigned short position[4];
    RoomOrbitSetState *view;

    view = FieldFx_GetScriptContext();
    seed0 = s_OrbitSpriteSeed0;
    seed1 = s_OrbitSpriteSeed1;
    seed2 = s_OrbitSpriteSeed2;
    seed3 = s_OrbitSpriteSeed3;

    if (*FieldFx_RegPtr(1) == 0) {
        ApplyMatrixSV((GteMatrix *)&view->matrix, &seed0.vector,
                      &state->velocity);
        ApplyMatrixSV((GteMatrix *)&view->matrix, &seed1.vector,
                      (GteShortVector *)position);
    } else {
        ApplyMatrixSV((GteMatrix *)&view->matrix, &seed2.vector,
                      &state->velocity);
        ApplyMatrixSV((GteMatrix *)&view->matrix, &seed3.vector,
                      (GteShortVector *)position);
    }

    state->x = position[0] + view->matrix.translation[0];
    state->y = g_RoomFloorY->y;
    state->z = position[2] + view->matrix.translation[2];
    state->depth = 0x80;
    state->scale = 0x800;
    state->phase = 0x258;
    state->alpha = 0;
    return 0;
}

int RoomEffect_DrawOrbitSprite(void *data0, void *data1, void *data2) {
    void * unused = data0;
    void * unused2 = data1;
    RoomOrbitSpriteFxParams * fx = data2;

    RoomSpriteMatrix matrix;
    RoomFxVec4 scratchScale;
    RoomFxVec4 scale;
    RoomOrbitSetState *owner;
    u8 *drawSlot;

    owner = FieldFx_GetScriptContext();
    FieldFx_SetSpriteTexturePreset(owner->asset.page);
    FieldFx_SetSpriteCellSize(0x20, 0x20);
    FieldFx_SetSpriteColorMode(0x10);
    FieldFx_SetSpriteBlendMode(2);

    matrix.m[2][2] = 0x1000;
    matrix.m[1][1] = 0x1000;
    matrix.m[0][0] = 0x1000;
    matrix.t[0] = matrix.t[1] = matrix.t[2] = 0;
    matrix.m[0][1] = matrix.m[0][2] = matrix.m[1][0] =
        matrix.m[1][2] = matrix.m[2][0] = matrix.m[2][1] = 0;

    memset(&scale, 0, 0x10);
    scale.x = fx->scale;
    scale.y = fx->scale;
    scale.z = 0x400;
    scratchScale = scale;
    ScaleMatrix(&matrix, &scratchScale);

    drawSlot = &g_RoomOrbitSpritePacket.code;
    matrix.t[0] = fx->x;
    matrix.t[1] = fx->y;
    matrix.t[2] = fx->z;
    *drawSlot = fx->alpha & 0xfffeu;
    g_RoomOrbitSpritePacket.depth = fx->depth;
    FieldFx_DrawGlowSprite((FieldGlowSprite *)(drawSlot - 4), &matrix, 1);
    return 0;
}

int RoomEffect_UpdateOrbitSprite(void *data0, void *data1, void *data2) {
    unsigned char * arg0 = data0;
    RoomFxControl * signal = data1;
    RoomOrbitSpriteFxParams * state = data2;

    RoomOrbitSpriteFxParams *p = state;
    unsigned short counter = state->alpha;

    state->alpha = counter + 1;
    if ((short)counter >= 16) {
        state->alpha = 15;
    }

    if ((s16)state->depth >= 9) {
        state->depth -= 8;
    }
    if (state->phase >= 21) {
        state->phase -= 20;
    }

    p->scale -= 0x64;
    p->x += (p->velocity.x * p->phase) >> 16;
    p->z += (p->velocity.z * p->phase) >> 16;

    if (signal->frame >= 21) {
        signal->state = 2;
    }
    return 0;
}

/* Complete authored M187 lists at FFA8/FFBC/FFD0, class FFF0 and
 * script 9000C..9007C. Only the pointer-bearing controller grows on PC;
 * retain the particles' and burst's two authored spare bytes. */
FieldFxHandler g_RoomOrbitSetInitList[] = {RoomEffect_OrbitSetInit,
    RoomEffect_InitOrbitParticles,RoomEffect_InitOrbitBurst,RoomEffect_InitOrbitSprite,
    (FieldFxHandler)(intptr_t)-1};
FieldFxHandler g_RoomOrbitSetDrawList[] = {RoomEffect_OrbitSetDraw,
    RoomEffect_DrawOrbitParticles,RoomEffect_DrawOrbitBurst,RoomEffect_DrawOrbitSprite,
    (FieldFxHandler)(intptr_t)-1};
FieldFxHandler g_RoomOrbitSetUpdateList[] = {RoomEffect_OrbitSetUpdate,
    RoomEffect_UpdateOrbitParticles,RoomEffect_UpdateOrbitBurst,RoomEffect_UpdateOrbitSprite,
    (FieldFxHandler)(intptr_t)-1};
u16 g_RoomOrbitSetSpawnLayout[] = {sizeof(RoomOrbitSetState),sizeof(RoomOrbitParticleState)+2,
    sizeof(RoomOrbitBurstState)+2,sizeof(RoomOrbitSpriteFxParams),65535,0};
int g_RoomOrbitSetScript[] = {
    0x00010000,0x00100000,0x10040014,0x10180015,0x00200001,0x00020000,
    0x30fb0000,0x00420000,0x12010000,0x00010002,0x00010001,0x00020004,
    0x00010001,0x00420000,0x120c0000,0x00020002,0x00110001,0x00010003,
    0x00020008,0x00110000,0x00010003,0x00020008,0x00110001,0x00010003,
    0x00020008,0x00110000,0x00010003,0x30e80000,0xffffffff,
};
PmCommand g_RoomOrbitSetHandlers = {RoomEffect_OrbitSetNop0,RoomEffect_OrbitSetPlantScript,
    RoomEffect_OrbitSetSpawn6,RoomEffect_OrbitSetRegister,RoomEffect_OrbitSetStart,
    RoomEffect_OrbitSetClose,RoomEffect_OrbitSetNop6};
RoomFxSpritePacket g_RoomOrbitSpritePacket,g_RoomOrbitParticlePacket;
RoomFxSpritePacket g_RoomOrbitBurstPacket,g_RoomOrbitShadowPacket;
