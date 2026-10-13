#ifndef PE1_FIELDFX_TASK_POOL_H
#define PE1_FIELDFX_TASK_POOL_H

#include "common.h"

/* Field effect task pool and the active effect owner, shared by fieldfx_task.h
 * and field_effect_pool.h so both declare g_FieldFxActiveTaskSlot and g_FieldFxOwner with one
 * type. */
typedef struct FieldFxTaskSlot {
    u16 id;              /* 0xFFFF marks a free slot. */
    u16 age;
    char *start;
    char *end;
} FieldFxTaskSlot;

/* Observed prefix of the active effect owner; actor is at +0x08. */
typedef struct FieldFxObjectPrefix {
    u8 reserved00;
    u8 asset_type;
    u8 owner_kind;   /* 1 for a task-list owner, 0 after a script reset */
    u8 live_slots;   /* script slots alive at the last update pass */
    u8 reserved04[4];
    struct FieldActor *actor;
} FieldFxObjectPrefix;
PE1_STATIC_ASSERT(PE1_OFFSETOF(FieldFxObjectPrefix, actor) == 8,
                  fieldfx_object_actor_offset);

/* The running task context (FieldFxTaskContext in fieldfx_task.h). */
extern struct FieldFxTaskContext *g_FieldFxTaskContext;
extern FieldFxTaskSlot *g_FieldFxActiveTaskSlot;
extern FieldFxObjectPrefix *g_FieldFxOwner;

/* Each effect reads the pool and the active owner through its own view of
 * the record; the cast is free (a pointer copy). The argument names the
 * view the caller reads, so a call site never casts the result again;
 * code that needs only the common prefix (the owner's actor) reads
 * g_FieldFxOwner directly. */
#define ANIM_POOL_AS(type) ((type *)g_FieldFxActiveTaskSlot)
#define ANIM_OWNER_AS(type) ((type *)g_FieldFxOwner)
/* The same for the running task context, which effects read as their own
 * event state. */
#define FX_TASK_AS(type) ((type *)g_FieldFxTaskContext)

#endif
