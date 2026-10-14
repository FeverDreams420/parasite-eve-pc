#include "pe1/pad_state.h"
#include "pe1/scene_flags.h"
#include <stdio.h>
#include "pe1/battle.h"
static unsigned calls;
static void trigger(struct BattleEntity *ctx, int *index) { ++calls; ctx->motionX+=1; *index=1; }
static void fallback(struct BattleEntity *ctx, int *index) { (void)index; ++calls; ctx->motionX+=10; }
int main(void)
{
    SceneFlagRecord rules[]={{1|4,3,4,trigger},{0,0,0,fallback}};
    SceneFlagRecord *table[]={rules,0};
    int mode=0;BattleEntity actor={0};
#define value actor.motionX
    g_FieldPadBits=3; g_FieldPadPressed=0;
    Scene_CheckFlagBits(&actor,table,&mode);
    if (value!=10 || calls!=1) return 1;
    mode=0; value=0; calls=0; g_FieldPadPressed=4;
    Scene_CheckFlagBits(&actor,table,&mode);
    if (value!=1 || calls!=1 || mode!=1) return 2;
    /* Callback mode changes do not redirect the current record walk. */
    SceneFlagRecord continued[]={{0x20|0x10,0,0,trigger},{0x20,0,0,trigger},{0,0,0,fallback}};
    table[0]=continued; mode=0;value=0;calls=0;
    Scene_CheckFlagBits(&actor,table,&mode);
    if (value!=2 || calls!=2) return 3;
    rules[0].flags=2|8; rules[0].mask_any=4; table[0]=rules;
    mode=0;value=0;calls=0;g_FieldPadBits=4;g_FieldPadReleased=4;
    Scene_CheckFlagBits(&actor,table,&mode);
    if (value!=1 || calls!=1) return 4;
    puts("Original input rule masks, callback continuation and fallback passed");
    return 0;
}
