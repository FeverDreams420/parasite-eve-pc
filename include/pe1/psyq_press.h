#ifndef PE1_PSYQ_PRESS_H
#define PE1_PSYQ_PRESS_H

/* Psy-Q LIBPRESS entry points linked into the mdec_diag overlay. */

/* DecDCToutCallback: installs the MDEC-out DMA completion callback. */
void DecDCToutCallback(void *callback);

#endif
