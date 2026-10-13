/* CC1_FLAGS: -mno-split-addresses */
/* ASSEMBLER: GNU */
/* GCC_VERSION: 2.8.1 */
/* Psy-Q LIBETC VMODE.OBJ: SetVideoMode and GetVideoMode. The object is linked
 * between LIBGPU EXT and SYS; its zero tail is a manifest pad. */

#include "vmode_externs.h"

int SetVideoMode(int mode) {
    int old;

    old = g_VideoMode;
    g_VideoMode = mode;
    return old;
}

int GetVideoMode(void) {
    return g_VideoMode;
}
