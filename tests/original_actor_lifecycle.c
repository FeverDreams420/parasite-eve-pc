#include "pe1/entity_frame_update.h"
#include "pe1/entity_allocation.h"
#include "pe1/pm.h"
#include "pe1/render_buffers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"line %d: %s\n",__LINE__,#x); return 1; } } while (0)
/* Fixture task registry; cleanup itself is the original process manager. */
PmPrimarySlot primary[11];
PmSecondarySlot secondary[11];
PmPrimarySlot *g_PmSlotTable=primary;
PmSecondarySlot *g_PmSlotTable2=secondary;
static PmCommand *handlers[86];
PmCommand **g_PmCmdHandlerTable=handlers;
void *g_FieldFxTaskPrograms[104];
static unsigned stopped;
static int stop(PmSlotHeader *slot) { ++stopped; return 0; }
int Pm_ScriptSendCommand1(int **args);
int Pm_ScriptSetState(int **args);
int Pm_ScriptGetState(int **args);
static intptr_t sent[3];
static int send_command(PmSlotHeader *slot, int mode, int command,
                        intptr_t a, intptr_t b, intptr_t c)
{
    sent[0]=a; sent[1]=b; sent[2]=c;
    return 42;
}
static int command_addresses(void)
{
    PmCommand command={0}; command.send=send_command;
    handlers[0]=handlers[85]=&command;
    primary[0].header=(PmSlotHeader){.state=2,.command=0,.field02=17,.field03=29,.ticks=123456};
    int slot=0, subcommand=0, a=-1,b=-1,c=-1;
    int *args[]={&slot,&subcommand,&a,&b,&c};
    CHECK(Pm_ScriptSendCommand1(args)==1);
    CHECK(a==17 && b==29 && c==123456);
    CHECK(sent[0]==(intptr_t)&a && sent[1]==(intptr_t)&b && sent[2]==(intptr_t)&c);
    CHECK(Pm_SendCmd(0,0,1,-17,0,123)==42 && sent[0]==-17 && sent[2]==123);
    subcommand=3;
    CHECK(Pm_ScriptSetState(args)==1 && primary[0].header.state==3);
    subcommand=-1;
    CHECK(Pm_ScriptGetState(args)==1 && subcommand==3);
    CHECK(Pm_SetGetState(0,0,6)==3 && Pm_SetGetState(0,0,-1)==3);
    primary[0].header.command=114;
    CHECK(Pm_SendCmd(0,0,1,(intptr_t)&a,0,0)==42 && sent[0]==(intptr_t)&a);
    CHECK(Pm_SendCmd(-1,0,0,0,0,0)==-10);
    CHECK(Pm_SetGetState(22,0,0)==-13);
    memset(primary,0,sizeof(primary));
    handlers[0]=handlers[85]=NULL;
    return 0;
}
int main(void)
{
    CHECK(command_addresses()==0);
    unsigned char *memory=malloc(16u*ENTITY_ALLOCATION_BLOCK_SIZE);
    CHECK(memory);
    g_RenderScratchBufferBase[0]=memory;
    PmCommand command={0}; command.stop=stop; handlers[0]=&command;
    /* Delete head, middle, tail, and the only remaining actor. */
    const unsigned order[]={0,2,3,1};
    Entity_ResetAllPools(); Entity_InitFreePool();
    FieldActor *actors=g_EntityPool;
    g_EntityFreeListHead=&actors[4];
    g_FieldActorListHead=actors; g_EntityFreePoolCount=4;
    for(unsigned i=0;i<4;++i) {
        actors[i].prev=i?&actors[i-1]:NULL;
        actors[i].next=i<3?&actors[i+1]:NULL;
        actors[i].allocation_block=(uintptr_t)Entity_AllocBlock(1);
        CHECK(actors[i].allocation_block);
        actors[i].model_state.allocation_active=1;
        primary[i].header=(PmSlotHeader){.state=2,.command=0,.owner=&actors[i]};
        secondary[i].header=(PmSlotHeader){.state=2,.command=0,.owner=&actors[i]};
    }
    g_PlayerEntity=&actors[1]; g_FieldMoveLock=0x1f;
    for(unsigned n=0;n<4;++n) {
        unsigned index=order[n];
        actors[index].flags=0x800010;
        Entity_CollectGarbage();
        CHECK(g_EntityFreePoolCount==3-n && stopped==2*(n+1));
        CHECK(g_EntityFreeListHead==&actors[index]);
        CHECK(!(actors[index].flags&0x800000));
        CHECK(primary[index].header.state==0 && primary[index].header.owner==NULL);
        CHECK(secondary[index].header.state==0 && secondary[index].header.owner==NULL);
        CHECK(g_EntityAllocationTable[index].address==0);
        FieldActor *previous=NULL; unsigned live=0;
        for(FieldActor *p=g_FieldActorListHead;p;p=p->next) {
            CHECK(p->prev==previous && !(p->flags&0x10)); previous=p; ++live;
            CHECK(live<=4);
        }
        CHECK(live==3-n);
    }
    CHECK(!g_FieldActorListHead && !g_PlayerEntity && g_FieldMoveLock==0x12);
    unsigned available=0;
    for(FieldActor *p=g_EntityFreeListHead;p;p=p->next) CHECK(++available<=14);
    CHECK(available==14);
    Entity_CollectGarbage(); CHECK(stopped==8);
    g_RenderScratchBufferBase[0]=NULL; free(memory);
    puts("Original actor cleanup: head/middle/tail/last, player, model blocks and both task banks passed");
    return 0;
}
