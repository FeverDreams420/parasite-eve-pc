#include "pe1/battle.h"
#include "pe1/field_effect_modules.h"
#include "pe1/fieldfx_task.h"
#include "pe1/psyq_bios.h"
#include "pe1/scene_assets.h"

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
  int i;
  register char *entry;
  FieldFxCallbackList *list = (FieldFxCallbackList *)out;
  stride += 4;
  entry = out;
  entry = entry + 0xC;
  list->callback = callback;
  list->stride = stride;
  list->count = count;
  for (i = 0; i < count; i++)
  {
    ((FieldFxCallbackListEntry *)entry)->active = 0;
    entry += stride;
  }

  return (stride * count) + 0xC;
}

int FieldFx_AllocCallbackList(void *arg0, int arg1, int arg2, int arg3, void *arg4) {
    char *header;
    char *entry;
    FieldFxCallbackList *list;
    int stride;
    int i;

    i = 0;
    stride = arg2 + 4;
    header = g_FieldFxActiveTaskSlot->end + arg1;
    list = (FieldFxCallbackList *)header;
    entry = header + 0xC;

    *(char **)arg0 = header;
    list->callback = (FieldFxCallbackListCallback)arg4;
    list->stride = stride;
    list->count = arg3;

    while (i < arg3) {
        ((FieldFxCallbackListEntry *)entry)->active = 0;
        i++;
        entry += stride;
    }

    return (stride * arg3) + 0xC;
}

void *FieldFx_AcquireCallbackSlot(char *list) {
    char frame[8];
    FieldFxCallbackList *header;
    FieldFxCallbackList *base;
    FieldFxCallbackListEntry *slot;
    int count;
    int limit;
    int stride;
    int i;

    (void)frame;
    /* The parameter itself becomes the entry cursor, so the header is read
     * through two copies taken before it advances, as retail does. */
    header = (FieldFxCallbackList *)list;
    base = (FieldFxCallbackList *)list;
    list += 0xC;
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
            return list + 0x4;
        }
    }
    return 0;
}

int FieldFx_UpdateCallbackList(void *data) {
    FieldFxCallbackList *list = data;
    int saved_age = g_FieldFxCurrentAge;
    char *entry = list->entries;
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
    char *entry = list->entries;
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
