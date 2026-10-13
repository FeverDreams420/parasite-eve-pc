#ifndef PE1_SCENE_FLAGS_H
#define PE1_SCENE_FLAGS_H

#include "common.h"

/* Callback-filter record consumed by the entity position integration path. */
typedef struct SceneFlagRecord {
    u32 flags;
    u32 mask_all;
    u32 mask_any;
    void (*callback)(void *, void *);
} SceneFlagRecord;
PE1_STATIC_ASSERT(sizeof(SceneFlagRecord) == 0x10, scene_flag_record_size);
PE1_STATIC_ASSERT(PE1_OFFSETOF(SceneFlagRecord, callback) == 0x0C,
                  scene_flag_record_callback_offset);

extern SceneFlagRecord *g_EntityInputRulesByMode[];
void Scene_CheckFlagBits(void *ctx, SceneFlagRecord **table, int *index_ptr);

/* Pad buttons released this frame (old & changed), tested by scene flag
 * records and task value ops. */
extern unsigned int g_FieldPadReleased;

#endif /* PE1_SCENE_FLAGS_H */
