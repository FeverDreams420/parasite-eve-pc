#include "pe1/battle.h"
#include "pe1/field_effect_modules.h"
#include "pe1/fieldfx_task.h"
#include <stdio.h>
#include <limits.h>
#include "pe1/scene_assets.h"

/* Shared state of the original task/list walkers. */
FieldFxTaskContext *g_FieldFxTaskContext;
FieldFxTaskSlot *g_FieldFxActiveTaskSlot;
int g_FieldFxCurrentAge;

/* Binds task program `index` to the owner's task context. */
int FieldFx_BindTaskProgram(FieldFxTaskOwner *owner, int index)
{
    FieldFxTaskProgram *program = g_FieldFxTaskPrograms[index];

    if (program == 0) {
        printf(g_FieldFxTaskProgramMissingMsg, index);
        return -1;
    }
    owner->tasks.table = &program->table;
    owner->tasks.pc.script = program->script;
    return 0;
}

/* Effect resource `index`: the two texture pages, then the third resource. */
void *FieldFx_GetEffectResource(int index) {
    void *ret;

    switch (index) {
    case 0:
        ret = g_FieldEffectTexturePage0;
        return ret;
    case 1:
        ret = g_FieldEffectTexturePage1;
        return ret;
    case 2:
        ret = g_FieldEffectResource2;
        return ret;
    default:
        ret = 0;
        return ret;
    }
}
int FieldFx_InitCallbackList(char *out, int stride, int count, FieldFxTaskCallback callback)
{
    if (!out || (uintptr_t)out % _Alignof(FieldFxCallbackList) || stride < 0 || count < 0)
        return -1;
    size_t alignment = _Alignof(FieldFxCallbackListEntry);
    size_t native_stride = (size_t)stride + sizeof(FieldFxCallbackListEntry);
    native_stride = (native_stride + alignment - 1) / alignment * alignment;
    if (native_stride > INT_MAX || (size_t)count > (INT_MAX - sizeof(FieldFxCallbackList)) / native_stride)
        return -1;
    FieldFxCallbackList *list = (FieldFxCallbackList *)out;
    list->callback = callback;
    list->stride = (int)native_stride;
    list->count = count;
    char *entry = (char *)list->entries;
    for (int i = 0; i < count; ++i, entry += native_stride)
        ((FieldFxCallbackListEntry *)entry)->active = 0;
    return (int)(sizeof(FieldFxCallbackList) + native_stride * (size_t)count);
}

int FieldFx_AllocCallbackList(void *out, int offset, int stride, int count,
                            FieldFxTaskCallback callback)
{
    if (!out || offset < 0 || !g_FieldFxActiveTaskSlot || !g_FieldFxActiveTaskSlot->end)
        return -1;
    char *header = g_FieldFxActiveTaskSlot->end + offset;
    int bytes = FieldFx_InitCallbackList(header, stride, count, callback);
    if (bytes < 0) return bytes;
    *(char **)out = header;
    return bytes;
}

void *FieldFx_AcquireCallbackSlot(char *list) {
    FieldFxCallbackList *header;
    FieldFxCallbackList *base;
    FieldFxCallbackListEntry *slot;
    int count;
    int limit;
    int stride;
    int i;

    /* The parameter itself becomes the entry cursor, so the header is read
     * through two copies taken before it advances, as retail does. */
    header = (FieldFxCallbackList *)list;
    base = (FieldFxCallbackList *)list;
    list += sizeof(FieldFxCallbackList);
    count = header->count;
    stride = header->stride;
    i = 0;
    if (count > 0) {
        limit = count;
        while (1) {
            slot = (FieldFxCallbackListEntry *)list;
            if (slot->active == 0) {
                break;
            }
            i++;
            list += stride;
            if (i >= limit) {
                break;
            }
        }

        if (i < base->count) {
            slot->active = 1;
            slot->age = 0;
            return list + sizeof(FieldFxCallbackListEntry);
        }
    }
    return 0;
}

int FieldFx_UpdateCallbackList(void *data) {
    FieldFxCallbackList *list = data;
    int saved_age = g_FieldFxCurrentAge;
    char *entry = (char *)list->entries;
    int i;
    int active = 0;
    FieldFxTaskCallback callback = list->callback;
    int stride = list->stride;

    for (i = 0; i < list->count;) {
        if (((FieldFxCallbackListEntry *)entry)->active != 0) {
            active++;
            g_FieldFxCurrentAge = ((FieldFxCallbackListEntry *)entry)->age;
            if (callback(1, ((FieldFxCallbackListEntry *)entry)->state, g_FieldFxTaskContext->argument) != 0) {
                ((FieldFxCallbackListEntry *)entry)->active = 0;
            } else {
                ((FieldFxCallbackListEntry *)entry)->age++;
            }
        }
        i++;
        entry += stride;
    }
    g_FieldFxCurrentAge = saved_age;
    return active;
}

int FieldFx_DrawCallbackList(char *data) {
    FieldFxCallbackList *list = (FieldFxCallbackList *)data;
    int saved_age = g_FieldFxCurrentAge;
    char *entry = (char *)list->entries;
    int i;
    int active = 0;
    FieldFxTaskCallback callback = list->callback;
    int stride = list->stride;

    for (i = 0; i < list->count;) {
        if (((FieldFxCallbackListEntry *)entry)->active != 0) {
            active++;
            g_FieldFxCurrentAge = ((FieldFxCallbackListEntry *)entry)->age;
            callback(2, ((FieldFxCallbackListEntry *)entry)->state, g_FieldFxTaskContext->argument);
        }
        i++;
        entry += stride;
    }
    g_FieldFxCurrentAge = saved_age;
    return active;
}

void FieldFx_GetActorPosition(char *arg0, int arg1, s16 *arg2) {
    BattleEntity *actor = (BattleEntity *)arg0;

    switch (arg1) {
    case 0:
        arg2[0] = actor->renderObject.matrices[0].translation[0];
        arg2[1] = actor->renderObject.matrices[0].translation[1];
        arg2[2] = actor->renderObject.matrices[0].translation[2];
        break;
    case 1: {
        int value;

        value = actor->posX.parts.integer;
        arg2[0] = value;
        value = actor->posY.parts.integer;
        arg2[1] = value;
        value = actor->posZ.parts.integer;
        arg2[2] = value;
        break;
    }
    }
}
