#include "pe1/entity_frame_update.h"
#include "pe1/entity_allocation.h"
#include "pe1/pm.h"
#include "pe1/geom_state.h"
#include "pe1/gte.h"
#include "pe1/scene_assets.h"
#include "pe1/render_buffers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"line %d: %s\n",__LINE__,#x); return 1; } } while (0)
/* Fixture task registry; cleanup itself is the original process manager. */
PmPrimarySlot primary[11];
PmSecondarySlot secondary[11];
static PmCommand *handlers[86];
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
static unsigned event_count;
static struct { PmSlotHeader *slot; int start; u32 x,y; } events[44];
static int start_task(PmSlotHeader *slot)
{
    events[event_count].x=Psyz_GteCtrlRead(24);
    events[event_count].y=Psyz_GteCtrlRead(25);
    events[event_count].slot=slot; events[event_count++].start=1;
    return 0;
}
static int execute_task(PmSlotHeader *slot)
{
    events[event_count].x=Psyz_GteCtrlRead(24);
    events[event_count].y=Psyz_GteCtrlRead(25);
    events[event_count].slot=slot; events[event_count++].start=0;
    return 0;
}
static int frame_tasks(void)
{
    PmCommand handler={0}; handler.start=start_task; handler.execute=execute_task;
    handlers[0]=handlers[85]=&handler;
    g_GeomScroll.state.position.screenOffsetX=125;
    g_GeomScroll.state.position.screenOffsetY=93;
    const u32 flags[]={0,0x80,0x84,0x180};
    for(unsigned trial=0;trial<4;++trial) {
        memset(primary,0,sizeof(primary)); memset(secondary,0,sizeof(secondary));
        for(unsigned i=0;i<11;++i) {
            primary[i].header.state=secondary[i].header.state=1;
        }
        primary[1].header.command=85;
        g_GameStateFlags=flags[trial]; event_count=0;
        CHECK(Pm_RunBatch()==0 && Pm_RunPrimary()==0);
        unsigned cursor=0;
        if(flags[trial]&0x80) {
            for(unsigned bank=0;bank<2;++bank) {
                for(unsigned i=0;i<11;++i) {
                    PmSlotHeader *slot=bank?&primary[i].header:&secondary[i].header;
                    CHECK(events[cursor].slot==slot && events[cursor].start==1);
                    CHECK(events[cursor].x==(125u<<16) && events[cursor].y==(93u<<16)); ++cursor;
                }
                for(unsigned i=0;i<11;++i) {
                    int runs=!(flags[trial]&4) && (!(flags[trial]&0x100) || (bank && i==1));
                    PmSlotHeader *slot=bank?&primary[i].header:&secondary[i].header;
                    if(runs) {
                        CHECK(events[cursor].slot==slot && events[cursor].start==0);
                        CHECK(events[cursor].x==(160u<<16) && events[cursor].y==(112u<<16)); ++cursor;
                    }
                    CHECK(slot->ticks==(unsigned)runs && slot->state==(runs?2:1));
                }
            }
        }
        CHECK(event_count==cursor);
    }
    /* Deferred deletion occupies two execution passes, without callbacks. */
    primary[0].header=(PmSlotHeader){.state=4,.command=0,.ticks=20};
    CHECK(Pm_Exec(0)==0 && primary[0].header.state==5 && primary[0].header.ticks==20);
    CHECK(Pm_Exec(0)==0 && primary[0].header.state==0 && primary[0].header.ticks==0);
    memset(primary,0,sizeof(primary)); memset(secondary,0,sizeof(secondary));
    handlers[0]=handlers[85]=NULL; g_GameStateFlags=0;
    return 0;
}
static unsigned initialized;
static void init_task(void) { ++initialized; }
static int scene_initialization(void)
{
    static PmSlotBanks banks;
    struct SceneFixture {
        SceneAssetBlob header;
        SceneAssetDirectory directory;
        SceneAssetRecord records[4];
    } fixture={0};
    fixture.header.directoryOffset=offsetof(struct SceneFixture,directory);
    fixture.directory.entries=(4u<<22)|offsetof(struct SceneFixture,records);
    fixture.records[0].handlerId=9;
    fixture.records[1].handlerId=85;
    fixture.records[2].handlerId=7;
    fixture.records[3].handlerId=84;
    PmCommand handler={0}; handler.init=init_task;
    for(unsigned i=0;i<86;++i) handlers[i]=&handler;
    memset(&banks,0xa5,sizeof(banks));
    g_GameState.scene_process_slots=&banks;
    g_GameState.loaded_scene_assets=(SceneAssetView *)&fixture;
    g_GameState.flags=0x10002; g_GameStateFlags=0;
    Pm_InitScene();
    CHECK(g_PmSlotTable==banks.primary && g_PmSlotTable2==banks.secondary);
    CHECK(initialized==11 && g_GameState.flags==10 && g_GameStateFlags==0x80);
    for(unsigned i=0;i<11;++i) {
        PmSlotHeader *slots[]={&g_PmSlotTable[i].header,&g_PmSlotTable2[i].header};
        for(unsigned j=0;j<2;++j) {
            CHECK(!slots[j]->state && !slots[j]->owner && !slots[j]->ticks);
            CHECK(slots[j]->command==255 && slots[j]->field02==255 && slots[j]->field03==255);
        }
    }
    CHECK(Pm_AllocSlot(0)==0 && Pm_AllocSlot(70)==11);
    g_PmSlotTable[0].header.state=2; g_PmSlotTable2[0].header.state=2;
    CHECK(Pm_AllocSlot(0)==1 && Pm_AllocSlot(70)==12);
    Pm_InitScene();
    CHECK(initialized==11 && g_PmSlotTable[0].header.state==2 && g_PmSlotTable2[0].header.state==2);
    memset(handlers,0,sizeof(handlers));
    g_GameState.scene_process_slots=NULL; g_GameState.loaded_scene_assets=NULL;
    g_GameState.flags=0; g_GameStateFlags=0;
    g_PmTables=(PmTables){handlers,primary,secondary};
    return 0;
}
static int native_process_slot_capacity(void)
{
    static struct { PmSlotBanks banks; u8 guard[16]; } buffer;
    for(unsigned index=0;index<11;++index) {
        memset(&buffer,0xa5,sizeof(buffer));
        PmPrimarySlot *slot=&buffer.banks.primary[index];
        slot->header.state=1; slot->header.command=7; slot->header.owner=NULL;
        slot->header.ticks=0x12345678;
        CHECK(FieldFx_GetSlot(&slot->effect)==(void **)&slot->effect.state.commands);
        memset(slot->effect.data,0x33,sizeof(slot->effect.data));
        CHECK(slot->header.state==1 && slot->header.command==7 && slot->header.owner==NULL &&
              slot->header.ticks==0x12345678);
        CHECK(FieldFx_InitTaskContext(&slot->task)==0);
        CHECK(slot->task.tasks.cursor==slot->task.tasks.arena && slot->header.field02==1 &&
              slot->header.ticks==0x12345678 && slot->header.owner==NULL);
        memset(slot->task.tasks.arena,0x44,sizeof(slot->task.tasks.arena));
        const u8 *bytes=(const u8 *)&buffer;
        size_t first=(const u8 *)slot-bytes, last=first+sizeof(*slot);
        for(size_t i=0;i<sizeof(buffer);++i)
            if(i<first || i>=last) CHECK(bytes[i]==0xa5);
    }
    printf("Original process slots: native scripts %zu bytes / tasks %zu bytes fit %zu-byte slots; all eleven neighbours and secondary bank protected\n",
           sizeof(FieldFxObject),sizeof(FieldFxTaskOwner),sizeof(PmPrimarySlot));
    return 0;
}

int main(void)
{
    g_PmTables=(PmTables){handlers,primary,secondary};
    CHECK(native_process_slot_capacity()==0);
    CHECK(scene_initialization()==0);
    CHECK(command_addresses()==0);
    CHECK(frame_tasks()==0);
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
