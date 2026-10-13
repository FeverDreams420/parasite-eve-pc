/* Psy-Q LIBCD C_003.OBJ: StUnSetRing. */
/* GAS 2.7 retains the load-delay nop before ExitCriticalSection. */

#include "pe1/psyq_cd.h"
#include "pe1/psyq_ds.h"

#include "c_003_externs.h"

void StUnSetRing(void) {
    unsigned char *interrupt_enable;

    EnterCriticalSection();
    if (g_CdDsActive == 1) {
        DsDataCallback(0);
        DsReadyCallback(0);
    } else {
        CdDataCallback(0);
        CdReadyCallback(0);
    }
    *g_CdStreamIndexRegPtr = 0;
    interrupt_enable = g_CdStreamIrqRegPtr;
    *interrupt_enable = 0;
    ExitCriticalSection();
}
