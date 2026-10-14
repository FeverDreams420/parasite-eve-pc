#include "collision.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "line %d: %s\n", __LINE__, #x); exit(1); } } while (0)
static void half(uint8_t *p, unsigned v) { p[0]=(uint8_t)v; p[1]=(uint8_t)(v>>8); }
static void word(uint8_t *p, uint32_t v) { half(p,v); half(p+2,v>>16); }
int main(void) {
    uint8_t bytes[160] = {0}; char error[128]; PeCollision db = {0};
    half(bytes+2,1); word(bytes+4,1); half(bytes+8,2); word(bytes+24,80); word(bytes+28,96); word(bytes+40,48);
    half(bytes+48,65534); half(bytes+50,1); half(bytes+52,0);
    half(bytes+84,10); half(bytes+90,10);
    half(bytes+98,0); half(bytes+100,1); half(bytes+102,2);
    CHECK(pe_collision_open(&db,bytes,sizeof(bytes),error,sizeof(error)));
    PeFloorHit hit = {99,99,0xdeadbeef};
    CHECK(pe_collision_find_floor(&db,0x10000,0,0x10000,0,&hit));
    CHECK(hit.triangle == 0 && hit.region == 0 && hit.height == 0xfffe0000);
    CHECK(pe_collision_find_floor(&db,0x10000,0x12345678,0x10000,2,&hit) && hit.height == 0x12345678);
    CHECK(!pe_collision_find_floor(&db,0x10000,0,0x10000,0x80,&hit) && hit.height == 0x12345678);
    CHECK(!pe_collision_find_floor(&db,0x100000,0,0x100000,0,&hit) && hit.height == 0x12345678);
    CHECK(pe_collision_triangle_in_regions(&db,0));
    half(bytes+50,0); /* Geometry remains, but its region no longer lists it. */
    CHECK(pe_collision_open(&db,bytes,sizeof(bytes),error,sizeof(error)));
    CHECK(!pe_collision_triangle_in_regions(&db,0));
    CHECK(!pe_collision_find_floor(&db,0x10000,0,0x10000,0,&hit) && hit.height == 0x12345678);
    half(bytes+50,1);
    CHECK(pe_collision_open(&db,bytes,sizeof(bytes),error,sizeof(error)));
    uint8_t *previous = db.bytes;
    word(bytes+40,159);
    CHECK(!pe_collision_open(&db,bytes,sizeof(bytes),error,sizeof(error)) && db.bytes == previous);
    word(bytes+40,48); half(bytes+102,65535);
    CHECK(!pe_collision_open(&db,bytes,sizeof(bytes),error,sizeof(error)) && db.bytes == previous);
    half(bytes+102,2); half(bytes+52,1);
    CHECK(!pe_collision_open(&db,bytes,sizeof(bytes),error,sizeof(error)) && db.bytes == previous);
    half(bytes+52,0);
    half(bytes+2,2); word(bytes+44,56); half(bytes+48,3);
    half(bytes+56,7); half(bytes+58,1); half(bytes+60,0);
    CHECK(pe_collision_open(&db,bytes,sizeof(bytes),error,sizeof(error)));
    CHECK(pe_collision_find_floor(&db,0x10000,0,0x10000,0,&hit) && hit.height == 0x30000 && !hit.region);
    memset(bytes,0,sizeof(bytes)); half(bytes+2,1); word(bytes+4,1); half(bytes+8,2);
    half(bytes+6,1);
    word(bytes+24,80); word(bytes+28,110); word(bytes+32,140); word(bytes+40,48);
    half(bytes+52,1); half(bytes+54,0);
    half(bytes+86,10); half(bytes+96,10);
    half(bytes+118,0); half(bytes+120,1); half(bytes+122,2); word(bytes+114,0x60000);
    word(bytes+140,0x10000); word(bytes+144,0x10000); word(bytes+148,0x10000);
    CHECK(pe_collision_open(&db,bytes,sizeof(bytes),error,sizeof(error)));
    CHECK(pe_collision_find_floor(&db,0x18000,0,0x10000,0,&hit) && hit.height == 0x38000);
    word(bytes+140,0xffff0000); word(bytes+144,0xffff0000); word(bytes+148,0); word(bytes+114,0);
    CHECK(pe_collision_open(&db,bytes,sizeof(bytes),error,sizeof(error)));
    memset(bytes,0,sizeof(bytes)); /* owned data survives input destruction */
    CHECK(pe_collision_find_floor(&db,0x18000,0,0x10000,0,&hit) && hit.height == 0xfffe8000);
    pe_collision_close(&db); pe_collision_close(&db);
    puts("Collision floor: owned relative data, ordered regions, flat/sloped fixed heights, flags and bounds passed");
    return 0;
}
