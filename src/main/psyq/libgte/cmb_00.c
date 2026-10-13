/* PSY-Q LIBGTE CMB_00: RotTransPers4. */

#include "common.h"
#include "pe1/gte.h"

long RotTransPers4(void *v0, void *v1, void *v2, void *v3,
                   void * volatile sxy0, void * volatile sxy1,
                   void * volatile sxy2, void * volatile sxy3,
                   void * volatile p, void * volatile flag) {
    int first_flags;
    int flags;
    int result;
    void *gte_out0;
    void *gte_out1;
    void *gte_out2;

    gte_ldv0(v0);
    gte_ldv1(v1);
    gte_ldv2(v2);
    gte_rtpt();
    gte_bind_three_outputs(sxy0, sxy1, sxy2);
    gte_stsxy3(gte_out0, gte_out1, gte_out2);
    gte_getflag(first_flags);

    gte_ldv0(v3);
    gte_rtps_three_outputs();
    gte_bind_three_outputs(sxy3, p, flag);
    gte_stsxy2(gte_out0);
    gte_stir0(gte_out1);
    gte_getflag(flags);
    gte_getsz3(result);
    flags |= first_flags;
    *(s32 *)gte_out2 = flags;
    return result >> 2;
}
