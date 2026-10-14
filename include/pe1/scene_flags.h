#ifndef PE1_SCENE_FLAGS_H
#define PE1_SCENE_FLAGS_H

#include "common.h"

/* Host callback pointers; this is not a serialized disc record. */
/* Callback-filter record consumed by the entity position integration path. */
struct BattleEntity;
typedef struct SceneFlagRecord {
    u32 flags;
    u32 mask_all;
    u32 mask_any;
    void (*callback)(struct BattleEntity *, int *);
} SceneFlagRecord;



extern SceneFlagRecord *g_EntityInputRulesByMode[];
void Scene_CheckFlagBits(void *ctx, SceneFlagRecord **table, int *index_ptr);

/* Pad buttons released this frame (old & changed), tested by scene flag
 * records and task value ops. */
extern unsigned int g_FieldPadReleased;

#endif /* PE1_SCENE_FLAGS_H */
