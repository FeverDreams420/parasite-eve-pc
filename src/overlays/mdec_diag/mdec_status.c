#include "pe1/mdec_diag.h"

int MdecDiag_ReadStatus(void) {
    return *g_MdecStatusRegPtr;
}

int MdecDiag_HandleTimeout(char *message) {
    printf(g_MdecTimeoutFormat, message);
    *g_MdecStatusRegPtr = 0x80000000;
    *g_MdecDma0ChcrPtr = 0;
    *g_MdecDma1ChcrPtr = 0;
    *g_MdecDma1ChcrPtr;
    *g_MdecStatusRegPtr = 0x60000000;
    return 0;
}
