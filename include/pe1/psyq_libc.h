#ifndef PE1_PSYQ_LIBC_H
#define PE1_PSYQ_LIBC_H

#include "common.h"

/* Native C runtime; RNG names remain bound to PsyZ by common.h. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Comparators receive addresses of two elements in the sorted array. */
typedef int (*PsyqSortCompare)(const void *left, const void *right);

#endif /* PE1_PSYQ_LIBC_H */
