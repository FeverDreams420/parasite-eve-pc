#ifndef PE1_PSYQ_TYPES_H
#define PE1_PSYQ_TYPES_H

typedef signed char s_char;
typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned int u_int;
typedef unsigned long u_long;

/* DMA address registers, ordering-table links and packet tags carry a buffer
 * address as a plain word (the low 24 bits are the physical address). */
#define PSX_ADDRESS(pointer) ((u_long)(pointer))

#endif
