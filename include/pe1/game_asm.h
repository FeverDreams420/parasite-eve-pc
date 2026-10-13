#ifndef PE1_GAME_ASM_H
#define PE1_GAME_ASM_H

/* Legacy game routines that perform real MIPS operations.
 * Translate their behavior to C or a native adapter before host integration. */

/* One global routine of original game assembler. */
#define GAME_ASM_FUNCTION(name, body) \
    __asm__( \
        ".section .text, \"ax\"\n" \
        ".set push\n" \
        ".set noreorder\n" \
        ".set noat\n" \
        ".globl " #name "\n" \
        ".type " #name ", @function\n" \
        ".ent " #name "\n" \
        #name ":\n" \
        body \
        ".end " #name "\n" \
        ".size " #name ", . - " #name "\n" \
        ".set pop\n")

#endif
