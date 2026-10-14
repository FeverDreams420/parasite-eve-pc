#include "disc.h"
#include "assets.h"
#include "room.h"
#include <stdlib.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "line %d: %s\n", __LINE__, #expr); exit(1); } } while (0)

static void store32(unsigned char *p, unsigned value) {
    for (unsigned i = 0; i < 4; ++i) {
        p[i] = (unsigned char)(value >> (i * 8));
        p[4 + i] = (unsigned char)(value >> ((3 - i) * 8));
    }
}

static void store_le32(unsigned char *p, unsigned value) {
    for (unsigned i = 0; i < 4; ++i) p[i] = (unsigned char)(value >> (i * 8));
}

static unsigned record(unsigned char *p, const char *name,
                        unsigned sector, unsigned size, int directory) {
    unsigned length = (unsigned)strlen(name);
    unsigned bytes = 33 + length;
    if (bytes & 1) ++bytes;
    memset(p, 0, bytes);
    p[0] = (unsigned char)bytes;
    store32(p + 2, sector);
    store32(p + 10, size);
    p[25] = directory ? 2 : 0;
    p[28] = p[31] = 1;
    p[32] = (unsigned char)length;
    memcpy(p + 33, name, length);
    return bytes;
}

static void make_image(const char *name, int raw) {
    FILE *file = fopen(name, "wb");
    CHECK(file);
    for (unsigned sector = 0; sector < 24; ++sector) {
        unsigned char block[2352] = {0};
        unsigned char *data = block + (raw ? 24 : 0);
        if (raw) {
            memset(block + 1, 255, 10);
            block[15] = 2;
        }
        if (sector == 16) {
            data[0] = 1;
            memcpy(data + 1, "CD001", 5);
            data[6] = 1;
            data[129] = data[130] = 8;
            record(data + 156, "X", 18, 2048, 1);
        } else if (sector == 18) {
            record(data, "FMV1", 19, 2048, 1);
        } else if (sector == 19) {
            record(data, "INTRO.STR;1", 20, 3000, 0);
        } else if (sector == 20 || sector == 21) {
            for (unsigned i = 0; i < 2048; ++i)
                data[i] = (unsigned char)((sector - 20) * 7 + i);
        }
        CHECK(fwrite(block, 1, raw ? 2352 : 2048, file) == (size_t)(raw ? 2352 : 2048));
    }
    CHECK(fclose(file) == 0);
}

static void test_image(int raw) {
    const char *name = raw ? "pe-disc-test.bin" : "pe-disc-test.iso";
    make_image(name, raw);
    PeDisc disc;
    PeDiscFile file;
    unsigned char bytes[2352];
    CHECK(pe_disc_open(&disc, name));
    CHECK(disc.sector_size == (raw ? 2352u : 2048u));
    CHECK(pe_disc_find(&disc, "\\fmv1\\intro.str;1", &file));
    CHECK(file.sector == 20 && file.size == 3000 && !file.directory);
    CHECK(pe_disc_read(&disc, &file, 2040, bytes, 32));
    for (unsigned i = 0; i < 32; ++i)
        CHECK(bytes[i] == (unsigned char)(i < 8 ? 2040 + i : i - 8 + 7));
    CHECK(!pe_disc_read(&disc, &file, 2990, bytes, 11));
    CHECK(!pe_disc_find(&disc, "FMV1/MISSING.STR", &file));
    CHECK(pe_disc_find(&disc, "FMV1/INTRO.STR", &file));
    CHECK(pe_disc_read_raw(&disc, &file, 0, bytes) == raw);
    CHECK(!pe_disc_read_raw(&disc, &file, 2, bytes));
    PeRoomCatalog catalog = {0};
    catalog.disc = &disc;
    catalog.archive.size = 24 * 2048;
    catalog.audio_bounds[0] = 20;
    catalog.audio_bounds[1] = 22;
    uint8_t *bank = NULL; size_t bank_size = 0;
    CHECK(pe_audio_bank_read(&catalog, 0, &bank, &bank_size));
    CHECK(bank_size == 4096 && bank[0] == 0 && bank[2048] == 7);
    free(bank);
    bank = NULL; bank_size = 123;
    CHECK(!pe_audio_bank_read(&catalog, PE_USA_AUDIO_BANKS, &bank, &bank_size));
    CHECK(!bank && bank_size == 123);
    catalog.audio_bounds[1] = 20;
    CHECK(!pe_audio_bank_read(&catalog, 0, &bank, &bank_size));
    pe_disc_close(&disc);
    CHECK(!pe_audio_bank_read(&catalog, 0, &bank, &bank_size));
    /* Truncated records must fail rather than escape their directory sector. */
    FILE *image = fopen(name, "r+b");
    CHECK(image);
    CHECK(fseek(image, 19L * (raw ? 2352 : 2048) + (raw ? 24 : 0), SEEK_SET) == 0);
    CHECK(fputc(8, image) != EOF);
    CHECK(fclose(image) == 0);
    CHECK(pe_disc_open(&disc, name));
    CHECK(!pe_disc_find(&disc, "FMV1/INTRO.STR", &file));
    CHECK(strstr(disc.error, "Malformed"));
    pe_disc_close(&disc);
    CHECK(remove(name) == 0);
}

static void test_room_failures(void) {
    const char *name = "pe-room-fixture.bin";
    FILE *stream = fopen(name, "wb");
    CHECK(stream);
    uint8_t block[2048] = {0};
    store_le32(block, 84);
    store_le32(block + 4, 8);
    for (unsigned word = 0; word < 13; ++word)
        store_le32(block + 8 + word * 4, word == 10 ? (1u << 22) | 64u : 64u);
    for (unsigned i = 0; i < 3; ++i)
        CHECK(fwrite(block, 1, sizeof(block), stream) == sizeof(block));
    CHECK(fclose(stream) == 0);
    PeDisc disc = {0};
    disc.stream = fopen(name, "r+b");
    CHECK(disc.stream);
    disc.image_size = 3 * 2048;
    disc.sector_size = 2048;
    PeRoomCatalog catalog = {0};
    catalog.disc = &disc;
    catalog.archive.size = 3 * 2048;
    catalog.ranges[0].scratch_sectors = 1;
    catalog.ranges[0].texture_sectors = 1;
    catalog.ranges[0].room_sectors = 1;
    PeRoom room = {0};
    CHECK(pe_room_load(&catalog, 1, &room));
    uint8_t *previous = room.data.bytes;
    /* Corrupt the second block, after a new scratch block is allocated. */
    CHECK(fseek(disc.stream, 2048 + 4, SEEK_SET) == 0);
    CHECK(fputc(83, disc.stream) != EOF);
    CHECK(fflush(disc.stream) == 0);
    CHECK(!pe_room_load(&catalog, 1, &room));
    CHECK(strstr(catalog.error, "Malformed"));
    CHECK(room.data.bytes == previous && room.map == 1);
    /* A valid header with an out-of-bounds TIM table must also fail. */
    CHECK(fseek(disc.stream, 2048 + 4, SEEK_SET) == 0);
    CHECK(fputc(8, disc.stream) != EOF);
    CHECK(fseek(disc.stream, 2048 + 8 + 10 * 4, SEEK_SET) == 0);
    CHECK(fputc(83, disc.stream) != EOF);
    CHECK(fflush(disc.stream) == 0);
    CHECK(!pe_room_load(&catalog, 1, &room));
    CHECK(room.data.bytes == previous && room.map == 1);
    pe_room_close(&room);
    pe_disc_close(&disc);
    CHECK(remove(name) == 0);
}

static void test_assets(void) {
    make_image("pe-source1.bin", 1);
    make_image("pe-source2.bin", 1);
    FILE *image = fopen("pe-source2.bin", "r+b");
    CHECK(image);
    CHECK(fseek(image, 20 * 2352 + 24, SEEK_SET) == 0);
    CHECK(fputc(42, image) != EOF);
    CHECK(fclose(image) == 0);
    PeAssets assets;
    CHECK(pe_assets_open(&assets, "pe-source1.bin", "pe-source2.bin"));
    PeAssetFile files[2];
    unsigned char bytes[2352];
    for (unsigned source = 0; source < 2; ++source) {
        CHECK(pe_assets_find(&assets, source, "FMV1/INTRO.STR", &files[source]));
        CHECK(files[source].source == source);
    }
    for (unsigned pass = 0; pass < 10; ++pass) {
        for (unsigned source = 0; source < 2; ++source) {
            CHECK(pe_assets_read(&assets, &files[source], 0, bytes, 1));
            CHECK(bytes[0] == (source ? 42 : 0));
            CHECK(pe_assets_read_raw(&assets, &files[source], 0, bytes));
            CHECK(bytes[24] == (source ? 42 : 0));
        }
    }
    CHECK(!pe_assets_find(&assets, 2, "FMV1/INTRO.STR", &files[0]));
    CHECK(!pe_assets_read(&assets, &files[1], 3000, bytes, 1));
    CHECK(strstr(assets.error, "Disc 2"));
    pe_assets_close(&assets);
    CHECK(!pe_assets_read(&assets, &files[1], 0, bytes, 1));
    CHECK(!pe_assets_open(&assets, "pe-source1.bin", NULL));
    CHECK(!assets.discs[0].stream && !assets.discs[1].stream);
    CHECK(!pe_assets_open(&assets, "pe-source1.bin", "missing-source.bin"));
    CHECK(!assets.discs[0].stream && !assets.discs[1].stream);
    CHECK(strstr(assets.error, "Disc 2"));
    CHECK(remove("pe-source1.bin") == 0);
    CHECK(remove("pe-source2.bin") == 0);
}

static void write_cue(const char *text) {
    FILE *cue = fopen("pe-disc-test.cue", "wb");
    CHECK(cue);
    CHECK(fputs(text, cue) >= 0);
    CHECK(fclose(cue) == 0);
}

static void test_cue(void) {
    make_image("pe cue image.bin", 1);
    /* A stored pregap must offset both ISO reads and raw movie sectors. */
    FILE *input = fopen("pe cue image.bin", "rb");
    FILE *output = fopen("pe shifted image.bin", "wb");
    CHECK(input && output);
    unsigned char bytes[2352] = {0};
    CHECK(fwrite(bytes, 1, sizeof(bytes), output) == sizeof(bytes));
    while (fread(bytes, 1, sizeof(bytes), input) == sizeof(bytes))
        CHECK(fwrite(bytes, 1, sizeof(bytes), output) == sizeof(bytes));
    CHECK(!ferror(input));
    CHECK(fclose(input) == 0 && fclose(output) == 0);
    write_cue("REM generated fixture\nFILE \"pe shifted image.bin\" BINARY\n"
              "  TRACK 01 MODE2/2352\n    INDEX 00 00:00:00\n    INDEX 01 00:00:01\n");
    PeDisc disc;
    PeDiscFile file;
    CHECK(pe_disc_open(&disc, "./pe-disc-test.cue"));
    CHECK(disc.track_offset == 2352);
    CHECK(pe_disc_find(&disc, "FMV1/INTRO.STR", &file));
    CHECK(pe_disc_read(&disc, &file, 2048, bytes, 16));
    for (unsigned i = 0; i < 16; ++i) CHECK(bytes[i] == i + 7);
    CHECK(pe_disc_read_raw(&disc, &file, 0, bytes));
    CHECK(bytes[15] == 2 && bytes[24] == 0 && bytes[25] == 1);
    pe_disc_close(&disc);
    const char *bad[] = {
        "FILE \"pe cue image.bin\" BINARY\nTRACK 01 AUDIO\nINDEX 01 00:00:00\n",
        "FILE \"pe cue image.bin\" BINARY\nTRACK 01 MODE2/2352\n",
        "FILE \"pe cue image.bin\" BINARY\nTRACK 01 MODE2/2352\nINDEX 01 00:60:00\n",
        "FILE \"pe cue image.bin\" BINARY\nTRACK 01 MODE2/2352\nINDEX 01 01:00:00\n",
        "FILE \"pe cue image.bin\" BINARY\nTRACK 01 MODE2/2352\nINDEX 01 4294967296:00:00\n",
        "FILE \"pe cue image.bin\" BINARY\nTRACK 01 MODE2/2352\nINDEX 01 00:00:00\nTRACK 02 AUDIO\n",
        "FILE \"missing cue image.bin\" BINARY\nTRACK 01 MODE2/2352\nINDEX 01 00:00:00\n"
    };
    for (unsigned i = 0; i < sizeof(bad) / sizeof(bad[0]); ++i) {
        write_cue(bad[i]);
        CHECK(!pe_disc_open(&disc, "pe-disc-test.cue"));
        CHECK(!disc.stream && disc.error[0]);
    }
    CHECK(remove("pe-disc-test.cue") == 0);
    CHECK(remove("pe cue image.bin") == 0);
    CHECK(remove("pe shifted image.bin") == 0);
}

int main(void) {
    test_image(0);
    test_image(1);
    test_cue();
    test_assets();
    test_room_failures();
    PeDisc disc;
    CHECK(!pe_disc_open(&disc, "missing-disc-file.bin"));
    puts("disc: ISO, raw BIN, CUE, boundary reads and malformed inputs passed");
    return 0;
}
