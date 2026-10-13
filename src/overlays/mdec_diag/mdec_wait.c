#include "common.h"
#include "pe1/mdec_diag.h"

#include "mdec_wait.inc"

MDECDIAG_WAIT_MDEC(MdecDiag_WaitInputSync, g_MdecStatusRegPtr, 0x20000000, g_MdecInSyncName)

#undef MDECDIAG_WAIT_MDEC

#include "mdec_wait.inc"

MDECDIAG_WAIT_MDEC(MdecDiag_WaitOutputSync, g_MdecDma1ChcrPtr, 0x1000000, g_MdecOutSyncName)

#undef MDECDIAG_WAIT_MDEC
