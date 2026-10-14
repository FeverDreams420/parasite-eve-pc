#include "disc.h"
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
    pe_disc_close(&disc);
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

int main(void) {
    test_image(0);
    test_image(1);
    PeDisc disc;
    CHECK(!pe_disc_open(&disc, "missing-disc-file.bin"));
    puts("disc: ISO, raw BIN, boundary reads and malformed records passed");
    return 0;
}
