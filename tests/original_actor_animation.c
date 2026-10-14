#include "pe1/entity_animation.h"
#include "pe1/game_state.h"
#include "pe1/player_entity.h"
#include "pe1/scene_assets.h"
#include <stdio.h>
/* Record positional sound requests; no replacement audio ships with the game. */
AkaoVoiceBankEntry g_AkaoVoiceBankEntries[20];
static unsigned sounds;
int Asset_Find08w(int id,int mode,int x,int y,int z)
{
    (void)mode;(void)x;(void)y;(void)z;if(id==1000)++sounds;return 0;
}
int main(void)
{
    BattleEntity parent={0},child={0};
    RenderAnimationDataHeader actions[2]={{0}};
    actions[0].frame_count=4;actions[1].frame_count=8;
    for (unsigned i=0;i<2;++i) {
        g_GameState.bank_rows[i][0]=actions;g_GameState.bank_rows[i][1]=actions+1;
    }
    parent.next=&child;child.parent=&parent;child.modelIndex=1;
    g_FieldActorListHead=(FieldActor *)&parent;
    parent.entityFlags=0x100000;child.entityFlags=0x200000;
    Entity_SetActionMode(&parent,1);
    if(parent.animLastFrame!=7 || child.animLastFrame!=7 || child.actionMode!=1 || parent.actionData!=actions+1) return 1;
    parent.animFrame=0x18000;Entity_SetAction(&parent,1);
    if(parent.animFrame!=0x18000) return 2;
    parent.animStep=0x10000;Entity_AdvanceAnim(&child);
    if(parent.animFrame!=0x28000 || child.animFrame!=parent.animFrame || !(parent.entityFlags&0x800000)) return 3;
    parent.entityFlags=0;parent.animFrame=7<<16;Entity_AdvanceAnim(&parent);
    if(parent.animFrame || !(parent.entityFlags&8)) return 4;
    parent.entityFlags=0;parent.animStep=-0x10000;Entity_AdvanceAnim(&parent);
    if(parent.animFrame!=(7<<16) || !(parent.entityFlags&8)) return 5;
    parent.entityFlags=0x200;parent.animFrame=0;parent.animStep=0x30000;parent.animStopFrame=2;
    Entity_AdvanceAnim(&parent);
    if(parent.animFrame!=(2<<16)) return 6;
    parent.entityFlags=0;parent.actionMode=1;parent.animFrame=2<<16;parent.animPrev.fixed=0;parent.animStep=0x10000;
    g_PlayerEntity=(FieldActor *)&parent;
    g_AkaoVoiceBankEntries[0].index=1;g_AkaoVoiceBankEntries[0].frame=1;g_AkaoVoiceBankEntries[0].ids[0]=1000;
    Entity_AdvanceAnim(&parent);
    if(sounds!=1) return 7;
    g_PlayerEntity=0;
    g_GameState.bank_state_11=1;g_GameState.bank_state_12=1;
    g_AkaoVoiceBankEntries[4].group=parent.modelIndex;
    g_AkaoVoiceBankEntries[4].set=parent.teamId;
    g_AkaoVoiceBankEntries[4].index=parent.actionMode;
    g_AkaoVoiceBankEntries[4].frame=1;g_AkaoVoiceBankEntries[4].ids[1]=1000;
    parent.entityFlags=0;parent.animFrame=0;parent.animPrev.fixed=3<<16;parent.animStep=-0x10000;
    Entity_AdvanceAnim(&parent);
    if(sounds!=2) return 8;
    g_GameState.bank_state_11=0;g_GameState.bank_state_12=0;g_FieldActorListHead=0;
    puts("Original action propagation, parent clock, loop, clamp and sound trigger passed");
    return 0;
}
