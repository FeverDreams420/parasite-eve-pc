/* PSY-Q LIBGTE CMB_07: RotAverageNclip4. */

#include "pe1/gte.h"
#include "pe1/gte_types.h"

int RotAverageNclip4(const GteShortVector *v0, const GteShortVector *v1,
    const GteShortVector *v2, const GteShortVector *v3,
    u32 * volatile sxy0, u32 * volatile sxy1, u32 * volatile sxy2,
    u32 * sxy3, s32 * volatile p, s32 * volatile otz, u32 * volatile flag) {
    u32 *flagPtr;
    u32 firstFlag;
    u32 *sxy0Ptr;
    u32 *sxy1Ptr;
    u32 *sxy2Ptr;
    s32 clip;
    int zero = 0;
    u32 *sxy3Ptr;
    s32 *scalarOutput;
    u32 *finalFlagPtr;
    u32 finalFlag;
    s32 average;
    gte_lwc2_0_0(v0);
    gte_lwc2_1_4(v0);
    gte_lwc2_2_0(v1);
    gte_lwc2_3_4(v1);
    gte_lwc2_4_0(v2);
    gte_lwc2_5_4(v2);
    gte_cop2_hazard_slot();
    gte_rtpt_command();
    flagPtr = flag;
    gte_getflag(firstFlag);
    gte_cop2_hazard_slot();
    *flagPtr = firstFlag;
    gte_nclip_now();
    sxy0Ptr = sxy0;
    sxy1Ptr = sxy1;
    sxy2Ptr = sxy2;
    gte_getmac0(clip);
    gte_cop2_hazard_slot();
    if (clip <= 0) {
        /* Bind a defined value to the architectural zero register.  This keeps
         * the original two-branch rejection path without CPU instruction asm. */
            if (zero == 0) {
            return clip;
        }
    }
positive:
    gte_stsxy3(sxy0Ptr, sxy1Ptr, sxy2Ptr);
    gte_lwc2_0_0(v3);
    gte_lwc2_1_4(v3);
    gte_cop2_hazard_slot();
    gte_rtps_command();
    sxy3Ptr = sxy3;
    scalarOutput = p;
    finalFlagPtr = flag;
    gte_stsxy2(sxy3Ptr);
    gte_getflag(finalFlag);
    gte_stir0(scalarOutput);
    finalFlag |= firstFlag;
    *finalFlagPtr = finalFlag;
    gte_avsz4_command();
    scalarOutput = otz;
    gte_getotz(average);
    gte_cop2_hazard_slot();
    *scalarOutput = average;
done:
    return clip;
}
