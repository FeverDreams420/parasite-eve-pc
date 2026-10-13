#ifndef PE1_MENU_QUEUE_H
#define PE1_MENU_QUEUE_H

#include "common.h"

void MenuInput_InitQueue(void);

typedef struct MenuQueueEntry {
    struct MenuQueueEntry *next;
    union {
        struct {
            int value0;
            int value1;
        } values;
        struct {
            int type;
            int flags;
        } input;
    } payload;
} MenuQueueEntry;

typedef MenuQueueEntry MenuInputQueuedEvent;

void Queue_Enqueue(int value0, int value1);
void Queue_DequeueByMask(int mask, MenuQueueEntry *out);

PE1_STATIC_ASSERT(sizeof(MenuQueueEntry) == 0x0C, menu_queue_entry_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(MenuQueueEntry, next) == 0x00,
                  menu_queue_entry_next_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(MenuQueueEntry, payload) == 0x04,
                  menu_queue_entry_payload_offset);
PE1_STATIC_ASSERT(PE1_OFFSETOF(MenuQueueEntry, payload.input.flags) == 0x08,
                  menu_queue_input_flags_offset);

#define MENU_EVENT_QUEUE_SIZE 20
extern MenuQueueEntry g_MenuEventQueuePool[MENU_EVENT_QUEUE_SIZE];
extern MenuQueueEntry *g_MenuEventQueueFreeList;
extern int g_MenuInputActive;
extern int g_MenuInputPollingPaused;
extern int g_MenuInputHeldStatusMask;
/* Set while a pause request waits for the disc-change bits to clear
 * (Menu_DeferredExec). */
extern int g_MenuInputPausePending;
extern MenuQueueEntry *g_MenuEventQueueHead;
extern MenuQueueEntry *g_MenuEventQueueTail;


void MenuInput_DispatchQueuedEvents(void);
int MenuInput_GetStatusFlags(void);
int MenuInput_GetRepeatStep(void);
void Menu_ResetInputState(void);
/* Releases the paused input polling once a disc change settles. */
void Menu_DeferredExec(void);
void MenuInput_SetPollingPaused(int paused);
void Draw_SelectBuffer(void);
void Draw_PresentFrame(int mode);

void Draw_InitBuffers(void);

#endif /* PE1_MENU_QUEUE_H */
