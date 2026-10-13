/* GCC_VERSION: 2.8.1 */
/* Psy-Q LIBCD SYS.OBJ, part 1 of 2: CdStatus, CdMode, CdLastCom, CdLastPos
 * and CdReset. CdReset needs GCC 2.8.1 (the call result goes through $v1). */

#include "pe1/psyq_cd.h"

int CdStatus(void) {
    return *(u8 *)&g_CdStatus;
}

int CdMode(void) {
    return g_CdMode;
}

int CdLastCom(void) {
    return g_CdLastCom;
}

CdlLOC *CdLastPos(void) {
    return &g_CdLastPos;
}

void CD_initintr(void);
int CD_init(void);
int CD_initvol(void);

int CdReset(int arg0) {
    int ret;

    if (arg0 == 2) {
        CD_initintr();
        return 1;
    }

    if (CD_init() != 0) {
        return 0;
    }

    if (arg0 == 1) {
        ret = CD_initvol();
        if (ret != 0) {
            return 0;
        }
    }

    return 1;
}
