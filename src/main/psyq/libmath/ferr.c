/* ASSEMBLER: GNU */
/* PSY-Q LIBMATH FERR: _err_math. */

#include "common.h"
#include "pe1/math64.h"
#include "pe1/psyq_bios.h"

#include "ferr_externs.h"

int _err_math(int arg0, int arg1) {
    g_MathErrCode = arg0;
    g_MathErrArg = arg1;
    if (arg0 != 0x21) {
    if (arg0 != 0x22) {
        return 0;
    }
    } else {
    DeliverEvent(0xF4000002, 0x301);
    return 0;
    }
    DeliverEvent(0xF4000002, 0x302);
    return 0;
}
