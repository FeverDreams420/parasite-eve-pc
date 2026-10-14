#include "spu_bank.h"
#include <stdlib.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "line %d: %s\n", __LINE__, #expr); exit(1); } } while (0)

static void word(uint8_t *p, uint32_t value) {
    for (unsigned i = 0; i < 4; ++i) p[i] = (uint8_t)(value >> (i * 8));
}

int main(void) {
    uint8_t data[144] = {0};
    memcpy(data, "AKAO", 4);
    word(data + 16, 0x2000);
    word(data + 20, 16);
    word(data + 24, 7);
    word(data + 28, 8);
    word(data + 64, 0x2000);
    word(data + 68, 0x2008);
    for (unsigned i = 0; i < 8; ++i) data[72 + i] = (uint8_t)(80 + i);
    for (unsigned i = 0; i < 12; ++i) word(data + 80 + i * 4, 256 + i);
    for (unsigned i = 0; i < 16; ++i) data[128 + i] = (uint8_t)(i + 1);
    PeSpuBank *bank = calloc(1, sizeof(*bank)), *previous = malloc(sizeof(*previous));
    CHECK(bank && previous);
    bank->ram[0x1fff] = 42;
    bank->valid[3] = 1;
    bank->instruments[3].pitch[0] = 123;
    PeSpuBankStats stats;
    char error[256];
    CHECK(pe_spu_bank_upload(bank, data, sizeof(data), &stats, error, sizeof(error)));
    CHECK(stats.address == 0x2000 && stats.bytes == 16 && stats.first == 7 && stats.end == 8);
    CHECK(bank->valid[7] && bank->valid[3] && !bank->valid[8]);
    CHECK(bank->instruments[7].start == 0x2000 && bank->instruments[7].loop == 0x2008);
    CHECK(bank->instruments[7].adsr[7] == 87 && bank->instruments[7].pitch[11] == 267);
    CHECK(bank->instruments[3].pitch[0] == 123 && bank->ram[0x1fff] == 42);
    CHECK(!memcmp(bank->ram + 0x2000, data + 128, 16));
    memcpy(previous, bank, sizeof(*bank));
    for (size_t size = 0; size < sizeof(data); ++size) {
        CHECK(!pe_spu_bank_upload(bank, data, size, &stats, error, sizeof(error)));
        CHECK(!memcmp(previous, bank, sizeof(*bank)));
    }
    word(data + 68, PE_SPU_RAM_SIZE);
    CHECK(!pe_spu_bank_upload(bank, data, sizeof(data), &stats, error, sizeof(error)));
    CHECK(!memcmp(previous, bank, sizeof(*bank)));
    word(data + 68, 0x2008);
    word(data + 16, PE_SPU_RAM_SIZE - 8);
    CHECK(!pe_spu_bank_upload(bank, data, sizeof(data), &stats, error, sizeof(error)));
    word(data + 16, 0x2001);
    CHECK(!pe_spu_bank_upload(bank, data, sizeof(data), &stats, error, sizeof(error)));
    word(data + 16, 0x2000);
    word(data + 28, 257);
    CHECK(!pe_spu_bank_upload(bank, data, sizeof(data), &stats, error, sizeof(error)));
    word(data + 28, 6);
    CHECK(!pe_spu_bank_upload(bank, data, sizeof(data), &stats, error, sizeof(error)));
    word(data + 24, 255);
    word(data + 28, 0); /* original end-zero encoding means 256 */
    CHECK(pe_spu_bank_upload(bank, data, sizeof(data), &stats, error, sizeof(error)));
    CHECK(stats.first == 255 && stats.end == 256 && bank->valid[255]);
    free(previous);
    free(bank);
    puts("SPU bank: instruments, partial RAM writes, end-zero and atomic malformed-bank rejection passed");
    return 0;
}
