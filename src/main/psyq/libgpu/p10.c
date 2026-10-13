/* Psy-Q LIBGPU P10.OBJ: SetSemiTrans. */

void SetSemiTrans(unsigned char *packet, int enabled) {
    if (enabled != 0) {
        packet[7] |= 2;
    } else {
        packet[7] &= 0xFD;
    }
}
