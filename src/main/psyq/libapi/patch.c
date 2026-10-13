/* PSY-Q LIBAPI PATCH, part 1 of 2: EnablePAD, DisablePAD. */
/* LIBAPI PATCH stays in two units: _patch_pad (patch_2.c) is SDK
 * assembler source reported as original_asm, and a unit may not mix that
 * with the C functions EnablePAD and DisablePAD. */

#include "pe1/psyq_api_internal.h"

static inline void PadToggle(PadToggleFunc target) {
    PadToggleFunc callback = target;
    goto *(void *)callback;
}

void EnablePAD(void) {
    PadToggle(jtbl_800A34C8);
}

void DisablePAD(void) {
    PadToggle(jtbl_800A34CC);
}
