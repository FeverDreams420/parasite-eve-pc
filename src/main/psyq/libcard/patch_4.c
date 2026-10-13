/* PSY-Q LIBCARD PATCH, part 4 of 4: _copy_memcard_patch. */
/* Psy-Q _copy_memcard_patch. Retains the existing link symbol for callers.
 * Empty constraints preserve the retail schedule. */
/* Treat the patch instruction image as words; copy up to the next label. */

#include "pe1/psyq_card.h"

void _copy_memcard_patch(void) {
    unsigned *dst = (unsigned *)0xdf80;
    const unsigned *src = func_8007E344;
    const unsigned *end = (const unsigned *)LIBCARD_PATCH_text_70;
    unsigned value;
    do {
        value = *src;
        *dst = value;
        src++;
        dst++;
    } while (src != end);
}
