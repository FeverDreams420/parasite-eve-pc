#ifndef PE1_BOOT_STACK_H
#define PE1_BOOT_STACK_H

/* Legacy scratchpad stack switch. Native code should use the ordinary host
 * stack; this MIPS implementation is retained as a porting reference. */

/* Top word of the scratchpad (0x1F800000..0x1F8003FF). */
#define BOOT_SCRATCHPAD_STACK_TOP ((u32 *)0x1F8003FC)

#define BOOT_CALL_ON_SCRATCHPAD_STACK(top, call) \
    { \
        __asm__ volatile("move $8,%0\n" \
                         "sw $29,0($8)\n" \
                         "addiu $8,$8,-4\n" \
                         "move $29,$8" \
                         : : "r"(top) : "$8", "memory"); \
        call; \
        __asm__ volatile("addiu $29,$29,4\n" \
                         "lw $29,0($29)" \
                         : : : "memory"); \
    }

#endif
