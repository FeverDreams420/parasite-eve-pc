#include "pe1/akao.h"
#include "pe1/akao/globals.h"
#include "pe1/akao/note_step.h"
#include <stdio.h>

/* Explicit input tables for isolated original procedures, not runtime audio. */
AkaoInstrument g_AkaoInstrumentTable[256];
int g_AkaoPitchPeriodTable[256 * 16];
static AkaoSequencerBank bank;
AkaoSequencerBank *g_AkaoCurTrack = &bank;
void SeqOp_SetTempo(void *track);
void SeqOp_LoadInstrumentFromIndex(void *track);
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"line %d: %s\n",__LINE__,#x); return 1; } } while(0)

int main(void)
{
    unsigned char tempo[] = {0x34, 0x12};
    AkaoTrack track = {0};
    track.pc = tempo;
    bank.tempo_slide_duration = 37;
    SeqOp_SetTempo(&track);
    CHECK(bank.tempo == 0x12340000u);
    CHECK(bank.tempo_slide_duration == 0);
    CHECK(track.pc == tempo + 2);
    for (unsigned i = 0; i < 256; ++i) {
        AkaoInstrument *instrument = &g_AkaoInstrumentTable[i];
        instrument->adsr_attack_rate = i;
        instrument->adsr_decay_rate = i ^ 0x55;
        instrument->adsr_sustain_level = i ^ 0xaa;
        instrument->adsr_sustain_rate = i + 1;
        instrument->adsr_release_rate = i + 2;
        instrument->adsr_attack_mode = i & 1;
        instrument->adsr_sustain_mode = (i >> 1) & 1;
        instrument->adsr_release_mode = (i >> 2) & 1;
        track.note_pitch = i;
        track.update_flags = 3;
        SeqOp_LoadInstrumentFromIndex(&track);
        CHECK(track.adsr_attack_rate == instrument->adsr_attack_rate);
        CHECK(track.adsr_decay_rate == instrument->adsr_decay_rate);
        CHECK(track.adsr_sustain_level == instrument->adsr_sustain_level);
        CHECK(track.adsr_sustain_rate == instrument->adsr_sustain_rate);
        CHECK(track.adsr_release_rate == instrument->adsr_release_rate);
        CHECK(track.adsr_attack == instrument->adsr_attack_mode);
        CHECK(track.adsr_sustain == instrument->adsr_sustain_mode);
        CHECK(track.adsr_release == instrument->adsr_release_mode);
        CHECK(track.update_flags == (3 | AKAO_VOICE_PARAM_ADSR_ALL));
        CHECK(track.pc == tempo + 2);
        for (unsigned note = 0; note < 12; ++note)
            g_AkaoPitchPeriodTable[i * 16 + note] = 4096 + note * 32 + i;
        CHECK(Akao_LookupPitchPeriod(i, 72, 0) == 4096 + i);
        CHECK(Akao_LookupPitchPeriod(i, 60, 0) == (4096 + i) / 2);
        CHECK(Akao_LookupPitchPeriod(i, 84, 0) == (4096 + i) * 2);
        CHECK(Akao_LookupPitchPeriod(i, 73, 0) == 4128 + i);
        for (unsigned note = 0; note < 256; ++note) {
            for (int detune = -128; detune <= 127; detune += 17) {
                unsigned octave = note / 12;
                unsigned period = g_AkaoPitchPeriodTable[i * 16 + note % 12];
                period += (period * (unsigned)detune) >> 7;
                if (octave >= 7) period <<= octave - 6;
                else if (octave < 6) period >>= 6 - octave;
                CHECK(Akao_LookupPitchPeriod(i, note, detune) == (int)(period & 0xffff));
            }
        }
    }
    puts("original AKAO: tempo cursor and 256 instrument/pitch entries pass");
    return 0;
}
