#include "room.h"
#include <stdlib.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "line %d: %s\n", __LINE__, #x); exit(1); } } while (0)
int main(void) {
    PeAkaoGrammar grammar = {0}; grammar.lfo_valid = 1;
    grammar.lfo_words[0] = 32767; grammar.lfo_words[1] = 32769;
    grammar.lfo_words[4] = 65534; /* marker at 2 jumps back two words */
    uint16_t cursor = 0; int16_t sample = 0;
    for (unsigned i = 0; i < 1000; ++i) {
        CHECK(pe_akao_lfo_next(&grammar, &cursor, &sample));
        CHECK(sample == (i & 1 ? -32767 : 32767));
    }
    cursor = 2; sample = 99; grammar.lfo_words[4] = 65533;
    CHECK(!pe_akao_lfo_next(&grammar, &cursor, &sample) && cursor == 2 && sample == 99);
    grammar.lfo_words[4] = PE_AKAO_LFO_WORDS;
    CHECK(!pe_akao_lfo_next(&grammar, &cursor, &sample) && cursor == 2 && sample == 99);
    cursor = PE_AKAO_LFO_WORDS - 2;
    CHECK(!pe_akao_lfo_next(&grammar, &cursor, &sample) && sample == 99);
    grammar.lfo_valid = 0; cursor = 0;
    CHECK(!pe_akao_lfo_next(&grammar, &cursor, &sample) && !cursor && sample == 99);
    puts("AKAO LFO data: signed samples, relative loop markers and transactional bounds passed");
    return 0;
}
