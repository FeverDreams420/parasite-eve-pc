/* ASSEMBLER: GNU */
/* PSY-Q LIBSN SNMAIN: __SN_ENTRY_POINT, __main, __do_global_dtors. */
/*
 * PSY-Q LIBSN SNMAIN: SN Systems' startup object. __SN_ENTRY_POINT clears
 * the BSS, derives the stack and heap from the executable header, keeps ra in
 * a static word across InitHeap, and calls main. __main and __do_global_dtors
 * walk the constructor/destructor tables with a 16-byte frame that saves s0,
 * s1 and ra at 4/8/12 and leaves no outgoing argument area, which GCC never
 * produces. PSY-Q 4.6 LIBSN.LIB SNMAIN has all 67 non-relocated words
 * identical; the 4.3 object is in configs/USA/psyq_provenance.json.
 * The BSS bounds are linker symbols (tools/scripts/ld_postpass.py):
 * `__bss_start` is the end of .sdata and `_end` the end of main, where the
 * heap begins. The constructor and destructor tables are the linker symbols
 * `__sn_ctor_table` and `__sn_dtor_table`; both tables are empty in this
 * game, so they sit at the first address of the image.
 */

#include "pe1/psyq_asm.h"

PSYQ_ASM_OBJECT(LIBSN, SNMAIN)

PSYQ_ASM_FUNCTION(__SN_ENTRY_POINT,
    "    lui     $v0, %hi(__bss_start)\n"
    "    addiu   $v0, $v0, %lo(__bss_start)\n"
    "    lui     $v1, %hi(_end)\n"
    "    addiu   $v1, $v1, %lo(_end)\n"
    ".Lclear_bss:\n"
    "    sw      $zero, 0x0($v0)\n"
    "    addiu   $v0, $v0, 0x4\n"
    "    sltu    $at, $v0, $v1\n"
    "    bnez    $at, .Lclear_bss\n"
    "    nop\n"
    "    lui     $v0, %hi(_ramsize)\n"
    "    lw      $v0, %lo(_ramsize)($v0)\n"
    "    nop\n"
    "    addi    $v0, $v0, -0x8\n"
    "    lui     $t0, 0x8000\n"
    "    or      $sp, $v0, $t0\n"
    "    lui     $a0, %hi(_end)\n"
    "    addiu   $a0, $a0, %lo(_end)\n"
    "    sll     $a0, $a0, 3\n"
    "    srl     $a0, $a0, 3\n"
    "    lui     $v1, %hi(_stacksize)\n"
    "    lw      $v1, %lo(_stacksize)($v1)\n"
    "    nop\n"
    "    subu    $a1, $v0, $v1\n"
    "    subu    $a1, $a1, $a0\n"
    "    lui     $at, %hi(g_SnHeapSize)\n"
    "    sw      $a1, %lo(g_SnHeapSize)($at)\n"
    "    or      $a0, $a0, $t0\n"
    "    lui     $at, %hi(g_SnProgramEnd)\n"
    "    sw      $a0, %lo(g_SnProgramEnd)($at)\n"
    "    lui     $at, %hi(g_SnEntrySavedRa)\n"
    "    sw      $ra, %lo(g_SnEntrySavedRa)($at)\n"
    "    lui     $gp, %hi(_gp)\n"
    "    addiu   $gp, $gp, %lo(_gp)\n"
    "    addu    $fp, $sp, $zero\n"
    "    jal     InitHeap\n"
    "    addi    $a0, $a0, 4\n"
    "    lui     $ra, %hi(g_SnEntrySavedRa)\n"
    "    lw      $ra, %lo(g_SnEntrySavedRa)($ra)\n"
    "    nop\n"
    "    jal     main\n"
    "    nop\n"
    "    break   0, 1\n");

PSYQ_ASM_FUNCTION(__main,
    "    lui     $t0, %hi(g_SnCtorsDone)\n"
    "    lw      $t0, %lo(g_SnCtorsDone)($t0)\n"
    "    addiu   $sp, $sp, -0x10\n"
    "    sw      $s0, 0x4($sp)\n"
    "    sw      $s1, 0x8($sp)\n"
    "    sw      $ra, 0xC($sp)\n"
    "    bnez    $t0, .Lmain_done\n"
    "    ori     $t0, $zero, 0x1\n"
    "    lui     $at, %hi(g_SnCtorsDone)\n"
    "    sw      $t0, %lo(g_SnCtorsDone)($at)\n"
    "    lui     $s0, %hi(__sn_ctor_table)\n"
    "    addiu   $s0, $s0, %lo(__sn_ctor_table)\n"
    /* Table length from the SN linker; it links as zero in this game. */
    "    lui     $s1, 0x0\n"
    "    addiu   $s1, $s1, 0x0\n"
    "    beqz    $s1, .Lmain_done\n"
    "    nop\n"
    ".Lcall_ctor:\n"
    "    lw      $t0, 0x0($s0)\n"
    "    addiu   $s0, $s0, 0x4\n"
    "    jalr    $t0\n"
    "    addiu   $s1, $s1, -0x1\n"
    "    bnez    $s1, .Lcall_ctor\n"
    "    nop\n"
    ".Lmain_done:\n"
    "    lw      $ra, 0xC($sp)\n"
    "    lw      $s1, 0x8($sp)\n"
    "    lw      $s0, 0x4($sp)\n"
    "    addiu   $sp, $sp, 0x10\n"
    "    jr      $ra\n"
    "    nop\n");

PSYQ_ASM_FUNCTION(__do_global_dtors,
    "    lui     $t0, %hi(g_SnCtorsDone)\n"
    "    lw      $t0, %lo(g_SnCtorsDone)($t0)\n"
    "    addiu   $sp, $sp, -0x10\n"
    "    sw      $s0, 0x4($sp)\n"
    "    sw      $s1, 0x8($sp)\n"
    "    sw      $ra, 0xC($sp)\n"
    "    beqz    $t0, .Ldtors_done\n"
    "    nop\n"
    "    lui     $s0, %hi(__sn_dtor_table)\n"
    "    addiu   $s0, $s0, %lo(__sn_dtor_table)\n"
    /* Table length from the SN linker; it links as zero in this game. */
    "    lui     $s1, 0x0\n"
    "    addiu   $s1, $s1, 0x0\n"
    "    beqz    $s1, .Ldtors_done\n"
    "    nop\n"
    ".Lcall_dtor:\n"
    "    lw      $t0, 0x0($s0)\n"
    "    addiu   $s0, $s0, 0x4\n"
    "    jalr    $t0\n"
    "    addiu   $s1, $s1, -0x1\n"
    "    bnez    $s1, .Lcall_dtor\n"
    "    nop\n"
    ".Ldtors_done:\n"
    "    lw      $ra, 0xC($sp)\n"
    "    lw      $s1, 0x8($sp)\n"
    "    lw      $s0, 0x4($sp)\n"
    "    addiu   $sp, $sp, 0x10\n"
    "    jr      $ra\n"
    "    nop\n");
