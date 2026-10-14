#include "vram.h"
#include <stdlib.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "line %d: %s\n", __LINE__, #expr); exit(1); } } while (0)

static void word(uint8_t *p, uint32_t value) {
    for (unsigned i = 0; i < 4; ++i) p[i] = (uint8_t)(value >> (i * 8));
}

int main(void) {
    uint8_t scratch[1024] = {0}, textures[1024] = {0};
    PeRoom room = {0};
    room.map = 1;
    room.scratch.bytes = scratch;
    room.scratch.size = sizeof(scratch);
    room.textures.bytes = textures;
    room.textures.size = sizeof(textures);
    room.scratch.directory[10] = room.textures.directory[10] = (1u << 22) | 8;
    word(scratch + 8 + 4, (2u << 24) | 128);
    word(scratch + 8 + 8, 2 | (10u << 10) | (20u << 21));
    word(scratch + 8 + 12, (1u << 24) | 8);
    word(scratch + 8 + 16, 2 | (20u << 10) | (30u << 21));
    for (unsigned i = 0; i < 6; ++i) {
        scratch[128 + i * 2] = (uint8_t)(i + 1);
        scratch[129 + i * 2] = 0x80;
    }
    word(textures + 8 + 4, (1u << 24) | 128);
    word(textures + 8 + 8, 1 | (10u << 10) | (20u << 21));
    textures[128] = 0x34;
    textures[129] = 0x12;
    PeVram *vram = calloc(1, sizeof(*vram)), *previous = malloc(sizeof(*previous));
    CHECK(vram && previous);
    PeVramStats stats;
    char error[256];
    CHECK(pe_room_upload_vram(&room, vram, &stats, error, sizeof(error)));
    CHECK(stats.uploads == 3 && stats.words == 7);
    CHECK(vram->words[20 * PE_VRAM_WIDTH + 10] == 0x1234); /* later bank wins */
    CHECK(vram->words[20 * PE_VRAM_WIDTH + 11] == 0x8002);
    CHECK(vram->words[21 * PE_VRAM_WIDTH + 10] == 0x8003);
    CHECK(vram->words[30 * PE_VRAM_WIDTH + 20] == 0x8005); /* primary + secondary */
    CHECK(vram->words[30 * PE_VRAM_WIDTH + 21] == 0x8006);
    memcpy(previous, vram, sizeof(*vram));
    /* Invalid final bank must not partially apply the first bank. */
    scratch[130] = 99;
    word(textures + 8 + 8, 2 | (1023u << 10));
    CHECK(!pe_room_upload_vram(&room, vram, &stats, error, sizeof(error)));
    CHECK(!memcmp(previous, vram, sizeof(*vram)));
    CHECK(strstr(error, "texture-bank"));
    word(textures + 8 + 8, 1 | (10u << 10) | (20u << 21));
    word(scratch + 8 + 12, (1u << 24) | 1020);
    CHECK(!pe_room_upload_vram(&room, vram, &stats, error, sizeof(error)));
    CHECK(!memcmp(previous, vram, sizeof(*vram)));
    /* Primary height zero means 256; secondary height zero stays empty. */
    word(scratch + 8 + 4, 128);
    word(scratch + 8 + 8, 1 | (10u << 10) | (20u << 21));
    word(scratch + 8 + 12, 8);
    CHECK(pe_room_upload_vram(&room, vram, &stats, error, sizeof(error)));
    CHECK(stats.uploads == 2 && stats.words == 257);
    CHECK(vram->words[21 * PE_VRAM_WIDTH + 10] == 0x8063);
    memcpy(previous,vram,sizeof(*vram));word(scratch+16,2|(1023u<<10));
    CHECK(!pe_blob_upload_vram(&room.scratch,vram,&stats,error,sizeof(error)));
    CHECK(stats.words==257&&!memcmp(previous,vram,sizeof(*vram)));
    free(previous);
    free(vram);
    puts("VRAM: upload order, secondary-relative offsets, zero heights and atomic failures passed");
    return 0;
}
