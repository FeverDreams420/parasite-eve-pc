#ifndef PE1_PSYQ_PRESS_H
#define PE1_PSYQ_PRESS_H

/* Game-facing LIBPRESS entry points; native MDEC/DMA is provided by PsyZ. */

/* DecDCToutCallback: installs the MDEC-out DMA completion callback. */
typedef void (*Pe1MdecCallback)(void);
Pe1MdecCallback DecDCToutCallback(Pe1MdecCallback callback);

int DecDCTinSync(int mode);
int DecDCToutSync(int mode);

int DecDCTvlcSize2(int size);
int DecDCTvlc2(void *input, void *decoded, void *table);

#endif
