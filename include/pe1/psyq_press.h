#ifndef PE1_PSYQ_PRESS_H
#define PE1_PSYQ_PRESS_H

/* Psy-Q LIBPRESS entry points linked into the mdec_diag overlay. */

/* DecDCToutCallback: installs the MDEC-out DMA completion callback. */
typedef void (*Pe1MdecCallback)(void);
Pe1MdecCallback DecDCToutCallback(Pe1MdecCallback callback);

int DecDCTvlcSize2(int size);
int DecDCTvlc2(void *input, void *decoded, void *table);

#endif
