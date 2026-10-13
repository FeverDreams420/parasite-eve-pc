#define MDECDIAG_WAIT_MDEC(name, status, mask, message) \
    unsigned int g_##name##_MessageArg; \
    int name(void) { \
        volatile int timeout = 0x100000; \
        if ((*status & mask) != 0) { \
            g_##name##_MessageArg = -1; \
            do { \
                timeout = timeout - 1; \
                if (timeout == g_##name##_MessageArg) { \
                    MdecDiag_HandleTimeout(message); \
                    return -1; \
                } \
            } while ((*status & mask) != 0); \
        } \
        return 0; \
    }
