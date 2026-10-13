/* ASSEMBLER: GNU */
/* PSY-Q LIBSPU S_Q: SpuQuit. */

#include "pe1/psyq_bios.h"
#include "pe1/psyq_spu_internal.h"

#include "s_q_externs.h"

void SpuQuit(void) {
    if (_spu_isCalled == 1) {
        _spu_isCalled = 0;
        EnterCriticalSection();
        _spu_transferCallback = 0;
        _spu_IRQCallback = 0;
        _SpuDataCallback(0);
        CloseEvent(_spu_EVdma);
        DisableEvent(_spu_EVdma);
        ExitCriticalSection();
    }
}
