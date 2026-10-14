#include "akao_channel.h"
#include <stdlib.h>
#include <string.h>
#define CHECK(expr) do { if (!(expr)) { fprintf(stderr, "line %d: %s\n", __LINE__, #expr); exit(1); } } while (0)
int main(void) {
    PeSpuBank *samples = calloc(1, sizeof(*samples)); CHECK(samples);
    samples->valid[0] = samples->valid[32] = samples->valid[80] = 1;
    samples->instruments[0].pitch[0] = 100;
    samples->instruments[32].pitch[0] = 200;
    samples->instruments[80].pitch[0] = 300;
    samples->instruments[32].start = 0x8000; samples->instruments[32].loop = 0x8010;
    for (unsigned i = 0; i < 8; ++i) samples->instruments[32].adsr[i] = (uint8_t)(10 + i);
    PeAkaoControl control = {0};
    PeAkaoChannel channel = {0}; channel.control = &control; channel.voice_mask = 4;
    CHECK(pe_akao_channel_bind_samples(&channel, samples));
    channel.pitch_base = 1000; channel.flags |= UINT32_C(1) << 12;
    CHECK(pe_akao_channel_set_instrument(&channel, 32));
    CHECK(channel.instrument == 32 && channel.pitch_base == 2000);
    CHECK(channel.sample_start == 0x8000 && channel.sample_loop == 0x8010);
    CHECK(!memcmp(channel.adsr, samples->instruments[32].adsr, 8));
    CHECK(!(channel.flags & (UINT32_C(1) << 12)) && (channel.update_flags & 0x10));
    CHECK(pe_akao_channel_bind_samples(&channel, samples));
    channel.adsr[4] = 77; channel.adsr[7] = 88; channel.flags = UINT32_C(1) << 9;
    CHECK(pe_akao_channel_set_instrument(&channel, 32));
    CHECK(channel.adsr[4] == 77 && channel.adsr[7] == 88 && channel.adsr[0] == 10);
    CHECK(pe_akao_channel_bind_samples(&channel, samples));
    control.status_flags = 0x100;
    CHECK(pe_akao_channel_set_instrument(&channel, 32) && channel.instrument == 80);
    control.status_flags = 0;
    CHECK(pe_akao_channel_bind_samples(&channel, samples));
    control.allocated_voice_mask = control.spu_active_mask = 4;
    channel.pitch_base = 123;
    CHECK(pe_akao_channel_set_instrument(&channel, 32) && channel.pitch_base == 123);
    control.spu_active_mask = 0;
    CHECK(pe_akao_channel_bind_samples(&channel, samples));
    channel.pitch_base = 0x80000000;
    samples->instruments[32].pitch[0] = 2;
    CHECK(pe_akao_channel_set_instrument(&channel, 32) && !channel.pitch_base); /* wrapped multiply */
    samples->instruments[0].pitch[0] = 0;
    CHECK(pe_akao_channel_bind_samples(&channel, samples));
    channel.pitch_base = 456;
    CHECK(!pe_akao_channel_set_instrument(&channel, 32));
    CHECK(channel.instrument == 0 && channel.pitch_base == 456);
    channel.error[0] = 0; channel.instrument = 255;
    CHECK(pe_akao_channel_set_instrument(&channel, 32)); /* sentinel skips pitch ratio */
    CHECK(!pe_akao_channel_set_instrument(&channel, 999));
    free(samples);
    puts("AKAO instrument: sample/ADSR binding, release override, bank offset, allocation and pitch ratio passed");
    return 0;
}
