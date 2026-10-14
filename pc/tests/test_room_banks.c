#include "room_banks.h"
#include <stdlib.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"line %d: %s\n",__LINE__,#x); exit(1); } } while (0)
static void word(uint8_t *p,uint32_t v) { for(unsigned i=0;i<4;++i)p[i]=(uint8_t)(v>>(i*8)); }
int main(void) {
    uint8_t bytes[96] = {0}; PeRoom room = {0}; PeRoomBanks banks = {0}; char error[128];
    room.data.bytes=bytes; room.data.size=sizeof(bytes); room.map=7;
    room.data.directory[3]=(2u<<22)|16; room.data.directory[4]=(1u<<22)|40;
    word(bytes+16,4); word(bytes+20,(2u<<24)|64);
    word(bytes+28,8); word(bytes+32,(2u<<24)|68); bytes[39]=255; /* root ignores group byte */
    word(bytes+40,8); word(bytes+44,(47u<<24)|80); bytes[51]=9;
    CHECK(pe_room_banks(&room,&banks,error,sizeof(error)));
    CHECK(banks.root_records==2 && banks.row_records==1);
    CHECK(banks.roots[2].present && banks.roots[2].bytes==bytes+68 && banks.roots[2].size==8);
    CHECK(!banks.roots[1].present && banks.rows[9][47].bytes==bytes+80 && banks.rows[9][47].size==8);
    bytes[47]=48;
    CHECK(!pe_room_banks(&room,&banks,error,sizeof(error)) && banks.roots[2].bytes==bytes+68 && banks.row_records==1);
    bytes[47]=47; bytes[51]=10;
    CHECK(!pe_room_banks(&room,&banks,error,sizeof(error)) && banks.rows[9][47].bytes==bytes+80);
    bytes[51]=9; word(bytes+40,17);
    CHECK(!pe_room_banks(&room,&banks,error,sizeof(error)) && banks.roots[2].offset==68);
    word(bytes+40,8);bytes[51]=255;bytes[47]=7;
    CHECK(pe_entity_banks(&room.data,1,&banks,error,sizeof(error)));
    CHECK(banks.roots[0].bytes==bytes+64&&banks.rows[0][7].bytes==bytes+80&&banks.roots[2].bytes==bytes+68);
    bytes[47]=48;
    CHECK(!pe_entity_banks(&room.data,1,&banks,error,sizeof(error))&&banks.rows[0][7].bytes==bytes+80);
    bytes[47]=7;room.data.directory[3]=0;
    CHECK(pe_entity_banks(&room.data,0,&banks,error,sizeof(error))&&banks.roots[0].bytes==bytes+64);
    puts("Room banks: slot/group mapping, last override, borrowed data and transactional bounds passed");
    return 0;
}
