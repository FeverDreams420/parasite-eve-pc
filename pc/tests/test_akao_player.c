#include "akao_player.h"
#include <stdlib.h>
#include <string.h>
#define CHECK(expr) do { if (!(expr)) { fprintf(stderr, "line %d: %s\n", __LINE__, #expr); exit(1); } } while (0)
int main(void) {
    const uint8_t bytes[] = {0xfc,1,1,0,0x40,0x0a,0xa0, 0xfc,0,0,0x80,0x0a,0xa0};
    PeAkaoSequence sequence = {0};
    sequence.bytes = bytes; sequence.size = sizeof(bytes); sequence.id = 5;
    sequence.active = sequence.pending = sequence.yield = 3;
    for (unsigned i = 0; i < 24; ++i) sequence.channels[i] = UINT32_MAX;
    sequence.channels[0] = 0; sequence.channels[1] = 7;
    PeAkaoGrammar grammar = {0};
    const uint16_t lengths[] = {192,96,48,24,12,6,3,32,16,8,4};
    memcpy(grammar.note_lengths, lengths, sizeof(lengths));
    grammar.extended_lengths[0] = 3; grammar.extended_lengths[1] = 4;
    PeAkaoControl initial = {0}; initial.tempo = 0x80000000;
    PeAkaoPlayer player = {0};
    CHECK(pe_akao_player_start(&player, &sequence, &grammar, &initial));
    CHECK(player.channels[0].control == &player.control && player.channels[1].control == &player.control);
    CHECK(player.channels[0].grammar == &player.grammar);
    for (unsigned timer = 0; timer < 8; ++timer) {
        CHECK(pe_akao_player_tick(&player, 0, 0, 100));
        CHECK(player.note_events == (timer == 7 ? 3u : 0u));
    }
    CHECK(player.control.musical_ticks == 4 && player.channels[0].note_length == 4);
    CHECK(player.control.tempo == 0x80000000 && !player.control.tempo_duration);
    player.key_off_pending = 0;
    CHECK(pe_akao_player_tick(&player, 0, 1, 100));
    CHECK(!player.key_off_pending && !player.note_events);
    CHECK(pe_akao_player_tick(&player, 0, 1, 100));
    CHECK(player.key_off_pending == 3);
    player.key_off_pending = 0;
    CHECK(pe_akao_player_tick(&player, 0, 1, 100));
    CHECK(!player.key_off_pending);
    CHECK(pe_akao_player_tick(&player, 0, 1, 100));
    CHECK(!player.active && !player.pending && !player.yield && !player.bank_id);
    CHECK(player.control.musical_ticks == 8);
    CHECK(pe_akao_player_tick(&player, 0, 1, 100) && player.control.musical_ticks == 8);
    uint64_t previous_ticks = player.timer_ticks;
    sequence.channels[1] = UINT32_MAX;
    CHECK(!pe_akao_player_start(&player, &sequence, &grammar, &initial));
    CHECK(player.timer_ticks == previous_ticks && !player.active);
    sequence.active = 1; sequence.channels[0] = 0;
    const uint8_t missing[] = {0xa1,32};
    sequence.bytes = missing; sequence.size = sizeof(missing);
    CHECK(pe_akao_player_start(&player, &sequence, &grammar, &initial));
    for (unsigned i = 0; i < 3; ++i) CHECK(pe_akao_player_tick(&player, 0, 1, 100));
    CHECK(!pe_akao_player_tick(&player, 0, 1, 100));
    CHECK(strstr(player.error, "Voice 0: Missing AKAO instrument context"));
    CHECK(!player.note_events && player.channels[0].pc == 0);
    previous_ticks = player.timer_ticks;
    CHECK(!pe_akao_player_tick(&player, 0, 1, 100) && player.timer_ticks == previous_ticks);
    const uint8_t released[] = {0,0xa0};
    sequence.bytes = released; sequence.size = sizeof(released);
    CHECK(pe_akao_player_start(&player, &sequence, &grammar, &initial));
    player.channels[0].adsr[4] = 17; player.channels[0].adsr[7] = 9;
    player.channels[0].update_flags = 0;
    uint32_t previous_volume = player.control.volume;
    pe_akao_player_release(&player);
    CHECK(player.active == 1 && player.key_off_pending == 0xffffff);
    CHECK(player.channels[0].adsr[4] == 5 && player.channels[0].adsr[7] == 9);
    CHECK(player.channels[0].update_flags == 0x4400 && player.channels[0].note_length == 3);
    CHECK(player.control.volume == previous_volume);
    for (unsigned tick = 0; tick < 2; ++tick) {
        CHECK(pe_akao_player_tick(&player, 0, 1, 100) && player.active == 1 && !player.note_events);
    }
    CHECK(pe_akao_player_tick(&player, 0, 1, 100) && !player.active && !player.bank_id);
    player.channels[0].adsr[4] = 19; player.key_off_pending = 7;
    pe_akao_player_release(&player);
    CHECK(player.channels[0].adsr[4] == 19 && player.key_off_pending == 7);
    puts("AKAO player: ordered shared controls, timer/countdowns, release, stops and explicit failures passed");
    return 0;
}
