#include "akao_channel.h"
#include <stdlib.h>
#include <string.h>
#define CHECK(expr) do { if (!(expr)) { fprintf(stderr, "line %d: %s\n", __LINE__, #expr); exit(1); } } while (0)
int main(void) {
    PeSpuBank *samples = calloc(1, sizeof(*samples)); CHECK(samples);
    samples->valid[0] = 1;
    for (unsigned i = 0; i < 8; ++i) samples->instruments[0].adsr[i] = (uint8_t)(10 + i);
    PeAkaoSequence sequence = {0};
    for (unsigned i = 0; i < 24; ++i) sequence.channels[i] = UINT32_MAX;
    sequence.channels[0] = 0;
    PeAkaoChannel channel = {0}; channel.samples = samples;
    const struct { uint8_t opcode, field; uint32_t mask; } cases[] = {
        {0xad,0,0x0900},{0xae,1,0x1000},{0xaf,2,0x8000},
        {0xb1,3,0x2200},{0xb2,4,0x4400},{0xb7,5,0x0100},
        {0xbb,6,0x0200},{0xbf,7,0x0400}
    };
    for (unsigned i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        uint8_t bytes[] = {cases[i].opcode,31,0x11};
        sequence.bytes = bytes; sequence.size = sizeof(bytes);
        CHECK(pe_akao_channel_start(&channel, &sequence, 0));
        channel.update_flags = 0;
        CHECK(pe_akao_channel_next(&channel, 100) == PE_AKAO_CHANNEL_NOTE);
        CHECK(channel.update_flags == cases[i].mask && channel.adsr[cases[i].field] == 31);
        for (unsigned j = 0; j < 8; ++j)
            if (j != cases[i].field) CHECK(channel.adsr[j] == 10 + j);
    }
    const uint8_t paired[] = {0xb0,7,8,0x11};
    sequence.bytes = paired; sequence.size = sizeof(paired);
    CHECK(pe_akao_channel_start(&channel, &sequence, 0)); channel.update_flags = 0;
    CHECK(pe_akao_channel_next(&channel, 100) == PE_AKAO_CHANNEL_NOTE);
    CHECK(channel.adsr[1] == 7 && channel.adsr[2] == 8 && channel.update_flags == 0x9000);
    const uint8_t restore[] = {0xb3,0x11};
    sequence.bytes = restore; sequence.size = sizeof(restore);
    CHECK(pe_akao_channel_start(&channel, &sequence, 0));
    memset(channel.adsr, 99, 8); channel.update_flags = 0; channel.flags = UINT32_C(1) << 9;
    CHECK(pe_akao_channel_next(&channel, 100) == PE_AKAO_CHANNEL_NOTE);
    CHECK(!memcmp(channel.adsr, samples->instruments[0].adsr, 8));
    CHECK(channel.update_flags == 0xff00 && (channel.flags & (UINT32_C(1) << 9)));
    const uint8_t release[] = {0xfc,0x10,77,0x11,0xfc,0x11,0x11};
    sequence.bytes = release; sequence.size = sizeof(release);
    CHECK(pe_akao_channel_start(&channel, &sequence, 0)); channel.update_flags = 0;
    CHECK(pe_akao_channel_next(&channel, 100) == PE_AKAO_CHANNEL_NOTE);
    CHECK(channel.adsr[4] == 77 && channel.update_flags == 0x4400 && (channel.flags & (UINT32_C(1) << 9)));
    channel.adsr[7] = 88;
    CHECK(pe_akao_channel_next(&channel, 100) == PE_AKAO_CHANNEL_NOTE);
    CHECK(channel.adsr[4] == 14 && channel.adsr[7] == 88 && !(channel.flags & (UINT32_C(1) << 9)));
    sequence.bytes = restore; sequence.size = sizeof(restore);
    channel.samples = NULL;
    CHECK(pe_akao_channel_start(&channel, &sequence, 0));
    CHECK(pe_akao_channel_next(&channel, 100) == PE_AKAO_CHANNEL_ERROR && !channel.pc);
    free(samples);
    puts("AKAO ADSR: field writes, dirty masks, combined decay/sustain and distinct restore semantics passed");
    return 0;
}
