#ifndef PE1_RELOC_BLOCK_H
#define PE1_RELOC_BLOCK_H

typedef unsigned int RelocU32;

typedef struct RelocBlock {
    union {
        char *base;
        RelocU32 baseOffset;
    } u0;
    RelocU32 count;
    RelocU32 offsets[1];
} RelocBlock;

/* A block's own address, added to the offsets it holds on disk. */
typedef union RelocAddress {
    RelocBlock *block;
    RelocU32 word;
} RelocAddress;

extern RelocBlock *g_SceneDataTable1;

#endif /* PE1_RELOC_BLOCK_H */
