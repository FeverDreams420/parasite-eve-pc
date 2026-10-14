#include "pe1/aya.h"
#include "pe1/resident_tables.h"
#include "room.h"
#include <stdlib.h>
#include <stdio.h>
int Aya_StatBinarySearch(int value,int *table);
int Aya_StatOverflowQuery(int kind,int value);
#define CHECK(x) do { if(!(x)) { fprintf(stderr,"line %d: %s\n",__LINE__,#x); return 1; } } while(0)
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
    pe_disc_close(&disc);
    printf("Original stat queries: %u values across seven real growth tables, level/sublevel/distance/overflow passed\n",queries);
    return 0;
}
