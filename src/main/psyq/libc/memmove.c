/* PSY-Q LIBC MEMMOVE (byte-identical to LIBC2 MEMMOVE): memmove. */
void *memmove(void *dst, const void *src, int len) {
    unsigned char *out = dst;
    const unsigned char *in = src;
    int count = len;
    int test;
    void *ret;
    if (out >= in) {
        test = count;
        count--;
        if (test > 0) {
            do {
                out[count] = in[count];
            } while (count-- > 0);
        }
    } else {
        test = count;
        count--;
        if (test > 0) {
            do {
                *out++ = *in++;
            } while (count-- > 0);
        }
    }

    ret = out;
    return ret;
}
