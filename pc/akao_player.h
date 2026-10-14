#ifndef PE_PC_AKAO_PLAYER_H
#define PE_PC_AKAO_PLAYER_H

#include "akao_channel.h"

typedef struct PeAkaoPlayer {
    PeAkaoControl control;
    PeAkaoGrammar grammar;
    PeAkaoChannel channels[24];
    uint32_t active, pending, yield, key_off_pending, note_events;
    uint16_t bank_id;
    uint64_t timer_ticks;
    char error[256];
} PeAkaoPlayer;

/* Fresh primary-bank state; sequence bytes are borrowed. Invalid starts
 * preserve the previous player. Player stays at a stable address afterward.
 * Overlapping/nested banks, SPU allocation/registers and synthesis are pending. */
int pe_akao_player_start(PeAkaoPlayer *player, const PeAkaoSequence *sequence,
                          const PeAkaoGrammar *grammar, const PeAkaoControl *initial);
/* Timer callback: bank accumulator, ascending active voices, then slides and
 * meter. note_events includes raw note/tie/rest events; key_off_pending is
 * latched until acknowledged by the future SPU consumer. */
int pe_akao_player_tick(PeAkaoPlayer *player, uint32_t scale, int forced, unsigned instruction_limit);

#endif
