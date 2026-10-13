#ifndef PE1_PSYQ_NOP_H
#define PE1_PSYQ_NOP_H

/* Hardware hazard slots for the legacy MIPS/GTE implementation.
 * Native replacements use the timing contract of the selected runtime API. */
#define PE1_NOP() asm volatile("nop")
#define PE1_NOP_DEP(constraint, value) asm volatile("nop" : : constraint(value))
#define PE1_NOP_MEMORY_DEP(constraint, value) \
    asm volatile("nop" : : constraint(value) : "memory")
/* Keep two live arguments and a preceding result across a scheduling slot. */
#define PE1_NOP_IO2_DEP(first, second, value) \
    asm volatile("nop" : "=r"(first), "=r"(second) \
                 : "r"(value), "0"(first), "1"(second))

#endif
