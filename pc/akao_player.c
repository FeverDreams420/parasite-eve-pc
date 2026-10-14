#include "akao_player.h"
#include <string.h>

int pe_akao_player_start(PeAkaoPlayer *player, const PeAkaoSequence *sequence,
                          const PeAkaoGrammar *grammar, const PeAkaoControl *initial) {
    if (!grammar || !initial || (sequence->active & UINT32_C(0xff000000))) return 0;
    PeAkaoPlayer next = {0};
    next.control = *initial;
    next.grammar = *grammar;
    next.active = sequence->active;
    next.pending = sequence->pending;
    next.yield = sequence->yield;
    next.bank_id = sequence->id;
    for (unsigned voice = 0; voice < 24; ++voice) {
        if (!(next.active & (UINT32_C(1) << voice))) continue;
        next.channels[voice].control = &next.control;
        next.channels[voice].grammar = &next.grammar;
        if (!pe_akao_channel_start(&next.channels[voice], sequence, voice)) return 0;
    }
    *player = next;
    for (unsigned voice = 0; voice < 24; ++voice) {
        player->channels[voice].control = &player->control;
        player->channels[voice].grammar = &player->grammar;
    }
    return 1;
}

int pe_akao_player_tick(PeAkaoPlayer *player, uint32_t scale, int forced, unsigned instruction_limit) {
    if (player->error[0]) return 0;
    ++player->timer_ticks;
    player->note_events = 0;
    if (!player->active || !pe_akao_control_due(&player->control, scale, forced)) return 1;
    uint32_t scheduled = player->active;
    for (unsigned voice = 0; voice < 24; ++voice) {
        uint32_t bit = UINT32_C(1) << voice;
        if (!(scheduled & bit)) continue;
        PeAkaoChannel *channel = &player->channels[voice];
        PeAkaoChannelResult result = pe_akao_channel_tick(channel, instruction_limit);
        if (result == PE_AKAO_CHANNEL_ERROR) {
            snprintf(player->error, sizeof(player->error), "Voice %u: %s", voice, channel->error);
            player->note_events = 0; /* no events delivered from a failed tick */
            return 0;
        }
        if (channel->key_off_requested) {
            player->key_off_pending |= bit;
            channel->key_off_requested = 0;
        }
        if (result == PE_AKAO_CHANNEL_NOTE) player->note_events |= bit;
        else if (result == PE_AKAO_CHANNEL_STOP) {
            player->active &= ~bit;
            player->pending &= ~bit;
            player->yield &= ~bit;
            channel->flags = 0; /* SeqOp_StopVoice */
            if (!player->active) player->bank_id = 0;
        }
    }
    pe_akao_control_step(&player->control);
    return 1;
}
