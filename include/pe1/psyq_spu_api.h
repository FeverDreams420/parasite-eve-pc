#ifndef PE1_PSYQ_SPU_API_H
#define PE1_PSYQ_SPU_API_H

#include <stdint.h>
#include <stddef.h>

/* Psy-Q LIBSPU public stereo volume and external-input attributes. */
typedef struct SpuVolume {
    short left, right;
} SpuVolume;
typedef struct SpuExtAttr {
    SpuVolume volume;
    int reverb, mix;
} SpuExtAttr;
typedef struct SpuCommonAttr {
    uintptr_t mask;
    SpuVolume mvol;
    SpuVolume mvolmode;
    SpuVolume mvolx;
    SpuExtAttr cd;
    SpuExtAttr ext;
} SpuCommonAttr;
typedef char SpuVolumeSizeCheck[(sizeof(SpuVolume) == 4) ? 1 : -1];
typedef char SpuExtAttrSizeCheck[(sizeof(SpuExtAttr) == 12) ? 1 : -1];
_Static_assert(offsetof(SpuCommonAttr, mvol) == sizeof(uintptr_t),
               "PsyZ common attributes use a native-width mask");
_Static_assert(offsetof(SpuCommonAttr, cd) == sizeof(uintptr_t) + 12,
               "PsyZ CD attribute layout");

/* PsyZ uses a native-width u_long for this public SDK attribute mask. */
typedef struct SpuReverbAttr {
    uintptr_t mask;
    int mode;
    SpuVolume depth;
    int delay, feedback;
} SpuReverbAttr;
void SpuGetReverbModeParam(SpuReverbAttr *attr);

void SpuSetCommonAttr(SpuCommonAttr *attr);
int SpuSetReverb(int on_off);
int SpuSetReverbModeType(unsigned int mode);
void SpuSetReverbModeDepth(short left, short right);
void SpuInit(void);
void SpuStart(void);
void SpuQuit(void);
long SpuInitMalloc(long num, char *top);
int SpuSetTransferMode(int mode);
int SpuSetTransferStartAddr(int address);
unsigned int SpuWrite(void *address, unsigned int size);
unsigned int SpuRead(void *address, unsigned int size);
long SpuSetIRQ(long mode);
void SpuSetVoiceVolumeAttr(int voice, short left, short right,
                           short left_mode, short right_mode);

#endif
