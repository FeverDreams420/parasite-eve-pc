#include "pe1/gte.h"
#include <stdio.h>

int main(void)
{
    /* Call the game's macros directly, with an intentionally unaligned
     * buffer, to check host loads and stores as well as projection. */
    u8 bytes[17] = {0};
    u32 word = 100, output, mac;
    gte_ctc2_0(4096);
    gte_ctc2_1(0);
    gte_ctc2_2(4096);
    gte_ctc2_3(0);
    gte_ctc2_4(4096);
    gte_ctc2_5(0);
    gte_ctc2_6(0);
    gte_ctc2_7(0);
    gte_ctc2_24(160u << 16);
    gte_ctc2_25(112u << 16);
    gte_ctc2_26(256);
    memcpy(bytes + 1, &word, 4);
    word = 512;
    memcpy(bytes + 5, &word, 4);
    gte_lwc2_0_0(bytes + 1);
    gte_lwc2_1_4(bytes + 1);
    gte_rtps_command();
    gte_swc2_14_8(bytes + 1);
    memcpy(&output, bytes + 9, 4);
    if (output != (112u << 16 | 210u)) return 1;
    gte_mfc2_25(mac);
    if (mac != 100) return 2;
    gte_ctc2_5(-123);
    gte_cfc2_5(output);
    if (output != (u32)-123) return 3;
    puts("Original GTE macros run through PsyZ without a window");
    return 0;
}
