#ifndef MENU_MEMCARD_IMAGE_H
#define MENU_MEMCARD_IMAGE_H

#include "common.h"
#include "pe1/menu_sound.h"
#include "pe1/cdrom_buffers.h"

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
#define g_MemcardImagePresets ((MemcardImagePreset *)(g_MemcardImageBundle + 0x3DB08))
extern u8 g_MenuCardAdvanceFadeEntry[];
/* USA title bank retains its authored offsets and preset data as data. */
#define g_MemcardImageBundle (g_StrFileDirBuffer + 0x4264)
#define g_MemcardImageOffsets ((s32 *)(g_MemcardImageBundle + 4))
extern u8 g_MenuCardUpdateFadeCounterEntry[];

MemcardImageNode *MenuCard_CreateImageNode(s32 kind);
void MenuCard_CreateFixedImageNode(void);
void MenuCard_CreateImageGroup(void);
extern u8 g_MenuCardUpdateFadeDisplayEntry[], g_MenuCardInitFadeForNodeEntry[];

void MenuCard_BlendImage(MemcardImageNode *node, u8 *destination, s32 padding);

void MenuCard_BlendRotatedImage(MemcardImageNode *node, u8 *destination, s32 padding, s32 stride);

/* Memory card list screen input. */
extern s32 g_MemcardPrevPad; /* Pad state of the previous frame. */
extern s32 g_MemcardScreenTimer;
extern u8 g_MenuCardAttachFadePayloadEntry[], g_MenuCardDecreaseFadeEntry[];
s32 Draw_RemapStatusFlags(void); /* Draw_RemapStatusFlags: pad state */
s32 MemCard_IsPortPresent(s32 port); /* MemCard_IsPortPresent */
s32 MemCard_HasAnyPresentFile(void);

#endif
