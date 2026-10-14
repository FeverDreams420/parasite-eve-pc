#include "pe1/scene_flags.h"
#include "pe1/field_movement.h"

/* USA executable input records 0x800942F0..0x80094487, with host function
 * pointers. Preserve record order, fallbacks and shared table entries. */
static SceneFlagRecord mode15[] = {
    {3,1,0x78,Scene_UpdateEntityFacing},
    {2,0,0x78,Scene_UpdatePlayerEntity},
    {0,0,0,0}
};
static SceneFlagRecord mode16[] = {
    {3,1,0x78,Scene_UpdateEntityFacing},
    {2,0,0x78,Scene_UpdatePlayerEntity},
    {0,0,0,Scene_Init}
};
static SceneFlagRecord mode05[] = {
    {3,1,0x78,Scene_UpdateEntityFacing},
    {2,0,0x78,Scene_UpdatePlayerEntity},
    {0,0,0,Scene_Init}
};
static SceneFlagRecord mode04[] = {
    {2,0,0x78,Scene_UpdateEntityFacing}, {0,0,0,0}
};
static SceneFlagRecord mode11[] = {
    {2,0,0x78,Scene_UpdateEntityFacingFromPad}, {0,0,0,Scene_SyncEntityAction}
};
SceneFlagRecord *g_EntityInputRulesByMode[50] = {
    [4]=mode04, [5]=mode05, [16]=mode05, [17]=mode11,
    [21]=mode15, [22]=mode16, [23]=mode05
};
