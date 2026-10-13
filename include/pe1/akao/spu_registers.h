#ifndef PE1_AKAO_SPU_REGISTERS_H
#define PE1_AKAO_SPU_REGISTERS_H

/* SPU voice-mask registers the AKAO driver writes directly, as low and
 * high halfwords (SpuRegs in psyq_spu_internal.h describes the block). */
extern volatile unsigned short g_SpuKeyOnLow, g_SpuKeyOnHigh; /* key on */
extern volatile unsigned short g_SpuKeyOffLow, g_SpuKeyOffHigh; /* key off */
extern volatile unsigned short g_SpuPitchModLow, g_SpuFmEnableHigh; /* FM (pitch modulation) */
extern volatile unsigned short g_SpuNoiseEnableLow, g_SpuNoiseEnableHigh; /* noise */
extern volatile unsigned short g_SpuReverbEnableLow, g_SpuReverbEnableHigh; /* reverb */

void Spu_WriteKeyOn(unsigned int value);
void Spu_WriteKeyOff(unsigned int value);
void Spu_WriteFmEnable(unsigned int value);
void Spu_WriteNoiseEnable(unsigned int value);
void Spu_WriteReverbEnable(unsigned int value);

#endif
