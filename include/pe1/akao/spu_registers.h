#ifndef PE1_AKAO_SPU_REGISTERS_H
#define PE1_AKAO_SPU_REGISTERS_H

#include "pe1/psyq_spu_internal.h"

/* PsyZ native register access includes key-on/off backend side effects. */
void Psyz_SpuWrite(unsigned int offset, unsigned short value);
unsigned short Psyz_SpuRead(unsigned int offset);

/* The same original register words now refer to PsyZ's native SPU map. */
#define g_SpuKeyOnLow (_spu_RXX->key_on[0])
#define g_SpuKeyOnHigh (_spu_RXX->key_on[1])
#define g_SpuKeyOffLow (_spu_RXX->key_off[0])
#define g_SpuKeyOffHigh (_spu_RXX->key_off[1])
#define g_SpuPitchModLow (_spu_RXX->pitch_modulation[0])
#define g_SpuFmEnableHigh (_spu_RXX->pitch_modulation[1])
#define g_SpuNoiseEnableLow (_spu_RXX->noise_enable[0])
#define g_SpuNoiseEnableHigh (_spu_RXX->noise_enable[1])
#define g_SpuReverbEnableLow (_spu_RXX->reverb_enable[0])
#define g_SpuReverbEnableHigh (_spu_RXX->reverb_enable[1])

void Spu_WriteKeyOn(unsigned int value);
void Spu_WriteKeyOff(unsigned int value);
void Spu_WriteFmEnable(unsigned int value);
void Spu_WriteNoiseEnable(unsigned int value);
void Spu_WriteReverbEnable(unsigned int value);

#endif
