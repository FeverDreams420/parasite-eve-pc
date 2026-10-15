#include "pe1/game_timers.h"
#include "pe1/game_state.h"
#include "pe1/draw_wipe_rect.h"
#include "pe1/save.h"
#include "pe1/menu_queue.h"
#include "pe1/textbox.h"
#include "pe1/menu_flags.h"
#include "pe1/memcard.h"
#include "pe1/menu_inventory.h"
void Menu_FullInit(void);
void MemCard_ClearDelayedCallback(void);
int MemCard_HasDelayedCallback(void);
#include "pe1/aya.h"
#include "pe1/resident_tables.h"
#include "pe1/cdrom.h"
#include "pe1/battle.h"
#include "pe1/battle_cmd.h"
#include "pe1/player_entity.h"
#include "pe1/stat_modifiers.h"
#include "pe1/inventory.h"
#include "pe1/inventory_slots.h"
#include "pe1/inventory_state.h"
#include "pe1/weapon_list_output.h"
#include "pe1/battle_modifiers.h"
#include "pe1/battle_saved_state.h"
#include "pe1/menu_background.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
int Aya_StatBinarySearch(int value,int *table);
int Aya_StatOverflowQuery(int kind,int value);
#define CHECK(x) do { if(!(x)) { fprintf(stderr,"line %d: %s\n",__LINE__,#x); return 1; } } while(0)
static int pending_command_stack(void)
{
    ItemDataRecord records[3] = {0};
    BattleCmd_InitTableCursor();
    for (unsigned i = 0; i < 12; ++i) {
        BattleCmdEntry *entry = BattleCmd_AllocSlot();
        CHECK(entry == g_BattleCmdStack + (i < 4 ? i : 3));
        entry->header.word = i;
        entry->payload.ammo_restore.item_data0 = &records[0];
        entry->payload.ammo_restore.item_data1 = &records[1];
        entry->payload.ammo_restore.item_data2 = &records[2];
        entry->payload.ammo_restore.ammo2 = 100 + i;
        unsigned count = i < 4 ? i + 1 : 4;
        CHECK(g_BattleCmdStackTop == g_BattleCmdStack + count);
        for (unsigned j = 0; j < count; ++j) {
            CHECK(g_BattleCmdStack[j].header.word == i + 1 - count + j);
            CHECK(g_BattleCmdStack[j].payload.ammo_restore.item_data0 == &records[0]);
            CHECK(g_BattleCmdStack[j].payload.ammo_restore.item_data1 == &records[1]);
            CHECK(g_BattleCmdStack[j].payload.ammo_restore.item_data2 == &records[2]);
            CHECK(g_BattleCmdStack[j].payload.ammo_restore.ammo2 == (int)(101 + i - count + j));
        }
    }
    BattleCmd_InitTableCursor();
    CHECK(g_BattleCmdStackTop == g_BattleCmdStack);
    return 0;
}

static int menu_fade_parameters(void)
{
    for (int initial = 0; initial < 256; ++initial) {
        for (int threshold = 0; threshold < 256; ++threshold) {
            unsigned char expected[16] = {0};
            int count = 1;
            while (count < 16 && expected[count - 1] < threshold) {
                expected[count] = (initial * 256 + (256 - initial) * expected[count - 1]) / 256;
                ++count;
            }
            Menu_ComputeGammaLut(initial, threshold);
            CHECK(g_MenuSaveBgFadeLutLen == count);
            CHECK(!memcmp(expected, g_MenuSaveBgFadeLut, (size_t)count));
        }
    }
    for (int value = -1024; value <= 1024; ++value) {
        Menu_ClampRange(value);
        int expected = (unsigned)value % 512;
        CHECK(g_MenuItemContextFlag == (expected ? expected : 511));
    }
    Menu_SaveBgInitFade();
    return 0;
}

DRAWENV g_RenderDrawEnvArray[2];

static int menu_fade_pixels(void)
{
    static u16 source[8192], destination[8192];
    u16 *oldSource = g_GameState.save_background_source;
    u16 *oldDestination = g_GameState.save_background_destination;
    g_GameState.save_background_source = source;
    g_GameState.save_background_destination = destination;
    Menu_SaveBgInitFade();
    g_MenuSaveBgFadeHeight = 32;
    const int tints[] = {0, 72, 255};
    for (unsigned tint = 0; tint < 3; ++tint) {
        g_MenuSaveBgFadeTint = tints[tint];
        for (int frame = 0; frame < g_MenuSaveBgFadeLutLen; ++frame) {
            g_MenuSaveBgFadeStep = 0;
            g_MenuSaveBgFadeIndex = frame;
            unsigned level = g_MenuSaveBgFadeLut[frame];
            for (unsigned block = 0; block < 8; ++block) {
                for (unsigned i = 0; i < 8192; ++i) source[i] = block * 8192 + i;
                Menu_SaveBgApplyFadeStep();
                for (unsigned i = 0; i < 8192; ++i) {
                    unsigned pixel = source[i], expected = pixel & 0x8000;
                    if (pixel) {
                        for (unsigned channel = 0; channel < 3; ++channel) {
                            unsigned component = (pixel >> (channel * 5)) & 31;
                            unsigned scaled = (component * (65536 - level * 256) +
                                level * tints[tint] * (32 - component)) / 65536;
                            expected |= scaled << (channel * 5);
                        }
                    }
                    CHECK(destination[i] == expected);
                }
                unsigned shade = level * tints[tint] / 256;
                for (unsigned slot = 0; slot < 2; ++slot) {
                    CHECK(g_RenderDrawEnvArray[slot].r0 == shade);
                    CHECK(g_RenderDrawEnvArray[slot].g0 == shade);
                    CHECK(g_RenderDrawEnvArray[slot].b0 == shade);
                }
            }
        }
    }
    g_MenuSaveBgFadeStep = 1;
    Menu_SaveBgApplyFadeStep();
    CHECK(g_MenuSaveBgFadeState == 7 && g_MenuSaveBgFadeStep == 0);
    g_MenuSaveBgFadeIndex = 0;
    g_MenuSaveBgFadeStep = -1;
    Menu_SaveBgApplyFadeStep();
    CHECK(g_MenuSaveBgFadeState == 0 && g_MenuSaveBgFadeIndex == 0);
    CHECK(!memcmp(source, destination, sizeof(source)));
    g_GameState.save_background_source = oldSource;
    g_GameState.save_background_destination = oldDestination;
    Menu_SaveBgInitFade();
    return 0;
}

static unsigned armor_third(unsigned value)
{
    if(value>999)value=999;
    if(value>=85)return (value*249/208+402)/10;
    unsigned root=0; while((root+1)*(root+1)<=value*3000)++root;
    return root/10;
}
static int new_game_inventory(void)
{
    for(unsigned carry=0;carry<2;++carry) {
        memset(&g_PlayerInventoryState,0,sizeof(g_PlayerInventoryState));
        memset(g_InventoryRuntime.storage,0xa5,sizeof(g_InventoryRuntime.storage));
        memset(g_InventoryRuntime.specialStorage,0xa5,sizeof(g_InventoryRuntime.specialStorage));
        s16 override[2]={1,6};
        Inv_SetActiveListOverride(override,2);
        Inv_BuildItemGridFromCategory();
        CHECK(g_InvActiveListOverride==NULL);
        for(unsigned i=0;i<100;++i)CHECK(g_InventoryRuntime.storage[i]==0);
        for(unsigned i=0;i<82;++i)CHECK(g_InventoryRuntime.specialStorage[i]==0);
        for(unsigned i=0;i<9;++i) {
            ItemDataRecord expected=*Item_LookupBaseData(g_InvCategoryBaseItemId-1+i%3);
            expected.baseStats[2]=0; expected.bonusStats[2]=999;
            CHECK(!memcmp(&expected,&g_InvSpecialItemRecords[i],sizeof(expected)));
        }
        if(carry) {
            g_InventoryRuntime.equipment[120]=*Item_LookupBaseData(0x43);
            g_InventoryRuntime.equipment[121]=*Item_LookupBaseData(0x95);
            g_InventoryRuntime.equipment[120].flags|=16;
            g_InventoryRuntime.equipment[121].flags|=16;
            g_InventoryRuntime.specialStorage[0]=0x178;
            g_InventoryRuntime.specialStorage[1]=0x179;
        }
        Inv_InitNewGameInventory();
        AyaLevelStats *stats=Aya_LookupLevelStats(0);
        CHECK(g_AyaSaveState.current_hp==stats->hp && g_AyaSaveState.max_hp==stats->hp);
        CHECK(g_AyaSaveState.inventory_slot_count==stats->inventoryCapacity);
        for(unsigned i=0;i<7;++i)
            CHECK(g_AyaSaveState.stats.levels[i]==*(u16 *)Aya_GetStatGrowthTable(i));
        CHECK(g_AyaSaveState.parasite_spell_flags==1 && g_AyaSaveState.level==0 && g_AyaSaveState.total_exp==0);
        CHECK(g_AyaSaveState.equipped_weapon_slot==0 && g_AyaSaveState.equipped_armor_slot==1);
        const s16 expected[]={0x100,0x101,0x102,0x200,6};
        CHECK(!memcmp(g_AyaSaveState.inventory_items,expected,sizeof(expected)));
        CHECK(g_InvSpecialItemRecords[0].ammo==Item_LookupBaseData(0)->ammo);
        CHECK(!g_InvSpecialItemRecords[1].ammo && !g_InvSpecialItemRecords[2].ammo);
        CHECK(g_MenuItemContextFlag==61 && g_AyaSaveState.menu_clamp_value==61);
        CHECK(g_MenuSaveBgFadeState==0 && g_MenuSaveBgFadeStep==0 && g_MenuSaveBgFadeHeight==32 && g_MenuSaveBgFadeTint==72);
        unsigned value=0, length=0;
        for(unsigned i=0;i<16;++i) {
            CHECK(g_MenuSaveBgFadeLut[i]==value);
            length=i+1;
            if(value>=255)break;
            value=(144*256+112*value)/256;
        }
        CHECK(g_MenuSaveBgFadeLutLen==length);
        if(carry) {
            CHECK(g_AyaSaveState.inventory_items[5]==0x178 && g_AyaSaveState.inventory_items[6]==0x179);
            CHECK(!g_InventoryRuntime.specialStorage[0] && !g_InventoryRuntime.specialStorage[1]);
        }
    }
    puts("Original New Game inventory: stats, initial items, tagged carryover and menu fade passed");
    return 0;
}
static int item_addition(void)
{
    memset(&g_PlayerInventoryState,0,sizeof(g_PlayerInventoryState));
    g_AyaSaveState.inventory_slot_count=10;
    Inv_SelectActiveList(0);
    g_InvSpecialItemRecords[0].ammo=0;
    g_InvSpecialItemRecords[0].baseStats[2]=0;
    g_InvSpecialItemRecords[0].bonusStats[2]=999;
    const int initial[]={0x44,0x96,0x3f,1,6};
    for(unsigned i=0;i<5;++i)CHECK(Inv_AddItem(initial[i])==0);
    CHECK(g_InvItemPtr[0]==0x100 && g_InvItemPtr[1]==0x101);
    CHECK(!memcmp(&g_InventoryRuntime.equipment[0],Item_LookupBaseData(0x43),sizeof(ItemDataRecord)));
    CHECK(!memcmp(&g_InventoryRuntime.equipment[1],Item_LookupBaseData(0x95),sizeof(ItemDataRecord)));
    CHECK(g_InvItemPtr[2]==0x102 && g_InvItemPtr[3]==0x200 && g_InvItemPtr[4]==6);
    CHECK(g_InvSpecialItemRecords[0].ammo==Item_LookupBaseData(0)->ammo);
    for(unsigned i=5;i<10;++i)CHECK(Inv_AddItem(6)==0);
    CHECK(Inv_AddItem(6)==1 && Inv_AddItem(0x44)==1);
    unsigned before=g_InvSpecialItemRecords[0].ammo;
    CHECK(Inv_AddItem(1)==0);
    CHECK(g_InvSpecialItemRecords[0].ammo==before+Item_LookupBaseData(0)->ammo);
    g_InvSpecialItemRecords[0].ammo=998;
    CHECK(Inv_AddItem(1)==0 && g_InvSpecialItemRecords[0].ammo==999);
    g_InvItemPtr[9]=0;
    for(unsigned i=0;i<128;++i)g_InventoryRuntime.equipment[i].iconId=1;
    CHECK(Inv_AddItem(0x44)==1 && g_InvItemPtr[9]==0);
    CHECK(Inv_AddItem(0x203)==0 && g_InvItemPtr[9]==0x203);
    puts("Original item insertion: starting equipment/items/ammo, full inventory, ammo cap and full record pool passed");
    return 0;
}
static int battle_defaults(PeDisc *disc, PeDiscFile *exe)
{
    unsigned char original[0x70], tail[0x18];
    CHECK(pe_disc_read(disc,exe,0x10928-0x10000+0x800,original,sizeof(original)));
    CHECK(pe_disc_read(disc,exe,0x10998-0x10000+0x800,tail,sizeof(tail)));
    for(unsigned pass=0;pass<2;++pass) {
        memset(&g_AyaBattleState,0xa5,sizeof(g_AyaBattleState));
        memset(&g_SavedBattleStateTail,0xa5,sizeof(g_SavedBattleStateTail));
        memset(&g_BattleEquipStateBlock,0xa5,sizeof(g_BattleEquipStateBlock));
        Battle_SaveAyaState();
        const unsigned char *native=(const unsigned char *)&g_AyaBattleState;
        CHECK(!memcmp(native,original,0x1c));
        CHECK(!memcmp(native+PE1_OFFSETOF(Combatant,maxHP),original+0x1c,0x68-0x1c));
        CHECK(!g_AyaBattleState.action && !g_AyaBattleState.attributes);
        for(unsigned i=0x68;i<sizeof(original);++i)CHECK(original[i]==0);
        CHECK(!memcmp(&g_SavedBattleStateTail,tail,sizeof(tail)));
        CHECK(g_BattleEquipStateBlock.parameterWord.raw==0 && g_BattleEquipStateBlock.effectFlags==0);
    }
    puts("Original battle defaults: native reset matches USA executable fields and tail");
    return 0;
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
static unsigned delayed_calls;
static void delayed_probe(void) { ++delayed_calls; }
static int original_menu_bootstrap(void)
{
    memset(&g_SaveRuntimeState,0xA5,sizeof(g_SaveRuntimeState));
    MemCard_SetDelayedCallback(delayed_probe);
    CHECK(MemCard_HasDelayedCallback() && g_MemCardDelayedCallbackTimer==1);
    Menu_FullInit();
    /* Complete serialized image from USA Menu_FullInit 800527C8 in MIPS.
     * Only the three BIOS bzero calls use host memory-service semantics. */
    uint64_t hash=UINT64_C(14695981039346656037);
    for(size_t i=0;i<sizeof(g_SaveRuntimeState);++i)
        hash=(hash^((u8 *)&g_SaveRuntimeState)[i])*UINT64_C(1099511628211);
    CHECK(hash==UINT64_C(0x0b3a33783621c7a8));
    CHECK(&g_AyaSaveState.blend_color==&g_SaveRuntimeState.drawBlendColor);
    CHECK((u8 *)&g_PlayerInventoryState==(u8 *)&g_SaveRuntimeState+0x20);
    CHECK(!MemCard_HasDelayedCallback() && !g_MemCardDelayedCallbackTimer && !delayed_calls);
    CHECK(g_GameState.flags&0x40000000);
    CHECK(g_MenuWidgetFreeListHead==g_MenuWidgetNodePool && !g_MenuWidgetCurrentNode &&
          !g_MenuWidgetActiveListHead && !g_MenuWidgetNodePool[23].next);
    for(unsigned i=0;i<23;++i) CHECK(g_MenuWidgetNodePool[i].next==g_MenuWidgetNodePool+i+1);
    CHECK(g_SaveMetadataWindowIndex==1 && g_SaveMetadataWindowLength==8 &&
          g_SaveMetadataCursor==g_SaveRuntimeState.windows.metadata[1].text);
    CHECK(g_SaveRuntimeState.cursorMetadata[0].text[0]==255 &&
          g_SaveRuntimeState.cursorMetadata[1].text[0]==255);
    CHECK(g_DrawBlendColorPair[0]==0x404040 && g_DrawBlendColorPair[1]==0x404040);
    CHECK(g_DrawBaseY==8 && g_TextboxKernGlyph0F==1);
    CHECK(g_GameTimers[0].current==0 && g_GameTimers[1].current==0 &&
          g_GameTimers[2].current==0 && g_GameTimers[3].current==0);
    CHECK(g_TextboxNameGlyphs.count==3 && g_TextboxNameGlyphs.glyphs[0]==0x10 &&
          g_TextboxNameGlyphs.glyphs[1]==0x48 && g_TextboxNameGlyphs.glyphs[2]==0x30);
    MemCard_SetDelayedCallback(delayed_probe);
    MemCard_DelayedCallback(); CHECK(!delayed_calls && g_MemCardDelayedCallbackTimer==2);
    MemCard_DelayedCallback(); CHECK(!delayed_calls && g_MemCardDelayedCallbackTimer==3);
    MemCard_DelayedCallback(); CHECK(delayed_calls==1 && !MemCard_HasDelayedCallback());
    MemCard_DelayedCallback(); CHECK(delayed_calls==1);
    puts("Original full menu initialization with real resident data and delayed callback passed");
    return 0;
}

int main(void)
{
    CHECK(pending_command_stack() == 0);
    CHECK(menu_fade_parameters() == 0);
    CHECK(menu_fade_pixels() == 0);
    const char *path=getenv("PE_TEST_DISC");
    if(!path || !*path) return getenv("PE_REQUIRE_GAME_ASSETS")?1:77;
    PeDisc disc={0}; PeDiscFile archive, exe;
    unsigned char resident_range[4];
    CHECK(pe_disc_open(&disc,path));
    CHECK(pe_disc_find(&disc,"PE.IMG",&archive));
    CHECK(pe_disc_find(&disc,"SLUS_006.62",&exe));
    /* Serialized USA sector-table entries 2..3 identify the resident bank.
     * Read the data coordinates directly; no parallel room runtime is needed. */
    CHECK(pe_disc_read(&disc,&exe,0x930d8-0x10000+0x800+2*2,
                       resident_range,sizeof(resident_range)));
    unsigned first=resident_range[0] | (unsigned)resident_range[1]<<8;
    unsigned last=resident_range[2] | (unsigned)resident_range[3]<<8;
    CHECK(last>first);
    size_t resident_bytes=(size_t)(last-first)*2048;
    CHECK(resident_bytes<=sizeof(g_ResidentDataPack));
    CHECK((uint64_t)last*2048<=archive.size);
    memset(&g_ResidentDataPack,0,sizeof(g_ResidentDataPack));
    CHECK(pe_disc_read(&disc,&archive,(uint64_t)first*2048,
                       &g_ResidentDataPack,resident_bytes));
    CHECK(g_ResidentDataPack.directory.statGrowth>=0);
    CHECK((unsigned)g_ResidentDataPack.directory.statGrowth+7*512<=resident_bytes);
    CHECK(g_ResidentDataPack.directory.levelStats>=0);
    CHECK((unsigned)g_ResidentDataPack.directory.levelStats+99*sizeof(AyaLevelStats)<=resident_bytes);
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
    ItemDataRecord key_records[9];
    CHECK(pe_disc_read(&disc,&exe,0xa1e64-0x10000+0x800,key_records,sizeof(key_records)));
    memcpy(g_KeyItemRecords,key_records,sizeof(key_records));
    u8 effect_codes[6];
    CHECK(pe_disc_read(&disc,&exe,0x923d0-0x10000+0x800,effect_codes,sizeof(effect_codes)));
    CHECK(!memcmp(effect_codes,g_WeaponEffectCodes,sizeof(effect_codes)));
    CHECK(battle_defaults(&disc,&exe)==0);
    CHECK(item_lookups()==0);
    CHECK(item_addition()==0);
    CHECK(new_game_inventory()==0);
    CHECK(weapon_stats()==0);
    CHECK(armor_stats()==0);
    CHECK(equipment_init()==0);
    CHECK(inventory_selection()==0);
    CHECK(recalc_stats()==0);
    CHECK(original_menu_bootstrap()==0);
    puts("Original player stat recalculation: 297 level/modifier combinations and inventory capacity passed");
    pe_disc_close(&disc);
    printf("Original stat queries: %u values across seven real growth tables, level/sublevel/distance/overflow passed\n",queries);
    return 0;
}
