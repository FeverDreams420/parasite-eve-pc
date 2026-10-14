/* PSY-Q LIBSPU S_GVEX: SpuGetVoiceEnvelope. */

#include "pe1/psyq_spu_internal.h"

void SpuGetVoiceEnvelope(int voice, short *env)
{
    *env = (voice + _spu_RXX->voice)->envelope;
}
