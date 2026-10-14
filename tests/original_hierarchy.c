#include "pe1/field_actor.h"
#include <stdio.h>
extern FieldActor *g_FieldActorListHead;
void Entity_RollbackPositionHierarchy(FieldActor *);
void Entity_CopyParentPosition(void);
int main(void)
{
    FieldActor actors[4]={0};
    for (int i=0; i<4; ++i) {
        actors[i].next=i<3?&actors[i+1]:0;
        actors[i].base_x=10+i; actors[i].base_y=20+i; actors[i].base_z=30+i;
        actors[i].pos_x=100+i; actors[i].pos_y=200+i; actors[i].pos_z=300+i;
        actors[i].collision_face_mirror=5+i;
    }
    actors[1].parent=&actors[0];
    actors[2].parent=&actors[0];
    actors[3].parent=&actors[1];
    g_FieldActorListHead=actors;
    Entity_RollbackPositionHierarchy(&actors[3]);
    for (int i=0; i<4; ++i)
        if (actors[i].pos_x!=10+i || actors[i].pos_y!=20+i ||
            actors[i].pos_z!=30+i || actors[i].collision_face!=5+i) return 1;
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
