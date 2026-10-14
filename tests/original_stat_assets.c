#include "pe1/aya.h"
#include "pe1/resident_tables.h"
#include "room.h"
#include "pe1/battle.h"
#include "pe1/player_entity.h"
#include "pe1/stat_modifiers.h"
#include "pe1/inventory.h"
#include "pe1/inventory_slots.h"
#include "pe1/inventory_state.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
int Aya_StatBinarySearch(int value,int *table);
int Aya_StatOverflowQuery(int kind,int value);
#define CHECK(x) do { if(!(x)) { fprintf(stderr,"line %d: %s\n",__LINE__,#x); return 1; } } while(0)
static int recalc_stats(void)
{
    BattleEntity player={0}; Combatant core={0};
    player.core=&core; g_PlayerEntity=(FieldActor *)&player;
    const int modifiers[]={-3,0,5};
    for(int level=0;level<99;++level) for(unsigned variant=0;variant<3;++variant) {
        for(unsigned kind=0;kind<7;++kind) {
            int *growth=Aya_GetStatGrowthTable(kind);
            g_AyaSaveState.stats.levels[kind]=growth[level];
            g_StatScaleBase[kind]=modifiers[variant];
        }
        g_AyaSaveState.level=level;
        g_AyaSaveState.current_hp=core.curHP=core.hpMirror=9999;
        g_InvItemPtr=g_AyaSaveState.inventory_items;
        g_InvBonusSlotCount=variant*10;
        Inv_RecalcSlotStats();
        AyaLevelStats *base=Aya_LookupLevelStats(level);
        AyaLevelStats *scaled=Aya_LookupLevelStats(level+modifiers[variant]);
        int hp=(20+modifiers[variant])*base->hp/20;
        CHECK(core.maxHP==hp && core.curHP==hp && core.hpMirror==hp);
        CHECK(g_AyaSaveState.max_hp==hp && g_AyaSaveState.current_hp==hp);
        CHECK(core.stat1E==scaled->offense && core.stat20==scaled->defense);
        CHECK(core.maxAtk==scaled->battleMaxAtk && core.atbStep==scaled->battleAtbStep && core.atbRate==scaled->battleAtbRate);
        CHECK(core.statusStep3C==scaled->statusStep3C && core.statusStep3E==scaled->statusStep3E && core.stat22==scaled->battleStat22);
        CHECK(core.stat26==base->inventoryCapacity && core.field04.fieldId04==level+1);
        int capacity=base->inventoryCapacity<51?base->inventoryCapacity:50;
        CHECK(g_AyaSaveState.inventory_slot_count==capacity);
        capacity+=g_InvBonusSlotCount; if(capacity>50)capacity=50;
        CHECK(g_InvSlotLimit==capacity);
    }
    core.curHP=core.hpMirror=g_AyaSaveState.current_hp=1;
    Inv_RecalcSlotStats();
    CHECK(core.curHP==1 && core.hpMirror==1 && g_AyaSaveState.current_hp==1);
    s16 override[2]={0}; g_InvItemPtr=override; g_InvSlotLimit=2;
    Inv_SetAyaSlotCount(100); CHECK(g_AyaSaveState.inventory_slot_count==50 && g_InvSlotLimit==2);
    g_PlayerEntity=NULL; g_AyaSaveState.current_hp=123;
    Inv_RecalcSlotStats(); CHECK(g_AyaSaveState.current_hp==123);
    g_InvItemPtr=NULL;
    return 0;
}
int main(void)
{
    const char *path=getenv("PE_TEST_DISC");
    if(!path || !*path) return getenv("PE_REQUIRE_GAME_ASSETS")?1:77;
    PeDisc disc={0}; PeRoomCatalog catalog;
    CHECK(pe_disc_open(&disc,path) && pe_room_catalog_open(&catalog,&disc));
    CHECK(pe_disc_read(&disc,&catalog.archive,(uint64_t)catalog.sector_bounds[2]*2048,
                       &g_ResidentDataPack,sizeof(g_ResidentDataPack)));
    CHECK(g_ResidentDataPack.directory.statGrowth>=0);
    CHECK((unsigned)g_ResidentDataPack.directory.statGrowth+7*512<=sizeof(g_ResidentDataPack));
    CHECK(g_ResidentDataPack.directory.levelStats>=0);
    CHECK((unsigned)g_ResidentDataPack.directory.levelStats+99*sizeof(AyaLevelStats)<=sizeof(g_ResidentDataPack));
    unsigned queries=0;
    for(int kind=0;kind<7;++kind) {
        int *table=Aya_GetStatGrowthTable(kind);
        for(unsigned i=1;i<99;++i) CHECK(table[i]>=table[i-1]);
        for(int value=0;value<=32767;++value) {
            int expected=0; while(expected<98 && value>=table[expected+1]) ++expected;
            int fraction=48,distance=0;
            if(expected<98) {
                int span=table[expected+1]-table[expected];
                fraction=span?(int)(((int64_t)value-table[expected])*49/span):0;
                distance=table[expected+1]-value;
            }
            int level=-1,sub=-1,dist=-1,dist_sub=-1;
            Aya_StatQueryLevelAndSubLevel(kind,value,&level,&sub);
            Aya_StatQueryDistanceAndSubLevel(kind,value,&dist,&dist_sub);
            CHECK(level==expected && sub==(fraction>=49?48:fraction));
            CHECK(dist==distance && dist_sub==fraction);
            CHECK(Aya_StatBinarySearch(value,table)==expected);
            CHECK(Aya_StatOverflowQuery(kind,value)==(expected?value-table[expected-1]:0));
            ++queries;
        }
        Aya_StatQueryLevelAndSubLevel(kind,0,NULL,NULL);
        Aya_StatQueryDistanceAndSubLevel(kind,0,NULL,NULL);
    }
    CHECK(Aya_LookupLevelStats(-1)==Aya_LookupLevelStats(0));
    CHECK(Aya_LookupLevelStats(99)==Aya_LookupLevelStats(98));
    CHECK(recalc_stats()==0);
    puts("Original player stat recalculation: 297 level/modifier combinations and inventory capacity passed");
    pe_disc_close(&disc);
    printf("Original stat queries: %u values across seven real growth tables, level/sublevel/distance/overflow passed\n",queries);
    return 0;
}
