#include "room_banks.h"
#include <string.h>
static uint32_t word(const uint8_t *p) {
    return p[0] | (uint32_t)p[1]<<8 | (uint32_t)p[2]<<16 | (uint32_t)p[3]<<24;
}
int pe_entity_banks(const PeRoomBlob *blob,int install_root,PeRoomBanks *banks,char *error,size_t error_size){
    PeRoomBanks next=*banks;
    for(unsigned table=install_root?3:4;table<=4;++table){
        const uint8_t *records;unsigned count;
        if(!pe_room_blob_table(blob,table,12,&records,&count))goto malformed;
        if(table==3&&!count)goto malformed;
        unsigned used=table==3?1:count;
        for(unsigned i=0;i<used;++i){
            const uint8_t *record=records+i*12;unsigned id=table==3?0:record[7];
            uint32_t offset=word(record+4)&0xffffff;size_t size=word(record)&0xffffff;
            if(id>=48||offset>blob->size||size>blob->size-offset)goto malformed;
            PeRoomSlice *slice=table==3?&next.roots[0]:&next.rows[0][id];
            *slice=(PeRoomSlice){blob->bytes+offset,size,offset,1};
        }
    }
    *banks=next;if(error_size)error[0]=0;return 1;
malformed:snprintf(error,error_size,"Malformed external entity bank");return 0;
}
int pe_room_banks(const PeRoom *room, PeRoomBanks *banks, char *error, size_t error_size) {
    PeRoomBanks next = {0};
    for (unsigned table = 3; table <= 4; ++table) {
        const uint8_t *records; unsigned count;
        if (!pe_room_blob_table(&room->data, table, 12, &records, &count)) goto malformed;
        if (table == 3) next.root_records = count; else next.row_records = count;
        for (unsigned i=0; i<count; ++i) {
            const uint8_t *record = records+i*12;
            unsigned slot = record[7], group = record[11];
            if (table == 3 ? slot >= 10 : slot >= 48 || group >= 10) goto malformed;
            uint32_t offset = word(record+4) & UINT32_C(0xffffff);
            size_t size = word(record) & UINT32_C(0xffffff);
            if (offset > room->data.size || size > room->data.size-offset) goto malformed;
            PeRoomSlice *slice = table == 3 ? &next.roots[slot] : &next.rows[group][slot];
            *slice = (PeRoomSlice){room->data.bytes+offset,size,offset,1};
        }
    }
    *banks = next;
    if (error_size) error[0]=0;
    return 1;
malformed:
    snprintf(error,error_size,"Map %u: malformed room model/animation bank record",room->map);
    return 0;
}
