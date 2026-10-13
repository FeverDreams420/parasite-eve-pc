#ifndef FX_COMMON_H
#define FX_COMMON_H

#include "common.h"
#include "pe1/akao/pan3d.h"
#include "pe1/akao_send.h"
#include "pe1/asset_tim.h"
#include "pe1/gte_types.h"
#include "pe1/projection_slot.h"
#include "pe1/psyq_gpu.h"
#include "pe1/render_matrix_slot.h"
#include "pe1/room_fx.h"

#include "fx_common_vectors.h"

typedef struct FxCommonSelection {
    s16 record;
    s16 pad2;
} FxCommonSelection;

typedef struct FxCommonIndexLink {
    s16 previous;
    s16 next;
} FxCommonIndexLink;

typedef struct FxCommonRecord {
    u8 data[108];
} FxCommonRecord;

typedef struct FxCommonNode FxCommonNode;

struct FxCommonNode {
    s16 id;
    s16 type;
    union {
        void *payload;
        struct {
            s16 variant;
            s16 subtype;
            void *payload;
        } configured;
    } args;
    u8 pad0C[0x10];
    s32 state[4];
    u16 value2C;
    u16 value2E;
    u8 pad30[2];
    s16 value32;
    s16 value34;
    s16 value36;
    void *data38;
    void *data3C;
    u8 pad40[0x1C];
    s16 bucket;
    u8 pad5E[2];
    FxCommonNode *parent;
    FxCommonNode *previous;
    FxCommonNode *next;
};

PE1_STATIC_ASSERT(sizeof(FxCommonNode) == sizeof(FxCommonRecord),
                  fx_common_node_record_size);

typedef struct FxCommonBucket {
    FxCommonNode *head;
    u8 pad4[104];
} FxCommonBucket;

typedef union FxCommonPacketTag { u32 packed; struct { unsigned int address:24; unsigned int length:8; } bits; struct { u8 address[3]; u8 length; } bytes; } FxCommonPacketTag;

typedef struct FxCommonBuffer {
    u8 *data;
    FxCommonPacketTag *allocation;
} FxCommonBuffer;

typedef struct FxCommonBufferRegion {
    u8 data[0x15F90];
} FxCommonBufferRegion;

typedef struct FxCommonGradientQuad {
    u32 tag;
    u8 color0[4];
    s16 x0;
    s16 y0;
    u8 color1[4];
    s16 x1;
    s16 y1;
    u8 color2[4];
    s16 x2;
    s16 y2;
    u8 color3[4];
    s16 x3;
    s16 y3;
} FxCommonGradientQuad;

typedef struct FxCommonDrawModePacket {
    FxCommonPacketTag tag;
    u32 command;
} FxCommonDrawModePacket;

typedef union FxCommonPacketCursor {
    FxCommonGradientQuad quad;
    struct {
        FxCommonDrawModePacket packet;
        u8 pad08[0x1C];
    } mode;
} FxCommonPacketCursor;

typedef union FxCommonAddress {
    void *pointer;
    u32 word;
} FxCommonAddress;

PE1_STATIC_ASSERT(sizeof(FxCommonGradientQuad) == 0x24,
                  fx_common_gradient_quad_size);
PE1_STATIC_ASSERT(sizeof(FxCommonDrawModePacket) == 8,
                  fx_common_draw_mode_packet_size);
PE1_STATIC_ASSERT(sizeof(FxCommonPacketCursor) == 0x24,
                  fx_common_packet_cursor_size);

typedef union FxCommonTransformSeed {
    RoomFxSeed8 room;
    s16 component[4];
    GteShortVector angles;
} FxCommonTransformSeed;

typedef struct FxCommonTransformNode {
    s16 id;
    s16 kind;                    /* 0x02 */
    void *resource;              /* 0x04 */
    RoomSpriteMatrix matrix;     /* 0x08; translation Z doubles as lifetime */
    FxCommonTransformSeed seed;  /* 0x28 */
    u8 pad30[2];
    s16 margin;                  /* 0x32 */
    u8 pad34[0x2C];
    void *record;                /* 0x60 */
    void *previous;              /* 0x64 */
    void *next;                  /* 0x68 */
} FxCommonTransformNode;

PE1_STATIC_ASSERT(sizeof(FxCommonTransformSeed) == 8,
                  fx_common_transform_seed_size);
PE1_STATIC_ASSERT(sizeof(FxCommonTransformNode) == sizeof(FxCommonRecord),
                  fx_common_transform_node_record_size);

/* Level-of-detail model node drawn by FxCommon_DrawLodNode: the model kind is picked
 * by the view depth against nearDepth / farDepth. */
typedef struct FxCommonLodNode {
    s16 id;
    s16 farKind;                 /* 0x02 */
    s16 middleKind;              /* 0x04 */
    s16 nearKind;                /* 0x06 */
    void *resource;              /* 0x08 */
    RoomSpriteMatrix matrix;     /* 0x0C */
    RoomFxSeed8 seed;            /* 0x2C */
    u8 pad34[2];
    s16 margin;                  /* 0x36 */
    int farDepth;                /* 0x38 */
    int nearDepth;               /* 0x3C */
    u8 pad40[0x2C];
} FxCommonLodNode;

PE1_STATIC_ASSERT(sizeof(FxCommonLodNode) == sizeof(FxCommonRecord),
                  fx_common_lod_node_record_size);

typedef struct FxCommonShortVec3 {
    s16 x;
    s16 y;
    s16 z;
} FxCommonShortVec3;

typedef struct FxCommonOffsetTable {
    s32 offsets[1];
} FxCommonOffsetTable;

typedef struct FxCommonOffsetEntry {
    u8 pad00[0x30];
    s32 value;
} FxCommonOffsetEntry;

typedef struct FxCommonOffsetByte {
    u8 value;
} FxCommonOffsetByte;

/* One record of a sampled path. The first record is the header, whose last
 * halfword holds the point count; the points follow it. */
typedef struct FxCommonPathPoint {
    s16 x, y, z;
    u16 count;                  /* 0x06: header record only */
} FxCommonPathPoint;

/* Fixed-stride motion storage; the sampler writes only the vector prefix. */
typedef struct FxCommonMotionRecord {
    FxCommonVec3 vector;
    s32 pad0C;
} FxCommonMotionRecord;
PE1_STATIC_ASSERT(sizeof(FxCommonMotionRecord) == 16, fx_motion_record_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(FxCommonMotionRecord, vector) == 0,
                  fx_motion_record_vector_offset);

typedef struct FxCommonMotionLane {
    s32 value;
    s32 pad04[3];
} FxCommonMotionLane;

typedef struct FxCommonMotionByte {
    u8 value;
} FxCommonMotionByte;

typedef struct FxCommonResourceState {
    s32 active;
    union {
        s16 half;
        s32 word;
    } current;
} FxCommonResourceState;

extern s16 g_FxCommonMotionBlendFrames;
extern u8 g_FxCommonPathWrapped;
extern GteMatrix g_FxCommonMirrorYMatrix;
extern FxCommonMotionRecord g_FxCommonMotionPositions[10];
extern FxCommonMotionLane g_FxCommonMotionPositionZ[10];
extern FxCommonMotionByte g_FxCommonMotionDeltaX[1];
extern FxCommonMotionByte g_FxCommonMotionDeltaY[1];
extern FxCommonMotionByte g_FxCommonMotionDeltaZ[1];
extern FxCommonMotionRecord g_FxCommonMotionDeltas[10];
extern FxCommonResourceState g_FxCommonResourceState;

extern s16 g_FxCommonRecordCursor;
extern FxCommonBucket g_FxCommonNodeBuckets[];
extern FxCommonRecord g_FxCommonRecords[];
extern s16 g_FxCommonIndexFreeNext[];
extern s16 g_FxCommonIndexLast;
extern u16 g_FxCommonIndexFreeHead;
extern FxCommonIndexLink g_FxCommonIndexLinks[];

extern u8 g_Akao3dVolumeMin;
extern u8 g_Akao3dVolumeMax;
extern u8 g_FxCommonNight;
extern s16 g_FxCommonMarkerFade;
extern FxCommonBuffer *g_FxCommonActiveBuffer;
extern FxCommonNode g_FxCommonDefaultNode;
extern RoomSpriteMatrix g_FxCommonViewMatrix;
extern RoomSpriteMatrix *g_FxCommonWorkMatrix;
extern TimRelativeTable g_FxCommonTimTable;

void Akao_Cmd_A9(int mode, int arg1);
void SetSemiTrans(void *packet, int enabled);
s16 FxCommon_DrawEffectMarkers(s16 value);
int FxCommon_SamplePath(u32 position, int radius, void *allocation,
                   FxCommonVec3 *vector, GteShortVector *angles);
void *PushMatrix(void);
int PopMatrix(void);
void Akao_Cmd_A0(int handle, int index, int value);
void Akao_Cmd_A2(int handle, int index, int value);
void Draw_CloseTextboxPage(int resourceId);
void Menu_SetHighlightColor(int resourceId, int red, int green, int blue);
void Render_SetupColorTable(int resourceId, int mode, s16 *result);
void FxCommon_DrawFullScreenFade(int value);
void FxCommon_FreeIndexLink(s16 id);
void FxCommon_UnlinkNode(FxCommonNode *node);
void FxCommon_SelectBufferAllocation(void **buffer);
typedef struct FxCommonPolyResource FxCommonPolyResource;
void FxCommon_DrawPolyResource(void *context, FxCommonPolyResource *resource);
int FxCommon_CheckBoundsWithMargin(int *point, int margin);
int FxCommon_CheckFourBoundsWithMargin(int *point, int margin);
typedef union FxCommonPolyModel FxCommonPolyModel;
void FxCommon_DrawModel(void *context, FxCommonPolyModel *model, int index);
void FxCommon_DrawModelTinted(void *context, FxCommonPolyModel *model, int index);
void FxCommon_DrawModelFlat(void *context, FxCommonPolyModel *model, int index);

#endif
