#ifndef PE1_BOUNDS_CHECK_H
#define PE1_BOUNDS_CHECK_H

/* Retail leaves this diagnostic hook empty. Callers pass a failure code
 * and sometimes one or two addresses. */
void Inv_BoundsCheckAssertStub(int code, ...);

#endif
