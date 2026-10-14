#ifndef PE1_AKAO_SCRIPT_H
#define PE1_AKAO_SCRIPT_H

#include "pe1/akao/track.h"

/* Script handlers use the same native record as the sequencer and driver.
 * Historical PS1 byte offsets are not a separate runtime layout. */
typedef AkaoTrack AkaoScriptState;

#endif
