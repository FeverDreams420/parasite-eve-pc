#include "pe1/aya.h"
#include "pe1/resident_tables.h"
#include "room.h"
#include "pe1/battle.h"
#include "pe1/player_entity.h"
#include "pe1/stat_modifiers.h"
#include "pe1/inventory.h"
#include "pe1/inventory_slots.h"
#include "pe1/inventory_state.h"
#include "pe1/weapon_list_output.h"
#include "pe1/battle_modifiers.h"
#include "pe1/battle_saved_state.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
int Aya_StatBinarySearch(int value,int *table);
int Aya_StatOverflowQuery(int kind,int value);
#define CHECK(x) do { if(!(x)) { fprintf(stderr,"line %d: %s\n",__LINE__,#x); return 1; } } while(0)
static unsigned armor_third(unsigned value)
{
    if(value>999)value=999;
    if(value>=85)return (value*249/208+402)/10;
    unsigned root=0; while((root+1)*(root+1)<=value*3000)++root;
    return root/10;
}
static int equipment_init(void)
{
    unsigned pairs=0;
    g_PlayerEntity=NULL;
    memset(&g_AyaBattleState,0x5a,sizeof(g_AyaBattleState));
    unsigned char prefix[sizeof(g_AyaBattleState.words)];
    memcpy(prefix,g_AyaBattleState.words,sizeof(prefix));
    for(unsigned w=0;w<255;++w) {
        ItemDataRecord *weapon=Item_LookupBaseData(w);
        if(!weapon || weapon->kind<1 || weapon->kind>8)continue;
        for(unsigned a=0;a<255;++a) {
            ItemDataRecord *armor=Item_LookupBaseData(a);
            if(!armor || armor->kind!=9)continue;
            g_InventoryRuntime.equipment[0]=*weapon;
            g_InventoryRuntime.equipment[1]=*armor;
            g_AyaSaveState.inventory_slot_count=2;
            g_AyaSaveState.inventory_items[0]=0x100;
            g_AyaSaveState.inventory_items[1]=0x101;
            g_AyaSaveState.equipped_weapon_slot=0;
            g_AyaSaveState.equipped_armor_slot=1;
            g_SavedBattleStateTail.words[5]=0x12345678;
            AyaBattleState *out=NULL;
            Battle_InitEquipLists(&out);
            CHECK(out==&g_AyaBattleState);
            CHECK(out->action==&g_SavedBattleStateTail);
            CHECK(out->attributes==&g_BattleEquipStateBlock);
            CHECK(!memcmp(prefix,out->words,sizeof(prefix)));
            CHECK(out->action->words[5]==0x12345678);
            CHECK(out->action->weapon.kind==weapon->kind);
            unsigned attack=weapon->baseStats[0]+weapon->bonusStats[0];
            if(attack>999)attack=999;
            unsigned hit=0;
            for(unsigned e=0;e<weapon->tailCount;++e) {
                unsigned effect=weapon->tailData[e]&31;
                if(effect==16 || effect==17)hit=effect-15;
            }
            if(hit==1)attack/=2;
            CHECK(out->action->weapon.attack==attack);
            CHECK(out->attributes->parameterWord.fields.third==
                  armor_third(armor->baseStats[2]+armor->bonusStats[2]));
            CHECK(g_InvItemPtr==g_AyaSaveState.inventory_items);
            ++pairs;
        }
    }
    CHECK(pairs==86*51);
    printf("Original equipment initialization: %u real weapon/armor pairs passed\n",pairs);
    return 0;
}
static int armor_stats(void)
{
    ItemDataRecord *item=&g_InventoryRuntime.equipment[0];
    memset(item,0,sizeof(*item)); item->kind=9;
    g_AyaSaveState.inventory_slot_count=1; g_AyaSaveState.equipped_armor_slot=0;
    g_AyaSaveState.inventory_items[0]=0x100;
    for(unsigned value=0;value<=1200;++value) {
        item->bonusStats[0]=item->bonusStats[1]=item->bonusStats[2]=value;
        BattleAttributes out={0}; Inv_BuildArmorList(&out);
        CHECK(out.parameterWord.fields.first==(value>999?999:value));
        CHECK(out.parameterWord.fields.second==(value>999?999:value));
        CHECK(out.parameterWord.fields.third==armor_third(value) && out.effectFlags==0);
    }
    unsigned real_armors=0;
    for(unsigned id=0;id<255;++id) {
        ItemDataRecord *base=Item_LookupBaseData(id);
        if(!base || base->kind!=9)continue;
        CHECK(base->tailCount<=sizeof(base->tailData)); *item=*base;
        BattleAttributes out={0}; Inv_BuildArmorList(&out);
        CHECK(out.parameterWord.fields.third==armor_third(base->baseStats[2]+base->bonusStats[2]));
        ++real_armors;
    }
    CHECK(real_armors>0);
    const unsigned flags[]={0,1,2,4,8,16,0x4000,0x8000,0x20,0x40,0x80,0x800,0x2000,0x10000,0x200,0x20000,0x400,0x1000};
    memset(item,0,sizeof(*item)); item->kind=9; item->tailCount=1;
    for(unsigned effect=0;effect<18;++effect) {
        item->tailData[0]=effect;
        BattleAttributes out={0}; Inv_BuildArmorList(&out);
        CHECK(out.effectFlags==flags[effect]);
        for(unsigned stat=0;stat<7;++stat) {
            int expected=stat==0 && effect==11?3:stat==1 && effect==12?2:
                (stat==5 && effect==13)||(stat==1 && effect==15)?-2:0;
            CHECK(g_StatScaleBase[stat]==expected);
        }
        CHECK(g_InvBonusSlotCount==(effect>=8 && effect<=10 ? 1<<(effect-8):0));
    }
    /* Later effects win; modifier refresh restores the selected storage list. */
    item->tailCount=4; item->tailData[0]=8; item->tailData[1]=10; item->tailData[2]=12; item->tailData[3]=15;
    s16 storage[3]={0}; Inv_SetActiveListOverride(storage,3); Inv_SelectActiveList(1);
    BattleCmd_LoadWeaponModifiers();
    CHECK(g_InvBonusSlotCount==4 && g_StatScaleBase[1]==-2);
    CHECK(g_InvItemPtr==storage && g_InvSlotLimit==3);
    Inv_ResetActiveList(); g_AyaSaveState.equipped_armor_slot=-1;
    BattleAttributes out={0}; Inv_BuildArmorList(&out);
    CHECK(out.parameterWord.raw==0 && out.effectFlags==0 && g_InvBonusSlotCount==0);
    printf("Original armor parameters: %u real records, 1201 values and modifier/reset cases passed\n",real_armors);
    return 0;
}
static int weapon_stats(void)
{
    ItemDataRecord *item=&g_InventoryRuntime.equipment[0];
    memset(item,0,sizeof(*item)); item->kind=1;
    g_AyaSaveState.inventory_slot_count=1; g_InvBonusSlotCount=0;
    g_AyaSaveState.inventory_items[0]=0x100; g_AyaSaveState.equipped_weapon_slot=0;
    for(int value=0;value<=1200;++value) {
        item->bonusStats[0]=value; item->bonusStats[1]=value; item->bonusStats[2]=value;
        item->ammo=value%1024;
        WeaponListOutput out={0}; Inv_BuildWeaponList(0,&out);
        unsigned capped=value>999?999:value, square=capped*22500;
        CHECK(out.attack==capped && out.packed.bits.power==capped);
        CHECK(out.kind==1 && out.packed.bits.category==1 && out.packed.bits.count==item->ammo);
        CHECK((u32)out.range*out.range<=square && (u32)(out.range+1)*(out.range+1)>square);
        CHECK(out.effects.word==0x11);
    }
    unsigned real_weapons=0;
    for(unsigned id=0;id<255;++id) {
        ItemDataRecord *base=Item_LookupBaseData(id);
        if(!base || base->kind<1 || base->kind>8) continue;
        CHECK(base->tailCount<=sizeof(base->tailData));
        *item=*base;
        WeaponListOutput out={0}; Inv_BuildWeaponList(0,&out);
        unsigned range=base->baseStats[1]+base->bonusStats[1]; if(range>999)range=999;
        unsigned square=range*22500;
        CHECK((u32)out.range*out.range<=square && (u32)(out.range+1)*(out.range+1)>square);
        CHECK(out.kind==base->kind);
        ++real_weapons;
    }
    CHECK(real_weapons>0);
    printf("Original weapon parameters: %u real weapon records and 1201 boundary values passed\n",real_weapons);
    memset(item,0,sizeof(*item)); item->kind=1;
    item->bonusStats[0]=100; item->tailCount=1;
    for(unsigned effect=1;effect<=20;++effect) {
        item->tailData[0]=effect;
        WeaponListOutput out={0}; Inv_BuildWeaponList(0,&out);
        CHECK(out.attack==(effect==16?50:100));
        if(effect<=5) CHECK(out.effects.bits.actions==g_WeaponEffectCodes[effect]);
        if(effect>=6 && effect<=8) CHECK(out.effects.bits.mode==effect-5);
        if(effect==16 || effect==17) CHECK(out.effects.bits.hitState==effect-15);
        if(effect>=19) CHECK(out.effects.bits.slot==effect-17);
    }
    return 0;
}
static int item_lookups(void)
{
    unsigned base_count=(g_ResidentDataPack.directory.statGrowth-g_ResidentDataPack.directory.itemBaseData)/sizeof(ItemDataRecord);
    CHECK(base_count>0 && base_count<=255);
    ItemDataRecord *base=(ItemDataRecord *)(g_ResidentDataPack.bytes+g_ResidentDataPack.directory.itemBaseData);
    for(unsigned i=0;i<base_count;++i) CHECK(Item_LookupBaseData(i)==&base[i]);
    CHECK(Item_LookupBaseData(base_count)==NULL && Item_LookupBaseData(UINT32_MAX)==NULL);
    g_AyaSaveState.inventory_slot_count=1; g_InvBonusSlotCount=0; Inv_ResetActiveList();
    for(int id=-1;id<=0x209;++id) {
        ItemDataRecord *expected=NULL;
        if(id>=1 && (unsigned)id<=base_count) expected=&base[id-1];
        else if(id>=0x100 && id<0x180) expected=&g_InventoryRuntime.equipment[id-0x100];
        else if(id>=0x200 && id<=0x208) expected=&g_KeyItemRecords[id-0x200];
        g_AyaSaveState.inventory_items[0]=id;
        CHECK(Inv_LookupData(id)==expected && Inv_LookupActiveListData(0)==expected);
    }
    CHECK(Inv_LookupActiveListData(-1)==NULL && Inv_LookupActiveListData(1)==NULL);
    s16 override[]={0x100,0x208}; Inv_SetActiveListOverride(override,2); Inv_SelectActiveList(1);
    CHECK(Inv_LookupActiveListData(0)==&g_InventoryRuntime.equipment[0]);
    CHECK(Inv_LookupActiveListData(1)==&g_KeyItemRecords[8]);
    Inv_ResetActiveList();
    return 0;
}
static int inventory_selection(void)
{
    g_AyaSaveState.equipped_weapon_slot=3; g_AyaSaveState.equipped_armor_slot=7;
    CHECK(g_InventoryRuntime.tracked[0]==3 && g_InventoryRuntime.tracked[2]==7);
    g_InventoryRuntime.tracked[0]=9; g_InventoryRuntime.tracked[2]=11;
    CHECK(g_AyaSaveState.equipped_weapon_slot==9 && g_AyaSaveState.equipped_armor_slot==11);
    for(unsigned i=0;i<50;++i) {
        g_AyaSaveState.inventory_items[i]=i+1;
        CHECK(g_InventoryRuntime.slots[i]==i+1);
        g_InventoryRuntime.slots[i]=i+0x100;
        CHECK(g_AyaSaveState.inventory_items[i]==i+0x100);
    }
    g_AyaSaveState.inventory_slot_count=10; g_InvBonusSlotCount=2;
    Inv_ResetActiveList();
    CHECK(g_InvItemPtr==g_AyaSaveState.inventory_items && g_InvSlotLimit==12);
    CHECK(g_InvSelectionBits==g_InvSelectionBitStorage && g_InvSelectionBitWords==2);
    CHECK(!Inv_IsActiveListOverrideSelected());
    s16 storage[100]={0};
    Inv_SetActiveListOverride(storage,100); Inv_SelectActiveList(1);
    CHECK(g_InvItemPtr==storage && g_InvSlotLimit==100 && Inv_IsActiveListOverrideSelected());
    CHECK(g_InvSelectionBits==g_InvStorageSelectionBits && g_InvSelectionBitWords==4);
    Inv_SelectActiveList(0);
    CHECK(g_InvItemPtr==g_InventoryRuntime.slots && g_InvSlotLimit==12);
    Inv_SelectActiveList(1); Inv_ResetActiveList();
    CHECK(!g_InvActiveListOverride && !g_InvOverrideSlotLimit && !Inv_IsActiveListOverrideSelected());
    Inv_SelectActiveList(1); CHECK(g_InvItemPtr==g_AyaSaveState.inventory_items);
    return 0;
}
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
    PeDiscFile exe; ItemDataRecord key_records[9];
    CHECK(pe_disc_find(&disc,"SLUS_006.62",&exe));
    CHECK(pe_disc_read(&disc,&exe,0xa1e64-0x10000+0x800,key_records,sizeof(key_records)));
    memcpy(g_KeyItemRecords,key_records,sizeof(key_records));
    u8 effect_codes[6];
    CHECK(pe_disc_read(&disc,&exe,0x923d0-0x10000+0x800,effect_codes,sizeof(effect_codes)));
    CHECK(!memcmp(effect_codes,g_WeaponEffectCodes,sizeof(effect_codes)));
    CHECK(item_lookups()==0);
    CHECK(weapon_stats()==0);
    CHECK(armor_stats()==0);
    CHECK(equipment_init()==0);
    CHECK(inventory_selection()==0);
    CHECK(recalc_stats()==0);
    puts("Original player stat recalculation: 297 level/modifier combinations and inventory capacity passed");
    pe_disc_close(&disc);
    printf("Original stat queries: %u values across seven real growth tables, level/sublevel/distance/overflow passed\n",queries);
    return 0;
}
