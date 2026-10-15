#ifndef PE1_MEMCARD_H
#define PE1_MEMCARD_H

#include "common.h"
#include "pe1/save_blob.h"

typedef struct MemCardSaveSupportData {
    u16 cardProductCode;
    u16 reserved02;
    char saveFileFormat[12];
    char slotPathFormat[8];
    char deviceFormat[8];
    u8 reserved20[4];
} MemCardSaveSupportData;

PE1_STATIC_ASSERT(sizeof(MemCardSaveSupportData) == 0x24,
                  memcard_save_support_data_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(MemCardSaveSupportData, saveFileFormat) == 4,
                  memcard_save_support_data_file_format_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(MemCardSaveSupportData, slotPathFormat) == 0x10,
                  memcard_save_support_data_slot_format_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(MemCardSaveSupportData, deviceFormat) == 0x18,
                  memcard_save_support_data_device_format_offset);

extern const MemCardSaveSupportData g_MemCardSaveSupportData;

enum MemCardSaveSlotState {
    MEMCARD_SLOT_OCCUPIED = 1,
    MEMCARD_SLOT_EMPTY = 2,
    MEMCARD_SLOT_UNAVAILABLE = 3
};

/* Compact menu-side copy made from the first 0x80 bytes of the saved runtime
 * state.  The card scanner keeps one preview per port at g_MemCardPreviewBuffers and copies
 * the selected fields below into MemCardSaveSlot. */
/* Title bytes; the scanner copies them as aligned words. */
typedef union MemCardTitleText {
    u8 text[8];
    u32 words[2];
} MemCardTitleText;

typedef union MemCardAlternateTitleText {
    u8 text[12];
    u32 words[3];
} MemCardAlternateTitleText;

typedef struct MemCardSavePreview {
    MemCardTitleText primaryTitle;
    s32 playTimeMinutes;
    s32 gameTimeMinutes;
    MemCardAlternateTitleText alternateTitle;
    u8 reserved1C[0x0A];
    u16 maxHp;
    u16 currentHp;
    u8 levelIndex;
    u8 exGameIndex;
    u8 reserved2C[0x30];
    s16 progressStage;
    s16 mapNumber;
    u8 reserved60[4];
    s32 blendColor;
    u8 reserved68[0x18];
} MemCardSavePreview;

/* Per-port read buffer: the card file header is read into the raw bytes and
 * then viewed as the preview record. */
typedef union MemCardPreviewBuffer {
    MemCardSavePreview preview;
    u8 bytes[0x80];
} MemCardPreviewBuffer;

typedef struct MemCardSaveSlot {
    u8 state;
    u8 metadataReady;
    u8 reserved02[2];
    MemCardTitleText primaryTitle;
    s32 playTimeMinutes;
    s32 gameTimeMinutes;
    MemCardAlternateTitleText alternateTitle;
    s32 blendColor;
    s16 currentHp;
    s16 maxHp;
    u8 levelIndex;
    u8 titleStyleFlag;
    u8 exGameIndex;
    u8 reserved2B;
    s16 progressStage;
    s16 mapNumber;
    u8 reserved30[0x14];
} MemCardSaveSlot;

typedef struct MemCardPortState {
    u8 present;
    u8 managerState;
    u8 fileCount;
    u8 selectedSlot;
    u8 hasFiles;
    u8 listCursor;
    u8 scanCursor;
    u8 pendingError;
    u8 cardState;
    u8 reserved09;
    u8 sequence;
    u8 nextState;
    s32 fileDescriptor;
    u8 reserved10[4];
    s16 transferSize;
    s16 retryCount;
    u8 *transferData;
    MemCardSaveSlot slots[15];
} MemCardPortState;

PE1_STATIC_ASSERT(sizeof(MemCardSaveSlot) == 0x44, memcard_save_slot_size);
PE1_STATIC_ASSERT(sizeof(MemCardSavePreview) == 0x80,
                  memcard_save_preview_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(MemCardSavePreview, maxHp) == 0x26,
                  memcard_save_preview_max_hp_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(MemCardSavePreview, progressStage) == 0x5C,
                  memcard_save_preview_progress_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(MemCardSavePreview, blendColor) == 0x64,
                  memcard_save_preview_blend_color_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(MemCardSaveSlot, playTimeMinutes) == 0x0C,
                  memcard_save_slot_play_time_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(MemCardSaveSlot, blendColor) == 0x20,
                  memcard_save_slot_blend_color_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(MemCardSaveSlot, levelIndex) == 0x28,
                  memcard_save_slot_level_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(MemCardSaveSlot, progressStage) == 0x2C,
                  memcard_save_slot_progress_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(MemCardSaveSlot, state) == 0,
                  memcard_save_slot_state_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(MemCardSaveSlot, metadataReady) == 1,
                  memcard_save_slot_metadata_ready_offset);
PE1_STATIC_ASSERT(sizeof(MemCardPortState) == ((0x18 + sizeof(void *) + 15 * sizeof(MemCardSaveSlot) + _Alignof(MemCardPortState) - 1) / _Alignof(MemCardPortState)) * _Alignof(MemCardPortState), memcard_port_state_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(MemCardPortState, fileCount) == 2,
                  memcard_port_state_file_count_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(MemCardPortState, transferSize) == 0x14,
                  memcard_port_state_transfer_size_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(MemCardPortState, fileDescriptor) == 0x0C,
                  memcard_port_state_file_descriptor_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(MemCardPortState, transferData) == 0x18,
                  memcard_port_state_transfer_data_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(MemCardPortState, slots) == 0x18 + sizeof(void *),
                  memcard_port_state_slots_offset);

extern MemCardPortState g_MemCardPortStates[];
extern MemCardPreviewBuffer g_MemCardPreviewBuffers[];
extern int g_MemCardActivePortOneBased;
extern MemCardPortState *g_MemCardActiveState;
extern int g_MemCardActiveBytesRemaining;
extern int g_MemCardActivePromptPending;
extern int g_MemCardLoadSucceeded;

extern char g_MemCardFilePath[];
extern s8 *g_MemCardFilenameFormatPtr;

PE1_STATIC_ASSERT(PE1_OFFSETOF(MemCardSaveSlot, titleStyleFlag) == 0x29,
                  memcard_save_slot_title_style_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(MemCardPortState, selectedSlot) == 3,
                  memcard_port_selected_slot_offset);

void Save_CancelUiFlow(void);
void Menu_CloseSaveSlotListView(int slot);
void MemCard_AbortActiveOperation(MemCardPortState *state);
void MemCard_InitSlotState(void);
void MemCard_CloseAll(void);
void Save_BuildHeader(void);
int Save_BuildCardFile(char *path);
void Save_StartWriteSlot(int port, int slot);

void MemCard_CloseAllAndResetState(void);
void MemCard_InitState(void);
int MemCard_IsPortPresent(int port);
int MemCard_CheckPresent(int port);
void MemCard_StartRead(int port, int slot);
int MemCard_IsOperationPending(void);
void MemCard_UpdateSavePolling(void);
void MemCard_DelayedCallback(void);
void MemCard_SetDelayedCallback(void (*callback)(void));
int MemCard_GetPortSequence(int port);
void Save_SetTitleStyleFlag(int value);

MemCardSaveSlot *MemCard_GetSlot(unsigned int port, int slot);

/* Card manager state (memcard/save_flow.c). */
extern unsigned int g_MemCardServicedPort;
extern int g_MemCardInfoPollCountdown;
extern int g_MemCardConnectDebounce;
/* Per port: a card was seen connecting and still has to be serviced. */
extern int g_MemCardConnectPending[2];
/* MemCard_InitManager registered events and prepared the host card directories. */
extern int g_MemCardManagerReady;
extern int g_McOpPending;
extern int g_MemCardSavePollTimeout;
extern int g_MemCardReadContext;
extern void (*g_MemCardDelayedCallback)(void);
extern int g_MemCardDelayedCallbackTimer;
extern int D_800A5D50;

/* BIOS events MemCard_InitManager opens: hardware card class 0xF4000001
 * then software card class 0xF0000011, each for the IO end (0x0004),
 * error (0x8000), timeout (0x0100) and new card (0x2000) specs. */
enum MemCardEventIndex {
    MEMCARD_EVENT_HW_IO_END,
    MEMCARD_EVENT_HW_ERROR,
    MEMCARD_EVENT_HW_TIMEOUT,
    MEMCARD_EVENT_HW_NEW_CARD,
    MEMCARD_EVENT_SW_IO_END,
    MEMCARD_EVENT_SW_ERROR,
    MEMCARD_EVENT_SW_TIMEOUT,
    MEMCARD_EVENT_SW_NEW_CARD,
    MEMCARD_EVENT_COUNT
};
extern long g_MemCardEventHandles[MEMCARD_EVENT_COUNT];
/* Set by the event handlers, cleared after the manager services them. */
extern int g_MemCardEventF400Spec0004Flag;
extern int g_MemCardRemovedEventPending;
extern int g_MemCardEventF400Spec2000Flag;
extern int g_MemCardEventF000Spec0004Flag;
extern int g_MemCardEventF000Spec8000Flag;
extern int g_MemCardEventF000Spec2000Flag;

/* Card file being written: the 0x100-byte directory/icon header (its
 * 16-colour icon palette at +0x60), then the whole image. */
typedef struct MemCardFileHeader {
    /* 0x00 */ u16 productCode;
    /* 0x02 */ u8 iconDisplay;
    /* 0x03 */ u8 blockCount;
    /* 0x04 */ char title[64];
    /* 0x44 */ u8 reserved[28];
    /* 0x60 */ u16 palette[16];
    /* 0x80 */ u8 icon[128];
} MemCardFileHeader;
PE1_STATIC_ASSERT(sizeof(MemCardFileHeader) == 0x100, memcard_file_header_size);
extern MemCardFileHeader g_MemCardIconHeader;
extern u8 g_MemCardIconPalette[];
extern u8 g_MemCardFileBuffer[];
/* Runtime save state as read back from the card file. */
extern SaveBytes12E4 g_MemCardSaveStateBuffer;

#endif
