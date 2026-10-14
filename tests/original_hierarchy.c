#include "pe1/battle.h"
#include "pe1/entity_animation.h"
#include "pe1/game_state_types.h"
#include "pe1/pm_types.h"
#include "pe1/render_shadow.h"
#include "pe1/scene_assets.h"
#include <string.h>
#include <stdio.h>
extern FieldActor *g_FieldActorListHead;
void Entity_RollbackPositionHierarchy(FieldActor *);
void Entity_CopyParentPosition(void);
PE1_STATIC_ASSERT(offsetof(EnemyCombatant, statusFlags2) == offsetof(FieldActorState, status_flags2), enemy_state_status_host_match);
PE1_STATIC_ASSERT(offsetof(EnemyCombatant, lootItemId) == offsetof(FieldActorState, loot_item_id), enemy_state_loot_host_match);
PE1_STATIC_ASSERT(sizeof(EnemyCombatant) == sizeof(FieldActorState), enemy_state_host_size_match);

int main(void)
{
    FieldActor actors[4]={0};
    BattleTarget target={0}; BattleTargetWords words;
    uintptr_t pointer;
    target.actor=(BattleEntity *)&actors[3];
    memcpy(&words,&target,sizeof(words));
    if (words.word_00!=(uintptr_t)&actors[3]) return 5;
    actors[0].model_state.model_header=(RenderObjectHeader *)&actors[1];
    if (actors[0].model_state.allocation_active!=(uintptr_t)&actors[1]) return 6;
    actors[0].parent=&actors[2];
    memcpy(&pointer,(u8 *)&actors[0]+offsetof(BattleEntity,parent),sizeof(pointer));
    if (pointer!=(uintptr_t)&actors[2]) return 7;
    actors[0].parent=0;
    actors[0].shadow_quads=(RenderShadowQuad *)&actors[2];
    if (actors[0].allocation_block!=(uintptr_t)&actors[2]) return 8;
    ((RenderShadowActor *)&actors[0])->shadow_matrix_index=3;
    ((RenderShadowActor *)&actors[0])->shadow_intensity=128;
    if (actors[0].shadow_matrix_index!=3 || actors[0].shadow_intensity!=128) return 9;
    for (int i=0; i<4; ++i) {
        actors[i].next=i<3?&actors[i+1]:0;
        actors[i].base_x=10+i; actors[i].base_y=20+i; actors[i].base_z=30+i;
        actors[i].pos_x=100+i; actors[i].pos_y=200+i; actors[i].pos_z=300+i;
        actors[i].collision_face_mirror=(void *)(uintptr_t)(5+i);
    }
    actors[1].parent=&actors[0];
    actors[2].parent=&actors[0];
    actors[3].parent=&actors[1];
    g_FieldActorListHead=actors;
    Entity_RollbackPositionHierarchy(&actors[3]);
    for (int i=0; i<4; ++i)
        if (actors[i].pos_x!=10+i || actors[i].pos_y!=20+i ||
            actors[i].pos_z!=30+i || actors[i].collision_face!=(void *)(uintptr_t)(5+i)) return 1;
    if (actors[0].flags!=0x40000 || actors[1].flags || actors[2].flags || actors[3].flags) return 2;
    actors[0].pos_x=999; actors[0].rot_y=123;
    actors[1].flags=actors[3].flags=0x400000;
    Entity_CopyParentPosition();
    if (actors[1].pos_x!=999 || actors[3].pos_x!=999 ||
        actors[3].rot_y!=123 || actors[2].pos_x!=12) return 3;
    /* Reverse list order: grandchild reads its parent's previous position. */
    actors[1].pos_x=222; actors[3].next=&actors[1]; actors[1].next=&actors[0]; actors[0].next=0;
    g_FieldActorListHead=&actors[3];
    Entity_CopyParentPosition();
    if (actors[3].pos_x!=222 || actors[1].pos_x!=999) return 4;
    g_FieldActorListHead=0;
    Entity_CopyParentPosition();
    puts("Original hierarchy rollback and attachment list order passed");
    return 0;
}
