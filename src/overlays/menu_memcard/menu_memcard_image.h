#ifndef MENU_MEMCARD_IMAGE_H
#define MENU_MEMCARD_IMAGE_H

#include "common.h"
#include "pe1/menu_sound.h"

typedef struct MemcardImage { u8 header[16]; s16 width, height; } MemcardImage;
typedef struct MemcardImageNode {
    struct MemcardImageNode *next;
    s16 x, y, width, height;
    void *draw;
    void *update;
    void *field14;
    MemcardImage *volatile image;
    s32 intensity, targetY, stepY, parameter, kind, freePending;
} MemcardImageNode;
typedef struct MemcardImagePreset { s32 x, y, parameter; } MemcardImagePreset;
extern MemcardImageNode *g_MemcardFreeImageNodes, *g_MemcardPendingNodeTail, *g_MemcardPendingNodeHead, *g_MemcardActiveNodeHead;
extern MemcardImageNode *g_MemcardActiveNodeTail; /* Active-list tail. */
extern MemcardImageNode g_MemcardImageNodePool[8];
extern MemcardImagePreset D_801D0D5C[];
extern MemcardImagePreset D_801D0D8C;
extern s32 D_80193268;
extern u8 D_80193084[];
extern s32 g_MemcardImageOffsets[];
extern u8 D_80193254[], D_80192FE8[], D_8018F7F0[];

MemcardImageNode *MenuCard_CreateImageNode(s32 kind);
void MenuCard_CreateFixedImageNode(void);
void MenuCard_CreateImageGroup(void);
extern MemcardImagePreset D_801D0DB0, D_801D0DA4;
extern s32 D_80193274, D_80193270;
extern u8 D_801930D8[], D_8018F958[], D_80192F98[];

void MenuCard_BlendImage(MemcardImageNode *node, u8 *destination, s32 padding);

void MenuCard_BlendRotatedImage(MemcardImageNode *node, u8 *destination, s32 padding, s32 stride);

/* Memory card list screen input. */
extern s32 g_MemcardPrevPad; /* Pad state of the previous frame. */
extern s32 g_MemcardScreenTimer;
extern u8 D_801931BC[], D_8019316C[];
s32 Draw_RemapStatusFlags(void); /* Draw_RemapStatusFlags: pad state */
s32 MemCard_IsPortPresent(s32 port); /* MemCard_IsPortPresent */
s32 MemCard_HasAnyPresentFile(void);

#endif
