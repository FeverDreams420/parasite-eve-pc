#ifndef PE_HOST_DISC_H
#define PE_HOST_DISC_H
#include "disc.h"
/* Borrow an open disc and its PE.IMG extent; NULL unmounts it.
 * Original sector-read callers provide storage for the full sector count,
 * including trailing padding beyond a bank header's logical byte size. */
void pe_host_disc_bind(PeDisc *disc, const PeDiscFile *archive);
#endif
