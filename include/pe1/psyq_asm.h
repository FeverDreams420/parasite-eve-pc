#ifndef PE1_PSYQ_ASM_H
#define PE1_PSYQ_ASM_H

/* Legacy PSY-Q routines that perform real MIPS/GTE operations.
 * Use PsyZ equivalents or native implementations when porting their callers. */

/* Names the SDK library and object this file reproduces. Emits nothing. */
#define PSYQ_ASM_OBJECT(library, object)

/* One global routine of an SDK assembler object. */
#define PSYQ_ASM_FUNCTION(name, body) \
    __asm__( \
        ".section .text, \"ax\"\n" \
        ".include \"gte_macros.inc\"\n" \
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
