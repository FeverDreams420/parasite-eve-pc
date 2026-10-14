#ifndef PE1_FIELDFX_CALLBACK_LIST_H
#define PE1_FIELDFX_CALLBACK_LIST_H

#include <stdint.h>

typedef int (*FieldFxCallbackListCallback)(int mode, void *state, intptr_t argument);

typedef struct FieldFxCallbackListEntry {
    signed short active;
    signed short age;
    _Alignas(void *) unsigned char state[];
} FieldFxCallbackListEntry;

typedef struct FieldFxCallbackList {
    int stride;
    int count;
    FieldFxCallbackListCallback callback;
    unsigned char entries[];
} FieldFxCallbackList;

/* Effect task pools: FieldFx_InitCallbackList lays out a list of count entries of
 * stride bytes (plus the active/age prefix) behind its header and returns
 * the bytes used; FieldFx_AcquireCallbackSlot claims the first inactive entry and returns
 * its state, or null when the list is full. */
int FieldFx_InitCallbackList(char *out, int stride, int count,
                  FieldFxCallbackListCallback callback);
void *FieldFx_AcquireCallbackSlot(char *list);

/* Age of the entry whose callback is running, saved and restored around
 * each list walk; callbacks read it as their elapsed frame count. */
extern int g_FieldFxCurrentAge;

int FieldFx_AllocCallbackList(void *arg0, int arg1, int arg2, int arg3,
                            FieldFxCallbackListCallback callback);

int FieldFx_DrawCallbackList(char *list);

#endif /* PE1_FIELDFX_CALLBACK_LIST_H */
